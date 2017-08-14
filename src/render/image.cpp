/****
 *
 *  Implements Image attributes and api.
 *	This is a generic implementation that doesn't really
 *	do anything except keep track of the attributes and file name.
 *	It implements common code shared across different ports of the scene manager.
 *
 ****/
#include "vixen.h"
#include "render/texfont.h"

namespace Vixen
{

VX_IMPLEMENT_CLASSID(Texture, SharedObj, VX_Image);

bool			Texture::DoTexturing = true;
static const TCHAR* opnames[] =
	{	TEXT("SetWidth"), TEXT("SetHeight"), TEXT("SetDepth"), TEXT("Load"), TEXT("SetTranspColor"),
		TEXT("SetFormat"), TEXT("SetBitmap"), TEXT("Touch"), TEXT("SetFileName"),
		TEXT("SetDeviceDepth"), TEXT("UpdateData") };
const TCHAR** Texture::DoNames = opnames;

Texture::Texture(const TCHAR* filename)
{
	DevHandle = 0;
	m_NumLod = 1;
	m_TranspColor = Col4(0,0,0);
	if (filename)
		Load(filename);
}

bool Texture::Copy(const SharedObj* obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(obj);
	if (!SharedObj::Copy(obj))
		return false;
	const Texture*	src = (const Texture*) obj;
	DevHandle = 0;
	*this = *src;
//	don't copy texture pointer
	return true;
}

Texture::~Texture()
{
}

DebugOut& Texture::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	const TCHAR* filename = GetFileName();
	int	width = GetWidth();
	int height = GetHeight();
	int depth = GetDepth();

	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	if (filename)
		endl(dbg << "\t<attr name='FileName'>" << filename << "</attr>");
	if (width > 0)
		endl(dbg << "\t<attr name='Width'>" << width << "</attr>");
	if (height > 0)
		endl(dbg << "\t<attr name='Height'>" << height << "</Heigattrht>");
	if (depth > 0)
		endl(dbg << "\t<attr name='Depth'>" << depth << "</attr>");
	if (HasColorKey())
		endl(dbg << "\t<attr name='TranspColor " << GetTranspColor() << "</attr>");
	SharedObj::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

void Texture::SetNumLod(int lod)
{
	m_NumLod = (short) lod;
}

void Texture::SetDepth(int n)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Image, IMAGE_SetDepth) << this << int32(n);
	VX_STREAM_END()

	Bitmap* bmap = m_Bitmap;

	if (bmap == NULL)
	{
		bmap = new Bitmap;
		bmap->Depth = n;
		m_Bitmap = bmap;
		return;
	}

	ObjectLock lock(bmap);
	bmap->Depth = n;
}

int Texture::BytesPerPixel() const
{
	if (m_Bitmap.IsNull())
		return 0;
	switch (m_Bitmap->Depth)
	{
		case 32: return 4;
		case 24: return 3;
		case 16: return 2;
		default: return 1;
	}
}

void Texture::SetWidth(int n)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Image, IMAGE_SetWidth) << this << int32(n);
	VX_STREAM_END()

	Bitmap* bmap = m_Bitmap;

	if (bmap == NULL)
	{
		bmap = new Bitmap;
		bmap->Width = n;
		m_Bitmap = bmap;
		return;
	}

	ObjectLock lock(bmap);
	bmap->Width = n;
}

void Texture::SetHeight(int n)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Image, IMAGE_SetHeight) << this << int32(n);
	VX_STREAM_END()

	Bitmap* bmap = m_Bitmap;

	if (bmap == NULL)
	{
		bmap = new Bitmap;
		bmap->Height = n;
		m_Bitmap = bmap;
		return;
	}

	ObjectLock lock(bmap);
	bmap->Height = n;
}

/*!
 * @fn void Texture::SetFormat(int format)
 *
 * The internal format of the image is expressed in terms of the
 * following bit flags:
 * @code
 *	Bitmap::HASALPHA		texture has alpha channel
 *	Bitmap::HASCOLOR		texture has color channel
 *	Bitmap::MIPMAP		texture is mip-mapped
 *	VXBItmap::HASCOLORKEY	texture has transparent color
 * @endcode
 *
 * @see Texture::HasAlpha Texture::HasColor Texture::IsMipMap Texture::SetTranspColor
 */
void Texture::SetFormat(int format)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Image, IMAGE_SetFormat) << this << int32(format);
	VX_STREAM_END()

	Bitmap* bmap = m_Bitmap;

	if (bmap == NULL)
	{
		bmap = new Bitmap;
		bmap->Format = format;
		m_Bitmap = bmap;
		return;
	}

	ObjectLock lock(bmap);
	bmap->Format = format;
}

/*!
 * @fn bool Texture::operator==(const Texture& src) const
 * Two images are considered equal if their width, height, depth and
 * filenames match. The pixels are \b not compared for equality.
 */
bool Texture::operator==(const Texture& src) const
{
	if (m_FileName.IsEmpty())
	{
		if (!src.m_FileName.IsEmpty())
			return false;
	}
	else if (!src.m_FileName.IsEmpty())
		return false;
	else if (m_FileName.CompareNoCase(src.m_FileName) != 0)
		return false;
	Bitmap* dstmap = m_Bitmap;
	Bitmap* srcmap = src.m_Bitmap;

	if (dstmap == srcmap)
		return true;
	if ((dstmap == NULL) || (srcmap == NULL))
		return false;
	if (dstmap->Width != srcmap->Width)
		return false;
	if (dstmap->Height != srcmap->Height)
		return false;
	if (dstmap->Depth != srcmap->Depth)
		return false;
	return true;
}

/*!
 * @fn void Texture::SetTranspColor(const Col4& c)
 * @param c	Color key for transparency. If NULL, the image is assumed
 *			to have no color key and the transparency color is not used.
 *
 * The transparency color is used to control which parts of
 * the image are drawn. Pixels that are this color are not drawn,
 * making the image appear completely transparent in these areas.
 * Internally, the system constructs an alpha channel for the image
 * to mask out those pixels which are the transparent color.
 *
 * @see Appearance::Set Texture::HasColorKey Bitmap::MakeAlpha Texture::SetFormat
 *
 */
void Texture::SetTranspColor(const Col4& c)
{
	SetTranspColor(&c);
}

void Texture::SetTranspColor(const Col4* c)
{
	VX_STREAM_BEGIN(s)
		if (c)
			*s << OP(VX_Image, IMAGE_SetTranspColor) << this << c->r << c->g << c->b;
	VX_STREAM_END()

	Bitmap* bmap = m_Bitmap;

	if (c)
		m_TranspColor = *c;
	if (bmap == NULL)
	{
		bmap = new Bitmap;
		bmap->Format = Bitmap::HASCOLORKEY;
		m_Bitmap = bmap;
		return;
	}

	ObjectLock lock(bmap);
	if (c)
		bmap->Format |= Bitmap::HASCOLORKEY;
	else
		bmap->Format &= ~Bitmap::HASCOLORKEY;
}

/****
 *
 * class Image override for SharedObj::Do
 *		IMAGE_SetWidth		<int32>
 *		IMAGE_SetHeight		<int32>
 *		IMAGE_SetDepth		<int32>
 *		IMAGE_Load			<char* filename>
 *		IMAGE_SetTranspColor <Col4>
 *
 ****/
bool Texture::Do(Messenger& s, int op)
{
	int32		ofs, n, size;
	Col4		c;
	TCHAR		fname[VX_MaxPath];
	char*		imagedata;
	Bitmap*		bmap = m_Bitmap;
	Core::String path;
	Core::String dir;

	switch (op)
	{
		case IMAGE_SetWidth:
		s >> n;
		SetWidth(n);
		break;

		case IMAGE_SetHeight:
		s >> n;
		SetHeight(n);
		break;

		case IMAGE_SetDepth:
		s >> n;
		SetDepth(n);
		break;

		case IMAGE_SetFormat:
		s >> n;
		SetFormat(n);
		break;

		case IMAGE_SetTranspColor:
		s >> c;
		SetTranspColor(c);
		break;

/*
 * When loading a texture from a stream, we prepend
 * the directory of the file that was opened.
 */
		case IMAGE_Load:
		s >> fname;
		path = fname;
		dir = (s.GetInStream())->GetDirectory();
		if (Core::Stream::IsRelativePath(fname) && !Core::Stream::IsRelativePath(dir))
		{
			path = dir;
			path += '/';
			path += fname;
		}
		Load(path);
		break;

		case IMAGE_UpdateData:
		s >> ofs >> n;
		if (bmap == NULL)
			break;
		size = bmap->Width * bmap->Height * BytesPerPixel();
		imagedata = (char*) bmap->Data;
		if (imagedata == NULL)
			imagedata = (char*) malloc(size);
		SetBitmap(bmap->Type, imagedata);
		VX_ASSERT(ofs + n <= size);
		s.Write((char*) imagedata + ofs, n);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Texture::DoNames[op - IMAGE_SetWidth]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Image override for SharedObj::Save
 *
 ****/
int Texture::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	if (h > 0)
	{
		const TCHAR* name = GetFileName();

		if (HasColorKey())
			s << OP(VX_Image, IMAGE_SetTranspColor) << h << GetTranspColor();
		if (name && *name)
			s << OP(VX_Image, IMAGE_Load) << h << name;
	}
	return h;
}


/*!
 * @fn bool Texture::Load(const TCHAR* filename)
 * @param filename	path name of bitmap file to load
 *					if NULL, the image filenaem is used
 *
 * Loads a texture from a file. File formats supported are
 * .BMP, .PNG, .TGA .JPG.
 *
 * If the image is loaded successfully, a load image event
 * (Event::LOAD_IMAGE) is generated giving the name of the bitmap file
 * and this image as the sender.
 *
 * If asynchronous file loading is enabled on the current file
 * loader, the image will be loaded by worker threads and the
 * current thread will not block. Otherwise, a blocking read is
 * issued directly and the function will return when it has
 * read and converted the entire file.
 *
 * @return \b true if image successfully loaded or asynchronous conversion requested, else \b false
 *
 * @see Texture::SetFileName World::LoadAsync Bitmap::Load
 */
bool Texture::Load(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Image, IMAGE_Load) << this << filename;
	VX_STREAM_END()

	if (filename == NULL)
	{
		filename = m_FileName;
		if (filename == NULL)
			return false;
	}
	if (!DoTexturing)
		return false;
	MakeLock();
	GetMessenger()->Observe(this, Event::LOAD_IMAGE, NULL);
	SetFileName(filename);
#ifdef VX_NOTEXTURE
	return true;
#else
	return World3D::Get()->LoadAsync(filename, this);
#endif
}

/*!
 * @fn void* Texture::GetBitmap(int* type) const
 * @param type	If not NULL, store type of image handle upon return
 *
 * Several types of bitmap pointers may be returned.
 * @code
 *	Bitmap::DIB		pointer to a DIB
 *	Bitmap::DDS		handle to DDS (S3TC) format texture
 *	Bitmap::TEXHANDLE	device-dependent texture handle
 *	Bitmap::GLBITMAP	texel data pointer RGB/RGBA
 *	Bitmap::DXBITMAP	texel data pointer BGR/BGRA
 *	Bitmap::NOTEXTURE	no bitmap available
 * @endcode
 *
 * Depending on when you call GetBitmap, you may get different return
 * values. If you call it after you have loaded content from a stream,
 * but before display, all texture types returned will be Bitmap::DIB.
 * Calls subsequent to display will return Bitmap::TEXHANDLE if the
 * image has been used to texture something not culled in the scene.
 * Bitmap::TEXHANDLE is only returned if the texture has been converted
 * to device-dependent format.
 *
 * @return texture handle or texel data pointer
 *
 * @see Texture::SetBitmap Texture::Load Texture::GetTexHandle
 */
void* Texture::GetBitmap(int* type) const
{
	Bitmap* bmap = m_Bitmap;

	if (bmap == NULL)
		return NULL;
	ObjectLock lock(bmap);
	if (type)
		*type = bmap->Type;
	return bmap->Data;
}

void* Texture::GetTexHandle() const
{
	Bitmap* bmap = m_Bitmap;

	if (bmap == NULL)
		return NULL;
	return (void*) bmap->DevHandle;
}


/*!
 * @fn void Texture::SetBitmap(int type, void* bmp)
 * @param bmp	new bitmap representation to set, can be Windows
 *				DIB, device dependent handle or texel data pointer
 * @param type	type of input handle supplied
 *
 * Depending on how the image was created and used, the bitmap representation
 * is stored differently. The types of in-memory bitmaps are:
 * @code
 *	IMAGE_DIB			pointer to Windows DIB
 *	IMAGE_GLBitmap		texel data pointer RGB/RGBA
 *	IMAGE_DXBitmap		texel data pointer BGR/BGRA
 * @endcode
 *
 * If you supply a texel data pointer, the Image will use that
 * data for the texture, ignoring all other representations.
 * This is considered a user-managed data area so the scene manager will never
 * try to free or relocate it.
 *
 * SetBitmap is not a distributed call. Setting the bitmap data area
 * will have no effect on remote versions of the image. To transfer
 * bitmap data areas, use Texture::UpdateData.
 *
 * @see Texture::GetBitmap Texture::Update Texture::Load Texture::UpdateData
 */
void Texture::SetBitmap(int type, void* data)
{
	ObjectLock imglock(this);
	Bitmap* bmap = m_Bitmap;

	if (bmap == NULL)
	{
		bmap = new Bitmap;
		bmap->Type = type;
		bmap->Data = (void*) data;
		bmap->SetChanged(true);
		m_Bitmap = bmap;
		return;
	}
	ObjectLock bmplock(bmap);
	bmap->Kill();
	bmap->Type = type;
	bmap->Data = (void*) data;
	bmap->SetChanged(true);
}

void Texture::SetBitmap(Bitmap* bmap)
{
	ObjectLock lock(this);
	m_Bitmap = bmap;
}

bool Texture::UpdateData(void* data)
{
	Bitmap*	bmap = m_Bitmap;
	int32		imagesize;
	char*		newdata = (char*) data;

	if (bmap == NULL)
		return false;
	imagesize = bmap->Width * bmap->Height * BytesPerPixel();

	VX_STREAM_BEGIN(s)
	int32	maxbytes = BufMessenger::MaxBufSize - 4 * sizeof(int32);
	int32	ofs = 0, n = imagesize;

		while (n > 0)
		{
			if (n < maxbytes)
				maxbytes = n;
			*s << OP(VX_Image, IMAGE_UpdateData) << this << ofs << maxbytes;
			s->Write(newdata + ofs, maxbytes);
			n -= maxbytes;
			ofs += maxbytes;
		}
	VX_STREAM_END()

	ObjectLock lock(bmap);
	//int		type;
	char*	imagedata = (char*) bmap->Data;

	if (imagedata == NULL)
		imagedata = (char*) malloc(imagesize);
	memcpy(imagedata, newdata, imagesize);
	//bmap->Type = type;
	bmap->Data = imagedata;
	return true;
}

/*!
 * @fn void Texture::SetFileName(const TCHAR* filename)
 * @param filename new texture file name to set
 *
 * This call does not initiate loading of the texture from disk,
 * it just changes the file name and dereferences the bitmap
 * currently associated with it.
 *
 * This function is used internally when a bitmap has finished
 * loading from disk and is attached to the image which initiated
 * the load. It can also be used to decouple the naming of images
 * from their disk loading.
 *
 * @see Texture::GetFileName Texture::Load
 */
void Texture::SetFileName(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Image, IMAGE_SetFileName) << this << filename;
	VX_STREAM_END()

	if (GetFileName() == filename)
		return;
	m_Bitmap = (Bitmap*) NULL;
	m_FileName = filename;
}


bool Texture::ReadFile(const TCHAR* imagename, Core::Stream* stream, LoadEvent* e)
{
	LoadSceneEvent* event = (LoadSceneEvent*) e;
	if (!Bitmap::ReadFile(imagename, stream, event))
		return false;
	Texture* image = (Texture*) (SharedObj*) event->Sender;
	Ref<Bitmap> bitmap = (Bitmap*) (SharedObj*) event->Object;

	if (image && image->IsClass(VX_Image))
	{
		ObjectLock imglock(image);
		int format = image->GetFormat() & Bitmap::HASCOLORKEY;
		image->SetFileName(imagename);
		Core::InterlockOr(&(bitmap->Format), format);
		image->SetBitmap(bitmap);
	}
	return true;
}

bool Texture::OnEvent(Event* e)
{
	Bitmap*			bmap = GetBitmap();
	LoadSceneEvent*	ev = (LoadSceneEvent*) e;

	if (bmap != NULL)
		return false;
	if (e->Code != Event::LOAD_IMAGE)
		return false;

	ObjectLock	lock(this);
	if (m_FileName.CompareNoCase(ev->FileName) != 0)
		return false;
	bmap = (Bitmap*) (SharedObj*) ev->Object;
	if (bmap && bmap->IsClass(VX_Bitmap))
	{
		Core::InterlockOr(&(bmap->Format), GetFormat() & Bitmap::HASCOLORKEY);
		SetBitmap(bmap);
	}
	return true;
}


}	// end Vixen