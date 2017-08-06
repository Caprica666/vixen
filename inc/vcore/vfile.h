/*!
 * @file vfile.h
 *
 * @brief Defines basic communication classes for I/O.
 *
 * @author Nola Donato
 * @ingroup vcore
 *
 * @see vstream.h vsocket.h
 */

#pragma once

namespace Core {

/*!
 * @class FileStream
 * @brief Stream to access disk file on the local machine.
 *
 * This class places no interpretation on the bytes.
 * To read and interpret a Vixen scene file, you would use FileMessenger
 *
 * @ingroup vcore
 * @see NetStream Stream
 */
class FileStream : public Stream
{
public:
	VX_DECLARE_CLASS(FileStream);

	FileStream(const TCHAR* filename = NULL);
	~FileStream();

	virtual	intptr		GetHandle() const;
	virtual bool		Open(const TCHAR* name, int mode = Stream::OPEN_RW);
	virtual bool		Close();
	virtual bool		IsEmpty() const;
	virtual size_t		Read(char* buffer, size_t nbytes);
	virtual size_t		Write(const char* buffer, size_t nbytes);
	virtual offset_t	Seek(offset_t seekto, int from = Stream::SEEK_FROM_START);
	virtual size_t		GetPos() const;
	virtual void		Flush();

protected:
	FILE*		m_stream;
};


} // end Core