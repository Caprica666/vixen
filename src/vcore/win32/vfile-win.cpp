#include "vcore/vcore.h"
#include <wininet.h>
#include <urlmon.h>


namespace Vixen {
namespace Core {

/****
 *
 * The code in this file is for the Windows operating system only!
 *
 ****/

VX_IMPLEMENT_CLASS(NetStream, FileStream);

HINTERNET NetStream::s_Session = NULL;
size_t	NetStream::s_BufSize = 1024;

NetStream::NetStream(const TCHAR* url) : MemFile(url)
{
	m_NetHandle = NULL;
	m_Buffer = new char[s_BufSize];
	m_WritePtr = m_Buffer;
	m_ReadPtr = m_Buffer;
	if (url)
		Open(url);
}


/*!
 * intptr NetStream::GetHandle() const
 * Provides access to the internet HANDLE associated
 * with an open stream. If the stream is open,
 * the return value will be a non-null internet HANDLE represented as an integer.
 *
 * @returns non-zero HANDLE or 0 if not open
 * @see Stream::GetHandle
 */
intptr NetStream::GetHandle() const { return (intptr) m_NetHandle; }

bool NetStream::Open(const TCHAR* url, int mode)
{
	if (STRNCMP(url, TEXT("file:"), 5) == 0)
	{
		m_CacheFileName = url;
		return MemFile::Open(url + 8, mode);
	}
	if (STRNCMP(url, TEXT("http:"), 5))
	{
		if (MemFile::Open(url, mode))	// is a local file
		{
			m_CacheFileName = url;
			return true;
		}
		else
		{
#if 0
			ErrorEvent* event = new ErrorEvent(World::Get());
			event->ErrString = "FILE OPEN FAILED ";
			event->ErrString += url;
			event->ErrLevel = 2;
			event->Log();
#endif
			return false;
		}
	}
	if (mode & OPEN_WRITE)					// internet is read-only
		return false;
	return NetOpen(url, mode);				// try to open the URL
}

bool	NetStream::NetOpen(const TCHAR* url, int mode)
{
	m_filepos = 0;
	m_NetHandle = NULL;
	m_ReadPtr = m_WritePtr = NULL;
/*
 * convert backslash to slash in names
 */
	TCHAR	newname[VX_MaxPath];
	const	TCHAR *s = url;
	TCHAR*	d = newname;
	DWORD	flags = 0;

	if (s == NULL)
		return false;
	while (*d = *s++)
	{
		if (*d == TEXT('\\'))
			*d = TEXT('/');
		++d;
	}
/*
 * Start initial session if necessary. Open internet file for binary read
 */
	if (s_Session == NULL)
		s_Session = InternetOpen(TEXT("Vixen"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (mode & OPEN_SEEK)
	{
		TCHAR	buf[VX_MaxPath];
		DWORD	bufsize = VX_MaxPath;

/*
 * If they want to reliably seek, they will need the information
 * cached in a real file on their hard disk
 */
		HRESULT hr = ::URLDownloadToCacheFile(NULL, newname, buf, bufsize, 0, NULL);

		if ((hr == S_OK) && MemFile::Open(buf, mode))
		{
			m_CacheFileName = buf;
			return true;
		}
		VX_ERROR(("NetStream::Open %s ERROR: CANNOT OPEN URL error", newname), false);	
	}
/*
 * Go to the internet for the file
 */
	m_NetHandle = ::InternetOpenUrl(s_Session, newname, NULL, -1, flags, INTERNET_NO_CALLBACK);
	if (m_NetHandle == NULL)
	{
		DWORD err = ::GetLastError();
		VX_ERROR(("NetStream::Open %s ERROR: CANNOT OPEN URL error = %d", newname, err), false);	
	}
	return Stream::Open(newname, mode);
}

size_t NetStream::Read(char* buffer, size_t n)
{
	size_t nread = 0;

	VX_ASSERT(buffer);
	if (m_NetHandle == NULL)
		return MemFile::Read(buffer, n);
	if (n <= 0)
		return 0;
	while (!IsEmpty())
	{
		size_t	availbytes = m_WritePtr - m_ReadPtr;
		if (availbytes > n)
			availbytes = n;
		memcpy(buffer, m_ReadPtr, availbytes);
		m_ReadPtr += availbytes;
		buffer += availbytes;
		nread += availbytes;
		n -= availbytes;
		m_filepos += availbytes;
		if (n <= 0)
			break;
	}
	return nread;
}

offset_t NetStream::Seek(offset_t seekto, int from)
{
	if (m_NetHandle == NULL)
		return MemFile::Seek(seekto, from);
	offset_t curpos = m_filepos;
	offset_t fpos = seekto;

	if (from == SEEK_FROM_HERE)
		fpos += curpos;
	/*
	 * Seek within data we have already buffered
	 */
	if ((from != SEEK_FROM_END) && (m_WritePtr > 0))
	{
		seekto = fpos - curpos;	// force relative seek
		if (((m_ReadPtr - m_Buffer + seekto) >= 0) &&
			(seekto <= (m_WritePtr - m_ReadPtr)))
		{
			m_ReadPtr += seekto;
			VX_ASSERT(fpos >= 0);
			m_filepos = (size_t) fpos;
			return fpos;
		}
		m_WritePtr = m_ReadPtr = m_Buffer;
	}
#ifdef _M_X64
	long lo_fpos = (long) fpos;
	long hi_fpos = fpos >> 32;			// high order 32 bits of file position
	lo_fpos = ::InternetSetFilePointer(m_NetHandle, lo_fpos, &hi_fpos, from, 0);
	if (lo_fpos >= 0)
	{
		m_filepos = (offset_t) (size_t(hi_fpos) << 32) | lo_fpos;
		return m_filepos;
	}
#else
	long lo_fpos = (long) fpos;
	long hi_fpos = 0;	// high order 32 bits of file position
	lo_fpos = ::InternetSetFilePointer(m_NetHandle, lo_fpos, &hi_fpos, from, 0);
	if (lo_fpos >= 0)
	{
		m_filepos = lo_fpos;
		return m_filepos;
	}
#endif
	DWORD err = ::GetLastError();
	VX_ERROR(("NetStream::Seek ERROR: CANNOT SEEK TO %d : error %d", seekto, err), fpos);	
}

size_t NetStream::GetPos() const
{
	if (m_NetHandle == NULL)
		return MemFile::GetPos();
	return m_filepos;
}

bool NetStream::Close()
{
	if (m_NetHandle)
	{
		::InternetCloseHandle(m_NetHandle);
		m_NetHandle = NULL;
		m_ReadPtr = m_WritePtr = m_Buffer;
	}
	return MemFile::Close();
}

bool NetStream::IsEmpty() const
{
	if (m_WritePtr > m_ReadPtr)			// still have data?
		return false;
	if (m_NetHandle)
	{
		DWORD bytes_read;
		DWORD n = (DWORD) s_BufSize;
		NetStream* cheat = (NetStream*) this;

		VX_ASSERT(m_ReadPtr == m_WritePtr);
		cheat->m_ReadPtr = cheat->m_WritePtr = m_Buffer;
		if (!::InternetReadFile(m_NetHandle, m_Buffer, n, &bytes_read) ||
			(bytes_read <= 0))
			return true;
		cheat->m_WritePtr += bytes_read;
		return false;
}
	return MemFile::IsEmpty();
}

NetStream::~NetStream()
{
	if (m_Buffer)
		delete m_Buffer;
	Close();
}

void NetStream::Shutdown()
{
	if (s_Session != NULL)
		::InternetCloseHandle(s_Session);
	s_Session = NULL; 
}


MemFile::MemFile(const TCHAR *filename) : FileStream(filename)
{
	hFile = NULL;
	hMapFile = NULL;
	lpMapAddress = NULL;
	lpAddress = NULL;
}

/*!
 * intptr MemFile::GetHandle() const
 * Provides access to the memory file HANDLE associated
 * with an open stream. If the stream has a memory file open,
 * the return value will be a non-null HANDLE represented as an integer.
 *
 * @returns non-zero HANDLE or 0 if no file open
 * @see Stream::GetHandle
 */
intptr MemFile::GetHandle() const { return (intptr) hFile; }

bool MemFile::Open(const TCHAR* filename, int mode)
{
	if (mode & (OPEN_WRITE | OPEN_SEEK))
		return FileStream::Open(filename, mode);
	else								// open for read
	{
		WIN32_FILE_ATTRIBUTE_DATA   fileInfo;

		Close();
		hFile = CreateFile(filename, 
					GENERIC_READ,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_RANDOM_ACCESS,
					NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;
		if (!GetFileAttributesEx(filename, GetFileExInfoStandard, (void*) &fileInfo))
			return false;
		m_FileSize = fileInfo.nFileSizeLow;
		hMapFile = CreateFileMapping(hFile, 
					NULL, 
					PAGE_READONLY, 
					0, 
					0, 
					NULL);
		if (hMapFile == NULL)
			return false;
		lpAddress = ((LPCH) MapViewOfFile(hMapFile,
					FILE_MAP_READ,
					0,
					0,
					0));
		if(lpAddress == NULL)
			return false;
		lpMapAddress = lpAddress;
	}
	return Stream::Open(filename, mode);
}


bool MemFile::Close()
{
	if(lpAddress!=NULL)
	{
		Flush();
		FlushViewOfFile(lpAddress, NULL);
		UnmapViewOfFile(lpAddress);
		lpAddress = NULL;
		lpMapAddress = NULL;
	}
	if (hMapFile != NULL)
	{
		CloseHandle(hMapFile);
		hMapFile = NULL;
	}
	if (hFile != NULL)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	return FileStream::Close();
}


size_t MemFile::Read(char* buffer, size_t n)
{
	VX_ASSERT(buffer);
	if (lpMapAddress != NULL && n > 0)
	{
		intptr curofs = (lpMapAddress - lpAddress);
		if (curofs + n > m_FileSize)
		{
			n = m_FileSize - curofs;
			if (n == 0)
				return 0;
		}
		memcpy(buffer, lpMapAddress, n);
		lpMapAddress += n;
		return n;
	}
	return FileStream::Read(buffer, n);
}

offset_t MemFile::Seek(offset_t seekto, int from)
{
	if (lpMapAddress)
		switch (from)
		{
			case SEEK_FROM_HERE:	lpMapAddress += seekto; break;
			case SEEK_FROM_START:	lpMapAddress = lpAddress + seekto; break;
			default:				return -1;	// cannot seek from end
		}
	return FileStream::Seek(seekto, from);		// 0 indicates success
}

size_t MemFile::GetPos() const
{
	if (lpMapAddress == NULL)
		return FileStream::GetPos();
	return lpMapAddress - lpAddress;
}

bool MemFile::IsEmpty() const
{
	if (lpAddress == NULL)
		return FileStream::IsEmpty();
	int c;
	if (lpMapAddress != NULL)
	{
		c = *((LPINT)lpMapAddress);
		if (c == 0)
			return true;
		if (c == EOF)
			return true;
	}
	return false;
}


MemFile::~MemFile()
{
	Close();
}

}	// end Core
}	// end Vixen