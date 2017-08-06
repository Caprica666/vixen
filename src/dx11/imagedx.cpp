
/****
 *
 *  DirectX 10 Implementation of PSImage device specific functions CreateTexture and LoadDev
 *
 ****/
#include "dx11/vixendx11.h"
#include "win32/dds.h"

namespace Vixen
{
/*
 * Helper class to construct DirectX 11 textures from memory bitmaps
 */
class DXImage
{
public:
	DXImage(Texture& image) : m_Image(image), DXTexture(NULL), DXView(NULL) { m_Bitmap = image.GetBitmap(); }

	D3DTEXTURE2D*	CreateTexture(D3DDEVICE* device, D3DCONTEXT* ctx, bool mipmap, const Col4* transp);
	void*			ConvertFont(char* fontdata, int fontdepth);
	D3DSHADERVIEW*	CreateView(D3DDEVICE* device, D3DCONTEXT* ctx, bool mipmap);

	D3DTEXTURE2D*	DXTexture;
	D3DSHADERVIEW*	DXView;

protected:
	void			RGB2BGR(void *ptrToPixels);

	Texture&		m_Image;
	Bitmap*			m_Bitmap;
	DXGI_FORMAT		m_Format;
};

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

/*!
 * @fn intptr DXRenderer::UpdateTexture(Texture* texture, int texunit, bool mipmap)
 *
 * Gets the DX11 shader resource view for the given Vixen texture.
 * If the texture has already been loaded, the DevHandle member will contain
 * a reference to a DX11 shader resource view object. If not, this function
 * creates the texture resource and the shader resource view.
 *
 * @return -> shader resource view or 0 on error
 */
intptr DXRenderer::UpdateTexture(Texture* texture, int texunit, bool mipmap)
{
	Bitmap*			bmap = texture->GetBitmap();	// Image specifics (height, width...etc)
	D3DTEXTURE2D*	dxtex;
	D3DDEVICE*		device = GetDevice();

	if (bmap == NULL)
		return 0L;
	DXImage					dximage(*texture);
	ObjectLock				lock(bmap);
	DXRef<D3DSHADERVIEW>&	viewref = (DXRef<D3DSHADERVIEW>&) bmap->DevHandle;
	D3DSHADERVIEW*			view = *viewref;

	if (bmap->Type != Bitmap::TEXHANDLE)
	{
		dxtex = dximage.CreateTexture(device, m_Context, mipmap, texture->HasColorKey() ? &(texture->GetTranspColor()) : NULL);
		bmap->Kill();
		if (dxtex != NULL)
		{
			bmap->Type = Bitmap::TEXHANDLE;
			viewref = dximage.DXView;
			dxtex->Release();
			VX_TRACE(Debug, ("DXRenderer::UpdateTexture %s created texture\n", texture->GetFileName()));
		}
	}
	else if (view)
	{
		VX_TRACE2(Debug, ("DXRenderer::UpdateTexture %s selected texture\n", texture->GetFileName()));
	}
	bmap->Type = Bitmap::TEXHANDLE;
	return (intptr) view;
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
	int		size = m_Bitmap->Width * m_Bitmap->Height;
	uchar*	source = (uchar*) ptrToPixels;
	uchar	r, b, a = 255;
	int		i;
	int		stride = 3;
	int		d = m_Bitmap->Depth;

	if (d < 24)
		VX_ERROR(("DXRenderer: ERROR unsupported RGB depth %d\n", d));
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


/*
 * @fn bool DXImage::ConvertFont(char* fontdata, int depth)
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
 * selected and saved in DXRenderer::FontFormat.
 */
void* DXImage::ConvertFont(char* fontdata, int depth)
{
    char*	pDst;
	char*	newfont;
	int		y0;
	int		pitch = m_Bitmap->Width * (depth / 8);

	m_Bitmap->Format |= Bitmap::HASALPHA;
	if (depth == 8)
		return fontdata;
	pDst = newfont = (char*) malloc(m_Bitmap->Height * pitch);
	for (y0 = 0; y0 < m_Bitmap->Height; y0++)
	{
		pDst = fontdata + y0 * pitch;
		for (int x0 = 0; x0 < m_Bitmap->Width; x0++)
		{
			char a = *fontdata++;		// signed values

			if (depth == 32)
			{
				pDst[3] = a ? 255 : 0;
				pDst[0] = pDst[1] = pDst[2] = a;
				pDst += 4;
			}
			else
			{
				*((int16*) pDst) = a ? -1 : 0;
				pDst += 2;
			}
		}
	}
	return newfont;
}


D3DSHADERVIEW* DXImage::CreateView(D3DDEVICE* device, D3DCONTEXT* ctx, bool mipmap)
{
	D3DSHADERVIEW* view;
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	HRESULT hr;

	desc.Format = m_Format;
	desc.Texture2D.MostDetailedMip = 0;
	if (mipmap < 0)
		desc.Texture2D.MipLevels = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	else
		desc.Texture2D.MipLevels = 1;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	hr = device->CreateShaderResourceView(DXTexture, &desc, &view);
	if (FAILED(hr))
	{ VX_ERROR(("DXRenderer::UpdateTexture cannot generate mipmaps\n"), view); }
	return view;
}

D3DTEXTURE2D* DXImage::CreateTexture(D3DDEVICE* device, D3DCONTEXT* ctx, bool mipmap, const Col4* transp)
{
	D3D11_TEXTURE2D_DESC	desc;
	D3D11_SUBRESOURCE_DATA	srd;
	int32					bmap_format = m_Bitmap->Format;
	void*					pixels = m_Bitmap->Data;
	void*					newpixels = m_Bitmap->Data;
	HRESULT					hr;
	TexFont*				font;

	ZeroMemory(&desc, sizeof(desc));
	desc.Width = m_Image.GetWidth();
	desc.Height = m_Image.GetHeight();
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (m_Image.GetFormat() & Texture::RENDERTARGET)
		if (bmap_format & Bitmap::HASCOLOR)
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		else
			desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	if (bmap_format & Texture::CUBEMAP)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
	if (mipmap)
	{
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	/*
	 * Preprocess the pixels before giving them to DirectX
	 */
	srd.pSysMem = NULL;
	srd.SysMemPitch = desc.Width * 4;
	switch (m_Bitmap->Type)
	{
		case Bitmap::FONT:
		font = (TexFont*) m_Bitmap->Data;
		VX_ASSERT(m_Bitmap->Depth == 8);
		pixels = font->teximage;	// 8 bit luminance/alpha font bitmap
		if (pixels == NULL)
			return NULL;
		newpixels = ConvertFont((char*) pixels, Bitmap::FontDepth);
		break;

		case Bitmap::DIB:
		pixels = (BYTE*) (((BITMAPINFOHEADER*) pixels) + 1);
		RGB2BGR(pixels);
		break;

		case Bitmap::GLBITMAP:
		break;

		case Bitmap::DXBITMAP:
		RGB2BGR(pixels);
		break;

		default:
		VX_ERROR(("DXRenderer::CreateTexture unsupported image type %d for %s\n", m_Bitmap->Type, m_Image.GetFileName()), NULL);
	}
	/*
	 * establish the DX format based on image format and bit depth
	 */
	switch (m_Image.GetDepth())
	{
		case 8:
		m_Format = DXGI_FORMAT_R8_UNORM;
		srd.SysMemPitch = desc.Width;
		break;

		case 16:
		if (bmap_format & Bitmap::HASCOLOR)
			if (bmap_format & (Bitmap::HASALPHA | Bitmap::HASCOLORKEY))
				m_Format = DXGI_FORMAT_B5G5R5A1_UNORM;
			else
				m_Format = DXGI_FORMAT_B5G6R5_UNORM;
		else
			m_Format = DXGI_FORMAT_R16_UNORM;
		srd.SysMemPitch = desc.Width * 2;
		break;

		case 24:
		if (bmap_format & Bitmap::HASCOLORKEY)
			newpixels = m_Bitmap->MakeAlpha(pixels, transp);
		else
			newpixels = m_Bitmap->MakeAlpha(pixels, NULL);
		break;
	}
	/*
	 * Make the DirectX texture from the bitmap data and
	 * make the shader resource view for the texture
	 */
	desc.Format = m_Format;
	srd.pSysMem = newpixels;
	hr = device->CreateTexture2D(&desc, &srd, &DXTexture);
	if (SUCCEEDED(hr))
	{
		DXView = CreateView(device, ctx, mipmap);
		if (mipmap)
		{
			ctx->GenerateMips(DXView);
			m_Bitmap->Format |= Texture::MIPMAP;
			m_Image.SetFormat(m_Bitmap->Format);
		}
	}
	if (newpixels != pixels)			// made a temporary bitmap?
		free((void*) newpixels);		// free it now
	if (FAILED(hr))
	{
		m_Bitmap->Kill();
		m_Bitmap->Type = Bitmap::NOTEXTURE;
		VX_ERROR(("DXRenderer::UpdateTexture failed for %s\n", m_Image.GetFileName()), NULL);
	}
	m_Bitmap->Kill();
	m_Bitmap->Type = Bitmap::TEXHANDLE;
	return DXTexture;
}

}	// end Vixen