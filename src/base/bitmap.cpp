#include "vixen.h"
#include "render/texfont.h"
#include "win32/dds.h"

namespace Vixen {

#define BI_RGB        0L

typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned char byte;

typedef struct tagBITMAPFILEHEADER {
	word    bfType;
	dword   bfSize;
	word    bfReserved1;
	word    bfReserved2;
	dword   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	dword	biSize;
	long	biWidth;
	long	biHeight;
	word	biPlanes;
	word	biBitCount;
	dword	biCompression;
	dword	biSizeImage;
	long	biXPelsPerMeter;
	long	biYPelsPerMeter;
	dword	biClrUsed;
	dword	biClrImportant;
} BITMAPINFOHEADER;

VX_IMPLEMENT_CLASSID(Bitmap, SharedObj, VX_Bitmap);

int32 Bitmap::FontDepth = 8;
int32 Bitmap::RGBADepth = 32;
int32 Bitmap::RGBDepth = 24;
vint32 Bitmap::IsInitialized = 0;

Bitmap::Bitmap() : SharedObj()
{
	Type = NOTEXTURE;
	Data = NULL;
	Format = 0;
	Width = 0;
	Depth = 0;
	Height = 0;
	DevHandle = 0;
}

Bitmap::~Bitmap()
{
	Kill();
	KillLock();
}

Bitmap&	Bitmap::operator=(const Bitmap& src)
{
	Lock();
	Type = src.Type;
	Data = src.Data;
	Width = src.Width;
	Height = src.Height;
	Depth = src.Depth;
	Format = src.Format;
	ByteSize = src.ByteSize;
	Unlock();
	return *this;
};

void Bitmap::Kill()
{
#ifndef VX_NOTEXTURE
	if ((Data == NULL) || (Format & Bitmap::NOFREE_DATA))
		return;
	VX_TRACE(Bitmap::Debug, ("Bitmap::Kill freeing bitmap %p\n", Data));
	switch (Type)
	{
		case DDS:
#if defined(VX_DX11)
		free(Data);
#else
		free(((S3TCGenericImage*) Data)->pixels);
#endif

		case DIB:
		case GLBITMAP:
		case DXBITMAP:
		free(Data);
		break;

		case FONT:
		txfUnloadFont((TexFont*) Data);
		break;
	}
	Data = 0;
#endif
}

bool Bitmap::ReadFile(const TCHAR* imagename, Core::Stream* stream, LoadEvent* e)
{
	LoadSceneEvent*	ev = (LoadSceneEvent*) e;
	Bitmap*			bmap = (Bitmap*) (SharedObj*) ev->Object;

	ev->Code = Event::LOAD_IMAGE;
	ev->FileName = imagename;
	if (bmap == NULL)
	{
		bmap = new Bitmap();
		bmap->MakeLock();
		ev->Object = bmap;
	}
	if (bmap->Type != ISLOADING)
		bmap->Load(imagename, stream);
	return true;
}

/*!
 * @fn bool Bitmap::Load(const TCHAR* filename, Core::Stream* stream)
 * @param filename	name of bitmap file to load
 * @param stream	stream to use to read the file
 *
 * This function issues a blocking read until the complete image
 * file has been read. It should never be called from real-time code
 * (like inside an engine) because it will stop processing until the file is read.)
 * To load an image asynchronously, use Texture::Load.
 *
 * @return \b true if successfully loaded, else \b false
 *
 * @see Texture::Load World::LoadAsync FileLoader
 */
#ifdef VX_NOTEXTURE				// no texturing
bool Bitmap::Load(const TCHAR* filename, Core::Stream* stream)
{ return false; }

void *Bitmap::ReadBitmap(const TCHAR* filename, Core::Stream* stream)
{ return NULL; }

#else
bool Bitmap::Load(const TCHAR* filename, Core::Stream* stream)
{
	if (!TryLock() && (Type != NOTEXTURE))
	{
		VX_TRACE(Bitmap::Debug || FileLoader::Debug, ("Image::Load %s already loaded\n", filename));
		return Data != NULL;
	}
	if ((Type != NOTEXTURE) || Data)
	{
		VX_TRACE(Bitmap::Debug || FileLoader::Debug, ("Image::Load %s already loaded\n", filename));
		Unlock();
		return Data != NULL;
	}
	Type = ISLOADING;
	Unlock();
	ReadBitmap(filename, stream);
	if (Data)
	{
		VX_TRACE(Bitmap::Debug || FileLoader::Debug, ("Image::Load %s load complete\n", filename));
		return true;
	}
	else
	{
		Type = NOTEXTURE;
		VX_ERROR(("Bitmap::Load ERROR did not load %s\n", filename), false);
	}
}

#endif // VX_NOTEXTURE

void *Bitmap::ReadFont(const TCHAR* filename, Core::Stream* instream)
{
	Ref<Core::Stream>	stream = instream;
	if (instream == NULL)
	{
		stream = new Core::FileStream;
		if (!stream->Open(filename, Core::Stream::OPEN_READ))
			return NULL;
	}
	TexFont* texfont = txfLoadFont(filename, stream);
	stream->Close();
	if (texfont == NULL)
		return NULL;
	Lock();
	Data = texfont;
	Width = texfont->tex_width;
	Height = texfont->tex_height;
	Depth = FontDepth;
	Format = HASALPHA;
	Type = FONT;
	ByteSize = Width * Height;
	if (FontDepth == 16)
		ByteSize *= 2;
	Unlock();
	return texfont;
}

bool Bitmap::WriteBMP(const TCHAR *filename, void* pixels, int width, int height)
{
    BITMAPFILEHEADER	bitmapHdr;
	BITMAPINFOHEADER	dibHdr;

	FILE *fp = FOPEN(filename, TEXT("w"));
	if (!fp)
        return false;
	bitmapHdr.bfType = 0x4d42;
    bitmapHdr.bfReserved1 = 0; 
    bitmapHdr.bfReserved2 = 0; 
    bitmapHdr.bfOffBits = sizeof(dibHdr) + sizeof(bitmapHdr); 
	dibHdr.biSize = sizeof(dibHdr);
	dibHdr.biBitCount = 24;
	dibHdr.biPlanes = 1;
	dibHdr.biCompression = BI_RGB;
	dibHdr.biClrUsed = 0;
	dibHdr.biClrImportant = 0;
	dibHdr.biWidth = width;
	dibHdr.biHeight = height;
	dibHdr.biSizeImage = ((((width * dibHdr.biBitCount) + 31) & ~31) >> 3) * height;
	bitmapHdr.bfSize = sizeof(bitmapHdr) + sizeof(dibHdr) + dibHdr.biSizeImage; 

	if (fwrite(&bitmapHdr.bfType, 2, 1, fp) != 1)
        return false;
	if (fwrite(&bitmapHdr.bfSize, 4, 3, fp) != 3)
        return false;
	if (fwrite(&dibHdr, sizeof(dibHdr), 1, fp) != 1)
        return false;
	if (fwrite(pixels, dibHdr.biSizeImage, 1, fp) != 1)
		return false;
	return true;
}

inline uint16 swapshort(uint16 arg)
{
	return ((arg << 8) & 0xFF00) | ((arg >> 8) & 0x00FF); 
}

inline uint32 swaplong(uint32 arg)
{
	return ((arg & 0x000000FF) << 24) | ((arg & 0x0000FF00) << 8) | ((arg >> 8) & 0x0000FF00) | ((arg >> 24) & 0x000000FF); 
}

bool Bitmap::WriteRGB(const TCHAR *filename, void* pixels, int width, int height, int depth)
{
	struct
	{
		uint16  magic;
		uchar	rle;
		uchar	bpc;
		uint16  dim;
		uint16  xsize;
		uint16  ysize;
		uint16	zsize;
		uint32	min;
		uint32	max;
		uint32	waste;
		char	name[80];
		uint32	space[102];
	} rgbhdr;
	int			numchannels = (depth == 32) ? 4 : 3;
	FILE*		fp = FOPEN(filename, TEXT("w"));

	if (!fp)
        return false;
	VX_ASSERT((depth == 24) || (depth == 32));
	VX_ASSERT(sizeof(rgbhdr) == 512);
	rgbhdr.magic = swapshort(474);
	rgbhdr.rle = 0;
	rgbhdr.bpc = 1;
	rgbhdr.dim = swapshort(depth / 8);
	rgbhdr.xsize = swapshort(width);
	rgbhdr.ysize = swapshort(height);
	rgbhdr.zsize = swapshort(depth / 8);
	rgbhdr.min = 0;
	rgbhdr.max = swaplong(255);
	rgbhdr.waste = 0;
	for (int i = 0; i < 80; i++)
		rgbhdr.name[i] = 0;
	for (int i = 0; i < 102; i++)
		rgbhdr.space[i] = 0;
	if (fwrite(&rgbhdr, sizeof(rgbhdr), 1, fp) != 1)
        return false;
	for (int channel = 0; channel < numchannels; ++channel)
		for (int i = 0; i < width * height; ++i)
		{
			char* p = ((char*) pixels) + i * numchannels + channel;
			if (fwrite(p, 1, 1, fp) != 1)
			{
				fclose(fp);
				return false;
			}
		}
	fclose(fp);
	return true;
}

/*!
 * @fn void* Bitmap::MakeAlpha(void* data, const Col4* transp) 
 * @param data		pixel data to use, may be 16, 24 or 32 bit
 * @param transp	transparent color, if not null, a color key alpha is made
 *
 * Makes an alpha channel for this bitmap based on the
 * input transparency color. Pixels in the current texture
 * which match the transparent color will have an alpha of
 * zero and the others will have alpha of 1. If the current
 * image size is 24 bit, a new data area with 32 bit pixels
 * is allocated and returned
 *
 * @return pointer to pixel data with alpha channel or NULL on failure
 */
void* Bitmap::MakeAlpha(void* data, const Col4* transp) 
{
	uchar	red_transp, green_transp, blue_transp;
	uchar*	image = (uchar*) data;
	uchar*	dest = (uchar*) data;
	uchar*	source = (uchar*) data;
	int		size = Width * Height;
	int		i;

	VX_ASSERT(size > 16);
	if (transp)
	{
		Format |= HASCOLORKEY | HASALPHA;
		red_transp = uchar(255 * transp->r);
		green_transp = uchar(255 * transp->g);
		blue_transp = uchar(255 * transp->b);
	}

	switch (Depth)
	{
/*
 * Current data area is 24 bits per pixel and we want 32.
 * Allocate a new data area and copy the RGB pixels. If
 * a color key is requested, make the alpha from the transparent
 * color. Otherwise, alpha will be white (all opaque)
 */
		case 24:
		dest = image = (uchar *) malloc(size * 4);
		for (i = 0; i < size; i++)
		{
			*dest++ = source[0];
			*dest++ = source[1];
			*dest++ = source[2];
			if (transp &&
				(source[0] == red_transp) &&
				(source[1] == green_transp) &&
				(source[2] == blue_transp))
				*dest++ = 0;
			else
				*dest++ = 255;
			source += 3;
		}
		Depth = 32;
		break;
/*
 * Current data area is 32 bits per pixel but a color key
 * has been requested. Make the alpha channel from the transparent
 * color and the RGB pixels.
 */
		case 32:
		if (transp == NULL)
			break;
		dest += 3;
		for (i = 0; i < size; i++)
		{
			uchar diff = source[0] - red_transp;
			diff += (source[1] - green_transp);
			diff += (source[2] - blue_transp);
			if (diff < 8)
				*dest = 0;
			else
				*dest = 255;
			dest += 4;
			source += 4;
		}
		break;
/*
 * Current data area is 16 bits per pixel and a color key has
 * been requested. Convert the R5G6B5 image to A1R5G5B5.
 * Make the alpha channel from the nonzero pixels.
 */
		case 16:
		if (transp == NULL)
			break;
		for (i = 0; i < size; i++)
		{
			int16 v = *((int16*) source);
			if (v != 0)			// convert 565 to 555 and add alpha
			{
				v = ((v >> 1) & ~0x1F) | (v & 0x1F) | 0x8000;
				*((int16*) source) = v;
			}
			source += 2;
		}
		break;
	}
	return image;
}

void* Bitmap::Make16Bit(const void* srcbits, void* dstbits, int w, int h, int srcdepth, int stride) 
{
	uint16*	dest;
	uchar*	source = (uchar*) srcbits;
	int		i, j;

	if (dstbits == NULL)
		dstbits = (uint16*) malloc(w * h * 2);
	dest = (uint16*) dstbits;
	switch (srcdepth)
	{
/*
 * Current data area is 32 bits per pixel A8R8G8B8 and we want to make it 
 * 16 bits R5G6B5
 */
		case 24:
		for (i = 0; i < h; i++)
		{
			source = (uchar*) srcbits + (i * stride);
			for (j = 0; j < w; ++j)
			{
				uchar blue = (*source++ >> 3) & 0x01F;
				uchar green = (*source++ >> 2) & 0x03F;
				uchar red = (*source++ >> 3) & 0x01F;
				uint16 v = (red << 11) | (green << 5) | blue;
				*dest++ = v;
			}
		}
		break;
/*
 * Current data area is 32 bits per pixel A8R8G8B8 and we want to make it 
 * 16 bits A1R5G5B5
 */
		case 32:
		for (i = 0; i < h; i++)
		{
			source = (uchar*) srcbits + (i * stride);
			for (j = 0; j < w; ++j)
			{
				uchar blue = (*source++ >> 3) & 0x01F;
				uchar green = (*source++ >> 3) & 0x01F;
				uchar red = (*source++ >> 3) & 0x01F;
				uint16 v = (red << 10) | (green << 5)  | blue;
				if (*source++ > 128)
					v |= 0x8000;
				else
					v &= 0x7FFF;
				*dest++ = v;
			}
		}
		break;
	}
	return dstbits;
}

//
// Create an OpenGL-stype RGB/A image buffer, then fill it
// with image data from an existing DIB formatted image buffer.
//
void* Bitmap::GLImageFromDIB(const void *dib, const Col4* transp, void* pixels)
{
	const BITMAPINFOHEADER *bmi = (const BITMAPINFOHEADER *) dib;
	int i, j;
	unsigned char *image, *line, *source, *dest;

	if (!dib)
		return NULL;

	uchar red_transp, green_transp, blue_transp;
	int x = bmi->biWidth;
	int y = bmi->biHeight;
	int d = bmi->biBitCount;
	int stride = ((x * d + 31) >> 5) << 2;  // BMP line stride
	int gl_size = (x * y * d) / 8;          // gltextImage2D buffer size

	if (transp)
	{
		red_transp = uchar(255 * transp->r);
		green_transp = uchar(255 * transp->g);
		blue_transp = uchar(255 * transp->b);
		gl_size = x * y * 4;
	}
	image = (unsigned char *) malloc(gl_size);
	if (pixels == NULL)
		pixels = (unsigned char *)dib + sizeof(BITMAPINFOHEADER);
	dest = image;
	line = (unsigned char*) pixels;
	for (i = 0; i < y; i++)
	{
		source = line + i * stride;
		for (j = 0; j < x; j++)
		{
			*dest++ = source[2];
			*dest++ = source[1];
			*dest++ = source[0];
			if (d == 32)
			{
				*dest++ = source[3];
				source += 4;
			}
			else
			{
				if (transp)
					if ((source[0] == red_transp) &&
						(source[1] == green_transp) &&
						(source[2] == blue_transp))
						*dest++ = 0;
					else
						*dest++ = 255;
				source += 3;
			}
		}
	}
	return image;
}

void* Bitmap::DXImageFromDIB(const void *dib, const Col4* transp, void* pixels) 
{
	const BITMAPINFOHEADER *bmi = (const BITMAPINFOHEADER *)dib;
	int i, j;
	unsigned char *image, *line, *source, *dest;

	if (!dib)
		return NULL;

	uchar red_transp, green_transp, blue_transp;
	int x = bmi->biWidth;
	int y = bmi->biHeight;
	int d = bmi->biBitCount;
	int stride = ((x * d + 31) >> 5) << 2;	// BMP line stride
	int size = (x * y * d) / 8;				// buffer size

	if (transp)
	{
		red_transp = uchar(255 * transp->r);
		green_transp = uchar(255 * transp->g);
		blue_transp = uchar(255 * transp->b);
		size = x * y * 4;
	}
	image = (unsigned char*) malloc(size);
	if (pixels == NULL)
		pixels = (unsigned char*) dib + sizeof(BITMAPINFOHEADER);
	dest = image;
	line = (unsigned char*) pixels;
	for (i = 0; i < y; i++)
	{
		source = line + i * stride;
		for (j = 0; j < x; j++)
		{
			*dest++ = source[0];
			*dest++ = source[1];
			*dest++ = source[2];
			if (d == 32)
			{
				*dest++ = source[3];
				source += 4;
			}
			else
			{
				if (transp)
					if ((source[0] == red_transp) &&
						(source[1] == green_transp) &&
						(source[2] == blue_transp))
						*dest++ = 0;
					else
						*dest++ = 255;
				source += 3;
			}
		}
	}
	return image;
}

}	// end Vixen
