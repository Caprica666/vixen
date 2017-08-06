#pragma once

namespace Vixen {

/*!
 * @class Renderer
 * @brief Base class for encapsulating device-specific rendering functionality.
 *
 * @ingroup vixenint
 * @internal
 */
class Renderer : public SharedObj
{
	VX_DECLARE_CLASS(Renderer);

public:
	Renderer(int opts = 0);
	Renderer& operator=(const Renderer&);
	~Renderer() { }

	//! Called on startup to initialize the renderer and attach it to a scene.
	virtual	bool	Init(Scene* scene, Vixen::Window win, const TCHAR* options);

	//! Called when the scene changes to empty accumulated state.
	virtual void	Empty();

	//! Returns \b true if no geometry has been added to the renderer.
	bool			IsEmpty() const		{ return m_IsEmpty; }

	//! Get rendering  options.
	int				GetOptions() const	{ return m_Options; }

	//! Set rendering options.
	void			SetOptions(int options);

	//! Get the light list, if any
	virtual	LightList*	GetLights();

	//! Called from traversal thread at the start of each frame before display traversal for setup.
	virtual void	Reset();

	//! Called from traversal thread to add this shape to be rendered next frame.
	virtual int		AddShape(const Shape*, const Matrix*, int32 lightmask)	{ return -1; };

	//! Called from traversal thread to add this light to be used in this frame.
	virtual intptr	AddLight(Light*);

	//! Called from rendering thread before rendering begins.
	virtual void	Begin(int changed,int frame);

	//! Called from rendering thread after rendering completes (at the end of the frame).
	virtual	void	End(int frame)	{ }

	/*!
	 * Called internally from the rendering thread before
	 * rendering each frame. Copies the contents of the back
	 * buffer onto display (typically both are inside the graphics card).
	 *
	 * @see Renderer::End Renderer::Begin
	 */
	virtual	void	Flip(int frame)	{ }

	//! Called from rendering thread to render all the accumulated meshes.
	virtual void	Render(int frame, int opts = 0)	{ };

	//! Called from rendering thread to render a single mesh.
	virtual	void	RenderMesh(const Geometry* geo, const Appearance* appear, const Matrix* mtx) { }

	//! Called to free resources and/or completely shutdown.
	virtual	void	Exit(bool shutdown = true);

	//! Called on startup to initialize the renderer.
	virtual	bool	Init(Scene* scene, const TCHAR* options = NULL) { return false; }

	//! Sets the camera viewport.
	virtual	void	SetViewport(float left, float top, float right, float bottom) { }

	//! Make this shader available globally
	virtual	const Shader*	InstallShader(const Shader* inshader)	{ return NULL; }

	//! Find a global shader by name
	virtual	const Shader*	FindShader(const TCHAR* name)		{ return NULL; }

	//! Compile the shader into binary code and return a device-specific handle
	virtual	intptr	Compile(const Shader* shader) { return 0; }

	//! Class factory for renderers. Creates a renderer based on input options.
	static	Renderer*	CreateRenderer(int options = 0);

	int		RenderType;
	int		BackWidth;
	int		BackHeight;
	int		MaxLights;

protected:
	bool			m_IsEmpty;		// \b true if no primitives buffered
	uint32			m_Options;		// options
	Vixen::Scene*	m_Scene;		// scene we belong to
	LightList		m_LightList;	// cached information for light sources
};

/*!
 * @class GeoSorter
 * @brief Render state sorter for geometric primitives.
 *
 * Display traversal thread examines each Shape in the hierarchy
 * and puts each of its meshes into a bucket based on appearance
 * index. The rendering thread examines these buckets in appearance
 * order causing everything with a like appearance to be rendering
 * together. For most graphics hardware, this is more efficient because
 * it minimized render state changes.
 * 
 * A non locking allocator is used for the render state buckets and
 * its memory is reclaimed each frame after the renderer has finished.
 * This class attaches properties to each Shape in the hierarchy
 * it is attached to in order to keep traversal state used later by
 * the renderer. These come from a non locking fixed size allocator
 * bound to the traversal thread (which should be the only memory manager).
 *
 * @ingroup vixenint
 * @internal
 */
class GeoSorter : public Renderer
{
public:
	enum SortOptions
	{
		NoStateSort = 16,
		Flatten = 8,
	};

	enum RenderTypes
	{
		UserRendered = 0,
		Transparent = 1,
		Opaque = 2,
		All = Opaque | Transparent
	};

	/*!
	 * @brief Holds rendering-related state.
	 *
	 * If multi-threaded rendering is occurring, the sim/cull thread produces a
	 * RendrPrim structure for each non-culled mesh.
	 * The rendering thread concurrently works on the set of
	 * RendrPrims produced for the previous frame.
	 *
	 * @ingroup vixenint
	 * @internal
	 */
	class RenderPrim : public Core::List
	{
	public:
		RenderPrim(const Shape* shape) : Core::List()
		{
			Shape = shape;
			Matrix = NULL;
			LightMask = 0;
		}

	//! Determine if static or dynamic property (dynamic has matrix).
		bool	HasMatrix() const
		{ return (Matrix != NULL) && !Matrix->IsIdentity(); }

		const Shape*	Shape;		//!< shape to render
		Matrix*			Matrix;		//!< accumulated world matrix for owner shape
		uint32			LightMask;	//!< lights that illuminate this shape
	};

	typedef int CompareFunc(const RenderPrim* prim1, const RenderPrim* prim2);

	VX_DECLARE_CLASS(GeoSorter);

	//! Construct state sorter for the given static hierarchy.
	GeoSorter(int options = 0);

	virtual void	Exit(bool shutdown = true);
	virtual void	Empty();
	virtual	void	Reset();
	virtual void	Print(DebugOut& dbg = vixen_debug) const;
	virtual int		AddShape(const Shape*, const Matrix*, int32 lightmask);
	virtual void	Render(int frame, int opts = 0);

protected:

	//! Render the accumulated meshes for a single state.
	virtual	void		RenderState(int32 stateindex);

	//! Get state bucket index for this appearance.
	virtual int32		GetState(const Appearance*) const;

	virtual RenderPrim*	AddPrim(const Shape* shape, int stateindex, const Matrix* wmtx, int32 lightmask);
	virtual bool		SortBin(RenderPrim** listhead, CompareFunc* cmpfunc);
	static int			CompareZ(const RenderPrim* prim1, const RenderPrim* prim2);


	RenderPrim*			GetState(int32 stateindex)
	{ return m_States[stateindex]; }

	void				SetState(int32 stateindex, RenderPrim* p)
	{ m_States.SetAt(stateindex, p); }

	static	int		MaxStates;			// maximum number of states to allocate initially
	static	int		StateMask;			// bit mask to keep within range of MaxStates
	Array<RenderPrim*> m_States;		// render state buckets
	Core::FastAllocator	m_FrameAlloc;	// local heap (reused each frame)
};

} // end Vixen