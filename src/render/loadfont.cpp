/*
 * @file loadfont.cpp
 *
 * Font file loader
 *
 * @author Mark Kilgard
 * Copyright (c) Mark J. Kilgard, 1997
 * This program is freely distributable without licensing fees  and is
 * provided without guarantee or warrantee expressed or  implied. This
 * program is -not- in the public domain.
 */
#include "vixen.h"
#include "render/texfont.h"

namespace Vixen {

#define	TXF_FILE	Core::Stream

inline TXF_FILE* TXF_OPEN(const TCHAR* url, TXF_FILE* file)
{
	if (file && (file->IsOpen() || file->Open(url, Core::Stream::OPEN_READ)))
		return file;
	return NULL;
}

inline bool	TXF_READ(TXF_FILE* file, void* buf, int n)
{
	if (file && file->Read((char*) buf, n))
		return true;
	return false;
}

inline bool TXF_READI(TXF_FILE* file, void* ibuf)
{
	return TXF_READ(file, ibuf, 4);
}

inline void TXF_CLOSE(TXF_FILE* file)
{
	if (file)
		file->Close();
}


#if defined(VX_NOTEXTURE) || (!defined(VIXEN_OGL) && !defined(VIXENGLES))
int useLuminanceAlpha = 0;
#else
extern int useLuminanceAlpha;
#endif

/* byte swap a 32-bit value */
#define SWAPL(x, n) { \
	n = ((char *) (x))[0];\
	((char *) (x))[0] = ((char *) (x))[3];\
	((char *) (x))[3] = n;\
	n = ((char *) (x))[1];\
	((char *) (x))[1] = ((char *) (x))[2];\
	((char *) (x))[2] = n; }

/* byte swap a short */
#define SWAPS(x, n) { \
	n = ((char *) (x))[0];\
	((char *) (x))[0] = ((char *) (x))[1];\
	((char *) (x))[1] = n; }


TexGlyphVertexInfo *
getTCVI(TexFont * txf, int c)
{
  TexGlyphVertexInfo *tgvi;

  /* Automatically substitute uppercase letters with lowercase if not
     uppercase available (and vice versa). */
  if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
    tgvi = txf->lut[c - txf->min_glyph];
    if (tgvi) {
      return tgvi;
    }
    if (islower(c)) {
      c = toupper(c);
      if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
        return txf->lut[c - txf->min_glyph];
      }
    }
    if (isupper(c)) {
      c = tolower(c);
      if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
        return txf->lut[c - txf->min_glyph];
      }
    }
  }
//  VX_ERROR(("TextGeometry::GetTextExtent: unavailable font character %d\n", c), NULL);
	return NULL;
}


static const char *lastError;

const char *
txfErrorString(void)
{
  return lastError;
}

TexFont* txfLoadFont(const TCHAR *filename, Core::Stream* stream)
{
	TexFont *txf;
	TXF_FILE *file;
	float w, h, xstep, ystep;
	char fileid[4];
	char tmp;
	unsigned char *texbitmap;
	int min_glyph, max_glyph;
	int endianness, swap, format, stride, width, height;
	int i, j, got;

	txf = NULL;
	file = TXF_OPEN(filename, stream);
	if (file == NULL)
	{
		lastError = "file open failed.";
		goto error;
	}
	txf = (TexFont *) malloc(sizeof(TexFont));
	if (txf == NULL)
	{
		lastError = "out of memory.";
		goto error;
	}
  /* For easy cleanup in error case. */
	txf->tgi = NULL;
	txf->tgvi = NULL;
	txf->lut = NULL;
	txf->teximage = NULL;
	txf->texobj = 0;

	if (!TXF_READI(file, fileid) || strncmp(fileid, "\377txf", 4))
	{
		lastError = "not a texture font file.";
		goto error;
	}
	assert(sizeof(int) == 4);  /* Ensure external file format size. */
	got = TXF_READI(file, &endianness);
	if (got && endianness == 0x12345678)
		swap = 0;
	else if (got && endianness == 0x78563412)
		swap = 1;
	else
	{
		lastError = "not a texture font file.";
		goto error;
	}
#define EXPECT(n) if (got != n) { lastError = "premature end of file."; goto error; }
	got = TXF_READI(file, &format);
	EXPECT(1);
	got = TXF_READI(file, &txf->tex_width);
	EXPECT(1);
	got = TXF_READI(file, &txf->tex_height);
	EXPECT(1);
	got = TXF_READI(file, &txf->max_ascent);
	EXPECT(1);
	got = TXF_READI(file, &txf->max_descent);
	EXPECT(1);
	got = TXF_READI(file, &txf->num_glyphs);
	EXPECT(1);

	if (swap)
	{
		SWAPL(&format, tmp);
		SWAPL(&txf->tex_width, tmp);
		SWAPL(&txf->tex_height, tmp);
		SWAPL(&txf->max_ascent, tmp);
		SWAPL(&txf->max_descent, tmp);
		SWAPL(&txf->num_glyphs, tmp);
	}
	txf->tgi = (TexGlyphInfo *) malloc(txf->num_glyphs * sizeof(TexGlyphInfo));
	if (txf->tgi == NULL)
	{
		lastError = "out of memory.";
		goto error;
	}
	assert(sizeof(TexGlyphInfo) == 12);  /* Ensure external file format size. */
	got = TXF_READ(file, txf->tgi, sizeof(TexGlyphInfo) * txf->num_glyphs);
	EXPECT(1);

	if (swap)
		for (i = 0; i < txf->num_glyphs; i++)
		{
			SWAPS(&txf->tgi[i].c, tmp);
			SWAPS(&txf->tgi[i].x, tmp);
			SWAPS(&txf->tgi[i].y, tmp);
		}
	txf->tgvi = (TexGlyphVertexInfo *) malloc(txf->num_glyphs * sizeof(TexGlyphVertexInfo));
	if (txf->tgvi == NULL)
	{
		lastError = "out of memory.";
		goto error;
	}
	w = (float) txf->tex_width;
	h = (float) txf->tex_height;
	xstep = 0.5f / w;
	ystep = 0.5f / h;
	for (i = 0; i < txf->num_glyphs; i++)
	{
		TexGlyphInfo *tgi;

		tgi = &txf->tgi[i];
		txf->tgvi[i].t0[0] = tgi->x / w + xstep;
		txf->tgvi[i].t0[1] = tgi->y / h + ystep;
		txf->tgvi[i].v0[0] = tgi->xoffset;
		txf->tgvi[i].v0[1] = tgi->yoffset;
		txf->tgvi[i].t1[0] = (tgi->x + tgi->width) / w + xstep;
		txf->tgvi[i].t1[1] = tgi->y / h + ystep;
		txf->tgvi[i].v1[0] = tgi->xoffset + tgi->width;
		txf->tgvi[i].v1[1] = tgi->yoffset;
		txf->tgvi[i].t2[0] = (tgi->x + tgi->width) / w + xstep;
		txf->tgvi[i].t2[1] = (tgi->y + tgi->height) / h + ystep;
		txf->tgvi[i].v2[0] = tgi->xoffset + tgi->width;
		txf->tgvi[i].v2[1] = tgi->yoffset + tgi->height;
		txf->tgvi[i].t3[0] = tgi->x / w + xstep;
		txf->tgvi[i].t3[1] = (tgi->y + tgi->height) / h + ystep;
		txf->tgvi[i].v3[0] = tgi->xoffset;
		txf->tgvi[i].v3[1] = tgi->yoffset + tgi->height;
		txf->tgvi[i].advance = tgi->advance;
	}

	min_glyph = txf->tgi[0].c;
	max_glyph = txf->tgi[0].c;
	for (i = 1; i < txf->num_glyphs; i++)
	{
		if (txf->tgi[i].c < min_glyph)
			min_glyph = txf->tgi[i].c;
		if (txf->tgi[i].c > max_glyph)
			max_glyph = txf->tgi[i].c;
	}
	txf->min_glyph = min_glyph;
	txf->range = max_glyph - min_glyph + 1;

	txf->lut = (TexGlyphVertexInfo **) calloc(txf->range, sizeof(TexGlyphVertexInfo *));
	if (txf->lut == NULL)
	{
		lastError = "out of memory.";
		goto error;
	}
	for (i = 0; i < txf->num_glyphs; i++)
		txf->lut[txf->tgi[i].c - txf->min_glyph] = &txf->tgvi[i];

	switch (format)
	{
		case TXF_FORMAT_BYTE:
		if (Bitmap::FontDepth == 16)
		{
			unsigned char *orig = (unsigned char *) malloc(txf->tex_width * txf->tex_height);
			if (orig == NULL)
			{
				lastError = "out of memory.";
				goto error;
			}
			got = TXF_READ(file, orig, txf->tex_width * txf->tex_height);
			EXPECT(1);
			txf->teximage = (unsigned char *) malloc(2 * txf->tex_width * txf->tex_height);
			if (txf->teximage == NULL)
			{
				lastError = "out of memory.";
				goto error;
			}
			for (i = 0; i < txf->tex_width * txf->tex_height; i++)
			{
				txf->teximage[i * 2] = orig[i];
				txf->teximage[i * 2 + 1] = orig[i];
			}
			free(orig);
		}
		else
		{
			txf->teximage = (unsigned char *) malloc(txf->tex_width * txf->tex_height);
			if (txf->teximage == NULL)
			{
				lastError = "out of memory.";
				goto error;
			}
			got = TXF_READ(file, txf->teximage, txf->tex_width * txf->tex_height);
			EXPECT(1);
		}
		break;

		case TXF_FORMAT_BITMAP:
		width = txf->tex_width;
		height = txf->tex_height;
		stride = (width + 7) >> 3;
		texbitmap = (unsigned char *) malloc(stride * height);
		if (texbitmap == NULL)
		{
			lastError = "out of memory.";
			goto error;
		}
		got = TXF_READ(file, texbitmap, stride * height);
		EXPECT(1);
		if (Bitmap::FontDepth == 16)
		{
			txf->teximage = (unsigned char *) calloc(width * height * 2, 1);
			if (txf->teximage == NULL)
			{
				lastError = "out of memory.";
				goto error;
			}
			for (i = 0; i < height; i++)
				for (j = 0; j < width; j++)
					if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7)))
					{
						txf->teximage[(i * width + j) * 2] = 255;
						txf->teximage[(i * width + j) * 2 + 1] = 255;
					}
		}
		else
		{
			txf->teximage = (unsigned char *) calloc(width * height, 1);
			if (txf->teximage == NULL)
			{
				lastError = "out of memory.";
				goto error;
			}
			for (i = 0; i < height; i++)
				for (j = 0; j < width; j++)
					if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7)))
						txf->teximage[i * width + j] = 255;
		}
		free(texbitmap);
		break;
	}

  TXF_CLOSE(file);
  return txf;

error:
	if (file)
		TXF_CLOSE(file);
	if (txf == NULL)
		return NULL;
    if (txf->tgi)
		free(txf->tgi);
    if (txf->tgvi)
		free(txf->tgvi);
    if (txf->lut)
		free(txf->lut);
    if (txf->teximage)
		free(txf->teximage);
    free(txf);
	return NULL;
}


void
txfGetStringMetrics(
  TexFont * txf,
  TCHAR *string,
  size_t len,
  int *width,
  int *max_ascent,
  int *max_descent)
{
	TexGlyphVertexInfo *tgvi;
	size_t i;
	int w, h;

	w = 0;
	h = 1;
	*width = 0;
	for (i = 0; i < len; i++)
		switch (string[i])
		   {
			case 27:
			switch (string[i + 1])
			   {
				case TEXT('M'):	i += 4; break;
				case TEXT('T'):	i += 7; break;
				case TEXT('L'):	i += 7; break;
				case TEXT('F'):	i += 13; break;
			   }
			break;

			case TEXT('\r'): break;

			case TEXT('\n'):
			if (string[i + 1] == 0)
				break;
			if (w > *width)
				*width = w;
			w = 0;
			++h;
			break;

			default:
			if (tgvi = getTCVI(txf, string[i]))
				w += int(tgvi->advance);
		   }
	if (w > *width)
		*width = w;
	*max_ascent = txf->max_ascent;
	*max_descent = h * (txf->max_ascent + txf->max_descent) - *max_ascent;
}


int
txfInFont(TexFont * txf, int c)
{
//  TexGlyphVertexInfo *tgvi;

  /* NOTE: No uppercase/lowercase substituion. */
  if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
    if (txf->lut[c - txf->min_glyph]) {
      return 1;
    }
  }
  return 0;
}

void
txfUnloadFont(
  TexFont * txf)
{
  if (txf->teximage) {
    free(txf->teximage);
  }
  free(txf->tgi);
  free(txf->tgvi);
  free(txf->lut);
  free(txf);
}

}	// end Vixen