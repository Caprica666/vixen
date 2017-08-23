#include "vixen.h"

namespace Vixen {

static const TCHAR* opnames[] =
{
	TEXT("SetFileName"), TEXT("Run"), TEXT("Stop"),
};

const TCHAR** World::DoNames = opnames;
Core::Allocator* World::s_ObjectAllocator = NULL;

VX_IMPLEMENT_CLASSID(World, SharedObj, VX_World);

#ifndef VX_NOTHREAD
VX_IMPLEMENT_CLASS(Core::BytePool, Core::FixedLenAllocator);
#endif

World*		World::s_OnlyOne = NULL;			// singleton
bool		World::DoAsyncLoad = true;	// enable async loading
bool		World::MakeConsole = false;	// don't make console window on startup

/*!
 * @fn World::World()
 *
 * Creates the only instance of a world. This class is a singleton
 * and will not permit multiple instances to be created.
 * Upon construction of the world, the system startup routine
 * is called to initialize the memory management subsystem.
 *
 * @see World::Get
 */
World::World() : DebugOutput(vixen_debug)
{
//	if (Startup())
//		atexit(World::Shutdown);
	Startup();
	VX_ASSERT(s_OnlyOne == NULL);
	s_OnlyOne = this;
	m_Argc = 0;
	m_Argv[0] = NULL;
	m_RunArg = 0;
	m_MediaDir[0] = 0;
	m_FileName[0] = 0;
	m_LoadQueue = NULL;
	StartTime = Core::GetTime();
}

World::~World()
{
	m_Messenger->t_NoLog = true;		// disable transaction logging
	m_LoadQueue = (FileLoader*) NULL;
	m_Messenger = (Messenger*) NULL;
	s_OnlyOne = NULL;
	//if (GetAllocator() == NULL)			// not allocated on the heap?
	//	Shutdown();
	//else
	//	Core::TLSData::Free();			// reclaim all memory pools
}

/*!
 * @fn bool World::OnInit()
 *
 * This routine will be called once at startup, before the
 * communication between machines is established and before
 * threads are spawned. The default implementation
 * @li creates the messenger for the world
 * @li creates the file loader
 * @li sets up event observations
 *
 * @return \b true if initialization was successful, else \b false
 *
 * @see World::World World::Startup Messenger FileLoader
 */
bool World::OnInit()
{
	CLASS_(Event)->SetAllocator(&World::GetEventAlloc);
	Event::SetName(0, TEXT("None"));
	Event::SetDataTypes(0, NULL);
	Event::SetName(Event::LOAD_TEXT, TEXT("LoadText"));
	Event::SetName(Event::LOAD_IMAGE, TEXT("LoadImage"));
	Event::SetName(Event::LOAD_DATA, TEXT("LoadData"));
	Event::SetDataTypes(Event::LOAD_TEXT, TEXT("SSI"));
	Event::SetDataTypes(Event::LOAD_IMAGE, TEXT("SO"));
	Event::SetDataTypes(Event::LOAD_DATA, TEXT("SPP"));

	if (m_Messenger.IsNull())
		m_Messenger = new Messenger;
	else
		m_Messenger->Open(NULL, Messenger::OPEN_RW);
	if (m_LoadQueue == NULL)
		m_LoadQueue = new FileLoader;
	if (GetMediaDir())
		m_LoadQueue->SetDirectory(GetMediaDir());
	return true;
}


Core::Allocator* World::GetEventAlloc()
{
	Core::TLSData*		tlsptr = Core::TLSData::Get();
	Core::Allocator*	eventalloc = tlsptr->EventAlloc;

	if (eventalloc)
		return eventalloc;
	eventalloc = new (Core::ThreadAllocator::Get()) Core::FixedLenAllocator(Event::MAX_BYTES);
	tlsptr->EventAlloc = eventalloc;
	return eventalloc;
}

void World::Stop()
{
	OnExit();
}

void World::OnThreadExit(int threadtype)
{

}

/*!
 * @fn void World::OnExit()
 *
 * This function is called after all rendering threads have been shut down
 * to clean up the global data allocated by the application. If you override
 * this function to perform application-specific cleanup, you must call the
 * base class.
 *
 * @see World::OnInit World::Run
 */
void World::OnExit()
{
	Messenger*	mess = m_Messenger;
	FileLoader*	loader = m_LoadQueue;

	m_RunArg = 0;
	if (loader != (const FileLoader*) NULL)
	{
		loader->Kill();						// kill loader threads
		loader->Delete();
		m_LoadQueue = (FileLoader*) NULL;
	}
	Core::CritSec::DoLock = false;			// disable locking
	if (mess)
	{
		mess->t_NoLog = true;				// disable transaction logging
		mess->Ignore(NULL, 0, NULL);		// disconnect event observers
		mess->Close();
		m_Messenger = (Messenger*) NULL;
	}
	KillLock();
}


/*!
 * @fn float World::GetTime() const
 *
 * Returns the number of seconds since World::StartTime. By default,
 * this is the elapsed time since the application started.
 *
 * @see Scene::GetTime
 */
float World::GetTime() const
{
	double	time = Core::GetTime();
	float	t = (float) (time - StartTime);
	return t;
}

/*!
 * @fn bool World::ParseArgs(const TCHAR* commandline)
 *
 * Hook to permit application-specific argument parsing.
 * Default behavior is to parse the first argument as the
 * name of the file to open and store it as the application's
 * current file name. The zeroth argument is an empty string
 * on Windows and the name of the application on Linux.
 *
 * @return \b true indicates default parsing wanted,
 *			\b false to suppress default parsing
 *
 * @see World::SetFileName
 */
bool World::ParseArgs(const TCHAR* commandline)
{
	static TCHAR*	temp = (TCHAR*) TEXT("unknown");
	const TCHAR*	p = commandline;
	TCHAR*			q = m_ArgBuf;
	int				c;

	m_Argc = 1;
	m_Argv[0] = temp;
	m_Argv[1] = q;
	while (*q++ = (c = *p++))
		if ((c == ' ') || (c == '\t'))
		{
			*(q - 1) = 0;
			while ((*p == ' ') || (*p == '\t'))
				++p;
			m_Argv[++m_Argc] = q;
		}
	m_Argv[++m_Argc] = NULL;

	return ParseArgs(m_Argc, m_Argv);
}

/*!
 * @fn bool World::ParseArgs(int argc, TCHAR **argv)
 *
 * Hook to permit application-specific argument parsing.
 * Default behavior is to parse the first argument as the
 * name of the file to open and store it as the application's
 * current file name. The zeroth argument is an empty string
 * on Windows and the name of the application on Linux.
 *
 * @return	\b true indicates default parsing wanted,
 *			\b false to suppress default parsing
 *
 * @see World::SetFileName
 */
bool World::ParseArgs(int argc, TCHAR **argv)
{
	for (int i = 1; i < argc; ++i)
	{
		const TCHAR* p = argv[i];

		if ((p == NULL) || (*p == 0))
			continue;
		if (*p == TEXT('-'))
		{
			if (STRCMP(p, TEXT("-debug")) == 0)
				Debug = 2;
			else if (STRCASECMP(p, TEXT("-console")) == 0)
				MakeConsole = true;
			else if (STRCASECMP(p, TEXT("-media")) == 0)
			{
				if (++i >= argc)
					break;
				p = argv[i];
				if ((*p != 0) && (*p != TEXT('-')))
					SetMediaDir(p);
			}
		}
		else
		{
			if (i == (argc - 1))
				SetFileName(p);
			return true;
		}
	}
	return true;
}


/*!
 * @fn void World::SetFileName(const TCHAR* filename)
 *
 * The intial state of the application filename is the
 * first command line argument. The application may
 * also explicitly set the filename to use, overriding
 * the command line arguments. The application filename
 * specifies the file loaded on startup if no filename
 * argument is provided.
 *
 * The directory containing the input file is established as the
 * base directory for all subsequent file loads. Relative path names
 * are prepended with this directory to compose the fully qualified
 * pathnames for files loaded.
 *
 * @see World::ParseArgs FileLoader::SetDirectory
 */
void World::SetFileName(const TCHAR* filename)
{
	VX_STREAM_BEGIN(s)
		if (filename)
			*s << OP(VX_World, WORLD_SetFileName) << this << filename;
	VX_STREAM_END(  );


	if (filename == NULL)
		return;
	STRCPY(m_FileName, filename);
	VX_TRACE(FileLoader::Debug, ("World::SetFileName(%s)\n", filename));
	if (m_LoadQueue && (m_LoadQueue->GetDirectory() == NULL))
	{
		TCHAR	dirbuf[VX_MaxPath];
		TCHAR	filebuf[VX_MaxPath];

		Core::Stream::ParseDirectory(filename, filebuf, dirbuf);
		if (*dirbuf && !Core::Stream::IsRelativePath(dirbuf))
		{
			const TCHAR* ext = STRRCHR(filename, TEXT('.'));
			if (ext)
				STRCAT(filebuf, ext);
			if (m_MediaDir[0] == 0)
				SetMediaDir(dirbuf);
			m_LoadQueue->SetDirectory(dirbuf);
			STRCPY(m_FileName, filebuf);
		}
	}
}



/*!
 * @fn  void World::SetMediaDir(const TCHAR* name)
 * @param filename	fully qualified name of media directory
 *
 * This is the name of the directory containing media such as
 * texture bitmaps, video files, etc. If supplied, this directory
 * is used as a prefix for relative path names. Otherwise,
 * the base directory is defaulted based on the file type.
 * If a .VIX file is loaded, the default is the directory containing the
 * loaded file. Otherwise, the default to the directory from
 * which the application was run (current directory or web page).
 *
 * @see Texture::Load Core::Stream::SetDirectory
 */
void World::SetMediaDir(const TCHAR* name)
{
	VX_STREAM_BEGIN(s)
		if (name)
			*s << OP(VX_World, WORLD_SetMediaDir) << this << name;
	VX_STREAM_END(  );

	FileLoader* loader = GetLoader();

	if ((name == NULL) || (*name == 0))
		*m_MediaDir = 0;
	else
	{
		STRCPY(m_MediaDir, name);
		if (loader && (loader->GetDirectory() == NULL))
			loader->SetDirectory(m_MediaDir);
	}
}


/*!
 * @fn  bool World::GetMediaPath(const TCHAR* srcfile, TCHAR* dstpath, int maxlen)
 * @param srcfile	path to source file (may be absolute or relative)
 * @param dstpath	destination path, absolute if possible
 * @param maxlen	length of destination buffer
 *
 * If the input source path is absolute, it is simply copied into
 * the destination directory. If it is relative, the media directory
 * (see SetMediaDir) is prepended to make an absolute path.
 * If the media directory has not been set, the path returned
 * will still be relative (to the current directory or web page).
 *
 * @see SetMediaDir
 */
bool World::GetMediaPath(const TCHAR* srcfile, TCHAR* dstpath, int maxlen) const
{
	FileLoader* loader = GetLoader();
	if (loader)
	{
		loader->GetPath(srcfile, dstpath, maxlen);
		return true;
	}
	else
	{
		STRCPY(dstpath, srcfile);
		return false;
	}
}
/*!
 * @fn bool World::LoadAsync(const TCHAR* fname, SharedObj* observer)
 *	@param filename	Name of the file to load.
 *	@param observer	Object which initiated the load and will observe the result.
 *
 * Causes a file to be loaded asynchronously by putting a request
 * onto the load thread's work queue. When a load thread becomes
 * available, it will process the load request, updating the file
 * dictionary with the new file.
 *
 * The public member World::DoAsyncLoad enables and disables asynchronous
 * file loading. If it is \b true, the first call to this function will start
 * the load queue and it will be used for asynchronous loading. Setting it to
 * \b false suppresses asynchronous loading. In this case, this function will
 * not return until the file has been loaded and will not use load queue threads
 * to read the file.
 *
 * The default file loader is set up to load bitmaps and .VIX files.
 * You can call FileLoader::SetFileFunc to provide support for additional file types.
 *
 * @return \b true if load was successful. If DoAsyncLoad is \b false,
 *	the return code indicates whether the file was actually read.
 *	Otherwise, it will only be \b false if the input data was invalid.
 *
 * @see FileLoader::SetFileFunc FileLoader::Load World::Unload
 */
bool World::LoadAsync(const TCHAR* fname, SharedObj* observer)
{
	if (m_LoadQueue == (const FileLoader*) NULL)
		return false;
	m_LoadQueue->DoAsyncLoad = DoAsyncLoad;
	return m_LoadQueue->Load(fname, observer);
}

/*!
 * @fn bool World::Run(Window arg)
 * @param arg	application-specific argument
 *
 * The default implementation saves the Window handle.
 * Usually, subclasses override this routine to spawn their own threads.
 * This routine is not automatically called by the system, it must be initiated
 * by application code.
 *
 * @return \b true if initial file loaded, else \b false
 *
 * @see World::Stop World::OnInit World::SetFileName World::ParseArgs
 */
bool World::Run(Window arg)
{
	m_RunArg = (void*) arg;
	if (MakeConsole)
		Core::MakeConsole();
	return true;
}


/*!
 * @fn void World::Unload(const TCHAR* filename)
 * @param filename	name of file to unload.
 *
 * Unloads the memory representation of the given file.
 * When system loads a file, it sometimes caches the file objects in
 * memory so they can be reused. This function frees the cached objects
 * of the file which are not being currently used.
 *
 * @see FileLoader World::LoadAsync
 */
void World::Unload(const TCHAR* filename)
{
	m_LoadQueue->Unload(filename);
}

/*!
 * @fn Event* World::MakeEvent(int code)
 * @param code code for event to make
 *
 * Class factory for all the events handled by this application.
 * When an event is producedt, its code and data
 * are logged by the current messenger and later dispatched to all
 * the observers of that event code. This function creates an
 * empty event class for the input event code. If you define
 * your own event class and override any functions, you must also
 * override MakeEvent to ensure that the messenger can construct the right
 * event class to handle your new event code. The default implementation
 * only makes Event but no subclasses.
 *
 * @return pointer to event constructed or NULL on error
 *
 * @see Messenger::Observe SharedObj::OnEvent Event Event
 */
Event* World::MakeEvent(int code)
{
	if ((code == Event::LOAD_TEXT) || (code == Event::LOAD_IMAGE))
		return new LoadEvent(code);
	return new Event(code);
}


int World::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	if (h > 0)
	{
		const TCHAR* name;

		name = GetMediaDir();
		if (name && *name)
			s << OP(VX_World, WORLD_SetMediaDir) << h << name;
		name = GetFileName();
		if (name && *name)
			s << OP(VX_World, WORLD_SetFileName) << h << name;
	}
	return h;
}

bool World::Do(Messenger& s, int op)
{
	switch (op)
	{
		case WORLD_SetFileName:
		s >> m_FileName;
		break;

		case WORLD_SetMediaDir:
		s >> m_MediaDir;
		break;

//		case WORLD_Run:
//		s >> n;
//		Run(n);
//		break;

		case WORLD_Stop:
		Stop();
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << World::DoNames[op - WORLD_SetFileName]
					   << " " << this);
#endif
	return true;
}

}	// end Vixen