
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(ImageSwitch, Texture, VX_ImageSwitch);

static const TCHAR* opnames[] =
{	TEXT("SetIndex"), TEXT("SetAt") };

const TCHAR** ImageSwitch::DoNames = opnames;

/*****
 *
 * Name: LoadDev
 *
 * Description:
 *	Callback used during rendering to load a texture into
 *	the display device.	Here we override it to load the
 *	specific image we want
 *
 ****/
bool ImageSwitch::LoadDev(Scene* scene, int texunit, bool mipmap)
{
	bool		rc = false;
	int			size = m_Images.GetSize();
	Texture*	image = NULL;

	if (m_CurImage < 0)					// default to first image
		m_CurImage = 0;
	if (m_CurImage < size)				// within array bounds?
		image = m_Images[m_CurImage];	// get current image
	if (image == NULL)					// no image in this slot?
	   {
		if (size == 0)					// no images at all?
			return false;				// texture load failed
		image = m_Images[size - 1];		// try last texture
	   }
	if (image)
		rc = image->LoadDev(scene, texunit, mipmap);
	return rc;
} 


/*!
 * @fn void ImageSwitch::SetIndex(int index)
 * @param index	0 based index of next image to render
 *
 * Sets the index of the image to use as a texture next frame.
 * If set to -1, the image switcher will
 * advance to the next image (and cycle) each frame.
 * If you set a number out of range, no image is displayed.
 * GetIndex always returns the current image index of the
 * texture displayed (never -1).
 *
 * @see Switcher Texture
 */
void ImageSwitch::SetIndex(int index)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_ImageSwitch, IMAGESW_SetIndex) << this << int32(index);
   VX_STREAM_END(  )

	m_Index = index;
	if (index >= 0)
		m_CurImage = index;
}

/*!
 * @fn bool ImageSwitch::SetTexture(int n, Texture* image)
 * @param n		0 based index of image to replace
 * @param image	new image
 *
 * Sets the Nth image in the animation sequence.
 * If the image list already has an image in this slot, it is replaced.
 *
 *
 * @return \b true if image successfully replaced, \b false if index out of range
 *
 * @see ImageSwitch::AddImage
 */
bool ImageSwitch::SetTexture(int n, Texture* image)
{
   VX_STREAM_BEGIN(s)
      *s << OP(VX_ImageSwitch, IMAGESW_SetAt) << this << int32(n) << image;
   VX_STREAM_END(  )

	if (GetFormat() & Bitmap::HASCOLORKEY)
		image->SetTranspColor(GetTranspColor());
	return m_Images.SetAt(n, image);
}

/*!
 * @fn int ImageSwitch::AddImage(Texture* image)
 * @param image	new image to add
 *
 * Adds an image to the animation sequence (at the end
 * of the current image list).
 *
 * @return index of image if successfully added, else -1
 *
 * @see ImageSwitch::SetTexture ImageSwitch::Empty
 */
int ImageSwitch::AddImage(Texture* image)
{
	int n = GetSize();

	if (SetTexture(n, image))
		return n;
	return -1;
}

void ImageSwitch::Empty()
{
	m_Images.Empty();
}


/****
 *
 * class ImageSwitch override for SharedObj::Copy
 *
 ****/
bool ImageSwitch::Copy(const SharedObj* srcobj)
{
	const ImageSwitch* src = (const ImageSwitch*) srcobj;
	ObjectLock dlock(this);
	ObjectLock slock(srcobj);
	if (!Texture::Copy(srcobj))
		return false;
	if (srcobj->IsClass(VX_ImageSwitch))
	{
		m_Index = src->m_Index;
		m_Images.Copy(&(src->m_Images));
	}
	return true;
}


/****
 *
 * class ImageSwitch override for SharedObj::Do
 *
 ****/
bool ImageSwitch::Do(Messenger& s, int op)
{
	int32		n;
	SharedObj*		obj;

	switch (op)
	{
		case IMAGESW_SetIndex:
		s >> n;
		SetIndex(n);
		break;

		case IMAGESW_SetAt:
		s >> n >> obj;
		VX_ASSERT(obj->IsClass(VX_Image));
		m_Images.SetAt(n, (Texture*)obj);
		break;

		default:
		return Texture::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
			<< ImageSwitch::DoNames[op - IMAGESW_SetIndex]
			<< " " << this);
#endif
	return true;
}



/****
 *
 * class ImageSwitch override for SharedObj::Print
 *
 ****/
DebugOut& ImageSwitch::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Texture::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='Index'>" << GetIndex() << "</attr>");
	if (opts & PRINT_Data)
		m_Images.Print(dbg, opts | PRINT_Trailer);
	Texture::Print(dbg, opts & ~PRINT_Trailer);
	return dbg;
}


/****
 *
 * class ImageSwitch override for SharedObj::Save
 *
 ****/
int ImageSwitch::Save(Messenger& s, int opts) const
{
	int32 h = Texture::Save(s, opts);

	if (h < 0)
		return h;
	if (h > 0)
		s << OP(VX_ImageSwitch, IMAGESW_SetIndex) << h << int32(GetIndex());
	for (int32 i = 0; i < m_Images.GetSize(); ++i)
	   {
		const Texture* image =  m_Images.GetAt(i);
		if ((image->Save(s, opts) >= 0) && h)
			s << OP(VX_ImageSwitch, IMAGESW_SetAt) << h << i << image;
	   }
	return h;
}

}	// end Vixen