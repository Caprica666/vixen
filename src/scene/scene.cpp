#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Scene, SharedObj, VX_Scene);

int						Scene::DeviceDepth = 32;
int						Scene::NumTexUnits = 1;
bool					Scene::SupportDDS = false;
const TCHAR*			Scene::RenderOptions = NULL;
Scene::TLS				Scene::t_State;

void Scene::InitThreadGlobals()
{
	TLS* g = GetTLS();
	g->ThreadType = -1;
}

static const TCHAR* opnames[] =
{	TEXT("UNUSED"), TEXT("SetModels"), TEXT("SetCamera"),
	TEXT("SetLight"), TEXT("SetEngines"), TEXT("SetBackColor"), TEXT("SetAmbient"),
	TEXT("ShowAll"), TEXT("SetAutoAdjust"), TEXT("SetOptions"),
	TEXT("SetViewport"), TEXT("SetPostProcess")
};

const TCHAR** Scene::DoNames = opnames;

void Scene::Shutdown()
{
#ifndef VX_NOTHREAD
	if (s_Threads)
	{
		VX_ASSERT(s_Threads->NumThreads == 0);
		delete s_Threads;
		s_Threads = NULL;
	}
#endif
	DataLayout::FreeLayouts();
}

bool Scene::InitRender(Vixen::Window window)
{
	Renderer* gs = m_Renderer;
	if (gs && gs->Init(this, window, RenderOptions))
	{
		SetViewport();
		return true;
	}
	return false;
}


/*!
 * @fn SceneStats* Scene::GetStats() const
 *
 * Returns the statistics gathered about the currently
 * displayed scene for the last frame in SceneStats
 * structure. This structure may be device or platform
 * dependent and varies across Vixen ports.
 *
 * @return scene statistics or NULL if none available
 *
 * @see StatusLog FrameCounter
 */
SceneStats* Scene::GetStats() const
{
	return (SceneStats*) &m_Stats;
}

/*!
 * @fn Scene::Scene(Renderer* render)
 * @param renderer	-> renderer used to display the scene
 *
 * Initialize scene with default camera and light.
 * The scene will have no scene graph or simulation tree
 * and will not be attached to the display.
 */
Scene::Scene(Renderer* renderer) : SharedObj()
{
	m_pDevInfo = MakeDevInfo();
	m_Time = 0.0f;
	m_TimeInc = 0.0f;
	m_Changed = -1;
	m_Camera = new Camera();
	SetBackColor(Col4(0.2f, 0.5f, 0.8f, 0.0f));
	m_AutoAdjust = true;
	m_IsMultiframe = false;
	m_Options = CLEARALL;
	m_Renderer = NULL;
	m_ChangePending = false;
	m_Changed = 0;
	m_Child = m_Parent = NULL;
	m_Renderer = renderer;
#ifdef VX_NOTHREAD
	DoExit = false;
#endif
};

Scene::Scene(Scene& src) : SharedObj(src)
{
	m_pDevInfo = MakeDevInfo();
	m_Renderer = NULL;
	m_IsMultiframe = src.m_IsMultiframe;
	m_Changed = 0;
	m_Child = m_Parent = NULL;
	Copy(&src);
}

void Scene::Exit()
{
	Renderer*	gs = GetRenderer();

	Empty();
	if (m_Prop)
	{
		m_Prop->Empty();
		m_Prop = NULL;
	}
	m_Camera = (Camera*) NULL;
	if (gs)
		gs->Exit();
	m_Renderer = NULL;
}


/****
 *
 * class Scene override for SharedObj::Copy
 * Initialize scene with default values for parameters.
 *
 ****/
bool Scene::Copy(const SharedObj* src_obj)
{
	const Scene* src = (const Scene*) src_obj;
	float eyesep = m_Camera->GetEyeSep();

	VX_ASSERT(src->IsClass(VX_Scene));
	Suspend();
	Empty();
	Notify(SCENE_CameraChanged | SCENE_WindowChanged | SCENE_RootChanged | SCENE_EnvChanged);
	m_Camera = src->m_Camera;
	if (eyesep > 0)
	{
		m_Camera->SetEyeSep(eyesep);
		m_Camera->SetChanged(true);
	}
	Core::InterlockSet(&m_Options, src->m_Options);
	SetModels(src->m_Models);
	SetEngines(src->m_Engines);
	SetBackColor(src->m_BackColor);
	if (!src-> m_PostProcess.IsNull())
		SetPostProcess(src->m_PostProcess);
	SetAmbient(src->m_Ambient);
	SetTimeInc(src->m_TimeInc);
	if (src->m_Viewport.Width() && src->m_Viewport.Height())
		SetViewport(src->m_Viewport);
	m_AutoAdjust = src->m_AutoAdjust;
	Resume();
	return true;
}

/****
 *
 * class Scene override for SharedObj::Print
 *
 ****/
DebugOut& Scene::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "<attr name='Camera'>");
	m_Camera->Print(dbg, opts | PRINT_Trailer);
	endl(dbg << "</attr>");
	if (!m_Models.IsNull())
	{
		endl(dbg << "<attr name='Models'>");
		m_Models->Print(dbg, opts | PRINT_Trailer);
		endl(dbg << "</attr>");
	}
	if (!m_Engines.IsNull())
	{
		endl(dbg << "<attr name='Engines'>");
		m_Engines->Print(dbg, opts | PRINT_Trailer);
		endl(dbg << "</attr>");
	}
	if (!m_PostProcess.IsNull())
	{
		endl(dbg << "<attr name='PostProcess'>");
		m_PostProcess->Print(dbg, opts | PRINT_Trailer);
		endl(dbg << "</attr>");
	}

	endl(dbg << "\t<attr name='AutoAdjust'>" << m_AutoAdjust << "</AutoAdjust>");
	endl(dbg << "\t<attr name='Ambient'>" << ((SharedObj*) m_Ambient) << "</Ambient>");
	endl(dbg << "\t<attr name='BackColor'>" << m_BackColor << "</BackColor>");
	return SharedObj::Print(dbg, opts & PRINT_Trailer);
}


void Scene::Empty()
{
	Scene* child = GetChild();
	Renderer*	gs = GetRenderer();

	if (child)
	{
		child->m_Parent = NULL;
		child->Exit();
		m_Child = (Scene*) NULL;
	}
	if (gs)
		gs->Empty();
	m_Models = (const Model*) NULL;
	m_Engines = (const Engine*) NULL;
}

/*!
 * @fn void Scene::ShowAll()
 *
 * Orients the camera and moves it along the Z axis so that all of the
 * objects in the scene are visible. Camera movement is based on the
 * scene's bounding volume.
 *
 * @see Model::GetBound Camera Camera::SetViewVol
 */
void Scene::ShowAll()
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_ShowAll) << this;
	VX_STREAM_END( )

	Box3			bound;
	const Model*	group = m_Models;

	if (group)
	{
		group->GetBound(&bound, Model::WORLD);
		ZoomToBounds(bound);
	}
}

/*!
 * @fn const Shader* Scene::InstallShader(const Shader*)
 *
 * Installs the given shader in a global dictionary so a single
 * copy of the named shader is shared. The shader returned
 * is the global copy, which may not be the same copy as the
 * input shader. The name of the shader, as set by SharedObj::SetName,
 * is the global unique key. Shaders with the same name will be mapped
 * to the same global copy.
 *
 * @returns -> copy to global shader
 *
 * @see Renderer::InstallShader Appearance::SetPixelShader Appearance::SetVertexShader
 */
const Shader* Scene::InstallShader(const Shader* shader)
{
	Renderer* render  = GetRenderer();

	if (render)
		return render->InstallShader(shader);
	return NULL;
}

void Scene::ZoomToModel(Model *pMod)
{
	Box3	bound;

	pMod->GetBound(&bound, Model::WORLD);
	ZoomToBounds(bound);
}

void Scene::ZoomToBounds(Box3& bound)
{
	Vec3 ctr, loc;
	float fFOV = m_Camera->GetFOV() / 2.0f;
	float fWidth = bound.Width();
	float fHeight = bound.Height();
	float fDepth = bound.Depth();
	float dist = (fWidth > fHeight) ? fWidth : fHeight;

	ctr = bound.Center();					/* center of bounding volume */
	m_Camera->Reset();
	if (fWidth == 0.0f)
	{
		// Probably a light or empty group, not a shape
		loc.Set(ctr.x, ctr.y, ctr.z);	/* position of camera */
		m_Camera->Move(loc);
		return;
	}
	if (m_Camera->GetType() == Camera::ORTHOGRAPHIC)
		dist += fDepth / 2.0f;
	else
	{
		// find distance of camera from object
		dist *= tanf(fFOV) / 2.0f;
		dist = fDepth / 2.0f + dist;
	}
	if (dist < fDepth)
		dist = fDepth;
	loc.Set(ctr.x, ctr.y, ctr.z + dist);	// position of camera
	m_Camera->Move(loc);

	// make sure that the scene fits within the hither and yon planes
	if (m_Camera->GetHither() > dist)
		m_Camera->SetHither(dist);
	if (m_Camera->GetYon() < fDepth + dist)
		m_Camera->SetYon(fDepth + dist);
}

bool Scene::SetDepthBuffer(Bitmap* bmap)
{
	m_DepthBuffer = bmap;
	bmap->SetChanged(true);
	Notify(SCENE_FBChanged);
	return true;
}

bool Scene::SetColorBuffer(Bitmap* bmap)
{
	m_ColorBuffer = bmap;
	bmap->SetChanged(true);
	Notify(SCENE_FBChanged);
	return true;
}

/*!
 *@fn bool Scene::GetBound(Sphere* bound) const
 * @param bound	where to store bounding sphere
 *
 * Computes the closest fitting sphere in world coordinates which bounds
 * the entire scene graph.
 *
 * @return \b true if bounding sphere is not empty, else \b false
 *
 * @see Model::GetBound
 */
bool Scene::GetBound(Sphere* bound) const
{
	Sphere		s;
	const Model*	group = m_Models;
	bool			rc = false;

	bound->Empty();
	if (group && group->GetBound(&s, Model::WORLD))
	{
		bound->Extend(s);
		rc = true;
	}
	return rc;
}


/*!
 * @fn void Scene::SetTime()
 *
 * Establishes the time for the next frame to start.
 * The time is set from system time or from the time
 * increment if it is non-zero. If an argument is provided, the time
 * is set to that, overriding any other established time. This routine
 * should not be called at the user level. It is used internally for
 * sychronizing simulation across multiple processors.
 *
 * @see Scene::SetTimeInc World3D::GetTime Scene::GetTime
 */
void Scene::SetTime()
{
	if (m_TimeInc == 0.0f)				// animate in real time?
		m_Time = World3D::Get()->GetTime();
	else
		m_Time += m_TimeInc;			// no, frame based animation
	VX_TRACE2(Engine::Debug, ("Scene::SetTime(%0.3f)\n", m_Time));
}

/*!
 * @fn void Scene::OnFrame(int framenum)
 * Subclasses can override this routine to intercept control
 * once per frame. This routine is called right before
 * the scene display threads start a frame and holds off
 * display until it returns. You should not do time-consuming
 * operations here but you may safely update the hierarchy here.
 *
 * The base implementation computes the start time of the next
 * frame and processes updates from remote machines, applying
 * the necessary changes to the local hierarchy.
 * @note If you override this routine, you \b must
 * call the base implementation!
 *
 * @return starting time of frame (in seconds)
 *
 * @see Scene::DoFrame Scene::SetTime Messenger::Load Scene::GetTime
 */
float Scene::OnFrame(int framenum)
{
	Ref<Messenger> mess(GetMessenger());
	SetTime();
	if (IsSet(SharedObj::DOEVENTS))
	{
		FrameEvent* e = new FrameEvent(this);

		e->Frame = framenum;
		e->Log();		// generate frame event
	}
	mess->Load();		// process scene graph updates
	return m_Time;
}

/*!
 * @fn void Scene::SetOptions(int v)
 *
 * Defines scene display options. You can selectively
 * clear the display, the Z buffer or both. You can also enable
 * state sorting which will sort primitives based on appearance
 * before sending them to the display. This uses more memory
 * but is a significant performance enhancement for applications
 * which reuse textures a lot within a scene.
 *
 * @code
 *	CLEARCOLOR		flag will clear the display.
 *	CLEARDEPTH		flag will clear the Z buffer.
 *	CLEARALL		clears both (combination of both flags)
 *	STATESORT		enables state sorting
 *	FULLSCREEN		enables full screen operation
 *	REPAINT			suspends scene threads, suppressing continuous screen refresh,
 *					repaints a single frame when Repaint is called
 * @endcode
 *
 * @see Scene::EnableOptions Scene::DisableOptions Scene::Repaint
 */
void Scene::SetOptions(int v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetOptions) << this << int32(v);
	VX_STREAM_END( )

 	m_Options = v;

	Renderer* gs = GetRenderer();
	if (gs)
		gs->SetOptions((v & GeoSorter::NoStateSort) | gs->GetOptions());
}

/*!
 * @fn void Scene::EnableOptions(int opts)
 * @param opts	options to enable
 *
 * Enables the given scene display options. You can selectively
 * clear the display, the Z buffer or both. You can also enable
 * state sorting which will sort primitives based on appearance
 * before sending them to the display.
 * @code
 *	SCENE_ClearColor	flag will clear the display.
 *	SCENE_ClearDepth	flag will clear the Z buffer.
 *	SCENE_ClearAll		clears both (combination of both flags)
 *	SCENE_StateSort		enables state sorting
 *	SCENE_FullScreen	enables full screen operation
 *	SCENE_Repaint		suspends scene threads, suppressing continuous screen refresh,
 *						repaints a single frame when Repaint is called
 * @endcode
 *
 * @see Scene::SetOptions Scene::DisableOptions
 */
void Scene::EnableOptions(int opts)
{
	SetOptions(GetOptions() | opts);
}

/*!
 * @fn void Scene::DisableOptions(int opts)
 * @param opts	options to enable
 *
 * Disables the given scene display options.
 * @code
 *	SCENE_ClearColor	clear the frame buffer
 *	SCENE_ClearDepth	clear the Z buffer
 *	SCENE_ClearAll		clears both (default)
 *	SCENE_StateSort		enables state sorting (default)
 *	SCENE_FullScreen	enables full screen operation
 *	SCENE_Repaint		suspends scene threads, suppressing continuous screen refresh,
 *						repaints a single frame when Repaint is called
 * @endcode
 *
 * @see Scene::SetOptions Scene::EnableOptions
 */
void Scene::DisableOptions(int opts)
{
	SetOptions(GetOptions() & ~opts);
}


/*!
 * @fn void Scene::SetAutoAdjust(bool v)
 *
 * Determines whether or not the camera aspect ratio
 * is automatically adjusted as the scene viewport changes.
 *
 * @see Camera Camera::SetAspect Scene::SetViewport
 */
void Scene::SetAutoAdjust(bool v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetAutoAdjust) << this << int32(v);
	VX_STREAM_END( )

	m_AutoAdjust = v;
}


/*!
 * @fn void Scene::SetAmbient(const Col4& c)
 *
 * Establishes color and amount of ambient light in the scene.
 *
 * @see Light::SetAmbient Material::SetAmbient
 */
void Scene::SetAmbient(const Col4& c)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetAmbientColor) << this << c;
	VX_STREAM_END( )

	Light*	l = GetAmbient();
	bool	hasambient = true;

	if ((c.r == 0) && (c.g == 0) && (c.b == 0))
		hasambient = false;
	if (l == NULL)
	{
		if (!hasambient)
			return;
		m_Ambient = new AmbientLight();
		m_Ambient->SetName(TEXT("scene.ambientlight"));
		m_Ambient->SetColor(c);
	}
	else if (!hasambient)
	{
		m_Ambient->SetActive(false);
		return;
	}
	else
	{
		m_Ambient->SetActive(true);
		m_Ambient->SetColor(c);
	}
}

void Scene::SetAmbient(Light* ambient)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetAmbient) << this << ambient;
	VX_STREAM_END( )

	m_Ambient = ambient;
}

Light* Scene::GetAmbient() const
{
	Renderer*	render = GetRenderer();

	if (!m_Ambient.IsNull())
		return m_Ambient;
	if (render == NULL)
		return NULL;

	LightList::Iter	iter(render->GetLights());
	Light*			l;

	bool			enabled;
	while (l = iter.Next(enabled))
		if (l->ClassID() == VX_AmbientLight)
		{
			m_Ambient = l;
			return l;
		}
	return NULL;
}

/*!
 * @fn void Scene::SetEngines(const Engine* e)
 *
 * The set of engines used to compute the state of
 * the dynamic hierarchy over time. Like the Models
 * to display, the engines to compute are kept in a
 * hierarchy. Engines are executed before traversal
 * and culling. On multi-processor systems, engines
 * may be executed in a separate thread.
 *
 * @see Scene::DoSimulation Engine Engine::Compute
 */
void Scene::SetEngines(const Engine* e)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetEngines) << this << e;
	VX_STREAM_END( )

	m_Engines = e;
}

/*!
 * @fn void Scene::SetCamera(const Camera* v)
 *
 * The camera used to view the scene. By default,
 * the scene has a camera which is not in the hierarchy.
 * You can change the camera to be part of the hierarchy
 * in which case its movement over time affects the
 * viewpoint from which the scene is displayed. The
 * default light for the scene tracks the camera.
 *
 * @see Scene::DoDisplay Camera
 */
void Scene::SetCamera(const Camera* v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetCamera) << this << v;
	VX_STREAM_END( )

	if (v == NULL)
		v = new Camera();

	bool bNewCamera = (v != m_Camera);
	m_Camera = v;
	if (bNewCamera)
	{
		v->SetChanged(true);
		Notify(SCENE_CameraChanged);
	}
}


/*!
 * @fn void Scene::SetPostProcess(Appearance* post)
 * @param post	Appearance used to post-process the scene after 3D rendering.
 *				The shader is run on all of the pixels in the framebuffer
 *				at the end of the frame.
 *
 * Designates the shader to be used for pixel post processing.
 * The name of the Shader associated with this appearance and it's entry point
 * must be called "PostProcessor". If not supplied, no post-processing is done.
 *
 * @see Scene::DoDisplay Shader
 */
void Scene::SetPostProcess(Appearance* appear)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetPostProcess) << this << appear;
	VX_STREAM_END( )

	m_PostProcess = appear;
	Notify(SCENE_ShaderChanged);
}

/*!
 * @fn void Scene::SetBackColor(const Col4& c)
 *
 * If screen clearning is enabled, the background color is displayed in
 * the scene viewport before any meshes are rendered.
 *
 * @see Col4 Scene::SetOptions Scene::SetAmbient
 */
void Scene::SetBackColor(const Col4& c)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetBackColor) << this << c;
	VX_STREAM_END( )

	m_BackColor = c;
}

/*!
 * @fn void Scene::SetTimeInc(float timeinc)
 * @param timeinc	time increment (in seconds) to advance each frame
 *					zero (default) advances in real time
 *
 * The time increment of a scene controls the speed of the animations
 * in the simulation tree. Each frame, the simulation engines are evaluated
 * with the current time. If the time increment is zero (the default case),
 * the current time will be the system time at the start of the frame.
 * If the time increment is non-zero, the current time is incremented by
 * that amount each frame, regardless of the system time. This is useful
 * when debugging time-based simulation engines.
 *
 * @see Scene::GetTime World3D::GetTime Engine::Compute
 */
void Scene::SetTimeInc(float timeinc)
{
	if (m_TimeInc == timeinc)
		return;
	if (m_TimeInc != 0.0f)			// establish credible starting time
	{								// for application (NOW)
		World3D*	w = World3D::Get();
		w->StartTime = w->GetTime();
	}
	m_TimeInc = timeinc;
}

/*!
 * @fn void Scene::SetModels(const Model* m)
 * @param m	root of scene graph to use. If NULL, nothing is displayed.
 *
 * Sets the hierarchy this scene should display.
 *
 * @see Scene::SetModels Scene::DoDisplay Model::Display
 */
void Scene::SetModels(const Model* m)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetModels) << this << m;
	VX_STREAM_END( )

	m_Models = m;
}

/*!
 * @fn void Scene::DoFrame()
 *
 * Performs display traversal, simulation and rendering for the current frame.
 * @code
 *	1.	Process pending scene graph updates from remote
 *		clients from the messenger. The hierarchy is locked
 *		for update during this operation.
 *	2.	Compute the frame start time
 *	4.	Calculate bounding spheres for scene graph
 *	5.	Traverse scene graph and cull
 *	6.	Render geometry
 *	3.	Evaluate simulation engines
 * @endcode
 *
 * @see Model::Display Scene::DoDisplay Scene::DoRender Scene::DoSimulation Messenger::Load
 */
void Scene::DoFrame()
{
//
// compute start time of frame and process remote updates
//
	TLS*		g = GetTLS();
	Renderer*	r = GetRenderer();
	float		save_start = OnFrame(g->Frame);
	Scene*		scene = this;

	if (IsExit())						// got exit request?
		return;							// don't do next frame
//
//	REMOTE THREADS CAN UPDATE THE SCENE GRAPH HERE
//
	r->Flip(g->Frame);					// flip buffers
	AllowUpdate();						// allow update by other threads
	GetMessenger()->t_NoLog = true;		// no logging during traversal
 	Core::InterlockSet(&m_Stats.TotalVerts, 0);
	Core::InterlockSet(&m_Stats.CulledVerts, 0);
	Core::InterlockSet(&m_Stats.TotalModels, 0);
	Core::InterlockSet(&m_Stats.CulledModels, 0);
//
// do display traversal for this scene and all the child scenes
// which use its display context (procedural texture generation scenes)
//
	DoDisplay();						// do traversal, culling
	if (GetChild())						// force viewport change if multiple scenes
		Notify(SCENE_CameraChanged);
	DoRender();							// do rendering
//
// done rendering, enable logging and do simulation
//
	GetMessenger()->t_NoLog = false;	// enable logging again
	DoSimulation();						// run simulation (hierarchy locked here)
	GetMessenger()->Flush();			// distribute scene graph updates
	m_Stats.StartTime = save_start;
	++(g->Frame);
	m_Stats.EndTime = World3D::Get()->GetTime();
}


/*!
 * @fn void Scene::End()
 *
 * Called after rendering for a frame is complete.
 *
 * @see Scene::Begin Scene::DoRender
 */
void Scene::End()
{
	Renderer* r = GetRenderer();

	if (r)
		r->End(GetTLS()->Frame);
	m_Changed = 0;
}


/*!
 * @fn void Scene::DoSimulation()
 *
 * Traverses the simulation tree and calls Engine::Eval
 * for each active engine. This routine locks the entire hierarchy
 * and does not permit foreign threads to update the objects
 * while it is executing.
 *
 * @see Scene::GetTime Scene::SetTimeInc Scene::DoDisplay Scene::DoFrame Engine::Compute Engine::Eval
 */
void Scene::DoSimulation()
{
	Sphere	sphere;

	if (!m_Engines.IsNull())			// simulation engines?
		m_Engines->Compute(m_Time);		// run simulation
	if (!m_Models.IsNull())				// compute dynamic bounds
		m_Models->GetBound(&sphere);
}

/*!
 * @fn void Scene::DoRender()
 *
 * Render accumulated graphics primitives. Primitives are
 * accumulated into Renderer objects associated with
 * the scene as the scene graph is traversed each frame.
 * These non-culled primitives are sorted by appearance
 * and rendered after traversal by this routine.
 *
 * @see Scene::DoDisplay Scene::DoFrame Renderer Scene::Begin Scene::End
 */
void Scene::DoRender()
{
	Renderer*	r = GetRenderer();
	int			frame = GetTLS()->Frame;
	Scene*		child = GetChild();

	if (IsExit())
		return;
	if (child && child->IsActive())
	{
		child->Notify(SCENE_WindowChanged);
		child->DoRender();				// do rendering
	}
	m_Stats.PrimsRendered = m_Stats.RenderStateChanges = 0;
	Begin();
	if (r)
		r->Render(frame, GeoSorter::All);
	End();
}

/*!
 * @fn void Scene::Begin()
 *
 * Called internally from the rendering thread before
 * rendering each frame. Establishes initial conditions for
 * transformation matrix, lights and camera projection.
 *
 * @see Scene::DoRender Scene::End Scene::DoFrame
 */
void Scene::Begin()
{
	Scene::TLS*	tls = Scene::GetTLS();
	Renderer*	r = GetRenderer();

	if (r == NULL)
		return;
	if (m_Changed & SCENE_WindowChanged)
	{
		const Box2& vp = GetViewport();
		r->SetViewport(vp.min.x, vp.min.y, vp.max.x, vp.max.y);
	}
	if (m_Camera->HasChanged())
		m_Changed |= SCENE_CameraChanged;
	if (m_Changed & (SCENE_CameraChanged | SCENE_WindowChanged))
	{
		if (m_AutoAdjust)					// preserve aspect ratio of view volume?
			m_Camera->SetAspect(m_Viewport.Width() / m_Viewport.Height());
	}
	r->Begin(m_Changed, tls->Frame);
}


/*!
 * @fn void Scene::DoDisplay()
 *
 * Traverses the scene graph, does view volume culling and accumulates
 * meshes which should be rendered. This routine does not access the
 * graphics device directly.
 *
 * @see Model::Display Scene::DoFrame GeoSorter
 */
void Scene::DoDisplay()
{
	Renderer*	r = GetRenderer();
	TLS*		g = GetTLS();
	Camera*		cam = GetCamera();
	Scene*		child = GetChild();

	if (IsExit())
		return;
	if (child)
	{
		if (child->GetChanged() & SCENE_KillMe)
			RemoveChild();// link out the scene to delete
		else if (child->IsActive())
			child->DoDisplay();
	}
	InitCamera();
	if (r)
	{
		if (m_Changed & SCENE_RootChanged)
			r->Empty();
		r->Reset();
	}
	m_WorldMatrix.Identity();				// initialize view matrix
	if (!m_Ambient.IsNull())
		m_Ambient->Display(this);
	if (!cam->IsChild())					// not in the hierarchy
	{
		cam->Display(this);					// in case it has children
		m_WorldMatrix.Identity();			// initialize view matrix
	}
	m_WorldMatrix.SetChanged(false);
	if (!m_Models.IsNull())					// display dynamic models
	{
		m_WorldMatrix.Identity();			// initialize view matrix
		m_WorldMatrix.SetChanged(false);	// reset initial matrix
		m_Models->CalcStats();				// recalc vertex, face counts if necessary
		m_Models->Display(this);			// visit dynamic models
	}
	m_WorldMatrix.Identity();				// initialize view matrix
}

/*!
 * @fn void Scene::InitCamera()
 *
 * Called internally before scene traversal at the beginning
 * of each frame. Establishes initial conditions for
 * transformation matrix and camera parameters.
 *
 * @see Scene::Begin Scene::DoDisplay
 */
void Scene::InitCamera()
{
	Camera*		cam = m_Camera;

	if (m_Changed & SCENE_WindowChanged)
	{
		SetViewport();						// update viewport based on window size
		if (m_AutoAdjust)					// preserve aspect ratio of view volume?
			cam->SetAspect(m_Viewport.Width() / m_Viewport.Height());
	}
/*
 * Calculate the camera view matrix. The scene manager allows the
 * camera to be positioned anywhere. The display device supports a
 * fixed camera positioned at the origin looking down the Z axis.
 * The view matrix transforms the scene objects into this camera space.
 */
	if (cam->HasChanged())					// camera changed?
	{
		Notify(SCENE_CameraChanged);		// alert other threads
		cam->CalcViewPlanes();				// recalculate camera clip planes
		cam->SetChanged(false);
	}
	m_Camera->SetViewTrans(this);			// calculate new camera matrix and save it
}


/*!
 * @fn void	Scene::SetViewport(float left, float top, float right, float bottom)
 * @param left	X coordinate of upper left corner of screen viewport
 * @param top	Y coordinate of upper left corner of screen viewport
 * @param right	X coordinate of lower right corner of screen viewport
 * @param bottom Y coordinate of lower right corner of screen viewport
 *
 * Sets the screen viewport to which the 3D viewport maps.
 * You need to call this function whenever the window
 * origin or size changes to notify the scene manager to change the
 * 3D screen viewport the current scene is using.
 * You need to call SetViewport when you
 * handle window move and resize events.
 *
 * @see Scene::Begin Camera::SetViewVol
 */
void	Scene::SetViewport(float l, float t, float r, float b)
{
	m_Viewport.Set(l, r, t, b);
	Notify(SCENE_WindowChanged);
}

/*!
 * @fn void	Scene::SetViewport(const Box2& box)
 * @param box	2D box defining the upper left and lower right
 *				corners of the screen viewport
 *
 * Sets the screen viewport to which the 3D viewport maps.
 * You need to call this function whenever the window
 * origin or size changes to notify the scene manager to change the
 * 3D screen viewport the current scene is using.
 * You need to call SetViewport when you
 * handle window move and resize events.
 *
 * @see Scene::Begin Camera::SetViewVol
 */
void Scene::SetViewport(const Box2& b)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Scene, SCENE_SetViewport) << this << b.min.x << b.min.y << b.max.x << b.max.y;
	VX_STREAM_END( )

	SetViewport(b.min.x, b.min.y, b.max.x, b.max.y);
}


/****
 *
 * class Scene override for SharedObj::Save
 *
 ****/
int Scene::Save(Messenger& s, int opts) const
{
	const Model*		mod;
	const Camera*		cam;
	const Engine*		eng;
	const Appearance*	app;
	int32				h = SharedObj::Save(s, opts);

	if (h < 0)
		return h;
	if (h > 0)
	{
		s << OP(VX_Scene, SCENE_SetBackColor) << h << GetBackColor();
		s << OP(VX_Scene, SCENE_SetAmbient) << h << GetAmbient();
		s << OP(VX_Scene, SCENE_SetAutoAdjust) << h << int32(IsAutoAdjust());
	}
	cam = GetCamera();
	if (cam && (cam->Save(s, opts) >= 0) && h)
		s << OP(VX_Scene, SCENE_SetCamera) << h << cam;
	mod = GetModels();
	if (mod && (mod->Save(s, opts) >= 0) && h)
		s << OP(VX_Scene, SCENE_SetModels) << h << mod;
	eng = GetEngines();
	if (eng && (eng->Save(s, opts) >= 0) && h)
		s << OP(VX_Scene, SCENE_SetEngines) << h << eng;
	app = GetPostProcess();
	if (app && (app->Save(s, opts) >= 0) && h)
		s << OP(VX_Scene, SCENE_SetPostProcess) << h << app;
	return h;
}


/****
 *
 * class Scene override for SharedObj::Do
 *		SCENE_SetModels	<Model*>
 *		SCENE_SetCamera		<Camera*>
 *		SCENE_SetEngines	<Engine*>
 *		SCENE_SetBackColor	<Col4>
 *		SCENE_SetAutoAdjust <int32>
 *		SCENE_SetOptions	<int32>
 *		SCENE_SetFog		<Fog*>
 *
 ****/
bool Scene::Do(Messenger& s, int op)
{
	SharedObj*		obj;
	const Model*	mod;
	const Camera*	cam;
	const Engine*	eng;
	Col4			c;
	Box2			b;
	int32			n;
	Opcode			o = Opcode(op);

	switch (op)
	   {
		case SCENE_SetAutoAdjust:
		s >> n;
		SetAutoAdjust(n != 0);
		return true;

		case SCENE_SetOptions:
		s >> n;
		SetOptions(n);
		return true;

		case SCENE_SetBackColor:
		s >> c;
		SetBackColor(c);
		return true;

		case SCENE_SetAmbient:
		s >> c;
		SetAmbient(c);
		return true;

		case SCENE_SetViewport:
		s >> b.min.x >> b.min.y >> b.max.x >> b.max.y;
		SetViewport(b);
		return true;

		case SCENE_SetModels:
		s >> obj;
		VX_ASSERT((obj == NULL) || obj->IsClass(VX_Model));
		mod = (const Model*) obj;
		SetModels(mod);
		break;

		case SCENE_SetCamera:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Camera));
		cam = (const Camera*) obj;
		SetCamera(cam);
		break;

		case SCENE_SetEngines:
		s >> obj;
		VX_ASSERT((obj == NULL) || obj->IsClass(VX_Engine));
		eng = (const Engine*) obj;
		SetEngines(eng);
		break;

		default:
		return SharedObj::Do(s, op);
	   }
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Scene::DoNames[op - SCENE_UNUSED0]
					   << " " << this);
#endif
	return true;
}

} // end Vixen
