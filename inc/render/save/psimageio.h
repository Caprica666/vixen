/*!
 * @file psimageio.h
 * @brief Bitmap reader for JPG, PNG, TXF and DDS file formats
 *
 * The scene manager can import BMP, PNG, JPG, DDS and TXF formats
 * from files or URLs. It can export BMP format.
 *
 * @author Nola Donato
 * @ingroup psm
 *
 * @see psimag.h psstream.h psbitmap.h
 */
#ifndef _PS_IMAGEIO
#define _PS_IMAGEIO

class PSLoadEvent;
class PSBitmap;


/*!
 * @class PSImageReader
 * @brief Reads JPG, PNG, TXF and DDS file formats.
 *
 * Separate entry points are provided for each file format.
 * Results are returned in terms of a PSLoadEvent which
 * contains the name of the file reaq and a PSBitmap object
 * with the bitmap data from the file.
 *
 * The image reader is designed to work together with the
 * loader to provide asynchronous loading of bitmaps.
 * Image reader functions can be used with PSLoader::SetFileFunc
 * to allow them to be called asynchronously. The PSLoadEvent
 * is then posted and can be observed within the scene manager.
 *
 * @author Nola Donato
 * @ingroup psm
 *
 * @see psimage.h psstream.h psbitmap.h
 */
class PSImageReader
{
public:
//! Construct image reader for the given file
	PSImageReader(const char* filename);

//! Read bitmap data from input stream using established ReadFunc
	bool	Read(PSLoadEvent* event, PSStream* stream);

//! Read TXF (OpenGL binary font) from file
	static bool	ReadFont(const char* imagename, PSStream* stream, PSLoadEvent* event);

//! Read PNG (Portable Network Graphics) compressed bitmap from file
	static bool	ReadPNG(const char* imagename, PSStream* stream, PSLoadEvent* event);

//! Read BMP (Windows DIB) from file
	static bool	ReadBMP(const char* imagename, PSStream* stream, PSLoadEvent* event);

//! Read JPG compressed bitmap from file
	static bool	ReadJPG(const char* imagename, PSStream* stream, PSLoadEvent* event);

//! Read DDS compressed bitmap from file
	static bool	ReadDDS(const char* imagename, PSStream* stream, PSLoadEvent* event);

//! Unsupported file format - print error message
	static bool Unsupported(const char* filename, PSStream* stream, PSLoadEvent* event);

//! Convert pixel array to 15 bits per pixel
	static void* Make16BitData(const void* srcbits, void* dstbits, int w, int h, int srcdepth, int stride);

//! Create RGB / RGBA OpenGL bitmap from Windows DIB.
	static	void*	GLImageFromDIB(void *dib, PSCol4* transp, void* pixels = NULL);

//! Create RGB / RGBA DirectX bitmap from Windows DIB.
	static	void*	DXImageFromDIB(void *dib, PSCol4* transp, void* pixels = NULL);

	VString			FileName;		//!< name of file to read from
	int				Pitch;			//!< bytes per scan line
	bool			(PSImageReader::*ReadFunc)(PSBitmap* bmp, PSStream* stream);

	bool	BMPRead(PSBitmap* bmap, PSStream* stream);
	bool	PNGRead(PSBitmap* bmap, PSStream* stream);
	bool	JPGRead(PSBitmap* bmap, PSStream* stream);
	bool	TXFRead(PSBitmap* bmap, PSStream* stream);
	bool	DDSRead(PSBitmap* bmap, PSStream* stream);

protected:
	bool	NeedToRead(PSLoadEvent* event);
	bool	ConvertTo16Bit(PSBitmap* bmap, int pitch);
};

/*!
 * @class PSImageWriter
 * @brief Writes BMP file format from a bitmap
 *
 * @author Nola Donato
 * @ingroup psm
 *
 * @see psimage.h psstream.h psbitmap.h
 */
class PSImageWriter
{
public:
//! Construct image writer for the given file
	PSImageWriter(const char* filename);

//! Write bitmap data in BMP format
	bool	WriteBMP(PSBitmap* bmap);

	VString		FileName;		//!< name of file to read from
};


#endif
