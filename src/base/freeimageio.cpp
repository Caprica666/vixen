#include "vixen.h"

#ifdef VX_FREEIMAGE
#include "FreeImage.h"

namespace Vixen {

#define BI_RGB        0L

bool Bitmap::Startup()
{
	if (Core::InterlockTestSet(&IsInitialized, 1L, 0))
		FreeImage_Initialise();
	return true;
}

void Bitmap::Shutdown()
{
	Core::InterlockSet(&Bitmap::IsInitialized, 0);
	FreeImage_DeInitialise();
}

/*
 * FreeImageIO routines for bitmap reading
 */
unsigned int DLL_CALLCONV vxStreamRead(void *buffer, unsigned size, unsigned count, fi_handle handle)
{
	Core::FileStream*	stream = (Core::FileStream*) handle;
	int				nbytes = size * count;
	VX_ASSERT(stream->IsKindOf(CLASS_(Core::FileStream)));
	return (unsigned int) stream->Read((char*) buffer, nbytes);
}

unsigned int DLL_CALLCONV vxStreamWrite(void *buffer, unsigned size, unsigned count, fi_handle handle)
{
	Core::FileStream*	stream = (Core::FileStream*) handle;
	int				nbytes = size * count;
	VX_ASSERT(stream->IsKindOf(CLASS_(Core::FileStream)));
	return (unsigned int) stream->Write((const char*) buffer, nbytes);
}

int DLL_CALLCONV vxStreamSeek(fi_handle handle, long offset, int origin)
{
	Core::FileStream*	stream = (Core::FileStream*) handle;
	return (int) stream->Seek(offset, origin);
}

long DLL_CALLCONV vxStreamTell(fi_handle handle)
{
	Core::FileStream*	stream = (Core::FileStream*) handle;
	return (long) stream->GetPos();
}

void* Bitmap::ReadBitmap(const TCHAR* filename, Core::Stream* stream)
{
	FreeImageIO			freeio;
	FREE_IMAGE_FORMAT	fif;
	FIBITMAP*			fbmap = NULL;
	FREE_IMAGE_COLOR_TYPE fic;
	int32				format = 0;
	Core::String		fname(filename);
	Core::String		ext(fname.Right(4));
	char				fbuf[VX_MaxPath];

	if (stream == NULL)							// no stream?
		return NULL;
	if (ext.CompareNoCase(TEXT(".txf")) == 0)	// is a font?
		return ReadFont(filename, stream);
	freeio.read_proc = vxStreamRead;
	freeio.write_proc = vxStreamWrite;
	freeio.seek_proc = vxStreamSeek;
	freeio.tell_proc = vxStreamTell;

	if (!IsInitialized)
		return NULL;
	fname.AsMultiByte(fbuf, VX_MaxPath);
	fif = FreeImage_GetFIFFromFilename(fbuf);
	if (fif == FIF_UNKNOWN)						// see if freeimage reads this type
		return NULL;

	int			width, height, depth, stride;
	FIBITMAP*	newmap = NULL;
	char*		rgbdata;						// pixel area to return
	char*		dibdata;						// pixels from DIB

	fbmap = FreeImage_LoadFromHandle(fif, &freeio, stream); 
	if (fbmap == NULL)							// freeimage can't read the file
		return NULL;
	fic = FreeImage_GetColorType(fbmap);		// what color format
	if (fic == FIC_RGB)
		format = HASCOLOR;
	else if (fic == FIC_RGBALPHA)
		format = HASCOLOR | HASALPHA;
	if (Format & HASCOLORKEY)
		format |= HASCOLORKEY;
	depth = FreeImage_GetBPP(fbmap);			// bits per pixel
	width = FreeImage_GetWidth(fbmap);			// copy info from freeimage to this bitmap
	height = FreeImage_GetHeight(fbmap);		
	stride = FreeImage_GetLine(fbmap);
	dibdata = (char*) FreeImage_GetBits(fbmap);	// pixels to copy
	switch (depth)
	{
		case 8:									// indexed bitmaps not supported
		FreeImage_Unload(fbmap);				// free original bitmap
		return NULL;

		case 32:
		format |= HASCOLOR | HASALPHA;
		if (RGBADepth == 16)					// optimal RGBA depth is 16 bits A1R5G7B5
		{
			rgbdata = (char*) Make16Bit(dibdata, NULL, width, height, depth, FreeImage_GetPitch(fbmap));
			depth = 16;
			FreeImage_Unload(fbmap);			// free original bitmap
			fbmap = NULL;						// indicate our conversion is done
		}
		break;

		case 24:								// optimal RGB depth is 16 bits R5G6B5
		if (RGBDepth == 16)						// check to see if we downsize
		{
			rgbdata = (char*) Make16Bit(dibdata, NULL, width, height, depth, FreeImage_GetPitch(fbmap));
			depth = 16;
			FreeImage_Unload(fbmap);			// free original bitmap
			fbmap = NULL;						// indicate our conversion is done
		}
		break;
	}
	if (fbmap)									// still in freeimage format?
	{
		int pitch = FreeImage_GetPitch(fbmap);
		rgbdata = (char*) malloc(stride * height);	// allocate pixel data area
		#pragma omp PARALLEL_FOR(height)
		cilk_for (int row = 0; row < height; ++row)		// copy pixels
			memcpy(rgbdata + row * stride, dibdata + row * pitch, stride);
//			memcpy(rgbdata + row * stride, dibdata + (height - row - 1) * FreeImage_GetPitch(fbmap), stride);
		FreeImage_Unload(fbmap);
	}
	Lock();										// lock around bitmap update
	Width = width;								// copy dimensions
	Height = height;
	Depth = depth;
	Format = format;							// copy format
	Type = DXBITMAP;							// copy type and data
	Data = rgbdata;
	ByteSize = stride * height;
	SetName(filename);
	Unlock();
	return rgbdata;
}
}	// end Vixen

#endif