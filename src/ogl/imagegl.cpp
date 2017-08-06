/****
 *
 * Description:
 * OpenGL version of Texture.
 *
 ****/
#include "vixen.h"
#include "render/dib-glue.h"
#include "ogl/vxrendergl.h"

#ifndef VIXEN_GLES2
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
	Bitmap*	bmap = texture->GetBitmap();
	intptr	texhandle;
	int		debug;

	if (bmap == NULL)
		return NULL;
	debug = Texture::Debug | GLRenderer::Debug;
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
		GLenum		format;
		GLint		internalformat;
		void*		glb = bmap->Data;
		const Col4*	transp = (bmap->Format & Bitmap::HASCOLORKEY) ? &texture->GetTranspColor() : NULL;

		if (glb == NULL)				// in case it changed before lock
			return false;				// we'll come back next frame
	#ifdef _DEBUG
		if (!IS_POWER_OF_2(bmap->Width) ||
			!IS_POWER_OF_2(bmap->Height))
			VX_WARNING(("Texture::Create %s texture not a power of 2", texture->GetFileName()));
	#endif
		internalformat = (Bitmap::RGBDepth == 16) ? GL_RGB5 : GL_RGB8;
		switch (bmap->Depth)
		{
			case 32:
			VX_ASSERT(bmap->Format & Bitmap::HASCOLOR);
			format = GL_BGRA_EXT;
			internalformat = (Bitmap::RGBADepth == 16) ? GL_RGBA4 : GL_RGBA8;
			break;

			case 24:
			format = GL_BGR_EXT;
			break;

			case 16:
			format = GL_LUMINANCE;
			if ((bmap->Format & Bitmap::HASCOLOR) == 0)
				internalformat = GL_LUMINANCE16;
			break;

			case 8:
			format = GL_LUMINANCE;
			internalformat = GL_INTENSITY8;
			break;

			default:
			return NULL;
		}
		switch (bmap->Type)
		{
			case Bitmap::DIB:
			glb = ((BITMAPINFOHEADER*) glb) + 1;
			newglb = glb = Bitmap::DXImageFromDIB(bmap->Data, transp);
			if (transp)
				format = GL_BGRA_EXT;
			break;

			case Bitmap::DXBITMAP:	// can use DXBitmap asis if no color key
			if (transp)				// add alpha channel for color key
			{
				glb = newglb = bmap->MakeAlpha(glb, transp);
				format = GL_BGRA_EXT;
				internalformat = (Bitmap::RGBADepth == 16) ? GL_RGBA4 : GL_RGBA8;
			}
			break;

			case Bitmap::GLBITMAP:	// can use GLbitmap asis if no color key
			if (transp)		// add alpha channel for color key
			{
				format = GL_RGBA;
				glb = newglb = bmap->MakeAlpha(glb, transp);
				internalformat = (Bitmap::RGBDepth == 16) ? GL_RGBA4 : GL_RGBA8;
			}
			else if (bmap->Depth == 24)
				format = GL_RGB;
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
				internalformat = GL_INTENSITY4;
			}
			glb = ((TexFont*) bmap->Data)->teximage;
			break;

			case Bitmap::DDS:
			return CreateDDSTexture(bmap);

			default:
			return NULL;
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
		if (mipmap)
		{
			int rc = gluBuild2DMipmaps(GL_TEXTURE_2D, internalformat, bmap->Width,
							  bmap->Height, format, GL_UNSIGNED_BYTE, glb);
			bmap->Format |= Texture::MIPMAP;
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, internalformat, bmap->Width,
						bmap->Height, 0, format, GL_UNSIGNED_BYTE, glb);
			bmap->Format &= ~Texture::MIPMAP;
		}
		if (newglb == bmap->Data)
			newglb = NULL;
		if (bmap->Type != Bitmap::FONT)
			bmap->Kill();
		bmap->Type = Bitmap::TEXHANDLE;
		bmap->DevHandle = iFreeId;
		texhandle = iFreeId;
		if (newglb)
			free(newglb);
		VX_TRACE(debug, ("GLRenderer::UpdateTexture Create %s -> %d\n", texture->GetFileName(), iFreeId));
		bmap->SetChanged(false);
		texture->SetChanged(false);
		return iFreeId;
	}
	else
		glBindTexture(GL_TEXTURE_2D, (GLuint) texhandle);
	VX_TRACE(debug > 1, ("GLRenderer::UpdateTexture Bind %s -> %d\n", texture->GetFileName(), texhandle));
	return texhandle;
}

void GLRenderer::DeleteTextures()
{
	for (intptr i = 1; i < s_MaxTexID; ++i)
	{
		GLuint glid = (GLuint) i;
		glDeleteTextures(1, &glid);
	}
	s_MaxTexID = 0;
}

#include "win32/dds.h"

intptr GLRenderer::CreateDDSTexture(Bitmap* bmap)
{
	S3TCGenericImage* S3TCImage = NULL;
	int i;
	int width;
	int height;
	int size;
	int offset;
	int numMipmaps = 1;
	int blockSize;
	unsigned int iFreeId;

	/* Assume tightly packed textures. */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	S3TCImage = (S3TCGenericImage*) bmap->Data;
	height = bmap->Height;
	width = bmap->Width;
	offset = 0;
	blockSize = (S3TCImage->format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	glGenTextures(1, &iFreeId);
	glBindTexture(GL_TEXTURE_2D, iFreeId);

	/* load the mipmaps */
	for (i = 0; i < numMipmaps && (width || height); ++i)
	{
		if (width == 0)
		  width = 1;
		if (height == 0)
		  height = 1;

		size = ((width+3)/4)*((height+3)/4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, i, S3TCImage->format, width, height, 
			0, size, S3TCImage->pixels + offset);
		offset += size;
		width >>= 1;
		height >>= 1;
#ifdef _DEBUG
		int err = glGetError();
		if (err)
			VX_PRINTF(("GL ERROR %s\n", gluErrorString((enum GLenum) err)));
#endif
	}
	bmap->Type = Bitmap::TEXHANDLE;
	bmap->Kill();
	bmap->DevHandle = iFreeId;
	return iFreeId;
}

}	// end Vixen

#endif