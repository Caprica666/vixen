#include "vixen.h"

namespace Vixen {

#ifdef VX_NOTEXTURE

void *Bitmap::ReadDDS(const TCHAR* filename, Core::Stream* stream)
	{	return NULL; }

#else


#ifdef VIXEN_DX10

void *Bitmap::ReadDDS(const TCHAR* filename, Core::Stream* stream)
{
	int			size_inc = 4096;
	int			total_size = 0;
	char*		data = (char*) malloc(size_inc);
	char*		cur = data;
	int			bytes_read;
	D3DX10_IMAGE_INFO info;
	HRESULT		hr;

	/*
	 * Read the bitmap into memory
	 */
	if (!data)
		return NULL;
	while (bytes_read = stream->Read(cur, size_inc))
	{
		total_size += bytes_read;
		data = (char*) realloc(data, total_size + size_inc);
		cur = data + total_size;
	}
	/*
	 * Query DirectX for the dimensions and format of the bitmap
	 */
	hr = D3DX10GetImageInfoFromMemory(data, total_size, NULL, &info, NULL);
	VX_ASSERT(info.ResourceDimension == D3D10_RESOURCE_DIMENSION_TEXTURE2D);
	Lock();
	if (SUCCEEDED(hr))
	{
		Width = info.Width;
		Height = info.Height;
		Depth = info.Depth;
		Format = Bitmap::HASCOLOR | Bitmap::HASALPHA;
		if (info.ImageFileFormat == D3DX10_IFF_DDS)
			Type = Bitmap::DDS;
		else
			Type = Bitmap::DIB;
	}
	else
	{
		Width = Height = Depth = 0;
		Type = Bitmap::DIB;
	}
	Data = data;
	ByteSize = total_size;
	Unlock();
	return data;
}

#else

#include "win32/dds.h"
#include "win32/ddglue.h"

void *Bitmap::ReadDDS(const TCHAR* filename, Core::Stream* stream)
{
	S3TCGenericImage *S3TCImage;
	DDS_HEADER	S3TCHeader;
	int			vixformat = HASCOLOR;
	int			depth;
	char		filecode[4];
	int			factor = 1;

	/* verify the type of file */
	stream->Read(filecode, 4);
	if (strncmp(filecode, "DDS ", 4) != 0)
		return NULL;

	/* get the surface desc */
	stream->Read((char*)&S3TCHeader, sizeof(S3TCHeader));
	S3TCImage = (S3TCGenericImage*) malloc(sizeof(S3TCGenericImage));
	memset(S3TCImage,0,sizeof(S3TCGenericImage));

	if (S3TCHeader.dwSurfaceFlags & DDS_SURFACE_FLAGS_CUBEMAP)
		Format |= Texture::CUBEMAP;
#if defined(VIXEN_DX9)
	depth = 32;
	switch (S3TCHeader.ddspf.dwFourCC)
	{
		case D3DFMT_DXT1:
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
        S3TCImage->format = (D3DFORMAT) S3TCHeader.ddspf.dwFourCC;
		break;

		default:
		if (S3TCHeader.ddspf.dwFlags == DDS_RGBA && S3TCHeader.ddspf.dwRGBBitCount == 32 && S3TCHeader.ddspf.dwABitMask == 0xff000000)
		{
			S3TCImage->format = D3DFMT_A8R8G8B8;
			vixformat |= HASALPHA;
			depth = 32;
		}
		else if (S3TCHeader.ddspf.dwFlags == DDS_RGB  && S3TCHeader.ddspf.dwRGBBitCount == 24)
		{
			S3TCImage->format = D3DFMT_R8G8B8;
			depth = 24;
		}
		else if (S3TCHeader.ddspf.dwFlags == DDS_RGB  && S3TCHeader.ddspf.dwRGBBitCount == 16 && S3TCHeader.ddspf.dwGBitMask == 0x000007e0)
		{
			S3TCImage->format = D3DFMT_R5G6B5;
			depth = 16;
		}
		else if (S3TCHeader.ddspf.dwFlags == DDS_RGBA && S3TCHeader.ddspf.dwRGBBitCount == 16 && S3TCHeader.ddspf.dwABitMask == 0x00008000)
		{
			S3TCImage->format = D3DFMT_A1R5G5B5;
			vixformat |= HASALPHA;
			depth = 16;
		}
		else if (S3TCHeader.ddspf.dwFlags == DDS_RGBA && S3TCHeader.ddspf.dwRGBBitCount == 16 && S3TCHeader.ddspf.dwABitMask == 0x0000f000)
		{
			S3TCImage->format = D3DFMT_A4R4G4B4;	
			vixformat |= HASALPHA;
			depth = 16;
		}
		else
			return NULL; // unknown format
	}
#elif VIXEN_OGL 
	switch (S3TCHeader.ddspf.dwFourCC)
	{
		case D3DFMT_DXT1:
		S3TCImage->format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		vixformat |= HASALPHA;
		depth = 32;
		factor = 2;
		break;

		case D3DFMT_DXT3:
		S3TCImage->format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		vixformat |= HASALPHA;
		depth = 32;
		factor = 4;
		break;

		case D3DFMT_DXT5:
		S3TCImage->format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		vixformat |= HASALPHA;
		depth = 32;
		factor = 4;
		break;

		default:
		if (S3TCHeader.ddspf.dwFlags == DDS_RGBA && S3TCHeader.ddspf.dwRGBBitCount == 32 && S3TCHeader.ddspf.dwABitMask == 0xff000000)
		{
			S3TCImage->format = GL_COMPRESSED_RGBA_ARB;
			vixformat |= HASALPHA;
			depth = 32;
		}
		else if (S3TCHeader.ddspf.dwFlags == DDS_RGB  && S3TCHeader.ddspf.dwRGBBitCount == 24)
		{
			S3TCImage->format = GL_COMPRESSED_RGB_ARB;
			depth = 24;
		}
		else
			return NULL; // unknown format
	}
#else
	return NULL;
#endif

	/* how big is it going to be including all mipmaps? */
	int faces = 1;
	if(S3TCHeader.dwCubemapFlags & DDS_CUBEMAP_ALLFACES)
		faces = 6;

	S3TCImage->bufsize = S3TCHeader.dwMipMapCount > 1 ? S3TCHeader.dwPitchOrLinearSize * factor * faces : S3TCHeader.dwPitchOrLinearSize * faces;
	if (S3TCImage->bufsize == 0)
		S3TCImage->bufsize = S3TCHeader.dwWidth * depth / 8;
	S3TCImage->pixels = (unsigned char*)malloc(S3TCImage->bufsize * sizeof(unsigned char) * S3TCHeader.dwHeight);
	stream->Read((char*)S3TCImage->pixels, S3TCImage->bufsize * S3TCHeader.dwHeight);

	Lock();
	Width = S3TCHeader.dwWidth;
	Height = S3TCHeader.dwHeight;
	Depth = depth;
	Data = S3TCImage;
	Type = Bitmap::DDS;
	Format = vixformat;
	Unlock();
	S3TCImage->components  = (S3TCHeader.ddspf.dwFourCC == FOURCC_DXT1) ? 3 : 4;
	S3TCImage->numMipmaps = S3TCHeader.dwMipMapCount;

	/* return data */
	return S3TCImage;
}


#endif	// VX_DX10
#endif	// VX_NOTEXTURE

}	// end Vixen