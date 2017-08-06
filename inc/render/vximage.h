/*!
 * @file vximage.h
 * @brief An image is a 2D bitmap that can be mapped to 3D geometry.
 *
 * Image pixel formats are platform dependent. Images used with a rendering pipeline that
 * supports mip-mapping can have multiple levels of detail.
 * Images are loaded from disk or the web asynchronously and are automatically
 * loaded to the graphics display device.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxbitmap.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Texture
 * @brief 2D texture which map be mapped onto 3D objects.
 *
 * Images are loaded asynchronously by separate loading threads and efficiently
 * cached internally to ensure that each bitmap file is read only once.
 * Several different internal formats for the pixel data are supported
 * by the underlying renderer and do not require further translation.
 *
 * @code
 *	Bitmap::DXBITMAP	DirectX format RGB or RGBA pixel array. This is essentially
 *						the body of a Windows DIB without the header.
 *	Bitmap::GLBITMAP	OpenGL format RGB or RGBA pixel array. This is like a
 *						DX bitmap except reversed vertically.
 *	Bitmap::DIB			Windows DIB format memory area (RGBA or RGB)
 *	Bitmap::DDS			Direct X DDS format (aka S3TC compressed format)
 *	Bitmap::FONT		Binary font file (.TXF)
 * @endcode
 *
 * Multiple image objects may reference the same bitmap. The image class provides
 * locked and shared access to a pool of bitmap images loaded from disk.
 * This permits the loading thread to construct memory bitmaps which the
 * rendering thread loads into the graphics device.
 *
 * @ingroup vixen
 * @see Bitmap
 */
class Texture : public SharedObj
{
public:
	/*!
	 * @brief texture memory format options
	 * @see Bitmap::Format Texture::IsMipMap Texture::IsCubeMap
	 */
	enum
	{
		MIPMAP = 32,		//!< texture is mip-mapped
		CUBEMAP = 64,		//!< texture is a cube map
		NORMALIZED = 128,	//!< texture is a normalized cube map
		RENDERTARGET = 256	//!< texture is a render target
	};


	VX_DECLARE_CLASS(Texture);
	Texture(const TCHAR* fname = NULL);	//!< Construct empty image and initiate load.
	~Texture();							//!< Destroy image and dereference its bitmap.

//! @name bitmap properties
//!@{
	bool	HasColor() const;			//!< \b true if image has a color (RGB) channel
	bool	HasAlpha() const;			//!< \b true if image has an alpha channel
	bool	IsMipMap() const;			//!< \b true if image is mip-mapped
	bool	HasColorKey() const;		//!< \b true if image is color-keyed
	bool	IsCubeMap() const;			//!< \b true if image is a cube map
	void	SetWidth(int);				//!< Set pixel width of image.
	int		GetWidth() const;			//!< Get pixel width of largest level of detail.
	void	SetHeight(int);				//!< Set pixel height of image.
	int		GetHeight() const;			//!< Get pixel height of largest level of detail.
	void	SetDepth(int);				//!< Set pixel depth of image.
	int		GetDepth() const;			//!< Get pixel depth of image
	int		BytesPerPixel() const;		//!< Get bytes per pixel (2, 3, 4)
	void	SetNumLod(int);				//!< Set number of levels of detail (mip-maps).
	int		GetNumLod() const;			//!< Get number of levels of detail.
	void	SetFormat(int);				//!< Set image format options indicating bitmap characteristics.
	int		GetFormat() const;			//!< Get image format options.
	void*	GetTexHandle() const;		//!< Get device texture handle.
	void	SetTranspColor(const Col4&);//! Set transparent color for alpha channel construction.
	void	SetTranspColor(const Col4*);
	const Col4&	GetTranspColor() const;		//!< Get transparent color for alpha channel construction.
//!@}

//! @name pixel data manipulation
//!@{
	void			SetBitmap(Bitmap*);			//!< Supply bitmap information for image.
	void			SetBitmap(int type, void* bmp);
	Bitmap*			GetBitmap();				//!< Get pointer to bitmap referenced by this image.
	const Bitmap*	GetBitmap() const;
	void*			GetBitmap(int* type) const;//! Get type and pointer to pixel data.
	bool			UpdateData(void* data);		//! Change pixel data.
//!@}

//! @name image file loading
//!@{
//!< Asynchronously load image from file or URL.
	bool			Load(const TCHAR* filename);

//! Set name of file containing pixel data, may be a URL or NULL.
	void			SetFileName(const TCHAR* filename);

//!< Get name of file pixels came from.
	const TCHAR*	GetFileName() const;


	static	bool	ReadFile(const TCHAR* imagename, Core::Stream* stream, LoadEvent* event);
//!@}

//! Determine if two images are different.
	bool		operator!=(const Texture& src) const;

//! Determine if two images are equal.
	bool		operator==(const Texture& src) const;

	virtual	bool		Copy(const SharedObj*);
	virtual	bool		Do(Messenger&, int);
	virtual	int			Save(Messenger&, int) const;
	virtual	bool		OnEvent(Event* e);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

//	Public Data
	static	bool	DoTexturing;					//!< set to \b false to disable texturing
	mutable intptr	DevHandle;

	/*
	 * Image::Do opcodes (for binary file input)
	 */
	enum Opcode
	{
		IMAGE_SetWidth = SharedObj::OBJ_NextOp,
		IMAGE_SetHeight,
		IMAGE_SetDepth,
		IMAGE_Load,
		IMAGE_SetTranspColor,
		IMAGE_SetFormat,
		IMAGE_SetBitmap,
		IMAGE_Touch,
		IMAGE_SetFileName,
		IMAGE_SetDeviceDepth,
		IMAGE_UpdateData,
		IMAGE_NextOp = SharedObj::OBJ_NextOp + 20,
	};

protected:
	int32			m_NumLod;		// number of levels of detail
	Col4			m_TranspColor;	// transparency color
	Ref<Bitmap>		m_Bitmap;		// bitmap information
	Core::String	m_FileName;		// name of image file
};

} // end Vixen
