/****
 *
 * Description:
 * OpenGL version of Texture.
 *
 ****/
#include "vixen.h"
#include "render/dib-glue.h"
#include "ogl/vxrendergl.h"

inline bool IS_POWER_OF_2(int n)
{
	bool	ispow = false;

	while (n = (n >> 1))
		if (ispow)
			return true;
		else
			ispow = true;
	return false;
}

namespace Vixen {

int GLRenderer::s_MaxTexID = 0;

/*
 * @fn void BGR2RGB(void* ptrToPixels, int w, int h, int numbytes)
 *
 * Converts an BGR image to a RGB by swapping the red and blue
 * components of each pixel. This is useful for converting DirectX
 * format images to OpenGL format. If the image has an alpha
 * channel, it is left untouched.
 */
static void BGR2RGB(void* ptrToPixels, int w, int h, int d)
{
	int		size = w * h;
	uchar*	source = (uchar*) ptrToPixels;
	uchar	r, b, a = 255;
	int		i;
	int		stride = 3;

	if (d < 24)
		VX_ERROR_RETURN(("BGR2RGB: ERROR unsupported depth %d\n", d));
	if (d == 32)
		stride = 4;
	for (i = 0; i < size; ++i)
	{
		r = source[0];
		b = source[2];
		source[0] = b;
		source[2] = r;
		source += stride;
	}
}

/*!
 * @fn void* GLRenderer::UpdateTexture(Texture* texture, int texunit, bool mipmap)
 *
 * Callback used during rendering to load a texture into
 * the display device.
 *
 * @see Texture::Load Texture::CreateTexture
 */
intptr GLRenderer::UpdateTexture(Texture* texture, int texunit, bool mipmap)
{
/*
 * The texture pixels changed or the texture has no handle yet.
 * We look up the bitmap in the shared image dictionary.
 * This dictionary keeps the DIB handle and the device texture handle
 * for texture.
 */
	Bitmap*		bmap = texture->GetBitmap();
	intptr		texhandle;

	if (bmap == NULL)
		return 0;
/*
 * Image or its bitmap has changed and needs to be loaded into device.
 * We don't lock unless we are going to create a texture and update the bitmap.
 */
	texhandle = bmap->DevHandle;
	if ((texture->HasChanged() || bmap->HasChanged() || !texhandle) && bmap->Data)
	{
		ObjectLock	lock(bmap);
		uint32		iFreeId;
		void*		newglb = NULL;
		GLenum		format = GL_RGB;
		GLint		internalformat;
		GLint		gltype = GL_UNSIGNED_BYTE;
		void*		glb = bmap->Data;
		const Col4*	transp = (bmap->Format & Bitmap::HASCOLORKEY) ? &texture->GetTranspColor() : NULL;

		if (glb == NULL)				// in case it changed before lock
			return false;				// we'll come back next frame
	#ifdef _DEBUG
		if (!IS_POWER_OF_2(bmap->Width) ||
			!IS_POWER_OF_2(bmap->Height))
			VX_WARNING(("Texture::Create %s texture not a power of 2\n", texture->GetFileName()));
	#endif
		if (Bitmap::RGBADepth == 16)
		{
			if (bmap->Format & Bitmap::HASCOLOR)
			{
				if (bmap->Format & Bitmap::HASALPHA || transp)
				{
					gltype = GL_UNSIGNED_SHORT_5_5_5_1;
					format = internalformat = GL_RGB5_A1;
				}
				else
				{
					gltype = GL_UNSIGNED_SHORT_5_6_5;
					format = internalformat = GL_RGB565;
				}
			}
			else
				internalformat = format = GL_DEPTH_COMPONENT16;		
		}
		else
		{
			if (bmap->Format & Bitmap::HASCOLOR)
			{
				if (bmap->Format & Bitmap::HASALPHA || transp)
					format = internalformat = GL_RGBA;
			}
			else
				internalformat = format = GL_DEPTH_COMPONENT32;
		}

		switch (bmap->Type)
		{
			case Bitmap::DIB:
			glb = ((BITMAPINFOHEADER*) glb) + 1;
			newglb = glb = Bitmap::DXImageFromDIB(bmap->Data, transp);
			if ((bmap->Depth > 16) && (Bitmap::RGBADepth == 16))
			{
				newglb = Bitmap::Make16Bit(newglb, NULL, bmap->Width, bmap->Height, bmap->Depth, bmap->Width * 4);
				free(glb);
				glb = newglb;
			}
			break;

			case Bitmap::GLBITMAP:
			BGR2RGB(glb, bmap->Width, bmap->Height, bmap->Depth);
			break;

			case Bitmap::DXBITMAP:
			break;

			case Bitmap::FONT:
			if (bmap->Depth == 16)
			{
				format = GL_LUMINANCE_ALPHA;
				internalformat = GL_LUMINANCE_ALPHA;
			}
			else if (bmap->Depth == 8)
			{
				format = GL_LUMINANCE;
				internalformat = GL_LUMINANCE;
			}
			glb = ((TexFont*) bmap->Data)->teximage;
			break;

//			case Bitmap::DDS:
//			return CreateDDSTexture(bmap);

			default:
			VX_ERROR(("GLRenderer::UpdateTexture ERROR unsupported texture format %d\n", bmap->Type), 0);
		}
		if ((bmap->Depth > 16) && (Bitmap::RGBADepth == 16))
		{
			newglb = Bitmap::Make16Bit(glb, NULL, bmap->Width, bmap->Height, bmap->Depth, bmap->Width * bmap->Depth / 8);
			if (glb != bmap->Data)
				free(glb);
			glb = newglb;
		}
		// find the next available OpenGL texture id ("name")
		if (bmap->DevHandle)
		{
			iFreeId = (int32) bmap->DevHandle;	// GL texture IDs do not exceed 32 bits
			glDeleteTextures(1, &iFreeId);
		}
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &iFreeId);
		glBindTexture(GL_TEXTURE_2D, iFreeId);
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, bmap->Width,
					bmap->Height, 0, format, gltype, glb);
		if (mipmap)
			glGenerateMipmap(GL_TEXTURE_2D);
		if (newglb == bmap->Data)
			newglb = NULL;
		if (bmap->Type != Bitmap::FONT)
			bmap->Kill();
		bmap->Type = Bitmap::TEXHANDLE;
		bmap->DevHandle = iFreeId;
		texhandle = iFreeId;
		if (newglb)
			free(newglb);
	#ifdef _DEBUG
		if (texture->GetFileName() == NULL)
			{ VX_TRACE(Texture::Debug, ("UpdateTexture (mem) -> %d\n", iFreeId)); }
		else
			{ VX_TRACE(Texture::Debug, ("UpdateTexture %s -> %d\n", texture->GetFileName(), iFreeId)); }
	#endif
		bmap->SetChanged(false);
		texture->SetChanged(false);
		if (texhandle)
		{
			if ((int) texhandle > s_MaxTexID)
				s_MaxTexID = texhandle;
			VX_TRACE(GLRenderer::Debug, ("GLRenderer::UpdateTexture %d %s\n", texunit, texture->GetFileName()));
		}
	}
	else
		glBindTexture(GL_TEXTURE_2D, texhandle);
	return texhandle;
}

void GLRenderer::DeleteTextures()
{
	for (int i = 1; i <= s_MaxTexID; ++i)
	{
		GLuint glid = (GLuint) i;
		glDeleteTextures(1, &glid);
	}
	s_MaxTexID = 0;
}

}	// end Vixen