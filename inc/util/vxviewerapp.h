#pragma once
#pragma managed(push, off)

namespace Vixen {
/*!
 * @class Viewer
 * @brief Base class for deriving 3D viewers on different platforms.
 *
 * Supports some simple 3D content loading capabilities,
 * camera scripting and mouse navigation. This class is templatized
 * so you can derive from a platform-specific version of World3D
 * that suits your needs.
 *
 * Viewer maintains a separate applicationn-specific
 * simulation tree and scene graph and attaches each new scene
 * loaded into this infrastructure. The default scene graph
 * contains a single directional light source. The default simulation
 * tree contains the following engines:
 * @code
 *	StatLog		logs and displays per-frame statistics
 *	Flyer		navigates camera using the mouse
 *	Animator	camera animation
 *	Scriptor	parses startup script, if any
 *	JoyStick	joystick engine (only if joystick is detected)
 * @endcode
 *
 * Each time the application loads a scene from a file, the default
 * simulation tree and scene graph are included.
 *
 * @see Event SharedObj World3D
 */
#define VX_USE_SCRIPTOR

template <class WORLD> class Viewer : public WORLD
{
public:
	Viewer();

// New public functions
	Scriptor*		GetScriptor() const			//!< Get current camera scripting engine.
					{ return m_Scriptor; }
	Engine*			GetNavigator() const		//!< Get current camera navigation engine.
					{ return m_Navigator; }
	Engine*			GetStatLog() const			//!< Get current frame status logging engine
					{ return m_StatLog; }
	void			SetEngines(Engine* e);		//!< Replace default simulation tree.
	Engine*			GetEngines() const;			//!< Get default simulation tree.
	void			AddEngine(Engine*);			//!< Add engine to default simulation tree.
	void			SetCamera(Camera* cam);		//!< Change cameras.
	void			SetNavigator(Engine* e);	//!< Establish default camera navigation engine.
	void			SetStatLog(Engine* e)		//!< Establish default status logging engine.
					{ m_StatLog = e; }
	void			SetModels(Model* m);		//!< Replace default scene graph.
	Model*			GetModels() const;			//!< Get default scene graph.
	Model*			GetUserModels() const;		//!< Get models added by user
	Engine*			GetUserEngines() const;		//!< Get engines added by user
	void			AddModel(Model*);			//!< Add model to default scene graph.
	const TCHAR*	GetScriptFile() const;		//!< Get name of camera script file.
	void			SetScriptFile(const TCHAR* scriptfile);
	void			SetScriptor(Scriptor* e)	//!< Establish default camera scripting engine.
					{ m_Scriptor = e; }

	//! Find object in scene with given name.
	SharedObj*		Find(const TCHAR* name, const Scene* scene = NULL) const;

	//! Find objects in scene based on name matching.
	ObjArray*		FindAll(const TCHAR* name, const Scene* scene = NULL) const;

	//! Apply an appearance attribute to entire scene.
	void			Apply(Scene* scene, int32 attr, int32 val);
	void			AttachScene(Scene* scene);
	virtual bool	InitStats();

// Internal overrides
	virtual void	SetFileName(const TCHAR*);
	virtual Scene*	MakeScene(const TCHAR* filename = NULL);
	virtual void	SetScene(Scene*, Window);
	virtual void	OnExit();
	virtual bool	OnInit();
	virtual bool	ParseArgs(const TCHAR* command_line)	{ return WORLD::ParseArgs(command_line); }
	virtual bool	ParseArgs(int argc, TCHAR** argv);

	/*
	 * App::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		VIEWER_SetScriptFile = World3D::WORLD3D_NextOp,
		VIEWER_SetEngines,
		VIEWER_SetModels,
		VIEWER_AddEngine,
		VIEWER_AddModel,
		VIEWER_NextOp = World3D::WORLD3D_NextOp + 20
	};

protected:
	void			AttachNav(Camera*);
	void			FindTerrain(Transformer* navroot, const TCHAR* targname);
	virtual bool	Do(Messenger& s, int opcode);
	virtual Engine*	MakeEngines();		//!< Make default simulation tree.
	virtual Model*	MakeModels();		//!< Make default scene graph.
	virtual Scene*	NewScene(Window = NULL, Scene* = NULL);
	static const TCHAR*	OpcodeNames[VIEWER_NextOp - VIEWER_SetScriptFile];
	Ref<Sprite>		m_FrameStats;
	Ref<Engine>		m_Navigator;
	Ref<Engine>		m_StatLog;
	Ref<Engine>		m_SimRoot;
	Ref<Model>		m_SceneRoot;
	Ref<Flyer>		m_Flyer;
	Core::String	m_StatLogFile;
	float			m_StereoEyeSep;
	bool			m_IsFullScreen;
	Ref<Scriptor>	m_Scriptor;
	Core::String	m_ScriptFile;
};

#include "util/vxviewerapp.inl"

} // end Vixen

#pragma managed(pop)