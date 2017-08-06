#include "vcore/vcore.h"

namespace Vixen {
namespace Core {

VX_IMPLEMENT_CLASS(FileStream, Stream);

FileStream::FileStream(const TCHAR* filename) : Core::Stream(filename)
{
	m_stream = NULL;
	if (filename)
		Open(filename, OPEN_READ);
}

/***
 *
 * You would think we would not need this destructor to call Close since the
 * the Stream destructor calls close, too. However, Stream::~Stream
 * calls Stream::Close but we have overridden it in FileStream. These
 * overrides don't appear to work inside destructors, however. So we call it
 * here to make sure it closes the file properly.
 *
 ***/
FileStream::~FileStream()
{
	Flush();
	Close();
}

/*!
 * intptr FileStream::GetHandle() const
 * Provides access to the underlying FILE object associated
 * with an open stream. If the stream has a file open,
 * the return value will be a non-null FILE* represented as an integer.
 *
 * @returns pointer to FILE or 0 if no file open
 * @see Stream::GetHandle
 */
intptr FileStream::GetHandle() const { return (intptr) m_stream; }

bool FileStream::Open(const TCHAR* filename, int mode)
{
	if (filename == NULL)
		return false;
#ifdef _WIN32
	TCHAR namebuf[VX_MaxPath];
	TCHAR* p = namebuf;

	STRCPY(namebuf, filename);
	while (*p)			// convert slashes to backslashes
	{
		if (*p == TEXT('/'))
			*p = TEXT('\\');
		++p;
	}
#else
	const char* namebuf = filename;
#endif
	if (m_openmode)						// already open?
		VX_ERROR(("FileStream::Open stream is already open"), false);
	VX_ASSERT(m_stream == NULL);
	if (mode & OPEN_WRITE)			// open for write
		m_stream = FOPEN(namebuf, TEXT("wb"));
	else if (mode & OPEN_READ)		// open for read
		m_stream = FOPEN(namebuf, TEXT("rb"));
	if (!m_stream)
		return false;
	return Stream::Open(filename, mode);
}

void FileStream::Flush()
{
	if (m_stream)
		fflush(m_stream);
}

bool FileStream::Close()
{
	if (m_stream)
	{
		fflush(m_stream);
		fclose(m_stream);
		m_stream = NULL;
	}
	return 	Stream::Close();
}

size_t FileStream::Read(char* buffer, size_t n)
{
	VX_ASSERT(buffer);
	if ((n == 0) || (m_stream == NULL))
		return 0;
	size_t m = fread(buffer, 1, n, m_stream);
	//VX_TRACE(m != n, ("FileStream::Read error\n"));
	return m;
}

size_t FileStream::Write(const char* buffer, size_t n)
{
	VX_ASSERT(buffer);
	if ((n == 0) || (m_stream == NULL))
		return 0;
	size_t m = fwrite(buffer, 1, n, m_stream);
	VX_TRACE(m != n, ("FileStream::Write error\n"));
	return m;
}

bool FileStream::IsEmpty() const
{
	if (m_stream == NULL || feof(m_stream))
		return true;
	int c = getc(m_stream);
	if (c == EOF)
		return true;
	ungetc(c, m_stream);
	return false;
}

offset_t FileStream::Seek(offset_t seekto, int from)
{
	return FSEEK(m_stream, seekto, from);
}

size_t FileStream::GetPos() const
{
	return FTELL(m_stream);
}

}	// end Core
}	// end Vixen