#include "vcore/vcore.h"


namespace Vixen {
namespace Core {

MemStream::MemStream(char* memory, intptr size) : Stream()
{
	m_Start = memory;
	m_CurOfs = 0;
	m_Size = size;
}

/*!
 * intptr MemStream::GetHandle() const
 * Provides access to the memory area associated
 * with an open stream. If the stream has attached memory,
 * the return value will be a a pointer to it.
 *
 * @returns pointer to memory area or NULL if none
 * @see Stream::GetHandle
 */
intptr MemStream::GetHandle() const { return (intptr) m_Start; }

bool MemStream::Open(const TCHAR* filename, int mode)
{
	if (mode & OPEN_WRITE)
	{	VX_ERROR(("MemStream::Open ERROR cannot open memory stream for writing"), false); }
	else if (m_Start == NULL)
		VX_ERROR(("MemStream::Open ERROR cannot open memory stream with no attached memory"), false);
	return Stream::Open(filename, mode);
}

/*!
 * bool MemStream::Close()
 * Closes the memory stream and frees the associated memory block.
 */
bool MemStream::Close()
{
	if (m_Start)
	{
		free(m_Start);
		m_Start = NULL;
	}
	m_CurOfs = 0;
	return Stream::Close();
}


size_t MemStream::Read(char* buffer, size_t n)
{
	VX_ASSERT(buffer);
	if (m_Start && (n > 0))
	{
		if (m_CurOfs + n > m_Size)
		{
			n = m_Size - m_CurOfs;
			if (n == 0)
				return 0;
		}
		memcpy(buffer, m_Start + m_CurOfs, n);
		m_CurOfs += n;
		return n;
	}
	return 0;
}

offset_t MemStream::Seek(offset_t seekto, int from)
{
	if (m_Start)
		switch (from)
		{
			case SEEK_FROM_HERE:	m_CurOfs += seekto; break;
			case SEEK_FROM_START:	m_CurOfs = seekto; break;
			default:				m_CurOfs = m_Size; break;
		}
	return -1;
}

size_t MemStream::GetPos() const
{
	if (m_Start == NULL)
		return -1;
	return m_CurOfs;
}

bool MemStream::IsEmpty() const
{
	if ((m_Start == NULL) || (m_Size == 0) || (m_CurOfs == m_Size))
		return true;
	return false;
}


MemStream::~MemStream()
{
	Close();
}

}	// end Core
}	// end Vixen