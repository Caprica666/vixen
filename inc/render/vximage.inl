#pragma once

namespace Vixen {

inline bool	Texture::HasColor() const
	{ return (GetFormat() & Bitmap::HASCOLOR) != 0; }

inline bool	Texture::HasAlpha() const
	{ return (GetFormat() & Bitmap::HASALPHA)!=0; }

inline bool	Texture::IsMipMap() const
	{ return (GetFormat() & MIPMAP)!=0; }

inline bool	Texture::IsCubeMap() const
	{ return (GetFormat() & CUBEMAP)!=0; }

inline bool	Texture::HasColorKey() const
	{ return (GetFormat() & Bitmap::HASCOLORKEY)!=0; }

inline	bool Texture::operator!=(const Texture& src) const
	{ return !(*this == src); }

inline const TCHAR* Texture::GetFileName() const
{
	if (m_FileName.IsEmpty())
		return NULL;
	return m_FileName;
}

inline Bitmap* Texture::GetBitmap()
{
	return m_Bitmap;
}

inline const Bitmap* Texture::GetBitmap() const
{
	return m_Bitmap;
}

inline int Texture::GetNumLod() const
	{ return m_NumLod; }

inline int Texture::GetWidth() const
	{ return m_Bitmap.IsNull() ? 0 : m_Bitmap->Width; }

inline int Texture::GetHeight() const
	{ return m_Bitmap.IsNull() ? 0 : m_Bitmap->Height; }

inline int Texture::GetFormat() const
	{ return m_Bitmap.IsNull() ? 0 : m_Bitmap->Format; }

inline int Texture::GetDepth() const
{
	return m_Bitmap.IsNull() ? 0 : m_Bitmap->Depth;
}

inline const Col4& Texture::GetTranspColor() const
	{ return m_TranspColor; }

} // end Vixen