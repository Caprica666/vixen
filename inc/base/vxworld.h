/*!
 * @file vxworld.h
 *
 * @brief Defines a singleton container base class for distributed
 * object management.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 */
#pragma once

namespace Vixen {

class FileLoader;

/*!
 * @class World
 * @brief Base class for generic world controllers to load files
 * and asynchronously respond to events from communication.
 *
 * This base class encapsulates the platform-independent functionality to handle
 * file loading, event processing and task management.
 *
 * Each process has only one world that acts as a communication controller.
 * Operating system input is directed toward specific \b objects in the world.
 * The world handles the loading of an initial file and can be requested
 * to asynchronously load other files. It maintains a pool
 * of I/O handling threads that can obtain content from local disk files,
 * the Internet or another machine on a local network.
 *
 * In the distributed environment, one world is distributed across multiple
 * processors, each running on a separate machine and communicating changes
 * via a network. In this situation, the world maintains a set of communication
 * \b messengers that manage object updates on the remote processors.
 *
 * The world controls the startup and shutdown of distributed processing. 
 * The default system behavior when multi-threaded is to start several threads to
 * handle asynchronous loading of data. These threads are spawned when a
 * file load request is first made (see World::LoadAsync). Event
 * A world shuts down when World::Stop is called.
 *
 * Arguments from the command line or shortcut can be accessed
 * from the world. We use the \b UNIX convention that the 0th argument
 * is the name of the executable. Typically, the first argument is
 * the name of a file which is loaded at startup.
 *
 * @see FileLoader World3D Messenger
 */
class World : public SharedObj
{
public:
	VX_DECLARE_CLASS(World);
	World();
	~World();

//! @name World Domination
//! Elapsed time since world was created in seconds.
	virtual	float		GetTime() const;
//! Return the world pointer.
	static World*		Get()				{ return s_OnlyOne; }	
//! Return name of file to open on startup.
	const TCHAR*		GetFileName() const	{ return m_FileName[0] ? m_FileName : NULL; }
//! Set name of file to open on startup.
	virtual void		SetFileName(const TCHAR*);
//! Get root directory for relative path names.
	virtual void		SetMediaDir(const TCHAR* d);
//! Get root directory for relative path names.
	const TCHAR*		GetMediaDir() const;
//! Get the full path name of a file (prepend MediaDir if necessary).
	virtual bool		GetMediaPath(const TCHAR* srcfile, TCHAR* dstpath, int maxlen) const;
//! Asynchronously load a file and generate a load event when done.
	virtual	bool		LoadAsync(const TCHAR* fname, SharedObj* observer = NULL);
//! Unload memory representation of the given file.
	virtual	void		Unload(const TCHAR* fname);
//! Determine if world is running.
	bool				IsRunning() const	{ return m_RunArg != NULL; }
//! Start the world processing input.
	virtual bool		Run(Window);
//! Stop the world processing and shut down threads.
	virtual void		Stop();

//! Return file loader used to load content.
	FileLoader*			GetLoader() const	{ return m_LoadQueue; }
//! Return messenger used to dispatch events and changes.
	Messenger*			GetMessenger() const { return m_Messenger; }
//! Return event allocator
	static Core::Allocator*	GetEventAlloc();
//! Perform one-time system startup.
	static bool			Startup();
//! Perform one-time system shutdown.
	static void _cdecl 	Shutdown();

//! @name Overrides for subclasses
//!@{
//! Called to make events known to the world.
	virtual	Event*		MakeEvent(int code);
//! Parse command line arguments as a single string.
	virtual bool		ParseArgs(const TCHAR* command_line);
//! Parse invocation arguments as a vector.
	virtual bool		ParseArgs(int argc, TCHAR** argv);
//! Called to initialize the world at startup.
	virtual bool		OnInit();
//! Called to cleanly exit during shutdown.
	virtual	void		OnExit();
//! Callback when thread shuts down.
	virtual void		OnThreadExit(int threadtype);

protected:
//! Called to process events observed by the world.
	virtual bool		OnEvent(Event*)	{ return false; }
//!@}

public:
//	SharedObj Overrides
	virtual bool		Do(Messenger& s, int opcode);
	virtual int			Save(Messenger&, int) const;
	virtual SharedObj*	Clone() const				{ return (SharedObj*) 0; }
	virtual bool		Copy(const SharedObj*)		{ return false; }
//	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

//	Public Data
	DebugOut&		DebugOutput;	//!< debug output print stream
	double			StartTime;		//!< time world started (seconds)
	static	bool	DoAsyncLoad;	//!< true to enable asynchronous loading
	static	bool	MakeConsole;	//!< true to make console window on startup

	enum Opcode
	{
		WORLD_SetFileName = SharedObj::OBJ_NextOp,
		WORLD_Run,
		WORLD_Stop,
		WORLD_SetMediaDir,
		WORLD_NextOp = SharedObj::OBJ_NextOp + 10
	};
protected:
	Ref<Messenger>		m_Messenger;			// messenger for dispatch
	FileLoader*			m_LoadQueue;			// asynchronous load request queue
	TCHAR				m_FileName[VX_MaxPath];	// initial file to open
	TCHAR				m_MediaDir[VX_MaxPath];	// root directory for relative path names
	TCHAR				m_ArgBuf[256];			// command line
	int					m_Argc;					// argument count
	TCHAR*				m_Argv[50];				// argument values
	void*				m_RunArg;
	static World*		s_OnlyOne;				// the one world object
};



inline const TCHAR* World::GetMediaDir() const
{
	if (*m_MediaDir == 0)
		return NULL;
	return m_MediaDir;
}


} // end Vixen