#pragma once

namespace Core {

/*!
 * @class MemStream
 *
 * MemStream enables you to load Vixen content from a contiguous area of memory.
 * It provides the Vixen Stream API on top of a memory block. This is useful if
 * your underlying system separates file I/O from content translation.
 *
 * @see Stream
 */
class MemStream : public Stream
{
public:
	MemStream(char* memory, intptr size);
	~MemStream();

	virtual	bool		Open(const TCHAR* name, int mode = Stream::OPEN_RW);
	virtual	bool		Close();
	virtual	size_t		Read(char* buffer, size_t nbytes);
	virtual	offset_t	Seek(offset_t seekto, int from = Stream::SEEK_FROM_START);
	virtual	size_t		GetPos() const;
	virtual	bool		IsEmpty() const;
	virtual	intptr		GetHandle() const;

protected:
	char*		m_Start;
	intptr		m_Size;
	intptr		m_CurOfs;
};


} // end Core