#include "vixen.h"

namespace Vixen
{

static void* GLFWReadFile(const TCHAR* filename, Bitmap* bmap, Core::Stream* stream)
{
	GLFWimage	image;
	int			format = Bitmap::HASCOLOR;
	int			type = Bitmap::GLBITMAP;
	int			depth = 24;
	void*		pixels = NULL;

	if (glfwReadImage(filename, &image, GLFW_ORIGIN_UL_BIT) != GL_TRUE)
		return NULL;
	pixels = image.Data;
	switch (image.Format)
	{
		case GL_LUMINANCE:	format = 0;
		case GL_RGB:		break;
		case GL_RGBA:		format |= Bitmap::HASALPHA; break;

		default:
		VX_ERROR(("GLFWReadFile: ERROR unsupported GLFW file format\n"), NULL);
		break;
	}
	switch (image.BytesPerPixel)
	{
		case	2: depth = 16; break;
		case	3: depth = 24; break;
		case	4: depth = 32; break;

		default:
		VX_ERROR(("GLFWReadFile: ERROR unsupported GLFW file format\n"), NULL);
		break;
	}

	ObjectLock	lock(bmap);
	void*		dstpixels = bmap->Data;
	int			w = image.Width;
	int			h = image.Height;

	bmap->Type = type;
	bmap->Format = format;
	bmap->Depth = depth;
	bmap->Data = pixels;
	bmap->Width = w;
	bmap->Height = h;
	bmap->Type = Bitmap::DXBITMAP;
	return pixels;
}

void* Bitmap::ReadBitmap(const TCHAR* filename, Core::Stream* stream)
{
	Core::String		fname(filename);
	Core::String		ext(fname.Right(4));

	if (stream == NULL)							// no stream?
		return NULL;
	if (ext.CompareNoCase(TEXT(".txf")) == 0)	// is a font?
		return ReadFont(filename, stream);
	return GLFWReadFile(filename, this, stream);
}

bool Bitmap::Startup()
{
	return true;
}

void Bitmap::Shutdown()
{
}

}	// end Vixen
