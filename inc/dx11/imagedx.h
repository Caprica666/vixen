#pragma once
/*
 * Helper class to construct DirectX 11 textures from memory bitmaps
 */
class DXImage
{
public:
	DXImage(Texture& image, Bitmap& bmap) : m_Image(image), m_Bitmap(bmap) { }

	ID3D11Texture2D*	CreateDDSTexture(ID3D11Device* device, bool mipmap);
	ID3D11Texture2D*	CreateTexture(ID3D11Device* device, ID3D11DeviceContext* ctx, bool mipmap, const Col4* transp);
	void*				ConvertFont(char* fontdata, int fontdepth);
	bool				LoadMipMaps(ID3D11Device* device, ID3D11DeviceContext* ctx, ID3D11Texture2D* texture);
	void				RGB2BGR(void *ptrToPixels);

protected:
	Texture&	m_Image;
	Bitmap&		m_Bitmap;
	DXGI_FORMAT	m_Format;
};

#endif