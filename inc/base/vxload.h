#pragma once

namespace Vixen {

#ifdef VX_NOTHREAD
#define	LOAD_NumThreads	1
#else
#define	LOAD_NumThreads	1	// Nola 2013 changed because Havok loading not thread safe
#endif

class LoadEvent;
class LoadTextEvent;
class LoadDataEvent;

/*!
 * @class FileLoader
 * @brief Asynchronous file I/O manager that controls reading of files.
 *
 * The loader allows you to independently specify a load
 * function for individual file extentions. It manages a pool of worker
 * threads which perform the actual file reads by calling these load
 * functions.
 *
 * When instantiated, the loader spawns LOAD_NumThreads load threads.
 * When a file is loaded asynchronously, the load request is put into the
 * load queue to be completed later by one of the load threads.
 * Each work item in the queue is a request to load and convert a file.
 *
 * @see World::LoadAsync Texture::Load
 */
class FileLoader : public Core::BufferQueue
{
public:
	typedef bool FileFunc(const TCHAR* filename, Core::Stream* stream, LoadEvent* event);
	typedef bool LoadFunc(const TCHAR* filename, SharedObj* requestor);

	friend class LoadThread;

	VX_DECLARE_CLASS(FileLoader);
//! Construct loader, no threads spawned here.
	FileLoader();
//!	Destruct loader, shut down all threads and destroy dictionaries.
	~FileLoader();
//!	Make a new stream to read the input file.
	Core::Stream*	OpenStream(const TCHAR* filename, int opts = Core::Stream::OPEN_READ);
//! Establish load function for a file extension. 
	void			SetFileFunc(const TCHAR* ext, FileFunc* func, int code);
//!	Return the file load function for this file (based on extension)
	FileFunc*		GetFileFunc(const TCHAR* filename, int* code = NULL) const;
//! Establish base directory for relative pathnames.
	void			SetDirectory(const TCHAR* dirname);
//! Get base directory prepended to relative pathnames.
	const TCHAR*	GetDirectory() const;
//! Get full pathname of a file.
	const TCHAR*	GetPath(const TCHAR* infile, TCHAR* destbuf, int buflen);
//! Asynchronously load the given file and dispatch event to sender.
	virtual bool	Load(const TCHAR* filename, SharedObj* sender = NULL, LoadFunc* func = NULL);
//! Unload a file, freeing any in-memory representation.
	virtual void	Unload(const TCHAR* filename);
//! Get file dictionary (internal).
	NameTable*		GetFileDict()	{ return &m_FileDict; }
//! Shut down load processing
	virtual void	Kill();
//!	Read a file and send an event to the observer.
	virtual	bool	ReadFile(const TCHAR* fname, SharedObj* observer);
//!	Read a text file and send one or more events to the observer.
	static	bool	ReadText(const TCHAR* fname, Core::Stream* stream, LoadEvent* event);
//!	Read a binary file and send an eventwith the data to the observer.
	static	bool	ReadBinary(const TCHAR* fname, Core::Stream* stream, LoadEvent* event);

	Core::Class*	StreamClass;	//! stream class to use for reading files, defaults to FileStream
	bool			DoAsyncLoad;	//! if \b true, do asynchronous loads, \b false for blocking reads

protected:	
	// associates a file load function with an extension
	struct FileInfo
	{
		FileInfo(const TCHAR* ext, FileFunc* f, int code)
			{ Next = NULL; STRNCPY(FileExt, ext, 4); Func = f; EventCode = code; }
		FileInfo*	Next;
		TCHAR		FileExt[4];
		FileFunc*	Func;
		int			EventCode;
	};


	NameTable		m_FileDict;			// file dictionary
	Core::String	m_Directory;		// directory for relative paths
	int32			m_NumFileTypes;		// number of file types established
	FileInfo*		m_FileTypes;		// file types and load functions

#ifdef VX_NOTHREAD
	bool			IsExit()	{ return false; }
#else
	bool			MakeThreads();		// Spawn load threads.
	bool			IsExit()	{ return LoadThreads.DoExit != 0; }
	int32			m_QueueNum;			// queue index counter
	Core::ThreadPool LoadThreads;		// thread pool for file loading
#endif
};

} // end Vixen