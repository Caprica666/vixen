#include "vixen.h"
#ifndef VX_FREEIMAGE
extern "C"
{
#include "png.h"
};

/* Transparency parameters */

#define PNG_ALPHA     -2 /* Use alpha channel in PNG file, if there is one */
#define PNG_SOLID     -1 /* No transparency                                */
#define PNG_STENCIL    0 /* Sets alpha to 0 for r=g=b=0, 1 otherwise       */
#define PNG_BLEND1     1 /* a = r+g+b                                      */
#define PNG_BLEND2     2 /* a = (r+g+b)/2                                  */
#define PNG_BLEND3     3 /* a = (r+g+b)/3                                  */
#define PNG_BLEND4     4 /* a = r*r+g*g+b*b                                */
#define PNG_BLEND5     5 /* a = (r*r+g*g+b*b)/2                            */
#define PNG_BLEND6     6 /* a = (r*r+g*g+b*b)/3                            */
#define PNG_BLEND7     7 /* a = (r*r+g*g+b*b)/4                            */
#define PNG_BLEND8     8 /* a = sqrt(r*r+g*g+b*b)                          */

namespace Vixen
{

static void _cdecl
png_stream_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   Core::Stream* io_ptr = (Core::Stream*) png_get_io_ptr(png_ptr);
   if (!io_ptr->Read((char*) data, length))
      png_error(png_ptr, "read Error");
}


void* ReadPNGFile(Bitmap* bmap, Core::Stream* stream)
{
	int trans = PNG_ALPHA;
	FILE *fp = NULL;

//	GLint pack, unpack;
	png_structp png;
	png_infop	info;
	png_infop   endinfo;
	png_bytep   data;//, data2;
	png_bytep  *row_p;
	int		format = 0;
	double	fileGamma;
	double	screenGamma = 2.2 / 1.0;

	png_uint_32 width, height;
	int depth = 0, color;

	png_uint_32 i;
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct(png);
	endinfo = png_create_info_struct(png);

	png_set_read_fn(png, stream, png_stream_read_data);
	png_read_info(png, info);
	png_get_IHDR(png, info, &width, &height, &depth, &color, NULL, NULL, NULL);

//	if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
//		png_set_gray_to_rgb(png);
	if (color & PNG_COLOR_MASK_ALPHA && trans != PNG_ALPHA)
	{
		png_set_strip_alpha(png);
		color &= ~PNG_COLOR_MASK_ALPHA;
	}
//	if (color == PNG_COLOR_TYPE_PALETTE)
//		png_set_expand(png);
	png_set_bgr(png);							// read images as BGR or BGRA
	if (png_get_gAMA(png, info, &fileGamma))
		png_set_gamma(png, screenGamma, fileGamma);
	else
		png_set_gamma(png, screenGamma, 1.0/2.2);
	png_read_update_info(png, info);
	switch (color)
	{
		case PNG_COLOR_TYPE_GRAY_ALPHA:
		case PNG_COLOR_TYPE_RGB_ALPHA:
		depth = 32;
		format |= Bitmap::HASALPHA;
		break;

		default:
		depth *= png_get_channels(png, info);
	}
	data = (png_bytep) malloc(png_get_rowbytes(png, info)*height);
	row_p = (png_bytep *) malloc(sizeof(png_bytep)*height);

	for (i = 0; i < height; i++)
	{
#ifdef FLIP_ROWS
		row_p[i] = &data[png_get_rowbytes(png, info)*i];
#else
		row_p[height - 1 - i] = &data[png_get_rowbytes(png, info)*i];
#endif
	}

	png_read_image(png, row_p);
	free(row_p);

	png_read_end(png, endinfo);
	png_destroy_read_struct(&png, &info, &endinfo);
	stream->Close();
	bmap->Lock();
	bmap->Format = format;
	bmap->Depth = depth;
	bmap->Data = data;
	bmap->Width = width;
	bmap->Height = height;
	bmap->Type = Bitmap::DXBITMAP;
	bmap->Unlock();
	return data;
}

}	// end Vixen

#endif
