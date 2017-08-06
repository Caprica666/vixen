#if WIN32
#include <windows.h>
#endif

#include "psm.h"

PSImageReader::PSImageReader(const char* filename)
  :	FileName(filename)
{
	ReadFunc = NULL;
	Pitch = 0;
}


/*!
 * @fn bool PSImageReader::ReadFont(const char* filename, PSStream* stream, PSLoadEvent* event)
 * Reads a binary Font file in .TXF format and generates a load event
 * @param filename	name of font file
 * @param stream	stream to use to read font
 * @param event		load event to get resulting image
 *
 * The caller is assumed to have opened the input stream to
 * the .TXF file containing the font. Upon return, the
 * stream is closed and the event references a PSBitmap
 * object containing the font data.
 *
 * This function may be used directly with the loader.
 * It issues a blocking read and should not be called
 * directly from real time code.
 *
 * @see PSImage::Load PSImage::LoadDev PSBitmap PSLoader::SetFileFunc
 */
bool PSImageReader::ReadFont(const char* filename, PSStream* stream, PSLoadEvent* event)
{
	PSImageReader reader(filename);
	reader.ReadFunc = &PSImageReader::TXFRead;
	return reader.Read(event, stream);
}

/*!
 * @fn bool PSImageReader::ReadBMP(const char* filename, PSStream* stream, PSLoadEvent* event)
 * Reads a bitmap file in .BMP format and generates a load event
 * @param filename	name of bitmap file
 * @param stream	stream to use to read bitmap
 * @param event		load event to get resulting image
 *
 * The caller is assumed to have opened the input stream to
 * the .BMP file containing the bitmap. Upon return, the
 * stream is closed and the event references a PSBitmap
 * object containing the font data.
 *
 * This function may be used directly with the loader.
 * It issues a blocking read and should not be called
 * directly from real time code.
 *
 * @see PSImage::Load PSImage::LoadDev PSBitmap PSLoader::SetFileFunc
 */
bool PSImageReader::ReadBMP(const char* filename, PSStream* stream, PSLoadEvent* event)
{
	PSImageReader reader(filename);
	reader.ReadFunc = &PSImageReader::BMPRead;
	return reader.Read(event, stream);
}

/*!
 * @fn bool PSImageReader::ReadPNG(const char* filename, PSStream* stream, PSLoadEvent* event)
 * Reads a bitmap file in .PNG format and generates a load event
 * @param filename	name of bitmap file
 * @param stream	stream to use to read bitmap
 * @param event		load event to get resulting image
 *
 * The caller is assumed to have opened the input stream to
 * the .PNG file containing the bitmap. Upon return, the
 * stream is closed and the event references a PSBitmap
 * object containing the font data.
 *
 * This function may be used directly with the loader.
 * It issues a blocking read and should not be called
 * directly from real time code.
 *
 * @see PSImage::Load PSImage::LoadDev PSBitmap PSLoader::SetFileFunc
 */
bool PSImageReader::ReadPNG(const char* filename, PSStream* stream, PSLoadEvent* event)
{
	PSImageReader reader(filename);
	reader.ReadFunc = &PSImageReader::PNGRead;
	return reader.Read(event, stream);
}

/*!
 * @fn bool PSImageReader::ReadJPG(const char* filename, PSStream* stream, PSLoadEvent* event)
 * Reads a bitmap file in .JPG format and generates a load event
 * @param filename	name of bitmap file
 * @param stream	stream to use to read bitmap
 * @param event		load event to get resulting image
 *
 * The caller is assumed to have opened the input stream to
 * the .JPG file containing the bitmap. Upon return, the
 * stream is closed and the event references a PSBitmap
 * object containing the font data.
 *
 * This function may be used directly with the loader.
 * It issues a blocking read and should not be called
 * directly from real time code.
 *
 * @see PSImage::Load PSImage::LoadDev PSBitmap PSLoader::SetFileFunc
 */
bool PSImageReader::ReadJPG(const char* filename, PSStream* stream, PSLoadEvent* event)
{
	PSImageReader reader(filename);
	reader.ReadFunc = &PSImageReader::JPGRead;
	return reader.Read(event, stream);
}

/*!
 * @fn bool PSImageReader::ReadDDS(const char* filename, PSStream* stream, PSLoadEvent* event)
 * Reads a compressed texture in .DDS format and generates a load event
 * @param filename	name of bitmap file
 * @param stream	stream to use to read bitmap
 * @param event		load event to get resulting image
 *
 * The caller is assumed to have opened the input stream to
 * the .DDS file containing the bitmap. Upon return, the
 * stream is closed and the event references a PSBitmap
 * object containing the font data.
 *
 * This function may be used directly with the loader.
 * It issues a blocking read and should not be called
 * directly from real time code.
 *
 * @see PSImage::Load PSImage::LoadDev PSBitmap PSLoader::SetFileFunc
 */
bool PSImageReader::ReadDDS(const char* filename, PSStream* stream, PSLoadEvent* event)
{
	PSImageReader reader(filename);
	reader.ReadFunc = &PSImageReader::DDSRead;
	return reader.Read(event, stream);
}

bool PSImageReader::Unsupported(const char* filename, PSStream* stream, PSLoadEvent* event)
{
	PSM_ERROR(("PSImageReader::Unsupported - %s is an unsupported file format", filename), false);
}


/*!
 * @fn bool PSImageReader::Read(PSLoadEvent* event, PSStream* stream)
 * @param event		load event with bitmap object to load into
 * @param stream	stream to use to read the file
 *
 * Loads a PSBitmap from a file using the give stream
 * and attaches it to the load event provided.
 * The  ReadFunc associated with this reader is used to
 * read the pixel data from the file.
 *
 * PSBItmap::RGBDepth is used to determine the desired
 * depth for bitmaps. If it is set to 16, 24 and 32 bit
 * data is compressed to use 5 bit color channels.
 * 8 bit data and indexed bitmaps are not supported.
 *
 * This function issues a blocking read until the complete image
 * file has been read. It should never be called from real-time code
 * (like inside an engine) because it will stop processing until the file is read.)
 * To load an image asynchronously, use PSImage::Load.
 *
 * @return \b true if successfully loaded, else \b false
 *
 * @see PSImage::Load PSWorld::LoadAsync PSLoader
 */
bool PSImageReader::Read(PSLoadEvent* event, PSStream* stream)
{
	event->Code = EVENT_LoadImage;
	event->FileName = FileName;

    if (ReadFunc == NULL)		// no read function?
		{ PSM_ERROR(("PSImageReader::Read function not set"), false); }
	if (!NeedToRead(event))
		return false;

	Ref<PSBitmap> bmap = (PSBitmap*) (PSObj*) event->Object;
	if (!(this->*ReadFunc)(bmap, stream))
	    return false;

	if ((PSBitmap::RGBDepth == 16) &&
		 !ConvertTo16Bit(bmap, Pitch))
	{
		bmap->Type = IMAGE_NoTexture;
		PSM_ERROR(("Bitmap::Load ERROR cannot convert to 16 bits %s\n", FileName), false);
	}
	if (bmap->Data)
		{ PSM_TRACE(PSBitmap::Debug || PSLoader::Debug, ("Image::Load %s load complete\n", FileName)); }
	else
	{
		bmap->Type = IMAGE_NoTexture;
		PSM_ERROR(("Bitmap::Load ERROR cannot load %s\n", FileName), false);
	}

	PSImage* image = (PSImage*) (PSObj*) event->Sender;
	if (image && image->IsClass(PSM_Image))
	{
		PSObjLock imglock(image);
		int format = image->GetFormat() & IMAGE_HasColorKey;
		image->SetFileName(FileName);

		PSObjLock bmplock((PSBitmap*) bmap);
		bmap->Format |= format;
		image->SetBitmap(bmap);
	}
	return true;
}

/*!
 * @fn bool PSImageReader::NeedToRead(PSLoadEvent* event)
 * @param event	load event
 *
 * Determines whether a bitmap file actually needs to be read.
 * Upon entry, the load event is examined to see if it already
 * references an existing bitmap. If it does and that bitmap
 * is either loading or has been loaded, don't load it again.
 * Otherwise, set the IMAGE_IsLoading flag in the  Type
 * to indicate it is being loaded.
 *
 * @return  true to load bitmap,  false if already loaded
 */
bool PSImageReader::NeedToRead(PSLoadEvent* event)
{
	PSBitmap* bmap = (PSBitmap*) (PSObj*) event->Object;
	if (bmap == NULL)
	{
		bmap = new PSBitmap();
		bmap->MakeLock();
		event->Object = bmap;
	}
	if (bmap->Type == IMAGE_IsLoading)
		return false;
	if (!bmap->TryLock())
		if (bmap->Type != IMAGE_NoTexture)
		{
			PSM_TRACE(PSBitmap::Debug || PSLoader::Debug, ("ImageReader::Read %s already loaded\n", FileName));
			return false;
		}
		else
			bmap->Lock();
	if ((bmap->Type != IMAGE_NoTexture) || bmap->Data)
	{
		PSM_TRACE(PSBitmap::Debug || PSLoader::Debug, ("ImageReader::Read %s already loaded\n", FileName));
		bmap->Unlock();
		return false;;
	}
	bmap->Type = IMAGE_IsLoading;
	bmap->Unlock();
	return true;
}

/*!
 * @fn bool PSImageReader::ConvertTo16Bit(PSBitmap* bmap, int pitch)
 * @param bmap	bitmap to convert
 * @param pitch	number of bytes per scan line
 *
 * Convert the input bitmap to 16 bits per pixel.
 * The old data area is discarded. The new data
 * area will be 16 bit RGB or RGBA format.
 *
 * @see PSImageReader::Make16BitData
 */
bool PSImageReader::ConvertTo16Bit(PSBitmap* bmap, int pitch)
{
	char* data = NULL;
	int format = IMAGE_HasColor;

	if (bmap->Depth == 16)
		return true;
	if (bmap->Depth == 32)
	{
		format |= IMAGE_HasAlpha;
		data = (char*) Make16BitData(bmap->Data, NULL, bmap->Width, bmap->Height, bmap->Depth, pitch);
	}
	else if (bmap->Depth == 24)
		data = (char*) Make16BitData(bmap->Data, NULL, bmap->Width, bmap->Height, bmap->Depth, pitch);
	else if (bmap->Depth == 16)
		return true;
	else
		return false;
	bmap->Lock();
	delete bmap->Data;
	bmap->Format = format;
	bmap->Depth = 16;
	bmap->Data = data;
	bmap->Unlock();
	return true;
}

/*!
 * @fn bool void* PSImageReader::Make16BitData(const void* srcbits, void* dstbits, int w, int h, int srcdepth, int stride)
 * @param srcbits	source pixel data (24 or 32 bit)
 * @param dstbits	destination pixels (16 bit)
 * @param w			width in pixels
 * @param h			height in pixels
 * @param stride	number of bytes per scan line in source data
 *
 * Convert the source data area containing 24 or 32 bits per pixel
 * into a data area of the same dimensions using 16 bits per pixel.
 * The old data area is discarded.
 *
 * @see PSImageReader::ConvertTo16Bit
 */
void* PSImageReader::Make16BitData(const void* srcbits, void* dstbits, int w, int h, int srcdepth, int stride)
{
	uint16*	dest;
	uchar*	source = (uchar*) srcbits;
	int		i, j;

	if (dstbits == NULL)
		dstbits = (uint16*) malloc(w * h * 2);
	dest = (uint16*) dstbits;
	switch (srcdepth)
	{
/*
 * Current data area is 32 bits per pixel A8R8G8B8 and we want to make it
 * 16 bits R5G6B5
 */
		case 24:
		for (i = 0; i < h; i++)
		{
			source = (uchar*) srcbits + (i * stride);
			for (j = 0; j < w; ++j)
			{
				uchar blue = (*source++ >> 3) & 0x01F;
				uchar green = (*source++ >> 2) & 0x03F;
				uchar red = (*source++ >> 3) & 0x01F;
				uint16 v = (red << 11) | (green << 5) | blue;
				*dest++ = v;
			}
		}
		break;
/*
 * Current data area is 32 bits per pixel A8R8G8B8 and we want to make it
 * 16 bits A1R5G5B5
 */
		case 32:
		for (i = 0; i < h; i++)
		{
			source = (uchar*) srcbits + (i * stride);
			for (j = 0; j < w; ++j)
			{
				uchar blue = (*source++ >> 3) & 0x01F;
				uchar green = (*source++ >> 3) & 0x01F;
				uchar red = (*source++ >> 3) & 0x01F;
				uint16 v = (red << 10) | (green << 5)  | blue;
				if (*source++ > 128)
					v |= 0x8000;
				else
					v &= 0x7FFF;
				*dest++ = v;
			}
		}
		break;
	}
	return dstbits;
}



//
// Create an OpenGL-stype RGB/A image buffer, then fill it
// with image data from an existing DIB formatted image buffer.
//
void* PSImageReader::GLImageFromDIB(void *dib, PSCol4* transp, void* pixels)
{
	BITMAPINFOHEADER *bmi = (BITMAPINFOHEADER *)dib;
	int i, j;
	unsigned char *image, *line, *source, *dest;

	if (!dib)
		return NULL;

	uchar red_transp, green_transp, blue_transp;
	int x = bmi->biWidth;
	int y = bmi->biHeight;
	int d = bmi->biBitCount;
	int stride = ((x * d + 31) >> 5) << 2;  // BMP line stride
	int gl_size = (x * y * d) / 8;          // gltextImage2D buffer size

	if (transp)
	   {
		red_transp = uchar(255 * transp->r);
		green_transp = uchar(255 * transp->g);
		blue_transp = uchar(255 * transp->b);
		gl_size = x * y * 4;
	   }
	image = (unsigned char *) malloc(gl_size);
	if (pixels == NULL)
		pixels = (unsigned char *)dib + sizeof(BITMAPINFOHEADER);
	dest = image;
	line = (unsigned char*) pixels;
	for (i = 0; i < y; i++)
	{
		source = line + i * stride;
		for (j = 0; j < x; j++)
		{
			*dest++ = source[2];
			*dest++ = source[1];
			*dest++ = source[0];
			if (d == 32)
			{
				*dest++ = source[3];
				source += 4;
			}
			else
			{
				if (transp)
					if ((source[0] == red_transp) &&
						(source[1] == green_transp) &&
						(source[2] == blue_transp))
						*dest++ = 0;
					else
						*dest++ = 255;
				source += 3;
			}
		}
	}
	return image;
}

void* PSImageReader::DXImageFromDIB(void *dib, PSCol4* transp, void* pixels)
{
	BITMAPINFOHEADER *bmi = (BITMAPINFOHEADER *)dib;
	int i, j;
	unsigned char *image, *line, *source, *dest;

	if (!dib)
		return NULL;

	uchar red_transp, green_transp, blue_transp;
	int x = bmi->biWidth;
	int y = bmi->biHeight;
	int d = bmi->biBitCount;
	int stride = ((x * d + 31) >> 5) << 2;		// BMP line stride
	int size = (x * y * d) / 8;				// buffer size

	if (transp)
	{
		red_transp = uchar(255 * transp->r);
		green_transp = uchar(255 * transp->g);
		blue_transp = uchar(255 * transp->b);
		size = x * y * 4;
	}
	image = (unsigned char*) malloc(size);
	if (pixels == NULL)
		pixels = (unsigned char*) dib + sizeof(BITMAPINFOHEADER);
	dest = image;
	line = (unsigned char*) pixels;
	for (i = 0; i < y; i++)
	{
		source = line + i * stride;
		for (j = 0; j < x; j++)
		{
			*dest++ = source[0];
			*dest++ = source[1];
			*dest++ = source[2];
			if (d == 32)
			{
				*dest++ = source[3];
				source += 4;
			}
			else
			{
				if (transp)
					if ((source[0] == red_transp) &&
						(source[1] == green_transp) &&
						(source[2] == blue_transp))
						*dest++ = 0;
					else
						*dest++ = 255;
				source += 3;
			}
		}
	}
	return image;
}

/*!
 * @fn bool PSImageReader::TXFRead(PSBitmap* bmap, PSStream* stream)
 * Reads a binary Font file in .TXF format into a bitmap using the stream provided.
 * @param bmap		bitmap to read font data into
 * @param stream	stream to use to read font
 *
 * The caller is assumed to have opened the input stream to
 * the .TXF file containing the font. This function closes
 * the stream upon return. A font file contains bitmap data but
 * also has a binary header describing the font characters.
 * After the font is loaded as IMAGE_Font, the scene manager
 * decodes the binary font data and creates a font texture
 * that can be loaded directly into the device.
 */
bool PSImageReader::TXFRead(PSBitmap* bmap, PSStream* stream)
{
	TexFont* texfont = txfLoadFont((char*) (const char*) FileName, stream);
	stream->Close();
	if (texfont == NULL)
		return false;
	bmap->Lock();
	bmap->Data = texfont;
	bmap->Width = texfont->tex_width;
	bmap->Height = texfont->tex_height;
	bmap->Depth = PSBitmap::FontDepth;
	bmap->Format = IMAGE_HasAlpha;
	bmap->Type = IMAGE_Font;
	bmap->Unlock();
	return true;
}

/*!
 * @fn bool PSImageReader::BMPRead(PSBitmap* bmap, PSStream* stream)
 * Reads a bitmap file in .BMP format into a bitmap using the stream provided.
 * @param bmap		bitmap to read pixel data into
 * @param stream	stream to use to read pixels
 *
 * The caller is assumed to have opened the input stream to
 * the .BMP file containing the DIB. This function closes
 * the stream upon return. The resulting data area added
 * to the bitmap contains RGB or RGBA pixels in IMAGE_DXBitmap format.
 * The scene manager can directly load it into the device,
 * after which it deletes the bitmap data area.
 */
bool PSImageReader::BMPRead(PSBitmap* bmap, PSStream* stream)
{
    void*				result = NULL;
	int					pad = sizeof(int32) + sizeof(int16);
    char*				pDIB = NULL;
	BITMAPINFOHEADER	bitmapHdr;
	BITMAPFILEHEADER	fileHdr;
	int					PaletteSize;
	int					i;

	if (stream->IsEmpty())
		goto ErrExit;
	stream->Read((char*) &fileHdr.bfType, 2);
	stream->Read((char*) &fileHdr.bfSize, 12);
	if (fileHdr.bfType != 0x4d42)
        goto ErrExit;
    if (!stream->Read((char*) &bitmapHdr, sizeof(BITMAPINFOHEADER)))
        goto ErrExit;
   if (bitmapHdr.biCompression != BI_RGB)
	{
        if (bitmapHdr.biBitCount == 32 &&
			bitmapHdr.biCompression != BI_BITFIELDS)
           goto ErrExit;
    }
    if (bitmapHdr.biBitCount > 32 ||
		bitmapHdr.biBitCount < 8)
	{
        goto ErrExit;
    }
    if (bitmapHdr.biClrUsed == 0 && bitmapHdr.biBitCount <= 8)
		bitmapHdr.biClrUsed = 1 << bitmapHdr.biBitCount;
    Pitch = (((bitmapHdr.biWidth * (long) bitmapHdr.biBitCount) + 31) & ~31) >> 3;

    if (bitmapHdr.biSizeImage == 0)
        bitmapHdr.biSizeImage = Pitch * bitmapHdr.biHeight;

	PaletteSize = bitmapHdr.biClrUsed * sizeof(RGBQUAD);

 	// allocate enough room for the header
    pDIB = (char*) malloc(bitmapHdr.biSizeImage);
    stream->Read(pDIB, PaletteSize);	// skip past the palette

	for (i = 0; i < bitmapHdr.biHeight; ++i)
	{
		char* scanline = pDIB + (bitmapHdr.biHeight - i - 1) * Pitch;

		if (stream->Read(scanline, Pitch) != Pitch)
			goto ErrExit;
	}
	stream->Close();

	bmap->Lock();
	bmap->Type = IMAGE_DXBitmap;
	bmap->Data = pDIB;
	bmap->Width = bitmapHdr.biWidth;
	bmap->Height = bitmapHdr.biHeight;
	bmap->Depth = bitmapHdr.biBitCount;
	bmap->Unlock();
	return true;

ErrExit:
	free(pDIB);
	stream->Close();
    return false;

}

PSImageWriter::PSImageWriter(const char* filename)
  :	FileName(filename)
{
}

bool PSImageWriter::WriteBMP(PSBitmap* bmap)
{
    BITMAPFILEHEADER	bitmapHdr;
	BITMAPINFOHEADER	dibHdr;
    void*				pixels = bmap->Data;
	int					width = bmap->Width;
	int					height = bmap->Height;
	PSStream			stream;

	bitmapHdr.bfType = 0x4d42;
    bitmapHdr.bfReserved1 = 0;
    bitmapHdr.bfReserved2 = 0;
    bitmapHdr.bfOffBits = sizeof(dibHdr) + sizeof(bitmapHdr);
	dibHdr.biSize = sizeof(dibHdr);
	dibHdr.biBitCount = 24;
	dibHdr.biPlanes = 1;
	dibHdr.biCompression = BI_RGB;
	dibHdr.biClrUsed = 0;
	dibHdr.biClrImportant = 0;
	dibHdr.biWidth = width;
	dibHdr.biHeight = height;
	dibHdr.biSizeImage = ((((width * dibHdr.biBitCount) + 31) & ~31) >> 3) * height;
	bitmapHdr.bfSize = sizeof(bitmapHdr) + sizeof(dibHdr) + dibHdr.biSizeImage;

	if (!stream.Open(FileName, STREAM_Write))
		return false;
	if (stream.Write((const char*) &bitmapHdr.bfType, 2) != 2)
        return false;
	if (stream.Write((const char*) &bitmapHdr.bfSize, 12) != 12)
        return false;
	if (stream.Write((const char*) &dibHdr, sizeof(dibHdr)) != sizeof(dibHdr))
        return false;
	if (stream.Write((char*) pixels, dibHdr.biSizeImage) != dibHdr.biSizeImage)
		return false;
	stream.Close();
	return true;
}
