/*
 * SWIG interface file for Bitmap
 */
namespace Vixen
{
	%ignore Bitmap::Load;
	%ignore Bitmap::ReadFile;
	%ignore Bitmap::ReadFont;
	%ignore Bitmap::ReadBitmap;
	%ignore Bitmap::ReadDDS;
	%ignore Bitmap::WriteBMP;
	%ignore Bitmap::GLImageFromDIB;
	%ignore Bitmap::DXImageFromDIB;
	%ignore Bitmap::MakeAlpha;
	%ignore Bitmap::Make16Bit;
	%ignore Bitmap::Startup;
	%ignore Bitmap::Shutdown;
	%ignore Bitmap::DevHandle;
	%ignore Bitmap::FontDepth;
	%ignore Bitmap::RGBDepth;
	%ignore Bitmap::RGBADepth;

	%ignore Texture::GetBitmap;
	%ignore Texture::LoadDev;
	%ignore Texture::CreateTexture;
	%ignore Texture::CreateDDSTexture;
	%ignore Texture::ReadFile;
	%ignore Texture::s_MaxTexID;
	%ignore Texture::s_CurTexID;
	%ignore Texture::SetTranspColor(const Col4*);
	%rename(NotEqual)	Texture::operator!=;
	%rename(Equal)		Texture::operator==;
	%apply	SharedObj*	{ Texture* };
	%apply	SharedObj*	{ Bitmap* };
}

ATTR_DEFINE(Vixen::Texture, int, Width);
ATTR_DEFINE(Vixen::Texture, int, Height);
ATTR_DEFINE(Vixen::Texture, int, Depth);
ATTR_DEFINE(Vixen::Texture, int, NumLod);
ATTR_DEFINE(Vixen::Texture, int, Format);
ATTR_DEFINE_NEW(Vixen::Texture, Vixen::Col4, TranspColor);
ATTR_DEFINE(Vixen::Texture, Vixen::Bitmap, Bitmap);
ATTR_DEFINE(Vixen::Texture, const char*, FileName);

%include "base/vxbitmap.h"
%include "render/vximage.h"
