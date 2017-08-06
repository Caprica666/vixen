/*!
 * @file vxbitmap.h
 * @brief 2D array of pixels that can be loaded from
 * files or constructed in memory.
 *
 * The scene manager can import BMP, PNG, JPG, DDS and TXF formats
 * from files or URLs.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vximage.h vxstream.h
 */
#pragma once

namespace Vixen {

class LoadEvent;

/*!
 * @class Bitmap
 * @brief 2D array of pixels loaded from a file or URL.
 *
 * Bitmaps are primarily used internally during file loading.
 * You cannot use a bitmap directly as a texture on a 3D object.
 * Texture is a subclass of bitmap that represents a texture.
 *
 * A bitmap may have several different internal formats for the pixel data
 * after it has been loaded from a file. All of these pixel formats are
 * supported by the underlying renderer and do not require further translation.
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
 * @ingroup vixen
 * @see Texture FileLoader Core::Stream
 */
class Bitmap : public SharedObj
{
public:

	VX_DECLARE_CLASS(Bitmap);
	Bitmap();
	~Bitmap();

	/*!
	 * @brief Bitmap file formats (values for Type).
	 *
	 * These types describe the in-memory pixel formats which can be directly
	 * loaded by the scene manager into the graphics device. These types are
	 * supported for both OpenGL and DirectX implementations.
	 *
	 * @see Bitmap::Type
	 */
	enum
	{
		ISLOADING = -1,	//!< image in the process of loading
		NOTEXTURE = 0,	//!< no representation of texture
		DIB = 1,		//!< in-memory Windows DIB
		GLBITMAP = 2,	//!< OpenGL format RGB, RGBA
		DXBITMAP = 3,	//!< DirectX format RGB, RGBA
		DDS = 4,		//!< DDS (aka S3TC) format
		FONT = 5,		//!< TXF font format
		TEXHANDLE = 6,	//!< handle for device texture
	};

	/*!
	 * @brief Bitmap in memory format options (values for Format).
	 *
	 * These types describe the in-memory pixel formats which can be directly
	 * loaded by the scene manager into the graphics device. These types are
	 * supported for both OpenGL and DirectX implementations.
	 *
	 * @see Bitmap::Format Texture::HasColor Texture::HasAlpha Texture::IsMipMap Texture::HasColorKey
	 */
	enum
	{
		HASALPHA =	1,		//!< texture has alpha channel
		NOFREE_DATA = 2,	//!< data area user-owned - don't free
		HASCOLOR = 4,		//!< texture has color channel
		HASCOLORKEY = 8		//!< texture has transparent color
	};

//! Load pixels for this bitmap using input stream, issues blocking read.
	bool			Load(const TCHAR* filename, Core::Stream* = NULL);

//! Garbage collects the bitmap pixel data area.
	void			Kill();

//! Read pixels from bitmap file using given stream.
	static	bool	ReadFile(const TCHAR* filename, Core::Stream* stream, LoadEvent* e);

//! Write Windows BMP file from pixel array.
	static	bool	WriteBMP(const TCHAR *filename, void* pixels, int width, int height);

//! Write SGI RGB image file from pixel array.
	static bool		WriteRGB(const TCHAR *filename, void* pixels, int width, int height, int depth);

//! Create RGB / RGBA OpenGL bitmap from Windows DIB.
	static	void*	GLImageFromDIB(const void *INPUT, const Col4* transp, void* OUTPUT = NULL);

//! Create RGB / RGBA DirectX bitmap from Windows DIB.
	static	void*	DXImageFromDIB(const void *INPUT,const  Col4* transp, void* OUTPUT = NULL);

//! Construct alpha channel for this image.
	void*			MakeAlpha(void *INOUT, const Col4* INUT);

//! Convert 24 bit RGB to 16 bit R5G6B5, convert 32 bit RGBA to 16 bit A1R5G5B5
	static void*	Make16Bit(const void* INPUT, void* OUTPUT, int w, int h, int srcdepth, int stride);

	static bool		Startup();
	static void		Shutdown();

//! Copy pixel data info from source bitmap to this one
	Bitmap&		operator=(const Bitmap& src);


private:
	void*		ReadBitmap(const TCHAR* filename, Core::Stream* stream = NULL);
	void*		ReadFont(const TCHAR* filename, Core::Stream* stream = NULL);
#ifdef _WIN32
	void*		ReadDDS(const TCHAR* filename, Core::Stream* stream = NULL);
#endif
//	void*		ReadBMP(const TCHAR* filename, Core::Stream* stream = NULL);
//	void*		ReadPNG(const TCHAR* filename, Core::Stream* stream = NULL);

public:
	int			Type;		//!< format of pixel data
	int			Depth;		//!< bits per pixel
	int			Format;		//!< texture format flags
	int			Width;		//!< width in pixels
	int			Height;		//!< height in pixels
	int			ByteSize;	//!< size in bytes of the bitmap data area
/*!
 * Pointer to pixels as a contiguous block. This data area
 * is deleted using \b ::free when the bitmap is destroyed
 */
	void*		Data;
/*
 * Depth for font bitmaps. Can be either 8 or 16
 */
	static int	FontDepth;
/*!
 * Default depth for RGBA bitmaps read from files. If this is less
 * than the depth of the file image, it is converted to this depth.
 */
	static int	RGBADepth;
/*!
 * Default depth for RGB bitmaps read from files. If this is less
 * than the depth of the file image, it is converted to this depth.
 */
	static int	RGBDepth;

/*!
 * Device-dependent texture handle, NULL indicates bitmap not cached
 * on a graphics device yet.
 */
	mutable intptr	DevHandle;

protected:
	static	vint32	IsInitialized;
};

} // end Vixen