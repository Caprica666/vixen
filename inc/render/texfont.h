
/* Copyright (c) Mark J. Kilgard, 1997. */

/* This program is freely distributable without licensing fees  and is
   provided without guarantee or warrantee expressed or  implied. This
   program is -not- in the public domain. */

#pragma once

namespace Vixen {

#define TXF_FORMAT_BYTE		0
#define TXF_FORMAT_BITMAP	1

typedef struct {
  unsigned short c;       /* Potentially support 16-bit glyphs. */
  unsigned char width;
  unsigned char height;
  signed char xoffset;
  signed char yoffset;
  signed char advance;
  char dummy;           /* Space holder for alignment reasons. */
  short x;
  short y;
} TexGlyphInfo;

typedef struct {
  float t0[2];
  short v0[2];
  float t1[2];
  short v1[2];
  float t2[2];
  short v2[2];
  float t3[2];
  short v3[2];
  float advance;
} TexGlyphVertexInfo;

typedef struct {
  unsigned int texobj;
  int tex_width;
  int tex_height;
  int max_ascent;
  int max_descent;
  int num_glyphs;
  int min_glyph;
  int range;
  unsigned char *teximage;
  TexGlyphInfo *tgi;
  TexGlyphVertexInfo *tgvi;
  TexGlyphVertexInfo **lut;
} TexFont;

class Scene;

extern const char *txfErrorString(void);

extern TexFont *txfLoadFont(
  const TCHAR *filename, Core::Stream* stream);

extern void txfUnloadFont(
  TexFont * txf);

extern unsigned int txfEstablishTexture(
  TexFont * txf,
  unsigned int texobj,
  unsigned char setupMipmaps);

extern void txfBindFontTexture(
  TexFont * txf);

extern void txfGetStringMetrics(
  TexFont * txf,
  TCHAR *string,
  size_t len,
  int *width,
  int *max_ascent,
  int *max_descent);

extern void txfRenderGlyph(
  TexFont * txf,
  int c,
  Scene* scene = NULL);

extern void txfRenderString(
  TexFont * txf,
  char *string,
  int len,
  Scene* scene = NULL);

extern void txfRenderFancyString(
  TexFont * txf,
  char *string,
  int len);

extern TexGlyphVertexInfo* getTCVI(TexFont * txf, int c);

} // end Vixen
