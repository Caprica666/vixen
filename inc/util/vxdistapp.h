#pragma once

namespace Vixen {

#define	WORKING_DIR	"/vixen/bin/win32"

/*!
 * @class DistribWorld
 * @brief Distributed world infrastructure that distributes the code and data
 * of a scene manager application across a network of processors.
 *
 * The processors communicate using socket streams, sending updates
 * in Vixen binary protocol across the network to synchronize the scene.
 *
 *
 * @par Initialization
 *
 * Command line arguments are used for initialization:
 *
 *	 content -d  workdir -s  socket
 * @code
 *	content		is a fully qualified pathname of the initial content
 *				to load and display. This is unrelated to the working
 *				directory of the application
 *	-d workdir	working directory for display wall, contains
 *				config files and executables (default is /vixen/bin/win32)
 *	-s socket	name or IP address of client socket, \b master indicates
 *				this instance is the master server socket
 * @endcode
 *
 * The  content file must be the name of a valid .Vixen file or a file format
 * understood by the application (if subclassing DistribWorld). This name can be
 * retrieved by World::GetFileName and is passed to World3D::MakeScene
 * on startup.
 *
 * @see Messenger Viewer
 */
template <class BASE> class DistribWorld : public BASE
{
public:
	DistribWorld();

//! get name of working directory (from argument list)
	const TCHAR*	GetWorkingDir() const	{ return m_WorkingDir; }

//! Returns \b true for master application, \b false for slaves
	virtual bool	IsMaster() const		{ return m_IsMaster; }

//! Set whether this application is master or slave
	virtual void	SetMaster(bool f)		{ m_IsMaster = f; }
	bool			OnEvent(Event*);
	bool			OnInit();
	bool			ParseArgs(int argc, TCHAR** argv);
	bool			ParseArgs(const TCHAR* cmd)	{ return BASE::ParseArgs(cmd); }
	bool			Run(Window);

// Public data members
	bool			DoSync;

protected:
	bool			m_IsMaster;
	Core::String	m_WorkingDir;
	Core::String	m_SocketServer;
	Ref<Scriptor>	m_InitScript;
};

template <class BASE> DistribWorld<BASE>::DistribWorld()
:	BASE()
{
	DoAsyncLoad = true;
	m_IsMaster = false;
	m_InitScript = new Scriptor;
}

template <class BASE> bool DistribWorld<BASE>::OnEvent(Event* event)
{
	switch (event->Code)
	{
		case Event::EXEC:
		{
			LoadEvent* e = (LoadEvent*) event;
			if (e->FileName.CompareNoCase(TEXT("master.scp")) == 0)
			{
				Scene* scene = GetMainScene();
				if (scene->GetEngines() == (Scriptor*) m_InitScript)
					scene->SetEngines(NULL);
			}
			return true;
		}
	}
	return BASE::OnEvent(event);
}

template <class BASE> bool DistribWorld<BASE>::OnInit()
{
	Core::Stream*	stream = NULL;
	Synchronizer*	sync = new Synchronizer();

/*
 * Open a socket to our master or slave processors. The socket can be used
 * for frame or object synchronization depending on DoSync
 */
	m_Messenger = sync;
	if (!m_SocketServer.IsEmpty())
	{
		if (m_IsMaster)								// are we the server?
		{
			sync->SetName("master");
			stream = new Core::SocketArbitrator(6666);	// open server socket
			sync->SetInStream(stream);
			sync->SetOutStream(stream);
			sync->SendUpdates = true;
		}
		else
		{
			sync->SetName("slave");
			stream = new Core::Socket(6666);			// slave? single socket to master
			sync->SetInStream(stream);
			sync->SendUpdates = false;
		}
		if (!stream->Open(m_SocketServer))
		{
			stream->Delete();
			stream = NULL;
			VX_ERROR(("DistApp::Initialize ERROR: cannot open input socket"), false)
		}
	}
	sync->DoSync = DoSync;
	//sync->SendUpdates = !m_IsMaster;		// master doesn't send right away
	sync->SendEvents = false;				// disable event logging
/*
 * Attach to scene if there is one
 */
	sync->Attach(this, GLOBAL);
	sync->Attach(m_InitScript);
	sync->Observe(this, Event::EXEC, NULL);
	sync->Observe(this, Event::LOAD_SCENE, NULL);
	sync->Observe(this, Event::SET_TIME, NULL);
	sync->Observe(this, Event::SCENE_CHANGE, NULL);
	sync->Observe(this, Event::ATTACH_SCENE, NULL);
	sync->Observe(this, Event::LEAVE, NULL);
	sync->Observe(this, Event::PICK, NULL);
	sync->Observe(this, Event::NOPICK, NULL);
	sync->Observe(this, Event::KEY, NULL);
	sync->Observe(this, Event::CONNECT, NULL);
	sync->Observe(this, Event::EXEC, NULL);
	StartTime = (float) Core::GetTime();
	return BASE::OnInit();
}

/*!
 * @fn bool DistribWorld<BASE>::ParseArgs(int argc, TCHAR** argv)
 *
 * Parses command line arguments specific for a Distributed application.
 *		 content -s  socket -T
 *
 * @code	
 *	content		is the name of the file that contains the initial
 *				content to display.
 *	-d workdir	working directory for display wall, contains
 *				config files and executables (default is /vixen/bin)
 *	-s socket	name or IP address of client socket, <<master>> indicates
 *				this instance is the master server socket
 * @endcode
 *
 * @return \b true	do default parsing (filename), \b false suppress parsing
 *
 * @see World::ParseArgs
 */
template <class BASE> bool DistribWorld<BASE>::ParseArgs(int argc, TCHAR** argv)
{
	const TCHAR*	p;
	for (int i = 1; i < argc; ++i)
	{
		p = argv[i];
		if (*p++ != TEXT('-'))
			continue;
		char c = *p++;			// grab option
		switch (c)
		{
			case TEXT('m'):		// -m for master?
			SetMaster(true);
			break;

			case TEXT('d'):		// -d <workingdir>
			p = argv[++i];
			argv[i] = NULL;
			m_WorkingDir = p;
			break;

			case TEXT('s'):		// -s <socketserver>
			p = argv[++i];
			argv[i] = NULL;
			m_SocketServer = p;
			break;
		}
}
	return BASE::ParseArgs(argc, argv);
}

template <class BASE> bool DistribWorld<BASE>::Run(Window win)
{
	if (!BASE::Run(win))
		return false;
	if (win == NULL)
		return true;

	Scene*	scene = GetScene(win);
	Scriptor* scriptor = m_InitScript;
	FileLoader*	loader = GetLoader();
	Synchronizer* stream;

	scriptor->SetFlags(SharedObj::DOEVENTS);
	scene->SetEngines(scriptor);
	scriptor->SetDirectory(m_WorkingDir);
	if (IsMaster())
	{
		scriptor->SetDirectory(loader->GetDirectory());
		scriptor->Load("master.scp");
	}
	else
	{
		scriptor->Load("slave.scp");
		stream = (Synchronizer*) GetMessenger();
		VX_ASSERT(stream->IsClass(VX_Synchronizer));
		if (!stream->DoSync)
			stream->Connect("scene", scene);
	}
	return true;
}

}	// end Vixen