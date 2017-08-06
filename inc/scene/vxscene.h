/*!
 * @file vxscene.h
 * @brief 3D scene display class.
 *
 * A scene holds the display hierarchy and simulation code that
 * is processed every frame for real-time 3D display.
 * The display hierarchy is a tree of Model objects that are
 * rendered every frame. The simulation code is in a tree of
 * Engine objects that are evaluated every frame.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxworld3d.h vxmodel.h vxengine.h vxdistscene.h vxdualscene.h
 */
#pragma once

namespace Vixen {

class Renderer;
struct SceneStats;
class DeviceInfo;

#ifdef VX_NOTHREAD
typedef void*	SceneThread;
#endif

/*
 * scene change flags (m_Changed)
 */
#define	SCENE_RootChanged	1	// root of scene graph changed
#define	SCENE_WindowChanged	2	// window size/position changed
#define	SCENE_CameraChanged	4	// camera parameters changed
#define	SCENE_EnvChanged	8	// background color, ambient light changed
#define	SCENE_FBChanged		16	// framebuffer changed
#define	SCENE_KillMe		32	// kill this scene safely
#define	SCENE_ShaderChanged	64	// env or post processor shader changed

/*!
 * @class SceneStats
 * @brief Contains all the statistics computed about the traversal
 * and display of the scene each frame.
 *
 * This is kept in thread local storage because each thread has
 * its own statistics when multiple display threads are used.
 * @see Scene::GetStats
 * @ingroup vixen
 * @internal
 */
struct SceneStats
{
	vint32	TotalModels;		//!< total scene graph models this frame
	vint32	CulledModels;		//!< total models culled this frame
	vint32	TotalVerts;			//!< total vertices seen this frame
	vint32	CulledVerts;		//!< total vertices culled this frame
	vint32	RenderStateChanges;	//!< render state changes this frame
	vint32	PrimsRendered;		//!< primitives rendered this frame
	float	StartTime;			//!< time thread started this frame
	float	EndTime;			//!< time thread ended this frame
};

/*!
 * @class Scene
 * @brief Contains global characteristics that determine how 3D objects are displayed.
 *
 * A scene describes how the objects are
 * being viewed, the environment they are viewed in and
 * drives a simulation component which controls object behaviors.
 * Each scene has a display hierarchy which contains  static
 * models that do not move with respect to one another and a dynamic
 * hierarchy with models that move over time. A scene may also have
 * a simulation tree of  engines that animate the dynamic models.
 *
 * @image html app_structure.png
 *
 * <B>Distributed Traversal</B>
 *
 * In a single processor environment, changes to the scene graph made by
 * Vixen calls are applied immediately. In the distributed computing environment,
 * each processor has a Scene and updates to the scene graph are logged in addition
 * to directly modifying the local objects. Logging is done by saving scene graph
 * changes to the messenger. At the end of each frame, the messenger sends the transactions
 * to the other nodes in the system. Before displaying
 * the next frame, the Scene applies any scene graph changes sent by the other processors.
 *
 * @ingroup vixen
 * @see DistScene DualScene Engine Model World3D::NewScene World3D::SetScene Messenger
 */
class Scene : public SharedObj
{
	friend class World3D;			// to clean up s_StatAlloc, s_DynAlloc
public:
	VX_DECLARE_CLASS(Scene);

	/*!
	 * @brief Thread Local storage for a scene.
	 *
	 * These variables are used internally by the rendering and display traversal threads.
	 * @internal
	 * @ingroup vixen-int
	 */
	class TLS
	{
	public:
		uint32	ThreadType;			//!< type of thread
		int32	Frame;				//!< frame counter
		int32	ThreadIndex;		//!< 0 based thread index
	};

	/*!
	 * @brief Scene configuration and display options.
	 * @see Scene::SetOptions
	 */
	enum
	{
		CLEARDEPTH  = 1,	//!< clear hidden surface buffer
		CLEARCOLOR	= 2,	//!< clear screen to background color
		CLEARALL	= 3,	//!< clear screen and hidden surface buffer
		STATESORT	= 4,	//!< enable state sorting
		DOUBLEBUFFER= 8,	//!< enable double buffering
		FULLSCREEN	= 16,	//!< full screen operation
		OCCLUSIONCULL= 32,	//!< enable occlusion culling
		REPAINT		= 64,	//!< require explicit repaint call
	};
	/*
	 * Scene::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		SCENE_UNUSED0 = SharedObj::OBJ_NextOp,
		SCENE_SetModels,
		SCENE_SetCamera,
		SCENE_UNUSED1,
		SCENE_SetEngines,
		SCENE_SetBackColor,
		SCENE_SetAmbient,
		SCENE_ShowAll,
		SCENE_SetAutoAdjust,
		SCENE_SetOptions,
		SCENE_SetViewport,
		SCENE_SetAmbientColor,
		SCENE_SetPostProcess,
		SCENE_NextOp = SharedObj::OBJ_NextOp + 20
	};

/*!
 * @name Public Functions
 * These functions comprise the public scene API used to construct scenes
 * and prepare them for display.
 */
//@{
	Scene(Renderer* r = NULL);					//!< Construct empty scene.
	Scene(Scene&);								//!< Construct scene from another scene.
	virtual ~Scene();
	void			Suspend();					//!< suspend display processing.
	void			Resume();					//!< resume display processing.
	void			Repaint();					//!< cause display to be repainted once.
	bool			IsSuspended() const;		//!< check if display is processing.
	void			ZoomToModel(Model*);		//!< make camera look at given model.
	void			ZoomToBounds(Box3& bound);	//!< zoom camera so given area is visible.
	void			SetPriority(int p);			//!< set priority of graphics threads
	virtual	void	ShowAll();					//!< change camera to show all objects.
	virtual void	SetViewport(const Box2&);	//!< set 2D window viewport for display.
	virtual void	SetViewport(float l, float t, float r, float b);
	virtual	void	SetViewport();				//!< use entire window area for scene display.
	virtual void	SetAutoAdjust(bool);		//!< establish camera auto-adjust state.
	virtual void	SetOptions(int);			//!< set scene display options.
	virtual	void	EnableOptions(int);			//!< enable scene display options.
	virtual void	DisableOptions(int);		//!< disable scene display options.
	virtual void	SetAmbient(const Col4&);	//!< set ambient light color.
	virtual void	SetAmbient(Light* light);	//!< set ambient light source.
	virtual void	SetBackColor(const Col4&);	//!< set background color.
	virtual bool	SetColorBuffer(Bitmap*);	//!< set texture to get color buffer contents.
	virtual bool	SetDepthBuffer(Bitmap*);	//!< set texture to get depth buffer contents.
	virtual void	SetCamera(const Camera*);	//!< set camera.
	virtual void	SetModels(const Model*);	//!< set model hierarchy to display.
	virtual void	SetEngines(const Engine*);	//!< set simulation hierarchy.
	virtual void	SetTimeInc(float timeinc);	//!< set time increment between frames.
	virtual bool	SetWindow(Window win);		//!< set window to use for display.
	virtual	void	Append(Scene* child);		//!< add child scene which shares this scene's context
	virtual const Shader*	InstallShader(const Shader*);	//!< install a shader for global use

	Window			GetWindow();				//!< get window used for display.
	Model*			GetModels();				//!< get model hierarchy.
	const Model*	GetModels() const;
	Engine*			GetEngines();				//!< get simulation tree (engine hierarchy).
	const Engine*	GetEngines() const;
	Camera*			GetCamera();				//!< get scene's camera.
	const Camera*	GetCamera() const;
	Bitmap*			GetDepthBuffer() const;		//!< get depth buffer texture
	Bitmap*			GetColorBuffer() const;		//!< get color buffer texture
	const Box2&		GetViewport() const;		//!< get window viewport.
	void			GetViewport(Box2* OUTPUT) const;
	Light*			GetAmbient() const;			//!< get ambient light source.
	const Col4&		GetBackColor() const;		//!< get background color.
	int				GetOptions() const;			//!< get scene display options.
	bool			IsAutoAdjust() const;		//!< return \b true if camera autoadjusts.
	float			GetTime() const;			//!< get time this frame started.
	float			GetTimeInc() const;			//!< get time increment between frames.
	bool			GetBound(Sphere* sphere) const; //!< get bounding sphere for entire hierarchy.
	SceneStats*		GetStats() const;			//!< get scene display statistics.
	Appearance*		GetPostProcess() const;		//!< get appearance used for pixel post-processing
	void			SetPostProcess(Appearance*);//!< set appearance used for pixel post-processing
	const DeviceInfo*	GetDevInfo() const;		//!< get device information.
//@}


/*!
 * @name Overrides
 * These functions are called internally during scene startup, shutdown and traversal.
 * They are intended for subclasses to override to customize scene behavior for
 * specific devices or applications.
 * @internal
 */
//@{
	virtual	void	DoFrame();
	virtual float	OnFrame(int framecount);			//!< called before processing each frame.
	virtual void	InitCamera();						//!< initialize camera before traversal each frame.
	virtual	void	DoDisplay();						//!< traverse scene graph, choose meshes to render.
	virtual	void	Begin();							//!< called to start frame rendering.
	virtual	void	End();								//!< called to end frame rendering.
	virtual	void	DoRender();							//!< render accumulated primitives for this frame.
	virtual void	DoSimulation();						//!< evaluate simulation engines for this frame.
	virtual bool	InitThread(SceneThread*);			//!< called when a scene thread starts
	virtual	void	EndThread(SceneThread*);			//!< called when a scene thread exits
	virtual	void	Exit();								//!< called to clean up during shutdown
//@}
/*!
 * @name Internal Traversal and Rendering
 * These functions are used internally during traversal and rendering.
 * They are public to make them accessible to user-defined thread functions
 * but should not be called from user code.
 */
//@{
	void				SetTime();						//!< establish new frame time  @internal
	static TLS*			GetTLS();						//!< get scene thread storage  @internal
	virtual int			GetFrameIndex() const;			//!< get frame index for multi-threaded access  @internal
	bool				IsExit() const;					//!< \b true during shutdown  @internal
	long				GetChanged() const;				//!< get scene changes  @internal
	void				Notify(long flags);				//!< notify of a scene change internally @internal
	bool				IsChild() const;				//!< \b true if this is a child scene
	Scene*				GetChild() const;				//!< get first child scene to render
	Scene*				RemoveChild();					//!< remove first child
	Matrix*				GetWorldMatrix() const;			//!< get current world matrix
	virtual bool		InitRender(Vixen::Window win);	//!< initialize renderer @internal
	virtual	Renderer*	GetRenderer() const;			//!< get renderer for this frame @internal
	void				InitThreadGlobals();			//!< initialize thread storage @internal
	static	int			NumTexUnits;					//!< number of texture units in device
	static	int			DeviceDepth;					//!< desired framebuffer depth
	static	bool		SupportDDS;						//!< device supports compressed textures
	static const TCHAR*	RenderOptions;					//!< renderer-specific options
	static	Core::ThreadFunc FrameFunc;					//!< thread function for scene graph display
//@}

	virtual bool		Copy(const SharedObj*);
	virtual bool		Do(Messenger& s, int op);
	virtual int			Save(Messenger&, int) const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

protected:
	void				Empty();						//!< empty scene, dereference all nodes @internal
	virtual	bool		Run(Window);					//!< start up scene display threads
	virtual void		AllowUpdate();					//!< allow scene update by foreign threads @internal
	DeviceInfo*			MakeDevInfo(const DeviceInfo* = NULL);	//!< make device descriptor @internal
	static void			Shutdown();						//!< free static resources
	THREAD_LOCAL		TLS t_State;					//!< thread-specific state @internal

//	Data members (common to all devices)
	bool			m_AutoAdjust;
	bool			m_Transparent;
	bool			m_IsMultiframe;
	vint32			m_Options;
	float			m_Time;
	float			m_TimeInc;
	Box2			m_Viewport;
	Col4			m_BackColor;
	Ref<Model>		m_Models;
	Ref<Camera>		m_Camera;
	Ref<Bitmap>		m_ColorBuffer;
	Ref<Bitmap>		m_DepthBuffer;
	Ref<Engine>		m_Engines;
	Ref<Appearance>	m_PostProcess;
	mutable Ref<Light>	m_Ambient;		// ambient light source

//	Internal rendering information
	Ref<Scene>		m_Child;			// child scenes
	Ref<Renderer>	m_Renderer;			// device specific rendering stuff
	SceneStats		m_Stats;			// frame statistics
	DeviceInfo*		m_pDevInfo;			// device-specific window stuff
	vint32			m_Changed;			// mask of scene properties that changed
	Scene*			m_Parent;			// parent scene of this child
	Matrix			m_WorldMatrix;		// current world matrix for display thread


//	Multi-threading
	vint32			m_ChangePending;	//!< want to change scene graph @internal
	Core::Semaphore	m_UpdateOK;			//!< signals remote update legal now @internal
	Core::Semaphore	m_UpdateDone;		//!< signals remote update finished @internal

#ifdef VX_NOTHREAD
	bool			DoExit;				// exit flag
#else
	static Core::ThreadPool*	s_Threads;		//!< global thread pool @internal 
#endif
};

} // end Vixen
