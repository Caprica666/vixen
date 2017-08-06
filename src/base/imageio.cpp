#include "vixen.h"

#ifndef VX_FREEIMAGE
#include "render/texfont.h"
#include "win32/dds.h"

namespace Vixen {

extern void*		ReadJPGFile(Bitmap*, Core::Stream*);
extern void*		ReadPNGFile(Bitmap*, Core::Stream*);

void* Bitmap::ReadBitmap(const TCHAR* filename, Core::Stream* stream)
{
	Core::String		fname(filename);
	Core::String		ext(fname.Right(4));
	void*				pixels = NULL;

	if (stream == NULL)							// no stream?
		return NULL;
	if (ext.CompareNoCase(TEXT(".txf")) == 0)	// is a font?
		return ReadFont(filename, stream);
	if (ext.CompareNoCase(TEXT(".jpg")) == 0)	// is a JPEG image?
		return ReadJPGFile(this, stream);
	if (ext.CompareNoCase(TEXT(".png")) == 0)	// is a PNG image?
		return ReadPNGFile(this, stream);
	return NULL;
}

bool Bitmap::Startup()
{
	return true;
}

void Bitmap::Shutdown()
{
}

}	// end Vixen

#endif