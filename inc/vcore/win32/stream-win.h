#pragma once

namespace Core {

/*!
 * @class MemFileStream
 *
 * MemFileStream enables you to load Vixen content from binary files. 
 * It uses file mapping which associates a file’s contents with a portion of 
 * the virtual address space of a process. The operating system creates a file-mapping object 
 * to maintain this association. A file view is the portion of virtual address space that the
 * process uses to access the file’s contents.  Processes read from and write to the file 
 * view using pointers, just as they would with dynamically allocated memory.
 *
 * File mapping provides two major advantages- Shared memory and faster file access.
 * MemFile is only available on the Windows operating system.
 *
 * @see FileStream
 */
class MemFile : public FileStream
{
public:
	MemFile(const TCHAR* filename = NULL);
	~MemFile();

	virtual	bool		Open(const TCHAR* name, int mode = Stream::OPEN_RW);
	virtual	bool		Close();
	virtual	size_t		Read(char* buffer, size_t nbytes);
	virtual	offset_t	Seek(offset_t seekto, int from = Stream::SEEK_FROM_START);
	virtual	size_t		GetPos() const;
	virtual	bool		IsEmpty() const;
	virtual	intptr		GetHandle() const;

protected:
	HANDLE		hFile;
	HANDLE		hMapFile;
	char*		lpMapAddress;
	char*		lpAddress;
	int			m_FileSize;
};

class NetStream : public MemFile
{
public:
	VX_DECLARE_CLASS(NetStream);
	NetStream(const TCHAR* filename = NULL);
	~NetStream();
	
	const TCHAR*		GetCacheFile() const	{ return m_CacheFileName; }
	virtual	intptr		GetHandle() const;
	virtual	bool		Open(const TCHAR* name, int mode = Stream::OPEN_RW);
	virtual	bool		Close();
	virtual	bool		IsEmpty() const;
	virtual	size_t		Read(char* buffer, size_t nbytes);
	virtual	offset_t	Seek(offset_t seekto, int from = Stream::SEEK_FROM_START);
	virtual	size_t		GetPos() const;
	static void			Shutdown();

protected:
	bool			NetOpen(const TCHAR* url, int mode);

	static HANDLE	s_Session;
	static size_t	s_BufSize;
	Core::String	m_CacheFileName;
	HANDLE			m_NetHandle;
	size_t			m_filepos;
	char*			m_Buffer;
	char*			m_ReadPtr;
	char*			m_WritePtr;
};

} // end Core