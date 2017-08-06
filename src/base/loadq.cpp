#include "vixen.h"

namespace Vixen {
using namespace Core;

/****
 *
 * LoadThread encapsulates a worker thread that loads files
 * based on a queue of requests
 *
 ****/
VX_IMPLEMENT_CLASS(FileLoader, BufferQueue);

/****
 *
 * LoadRequest represents a single work item designating which file
 * to load and convert
 *
 ****/
struct LoadRequest	: public Buffer
{
	int32					FileType;		// type of file to load
	SharedObj*				Requestor;		// object which requested load
	String					FileName;		// name of file requested
	FileLoader::LoadFunc*	LoadFunc;		// load function to use
};

FileLoader::FileLoader() : BufferQueue(sizeof(LoadRequest), LOAD_NumThreads)
{
	StreamClass = CLASS_(NetStream);
	m_NumFileTypes = 0;
	m_FileTypes = NULL;
#ifndef VX_NOTHREAD
	m_QueueNum = 0;
	MakeLock();
	LoadThreads.DoExit = false;
#endif
	Debug = 1;
}

FileLoader::~FileLoader()
{
	FileInfo*	e = m_FileTypes;
	FileInfo*	temp;

	Kill();
	if (m_BufAlloc)			// have our own buffer allocator?
	{
		delete m_BufAlloc;	// free it here
		m_BufAlloc = NULL;
	}
	while (e)				// delete file types
	{
		temp = e->Next;
		delete e;
		e = temp;
	}
}

void FileLoader::Kill()
{
	m_FileDict.Empty();
#ifndef VX_NOTHREAD
	if (LoadThreads.NumThreads > 0)
	{
		LoadThreads.KillAll(true);
		LoadThreads.Empty();
	}
#endif
}

void FileLoader::SetDirectory(const TCHAR* dirname)
{
	VX_TRACE(FileLoader::Debug, ("FileLoader::SetDirectory(%s)\n", dirname));
	ObjLock	lock(this);
	m_Directory = dirname;
}

const TCHAR* FileLoader::GetDirectory() const
{
	if (m_Directory.IsEmpty())
		return NULL;
	return m_Directory;
}

/*!
 * @fn const TCHAR* FileLoader::GetPath(const TCHAR* fname, TCHAR* buf, int buflen)
 * @param fname		relative name of referenced file
 * @param buf		buffer to get filename
 * @param buflen	length of destination buffer
 *
 * If the input file name is not relative, it is copied to the
 * input buffer untouched. Relative path names are prefixed by the base
 * directory of the loader, if any.
 *
 * It is a good idea to specify a buffer to get the filename and
 * this should not be a static buffer or you will not be able to
 * run safely from more than one thread. If you omit the buffer, an
 * internal static buffer is used and the call is not re-entrant or thread-safe.
 *
 * @return pointer to fully qualified pathname (the input buffer)
 *
 * @see FileLoader::SetDirectory FileLoader::OpenStream FileLoader::Load
 */
const TCHAR* FileLoader::GetPath(const TCHAR* infile, TCHAR* buf, int buflen)
{
	static	TCHAR	fname[VX_MaxPath];
	const	TCHAR*	dir;

	if (buf == NULL)
	{
		buf = fname;
		buflen = VX_MaxPath;
	}
	if (Stream::IsRelativePath(infile) && (dir = GetDirectory()))
	{
		STRCPY(buf, dir);
		STRCAT(buf, TEXT("/"));
		STRCAT(buf, infile);
	}
	else
		STRCPY(buf, infile);
	return buf;
}

/*!
 * @fn bool FileLoader::ReadFile(const TCHAR* fname, SharedObj* loader)
 * @param filename	name of file to load.
 *
 * This function issues a blocking read that waits until the complete file has been loaded.
 * It is called from the load threads to process file load requests.
 * The appropriate load function set up for the file type is called to
 * actually read and interpret the file.
 *
 * @return true if file was loaded
 *
 * @see FileLoader::Load FileLoader::SetFileFunc
 */
bool FileLoader::ReadFile(const TCHAR* fname, SharedObj* loader)
{
	Ref<Core::Stream>	readstream = (Core::Stream*) StreamClass->CreateObject();
	TCHAR				fullpath[VX_MaxPath];
	int					eventcode = Event::LOAD_TEXT;
	const TCHAR*		dir = GetDirectory();
	const TCHAR*		start = fullpath;

	if (*fname == TEXT('#'))					// force relative path
		STRCPY(fullpath, fname + 1);			// don't prepend directory
	else
		GetPath(fname, fullpath, VX_MaxPath);	// get full path name of file
	readstream->SetDirectory(dir);				// establish directory
	/*
	 * Check to see if the file has already been loaded
	 */
	Lock();
	ObjRef*		oref = m_FileDict.Find(fullpath);
	SharedObj*	obj = NULL;
	FileFunc*	loadfunc = GetFileFunc(fname, &eventcode);
	bool		loaded = false;
	LoadEvent*	ev = NULL;

	if (loadfunc)
	{
		if (oref)
		{
			obj = (SharedObj*) *oref;
			if (obj == NULL)
			{
				Unlock();
				VX_TRACE(FileLoader::Debug, ("FileLoader::ReadFile file %s already being loaded\n", fname));
				return false;
			}
		}
		Unlock();
/*
 * Try to read an object file (image or scene). Create a file read stream,
 * open it and try to read the file. A successful read logs a file load event.
 */
		if (IsExit())
			return false;
		ev = (LoadEvent*) World3D::Get()->MakeEvent(eventcode);
		VX_ASSERT(ev);
		ev->Sender = loader;
		ev->FileName = fullpath;
		VX_TRACE(FileLoader::Debug, ("FileLoader::ReadFile opening %s\n", fullpath));
		if (readstream->Open(fullpath, Stream::OPEN_READ | Stream::OPEN_SEEK))
			loaded = (*(loadfunc))(fullpath, readstream, ev);
#if 0
		else
		{
			const TCHAR*	dir(GetDirectory());
			const TCHAR*	p = fullpath;
			size_t			n;
		
			if (dir && ((n = STRLEN(dir)) > 0) && (STRNCMP(dir, fullpath, n) != 0))
			{
				TCHAR	filename[VX_MaxPath];
				n = STRLEN(fullpath);
				p = fullpath + n;
				while (--p > fullpath)
					if ((*p == TEXT('/')) || (*p == TEXT('\\')))
					{
						GetPath(p + 1, filename, VX_MaxPath);
						VX_TRACE(FileLoader::Debug, ("FileLoader::ReadFile opening %s\n", fullpath));

						if (readstream->Open(filename, Stream::OPEN_READ | Stream::OPEN_SEEK))
							loaded = (*(loadfunc))(filename, readstream, ev);
						break;
					}
			}
		}
#endif
		if (loaded && !IsExit())
		{
			Lock();
			if (ev->Code == Event::LOAD_SCENE)
				obj = ((LoadSceneEvent*) ev)->Object;
			m_FileDict.Set(fullpath, obj);
			Unlock();
			if (ev->Sender.IsNull())
				delete ev;
			else
				ev->Log();
			VX_TRACE(FileLoader::Debug, ("FileLoader::ReadFile complete %s\n", fullpath));
		}
		else
		{
			ErrorEvent* err = new ErrorEvent(loader);
			delete ev;
			err->ErrString = TEXT("FILE OPEN FAILED ");
			err->ErrString += fullpath;
			err->ErrLevel = 2;
			err->Log();
			VX_ERROR(("FileLoader::ReadFile cannot open file %s\n", fullpath), false);
		}
	}
/*
 * Create a file read stream, open it and try to read a text file.
 * A successful read logs a load file event.
 */
	else
	{
		const TCHAR* text = NULL;
		Unlock();
		VX_TRACE(FileLoader::Debug, ("FileLoader::ReadFile opening %s\n", fullpath));
		if (readstream->Open(fullpath, Stream::OPEN_READ) && !IsExit())
		{
			LoadTextEvent* ev = new LoadTextEvent();
			ev->Sender = loader;
			ev->FileName = fname; 
			ReadText(fname, readstream, ev);
			delete ev;
		}
		else
		   { VX_ERROR(("FileLoader::ReadFile cannot open file %s\n", fullpath), false); }
	}
	return true;
}

/*!
 * @fn Core::Stream* FileLoader::OpenStream(const TCHAR* name)
 * @param name	name of URL or file associated with stream
 * @param opts	stream open options (STREAM_Read, STREAM_Write)
 *
 * The stream will be open upon return. If the filename
 * is relative, the loader base directory is prepended
 * to derive a full pathname.
 *
 * @return pointer to stream created or NULL if file could not be opened
 *
 * @see FileLoader::SetDirectory FileLoader::Load Core::FileStream
 */
Core::Stream* FileLoader::OpenStream(const TCHAR* name, int opts)
{
	NameProp		fname(name);
	TCHAR			pathbuf[VX_MaxPath];
	Core::Stream*	instream = (Core::Stream*) StreamClass->CreateObject();

	VX_ASSERT(instream->IsKindOf(CLASS_(Core::Stream)));
	instream->SetDirectory(GetDirectory());
	name = instream->GetPath(name, pathbuf, VX_MaxPath);
	if (instream->Open(name, opts))
		return instream;
	instream->Delete();
	return NULL;
}

/*!
 * @fn FileLoader::FileFunc* FileLoader::GetFileFunc(const TCHAR* filename,int* code) const
 * @param filename	name of file
 * @param code		pointer to where to store event code for loading (may be NULL)
 *
 * Looks at the file extension and returns the appropriate load function
 * for this type of file. The empty loader supports only text file reads.
 *
 * @return function to load the input file or NULL if extension not supported
 *
 * @see FileLoader::SetFileFunc FileLoader::Load
 */
FileLoader::FileFunc* FileLoader::GetFileFunc(const TCHAR* filename, int* code) const
{
	TCHAR*		p = (TCHAR*) filename + STRLEN(filename);
	FileInfo*	e = m_FileTypes;

	while (*--p != '.')						// find file extension
		if (p == filename)					// first dot from the end
			return NULL;					// no extension, error
	++p;
	while (e)								// look for extension in our table
		if (STRCASECMP(e->FileExt, p) == 0)
		{
			if (code)
				*code = e->EventCode;
			return e->Func;
		}
		else
			e = e->Next;
	return NULL;
}

/*!
 * @fn void FileLoader::SetFileFunc(const TCHAR* ext, FileLoader::FileFunc* func, int code)
 * @param ext	String with 3 letter file extension
 * @param func	Pointer to a function to load the file.
 * @param code	event code for loading: Event::LOAD_SCENE, Event::LOAD_TEXT, Event::LOAD_DATA
 *
 * Looks at the file extension and returns the appropriate load function
 * for this type of file. The default implementation supports loading of
 * text files only.
 *
 * Your load function should be declared like this:
 * @code
 *		bool my_load_func(const TCHAR* filename, Core::Stream* stream, LoadEvent* event)
 * @endcode
 *
 * @see FileLoader::Load FileLoader::GetLoadFunc
 */
void FileLoader::SetFileFunc(const TCHAR* ext, FileLoader::FileFunc* func, int code)
{
	FileInfo*	e = m_FileTypes;

	while (e)
		if (STRCASECMP(e->FileExt, ext) == 0)
		{
			e->Func = func;
			return;
		}
		else
			e = e->Next;
	e = new FileInfo(ext, func, code);
	e->Next = m_FileTypes;
	m_FileTypes = e;
}


/*!
 * @fn bool FileLoader::ReadText(const TCHAR* filename, Core::Stream* stream, LoadEvent* ev)
 * @param filename	name of scene file to load.
 * @param stream	stream to use for reading.
 * @param ev		event to initialize if load is successful.
 *
 * Called by the loader to load a text file.
 * ReadText will issue a blocking read that waits until all
 * of the file has loaded. To load a text file without blocking,
 * call FileLoader::Load.
 *
 * The load event provided on input will be logged every time
 * an internal buffer of text has been processed. Small files
 * will only generate a single load event but large ones may log
 * multiple events. These will come in sequential order and can
 * be used to reconstruct the entire text file.
 *
 * @return \b true if load was successful, else \b false
 *
 * @see FileLoader::Load FileLoader::ReadFile
 */
bool FileLoader::ReadText(const TCHAR* filename, Core::Stream* stream, LoadEvent* e)
{
	size_t	maxbufsize = BufMessenger::MaxBufSize - sizeof(LoadEvent) - 8; // 8 is slop factor
	char	readbuf[MESS_MaxBufSize];	// internal buffer size
	size_t	nread = 0;					// number of characters read into buffer
	int		seqid = 1;
	LoadTextEvent*	ev = (LoadTextEvent*) e;

	ev->Code = Event::LOAD_TEXT;
/*
 * Fast check to see if file is small and fits in a single buffer
 * or if data cannot be read
 */
	nread = ev->FileName.GetLength();
	maxbufsize -= nread;
	VX_ASSERT(stream);
	if (stream->IsEmpty() ||
		(nread = stream->Read(readbuf, maxbufsize)) <= 0)
		return false;
	readbuf[nread] = 0;
	if (nread < maxbufsize)				// entire file fits in the buffer?
	{
		ev->Text = (const TCHAR*) readbuf;
		ev->SequenceID = 0;
		*GetMessenger() << *ev;			// log but don't delete event
		return true;
	}
/*
 * File is longer than our buffer. Copy one line at a time into the event
 * text area until the maximum buffer size is exhausted.
 * Then read another buffer from the file.
 */
	TCHAR	writebuf[MESS_MaxBufSize];	// internal buffer size
	TCHAR*	outptr = writebuf;
	size_t	nwritten = 0;				// number characters written to output buffer
	LoadTextEvent* newevent;

	do
	{
		TCHAR* line = (TCHAR*) readbuf;
		size_t	n = nread;
		TCHAR* p = (TCHAR*) (readbuf + nread);
		*p = 0;
		readbuf[nread] = 0;
		ev->Text += (TCHAR*) readbuf;
		while (p = STRCHR(line, TEXT('\n')))// add newlines until read buffer exhausted
		{
			n = p - line + 1;				// number of bytes in line
			if (nwritten + n > maxbufsize)	// will this line fit?
			{
				newevent = new LoadTextEvent(*ev);
				newevent->Text = writebuf;	// add to event
				newevent->SequenceID = seqid++;
				newevent->Log();			// log the event
				nwritten = 0;
				outptr = writebuf;			// reuse output buffer
			}
			STRNCPY(outptr, line, n);		// copy line into output buffer
			outptr += n;
			*outptr = 0;
			nwritten += n;					// bump amount saved
			line = p + 1;					// start of next line
		}
		STRCPY(outptr, line);				// copy fragment into output buffer
		n = STRLEN(line);
		outptr += n;						// advance output ptrs
		nwritten += n;
	}
	while (!stream->IsEmpty() &&
		   (nread = stream->Read(readbuf, maxbufsize)) > 0);
	if (nwritten == 0)						// add extra newline
	{
		*outptr++ = TEXT('\n');				// if on buffer boundary
		*outptr = 0;
	}
	newevent = new LoadTextEvent(*ev);
	newevent->Text = writebuf;				// add to event
	newevent->SequenceID = -seqid;			// negative indicates end of file
	newevent->Log();						// log the event
	return true;
}

 /*!
 * @fn bool FileLoader::ReadBinary(const TCHAR* filename, Core::Stream* stream, LoadEvent* ev)
 * @param filename	name of binary file to load.
 * @param stream	stream to use for reading.
 * @param ev		event to initialize if load is successful.
 *
 * Called by the loader to load a binary file.
 * ReadBinary will issue a blocking read that waits until all
 * of the file has loaded. To load a text file without blocking,
 * call FileLoader::Load.
 *
 * The load event provided on input will be logged once and will contain
 * a pointer to a global memory area containing the data read.
 * To consume this buffer, set the pointer in the event to NULL and
 * use free to release the memory when you are done. Otherwise, you
 * must copy the data before returning from OnEvent as the system
 * will delete the data area in the event upon return.
 *
 * @return \b true if load was successful, else \b false
 *
 * @see FileLoader::Load FileLoader::ReadFile SharedObj::OnEvent
 */
bool FileLoader::ReadBinary(const TCHAR* filename, Core::Stream* stream, LoadEvent* e)
{
	char	readbuf[MESS_MaxBufSize];	// internal buffer size
	size_t	nread = 0;					// number of characters read into buffer
	size_t	total = 0;
	char*	data = NULL;
	LoadDataEvent* ev = (LoadDataEvent*) e;

	ev->Code = Event::LOAD_DATA;
	ev->Length = 0;
	ev->Data = 0;
/*
 * Fast check to see if file is small and fits in a single buffer
 * or if data cannot be read
 */
	VX_ASSERT(stream);
	while (!stream->IsEmpty())
	{
		char* newdata;
		char* data = (char*) ev->Data;

		if ((nread = stream->Read(readbuf, MESS_MaxBufSize)) <= 0)
			break;
		total += nread;
		newdata = (char*) GlobalAllocator::Get()->Alloc(total);
		if (newdata == NULL)
			VX_ERROR(("ReadBinary: ERROR out of memory\n"), false);
		if (data != NULL)
		{
			memcpy(newdata, data, ev->Length);
			GlobalAllocator::Get()->Free(data);
		}
		memcpy(newdata + ev->Length, readbuf, nread);
		ev->Data = (intptr) newdata;
		ev->Length += nread;
	}
	VX_TRACE(FileLoader::Debug, ("Loader::Load of binary data complete %s\n", filename));
	return true;
}

/*!
 * @fn void FileLoader::Unload(const TCHAR* filename)
 * @param filename	name of file to unload.
 *
 * When the loader loads a file, it caches the file objects in
 * memory so they can be reused if the file is loaded again.
 * This function clears any in-memory representation of the
 * file contents.
 *
 * @see FileLoader::Load
 */
void FileLoader::Unload(const TCHAR* filename)
{
	ObjLock lock(this);
	m_FileDict.Remove(NameProp(filename));
}

#ifdef VX_NOTHREAD
/*!
 * @fn bool FileLoader::Load(const TCHAR* name, SharedObj* requestor, FileLoader::LoadFunc* func)
 * @param name	Name of the object to load, passed to load function
 * @param requestor	Object which initiated the load and will observe any load events.
 *					if NULL, the world is used.
 * @param func	Function used to open and read the file. If NULL, default ReadFile function used.
 *
 * Causes a file to be loaded asynchronously by putting a request
 * onto the load thread's work queue. When a load thread becomes
 * available, it will process the load request, updating the file
 * dictionary with the new file. Thus a scene or texture file will be
 * loaded only once even if it is used multiple times in the scene.
 *
 * The public member \b FileLoader::DoAsyncLoad enables and disables asynchronous
 * file loading. If it is \b true, the first call to this function will start
 * the load queue and it will be used for asynchronous loading. Setting it to
 * \b false suppresses asynchronous loading. In this case, this function will
 * not return until the file has been loaded and will not use load queue threads
 * to read the file.
 *
 * The default file loader understands how to load image files (textures) and
 * scene content files (.VIX). You can call FileLoader::SetFileFunc to provide
 * support for loading other types of files.
 *
 * @return \b true if load was successful. If DoAsyncLoad is \b false, the return code
 *	indicates whether the file was actually read. Otherwise, it will only
 *	be \b false if the input data was invalid.
 *
 * @see SceneLoader FileLoader::Load FileLoader::GetFileFunc FileLoader::Unload
 */
bool FileLoader::Load(const TCHAR* filename, SharedObj* requestor, FileLoader::LoadFunc* func)
{
	if ((filename == NULL) || (*filename == 0))
		return false;
	if (requestor == NULL)
		requestor = World::Get();
	if (func == (LoadFunc*) NULL)
		return ReadFile(filename, requestor) != 0;
	return (*func)(filename, requestor);
}

#else


class LoadThread : public Thread
{
public:
	LoadThread(FileLoader* irq, int qnum = 0) : Thread(SCENE_LoadThread)
		{ m_Queue = irq; QueueID = qnum; m_ResumeEvent.TimeOut = VX_TimeOut * 60; }

	void Run(ThreadFunc* func = NULL)
		{ Thread::Run(func ? func : ThreadFunc); }

	static ThreadFunc	ThreadFunc;
	int					QueueID;

protected:
	static FileLoader::LoadFunc* GetLoadFunc(const TCHAR* filename);

	FileLoader*	m_Queue;
};

bool FileLoader::Load(const TCHAR* filename, SharedObj* requestor, FileLoader::LoadFunc* func)
{
	if ((filename == NULL) || (*filename == 0))
		return false;
	if (!DoAsyncLoad)
	{
		if (func == NULL)
			return ReadFile(filename, requestor) != NULL;
		return (*func)(filename, requestor);
	}
/*
 * Asynchronous file loads. Make thread pool if it does not already exist
 * and add a request to load the file to the next load queue (round robin style).
 */
	bool resume = true;
	if (LoadThreads.GetNumThreads() == 0)
	{
		if (MakeThreads())
			resume = false;
		else
			return false;
	}
	Lock();
	LoadRequest* req = (LoadRequest*) NewBuffer();
	if (req == NULL)
	{
		Unlock();
		return false;
	}
	req->Requestor = requestor;
	new ((void*) &(req->FileName)) String;
	req->Queue = m_QueueNum++ % GetNumQueues();
	req->FileName = filename;
	req->State = BUF_Ready;
	req->LoadFunc = func;
	VX_TRACE2(FileLoader::Debug, ("Loader::Load queueing %s on %d\n", filename, req->Queue));
	Submit(req);
	Unlock();
	if (resume)
		LoadThreads.ResumeAll();
	else
		LoadThreads.RunAll(LoadThread::ThreadFunc);
	return true;
}


/****
 *
 * LoadQueue initialization
 * Create the image read threads suspended
 *
 ****/
bool FileLoader::MakeThreads()
{
	ObjectLock	lock(this);
	if (LoadThreads.GetNumThreads() > 0)
		return false;
	if (m_BufAlloc == NULL)
	{
		m_BufAlloc = new BytePool(sizeof(LoadRequest), 4096);
		m_BufAlloc->SetOptions(ALLOC_Lock);
	}
	LoadThreads.DoExit = false;
	for (int i = 0; i < LOAD_NumThreads; ++i)
	{
		LoadThread* thread = new LoadThread(this);
		if (thread == NULL)
		{
			VX_ERROR(("Loader::MakeThreads - cannot make thread #%d\n", i), false);
		}
		thread->QueueID = i;
		LoadThreads.Add(thread);
	}
	return true;
}

/****
 *
 * LoadThread::ThreadFunc
 * Processes the image request queue bucket with the given ID (0 to LOAD_NumThreads-1)
 * Loads and converts the texture file indicated by the image request and returns the
 * item back to the free list for reuse. If LoadThread::DoExit is set, the thread will exit after
 * the work item and delete itself
 *
 ****/
#ifdef VX_PTHREAD
void *LoadThread::ThreadFunc( void *arg )
#else
void LoadThread::ThreadFunc( void *arg )
#endif
{
	LoadThread*		thread = (LoadThread*) arg;
	FileLoader*		lq = thread->m_Queue;
	TLSData*		tls = TLSData::Get();
	LoadRequest*	req;
	bool			exiting = false;

	do
	{
		while (req = (LoadRequest*) lq->Process(thread->QueueID))
		{
			if (lq->LoadThreads.DoExit)
				break;
			VX_TRACE2(FileLoader::Debug, ("Loader::Load processing %s on %d\n", (const TCHAR*) req->FileName, thread->QueueID));
			if (req->LoadFunc == NULL)
				lq->ReadFile(req->FileName, req->Requestor);
			else
				(*(req->LoadFunc))(req->FileName, req->Requestor);
			lq->Free(req);
		}
	}
	while (thread->Suspend() && !(exiting = lq->LoadThreads.DoExit != 0));
	InterlockDec(&(lq->LoadThreads.NumThreads));
	thread->Stop();
	VX_TRACE(FileLoader::Debug, ("Loader thread for queue #%d stopped\n", thread->QueueID));
	World3D::Get()->OnThreadExit(SCENE_LoadThread);
	if (!exiting)
	{
		lq->Lock();
		if (lq->LoadThreads.NumThreads == 0)
			lq->LoadThreads.Empty();
		lq->Unlock();
	}
#if !defined(_WIN32) || defined(VX_PTHREAD)
	return NULL;
#endif
}
#endif

}	// end Vixen
