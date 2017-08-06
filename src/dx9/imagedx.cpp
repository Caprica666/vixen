
/****
 *
 *  Implements Image attributes and api.
 *	This is a generic implementation that doesn't really
 *	do anything except keep track of the attributes and file name.
 *
 ****/
#include "vixen.h"
#include "dx9/renderdx.h"
#include "render/texfont.h"
#include "win32/dds.h"


namespace Vixen {

class DXImage
{
public:
	DXImage(Bitmap& bmap, DXRenderer& render)
	  :	m_Bitmap(bmap), m_Render(render) { NumLod = 0; }

	bool	ConvertFont(D3DTEXTURE* Texture, BYTE* pixels, D3DFORMAT fmt);
	bool	CopyTexture(D3DTEXTURE*, const void* pSrc, int newdepth, const Col4* transp);
	void*	CreateDDSTexture(bool mipmap, const Col4* transp); 
	void*	CreateTexture(bool mipmap, const Col4* transp); 
	void	RGB2BGR(void *ptrToPixels);
#ifdef DX_NORMAL_MAP
	bool	CreateNormalMap(D3DTEXTURE* Texture);
	HRESULT	FillNormalsOfSurface(LPD3DSURFACE pFace, DWORD side);
	LPD3DCUBETEXTURE	MakeNormalizedCubeMap(D3DDEVICE);
#endif

	int			NumLod;

protected:
	Bitmap&		m_Bitmap;
	DXRenderer&	m_Render;
};

/*!
 * @fn bool DXRenderer::UpdateTexture(Texture* texture, int texunit, bool mipmap) 
 * @param textire	Texture object to update
 * @param textunit	texture unit (aka texture stage) the image is associated with
 * @param mipmap	true if mip-mapping is enabled
 *
 * Callback used during rendering to load a texture into
 * the display device.
 *
 * @return \b true if image successfully loaded, else \b false
 *
 * @see Texture::Load Texture::CreateTexture
 */
void* DXRenderer::UpdateTexture(Texture* texture, int texunit, bool mipmap) 
{
/*
 * The cached data in the image is invalid. We look up the bitmap in
 * the shared image dictionary. This dictionary keeps the DIB handle and
 * the device texture handles for each scene using the texture.
 */
	Bitmap*	bmap = texture->GetBitmap();

	if (bmap == NULL)
		return NULL;

	DXRef<D3DTEXTURE>& texref = (DXRef<D3DTEXTURE>&) bmap->DevHandle;
	D3DTEXTURE* texhandle = *texref;
/*
 * Image or its bitmap has changed and needs to be loaded into device.
 * We don't lock unless we are going to create a texture and update the bitmap.
 */
	if ((HasChanged() || bmap->HasChanged() || !texhandle) && bmap->Data)
	{
		ObjectLock lock(bmap);
		DXImage		dximage(*bmap, *this);

		if (bmap->Data == NULL)		// in case it changed before lock
			return NULL;			// we'll come back next frame
		if (bmap->Format & Bitmap::HASCOLORKEY)
			texhandle = (D3DTEXTURE*) dximage.CreateTexture(mipmap, &(texture->GetTranspColor()));
		else
			texhandle = (D3DTEXTURE*) dximage.CreateTexture(mipmap, NULL);
		texture->SetNumLod(dximage.NumLod);
		bmap->SetChanged(false);
		texture->SetChanged(false);
		if (texhandle == 0)
			VX_ERROR(("Image::CreateTexture failed for %s", texture->GetFileName()), NULL);
		VX_TRACE(Texture::Debug || Appearance::Debug, ("<image_create stage='%d' file='%s'/>", texunit, texture->GetFileName(), this));
		texref = texhandle;
	}
	return texhandle;
}

/*!
 * @fn void* DXImage::CreateTexture(bool mipmap, const Col4* transp)
 * @param transp	pointer to the transparent color for this texture.
 *					if NULL, texture does not use chroma-key
 *
 * Callback used during rendering to create a device-dependent
 * texture suitable for loading. It is assumed that the caller has done
 * the necessary locking of the image and it's associated bitmap.
 *
 * @return \b true if successful, else \b false
 *
 * @see Texture::Load Texture::LoadDev Texture::CreateDDSTexture
 */
void* DXImage::CreateTexture(bool mipmap, const Col4* transp)
{ 
#if VX_NOTEXTURE
	return false;
#endif
	TexFont*	font = NULL;
	D3DFORMAT	format = D3DFMT_A8R8G8B8;
	BYTE*		pixels;
	HRESULT		hr;
	DWORD		usage = 0;
	D3DTEXTURE* d3dtex = NULL;
	int			texdepth;
	D3DDEVICE*	d3d = m_Render.GetDevice();

	pixels = (uchar*) m_Bitmap.Data;
	texdepth = m_Bitmap.Depth;

	switch (texdepth)
	{
		case 32:
		VX_ASSERT(Bitmap::RGBADepth == 32);
		if (m_Bitmap.Format & Texture::RENDERTARGET)
			usage = D3DUSAGE_RENDERTARGET;
		else if (!(m_Bitmap.Format & Bitmap::HASCOLOR))
			format = D3DFMT_D32;	// note - DX chokies on this
		break;

		case 8:
		format = D3DFMT_A4L4;
		break;

		case 24:
		VX_ASSERT(Bitmap::RGBDepth >= 24);
		if ((Bitmap::RGBDepth == 32) || (m_Bitmap.Format & Bitmap::HASCOLORKEY))
			texdepth = 32;
		else if (m_Bitmap.Format & Bitmap::HASCOLOR)
			format = D3DFMT_R8G8B8;
		break;

		case 16:
		if (m_Bitmap.Format & Bitmap::HASCOLOR)
			if (m_Bitmap.Format & (Bitmap::HASALPHA | Bitmap::HASCOLORKEY))
				format = D3DFMT_A1R5G5B5;
			else
				format = D3DFMT_R5G6B5;
		else
		{
			format = D3DFMT_D16;
			if (usage == D3DUSAGE_RENDERTARGET)
				usage |= D3DUSAGE_DEPTHSTENCIL;
		}
		break;
	}
	switch (m_Bitmap.Type)
	{
		case Bitmap::FONT:
		font = (TexFont*) m_Bitmap.Data;
		VX_ASSERT(m_Bitmap.Depth == 8);
		pixels = font->teximage;	// 8 bit luminance/alpha font bitmap
		if (pixels == NULL)
			goto ErrorExit;
		format = m_Render.FontFormat;
		hr = d3d->CreateTexture(m_Bitmap.Width, m_Bitmap.Height, 1, 0, format, D3DPOOL_MANAGED, &d3dtex, NULL);
		if (FAILED(hr))
			goto ErrorExit; 
		ConvertFont(d3dtex, pixels, format);
		return d3dtex;

		case Bitmap::DIB:
		pixels = (BYTE*) (((BITMAPINFOHEADER*) pixels) + 1);
		break;

		case Bitmap::GLBITMAP:
		RGB2BGR(pixels);
		break;

		case Bitmap::DXBITMAP:
		break;

		case Bitmap::DDS:				// have S3TC handle?
		return CreateDDSTexture(mipmap, transp);

		case Bitmap::TEXHANDLE:
		if (m_Bitmap.DevHandle)
		{
			DXRef<D3DTEXTURE>* tmp = (DXRef<D3DTEXTURE>*) m_Bitmap.DevHandle;
			d3dtex = *tmp;
			return d3dtex;
		}
		//	fall thru

		default:
		return NULL;
	}
	if (mipmap)
	{
		m_Bitmap.Format |= Texture::MIPMAP;
		usage = D3DUSAGE_AUTOGENMIPMAP;
		NumLod = 0;
	}
	else
		NumLod = 1;
	hr = d3d->CreateTexture(m_Bitmap.Width, m_Bitmap.Height, NumLod, usage, format, D3DPOOL_MANAGED, &d3dtex, NULL);
	if (FAILED(hr))
		goto ErrorExit; 
	CopyTexture(d3dtex, pixels, texdepth, transp);	

	m_Bitmap.Kill();
	m_Bitmap.Type = Bitmap::TEXHANDLE;
	return d3dtex;

ErrorExit:
	m_Bitmap.Kill();
	m_Bitmap.Type = Bitmap::NOTEXTURE;
	return NULL;
}

void* DXImage::CreateDDSTexture(bool mipmap, const Col4* transp) 
{
	HRESULT				hr;
	D3DTEXTURE*			dxtex;
	S3TCGenericImage*	S3TCImage = (S3TCGenericImage*) m_Bitmap.Data;
	DWORD				usage = 0;
	D3DDEVICE*			d3d = m_Render.GetDevice();
	
	if (S3TCImage->numMipmaps == 0)
		S3TCImage->numMipmaps = 1;
	int numMipmaps = S3TCImage->numMipmaps;
	if (mipmap)
	{
		m_Bitmap.Format |= Texture::MIPMAP;
		usage |= D3DUSAGE_AUTOGENMIPMAP;
		NumLod = 0;
	}
	if (m_Bitmap.Format & Texture::CUBEMAP)
	{
		// Cube Map
		if (FAILED(hr = d3d->CreateCubeTexture(m_Bitmap.Width, numMipmaps, usage,
								(D3DFORMAT) S3TCImage->format, D3DPOOL_MANAGED, (IDirect3DCubeTexture9**) &dxtex, NULL)))
		return FALSE;
	}
	else
	{
		if (FAILED(hr = d3d->CreateTexture(m_Bitmap.Width, m_Bitmap.Height, numMipmaps, usage,
								(D3DFORMAT) S3TCImage->format, D3DPOOL_MANAGED, (D3DTEXTURE**) &dxtex, NULL)))
			return FALSE;
	}
    if (dxtex)
		return dxtex;
    return NULL;
}

/*
 * @fn void DXImage::RGB2BGR(void* ptrToPixels)
 *
 * Converts an RGB image to a BGR by swapping the red and blue
 * components of each pixel. This is useful for converting OpenGL
 * format images to DirectX format. If the image has an alpha
 * channel, it is left untouched.
 */
void DXImage::RGB2BGR(void* ptrToPixels)
{
	int		size = m_Bitmap.Width * m_Bitmap.Height;
	uchar*	source = (uchar*) ptrToPixels;
	uchar	r, b, a = 255;
	int		i;
	int		stride = 3;
	int		d = m_Bitmap.Depth;

	if (d < 24)
		VX_ERROR_RETURN(("DXImage: ERROR unsupported RGB depth %d", d));
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

bool DXImage::CopyTexture(D3DTEXTURE* d3dtex, const void* pixels, int newdepth, const Col4* transp)
{
    D3DLOCKED_RECT d3dlr;
	if (FAILED(d3dtex->LockRect( 0, &d3dlr, 0, 0 )))
		return false;

	int		dwDstPitch	= (DWORD) d3dlr.Pitch;
    BYTE*	pDst		= (BYTE*) d3dlr.pBits;
	BYTE*	pSrc		= (BYTE*) pixels;
	int		dstbytes	= newdepth / 8;
	int		srcbytes	= m_Bitmap.Depth / 8;
	int		stride		= m_Bitmap.Width * dstbytes;
	int		y0;

	if (m_Bitmap.Depth == newdepth)				// source and dest same depth?
		if (stride == d3dlr.Pitch)				// are exactly same layout?
			memcpy(pDst, pSrc, stride * m_Bitmap.Height);
		else									// scanline padding different
			for (y0 = 0; y0 < m_Bitmap.Height; y0++)
			{
				pDst = (BYTE*) d3dlr.pBits + y0*d3dlr.Pitch;
				memcpy(pDst, pSrc, stride);
				pSrc += stride;
			}
	else if ((m_Bitmap.Depth > newdepth) && (newdepth == 16))	// downsize from 32, 24 to 16
		Bitmap::Make16Bit(pixels, pDst, m_Bitmap.Width, m_Bitmap.Height, m_Bitmap.Depth, stride);
	else										// upsizing from 24 to 32
	{
		for (y0 = 0; y0 < m_Bitmap.Height; y0++)
		{
			pDst = (BYTE*)	d3dlr.pBits + y0*d3dlr.Pitch;
			for (int x0 = 0; x0 < m_Bitmap.Width; x0++)
			{
				*((int32*) pDst) = 0;
				for (int i = 0; i < srcbytes; ++i)
					pDst[i] = *pSrc++;

				pDst += dstbytes;				
			}
		}
		m_Bitmap.Depth = newdepth;
	}
	if (transp)
		m_Bitmap.MakeAlpha(d3dlr.pBits, transp);
	d3dtex->UnlockRect(0);
	return true;
}

/*
 * @fn bool Texture::ConvertFont(LPDIRECTTEXTURE8* d3dtex, BYTE* pixels)
 * @param d3dtex	Direct3D destination texture handle
 * @param pixels	pointer to font pixels to copy
 * @param d3dfmt	D3DFORMAT descriptor (D3DFMT_A8R8G8B8, D3DFMT_A4R4G4B4, D3DFMT_A1R5G5B5)
 *
 * Converts a font image into a Direct3D texture.
 * Fonts are 8 bit luminance textures with color key alpha.
 * Most of the fonts we have actually are 1 bit
 * because they set luminance to either 0 or 255.
 * Many Direct3D device drivers do not support this format but almost
 * all of them take either 16 or 32 bit RGBA. This function converts
 * the 8 bit font format into one of several D3D formats.
 *
 * At the time the D3D device is initialized, the supported font format is
 * selected and saved in DXScene::FontFormat.
 */
bool DXImage::ConvertFont(D3DTEXTURE* d3dtex, BYTE* pixels, D3DFORMAT fmt)
{
    D3DLOCKED_RECT d3dlr;
	if (FAILED(d3dtex->LockRect(0, &d3dlr, 0, 0 )))
		return false;

	int		dwDstPitch	= (DWORD) d3dlr.Pitch;
    BYTE*	pDst		= (BYTE*) d3dlr.pBits;
	int		depth;

	switch (fmt)
	{
		case D3DFMT_A8R8G8B8: depth = 32; break;
		case D3DFMT_A4L4: depth = 8; break;
		default: depth = 16; break;
	}
	m_Bitmap.Format |= Bitmap::HASALPHA;
	if (depth == 8)
	{
		d3dtex->UnlockRect(0);
		return true;
	}
	for (int y0 = 0; y0 < m_Bitmap.Height; y0++)
	{
		pDst = (BYTE*) d3dlr.pBits + y0 * d3dlr.Pitch;
		for (int x0 = 0; x0 < m_Bitmap.Width; x0++)
		{
			BYTE	a = *pixels;		// signed values
			#pragma omp atomic
			pixels++;

			switch (fmt)
			{
				case D3DFMT_A8R8G8B8:	// 32 bit RGBA, 8 bits each
				pDst[3] = a ? 255 : 0;
				pDst[0] = pDst[1] = pDst[2] = a;
				pDst += 4;
				break;

				case D3DFMT_A8L8:		// 16 bit format, 8 bits luminance, 8 bits alpha
				case D3DFMT_A1R5G5B5:	// 16 bit RGB 5 bits each, alpha 1 bit
				case D3DFMT_A4R4G4B4:	// 16 bit RGBA, 4 bits each
				*((int16*) pDst) = a ? -1 : 0;
				pDst += 2;
				break;
			}
		}
	}
	d3dtex->UnlockRect(0);
	return true;
}

#ifdef DX_NORMAL_MAP
bool DXImage::CreateNormalMap(LPD3DTEXTURE d3dtex)
{
    LPD3DSURFACE psurf;
    D3DSURFACE_DESC sd;
    D3DLOCKED_RECT  d3dlr;
	HRESULT hr;
	int iLevel = 0;
	while(1)
	{
		hr = d3dtex->GetSurfaceLevel( iLevel, &psurf );
		if (FAILED(hr))
			break;
		psurf->GetDesc(&sd);
		d3dtex->LockRect( iLevel, &d3dlr, 0, 0 );
		DWORD* pPixel = (DWORD*)d3dlr.pBits;

		// For each pixel, generate a vector normal that represents the change
		// in thea height field at that pixel
		for( DWORD j=0; j<sd.Height; j++  )
		{
			for( DWORD i=0; i < sd.Width; i++ )
			{
				DWORD color00 = pPixel[0];
				DWORD color10 = pPixel[1];
				DWORD color01 = pPixel[d3dlr.Pitch/sizeof(DWORD)];

				FLOAT	fHeight00 = (FLOAT) ((color00 & 0x00ff0000) >> 16) /255.0f;
				FLOAT	fHeight10 = (FLOAT) ((color10 & 0x00ff0000) >> 16) /255.0f;
				FLOAT	fHeight01 = (FLOAT) ((color01 & 0x00ff0000) >> 16) /255.0f;

				Vec3	vPoint00(i + 0.0f, j + 0.0f, fHeight00);
				Vec3	vPoint10(i + 1.0f, j + 0.0f, fHeight10);
				Vec3	vPoint01(i + 0.0f, j + 1.0f, fHeight01);
				Vec3	v10 = vPoint10 - vPoint00;
				Vec3	v01 = vPoint01 - vPoint00;
				Vec3	vNormal = v10.Cross(v01);

				// Store the normal as an RGBA value in the normal map
				n.Normalize();
				DWORD r = (DWORD)( 127.0f * n.x + 128.0f );
				DWORD g = (DWORD)( 127.0f * n.y + 128.0f );
				DWORD b = (DWORD)( 127.0f * n.z + 128.0f );
				DWORD a = (DWORD)( 255.0f * fHeight00 );  
				*bl++ = (unsigned long) ((a<<24L) + (r<<16L) + (g<<8L) + (b<<0L));
			}
		}

		// Unlock the texture and return successful
		d3dtex->UnlockRect(0);
		iLevel++;
	}

    return S_OK;
}



#define FACE_POSX D3DCUBEMAP_FACE_POSITIVE_X
#define FACE_NEGX D3DCUBEMAP_FACE_NEGATIVE_X
#define FACE_POSY D3DCUBEMAP_FACE_POSITIVE_Y 
#define FACE_NEGY D3DCUBEMAP_FACE_NEGATIVE_Y
#define FACE_POSZ D3DCUBEMAP_FACE_POSITIVE_Z 
#define FACE_NEGZ D3DCUBEMAP_FACE_NEGATIVE_Z

LPD3DCUBETEXTURE DXImage::MakeNormalizedCubeMap(D3DDEVICE* d3d) 
{
    HRESULT hr;
    DWORD iLevel;
    LPD3DSURFACE pFace;
	LPD3DCUBETEXTURE ptexCube;


	DWORD FacesArray[] = 
	{
		FACE_POSX,
		FACE_NEGX,
		FACE_POSY, 
		FACE_NEGY,
		FACE_POSZ, 
		FACE_NEGZ
	};

	int NumFaces = 6;
    
    hr = D3DXCreateCubeTexture(d3d, m_Bitmap.Width, m_Texture.GetNumLod(), 0,
								D3DFMT_R8G8B8, D3DPOOL_MANAGED, &ptexCube);
	bool done = false;
	iLevel = 0;
	while(!done)
	{		
		for(int face = 0; face < NumFaces; face++)
		{
			if(FAILED(hr = ptexCube->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 
				iLevel, &pFace)))
			{
				done = true;
				break;
			}
			FillNormalsOfSurface( pFace, FacesArray[face] );
			if (pFace)
			{
				pFace->Release();
				pFace = NULL;
			}
		}
		iLevel++;
	}
	return ptexCube;
}

HRESULT  DXImage::FillNormalsOfSurface(LPD3DSURFACE pFace, DWORD side )
{
	HRESULT hr;

	float acolor = 0;
	D3DLOCKED_RECT LockedRect;
    float w,h;

	hr = pFace->LockRect(&LockedRect, NULL , NULL );
	DWORD width = LockedRect.Pitch / (m_Bitmap.Depth / 8);
	DWORD height = m_Bitmap.Height / (m_Bitmap.Width / width);

	for (DWORD j = 0; j < width; j += 1)
	{
		h = (float)j/ (float)(height-1);	// 0 to 1
		h *= 2.0f;							// 0 2
		h -= 1.0;							// -1 to 1
		
		unsigned long* bl = (unsigned long*)(((char*)LockedRect.pBits) +
			LockedRect.Pitch * j);
		
		for (DWORD i = 0; i < width; i += 1)
		{
			Vec3 n;
			w = (float)i / (float)(width-1);   // 0 to 1
			w *= 2.0;                          // 0 to 2
			w -= 1.0;                          // -1 to 1

			switch(side)
			{
				case FACE_POSX:					// +x
				n[0] = 1.0;
				n[1] = -h;
				n[2] = -w;
				break;
				
				case FACE_NEGX:					// -x
				n[0] = -1.0;
				n[1] = -h;
				n[2] = w;
				break;
				
				case FACE_POSY:					// y
				n[0] = w;
				n[1] = 1.0;
				n[2] = h;
				break;
				
				case FACE_NEGY:					// -y
				n[0] = w;
				n[1] = -1.0;
				n[2] = -h;
				break;
				
				case FACE_POSZ:					// +z
				n[0] = w;
				n[1] = -h;
				n[2] = 1.0;
				break;
				
				case FACE_NEGZ:					// -z
				n[0] = -w;
				n[1] = -h;
				n[2] = -1.0;
				break;
				
			}			
			n.Normalize();
			*bl++ = (unsigned long) acolor;
		}
	}	
	hr = pFace->UnlockRect();	
	return hr;
}
#endif

}	// end Vixen