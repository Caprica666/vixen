
#include "vixen.h"

namespace Vixen {

using namespace Core;

VX_IMPLEMENT_CLASS(Renderer, SharedObj);
VX_IMPLEMENT_CLASS(GeoSorter, Renderer);

int GeoSorter::MaxStates = 256;
int GeoSorter::StateMask = (MaxStates - 1);	// mask to keep within range of MaxStates


/*!
 * GeoSorter::GeoSorter(int options)
 * @param options	GeoSorter::Flatten will flatten this hierarchy by
 *					removing all matrices and applying them to the geometry.
 */
Renderer::Renderer(int options)
  :	m_Scene(NULL), m_Options(options), m_LightList()
{
	m_IsEmpty = true;
	BackWidth = 0;
	BackHeight = 0;
	RenderType = 0;
	MaxLights = LIGHT_MaxLights;
}

void Renderer::Exit(bool shutdown)
{
	Empty();
}

Renderer& Renderer::operator=(const Renderer& src)
{
	m_IsEmpty = true;
	m_Options = src.m_Options;
	m_Scene = src.m_Scene;
	BackWidth = src.BackWidth;
	BackHeight = src.BackHeight;
	RenderType = src.RenderType;
	MaxLights = src.MaxLights;
	return *this;
}

LightList*	Renderer::GetLights()
{ return &m_LightList; }

bool Renderer::Init(Scene* scene, Vixen::Window win, const TCHAR* options)
{
	if (m_Scene)
		return false;
	m_Scene = scene;
	return scene != NULL;
}

void Renderer::Reset()
{
	m_LightList.LightsOn(uint32(-1));
	m_LightList.LightsOff(uint32(-1));
	m_LightList.UpdateAll();				// transform light sources
}

void Renderer::Begin(int changed, int frame)
{
	m_LightList.LoadAll();
}

void Renderer::Empty()
{
	m_LightList.DetachAll();
}

void Renderer::SetOptions(int options)
{
	m_Options = options;
}

intptr Renderer::AddLight(Light* light)
{
	GPULight* prop = (GPULight*) light->DevHandle;

	if (prop)
		return prop->DevIndex;
	prop = new GPULight(*this, *light);
	if ((prop == NULL) ||
		(prop->ID >= MaxLights))
	{ VX_ERROR(("AddLight ERROR cannot add light #%d\n", prop->ID), -1); }
	prop->DevIndex = m_LightList.Attach(light, prop);
	return prop->DevIndex;
}

/*!
 * GeoSorter::GeoSorter(int options)
 * @param options	GeoSorter::Flatten will flatten this hierarchy by
 *					removing all matrices and applying them to the geometry.
 */
GeoSorter::GeoSorter(int options)
  :	Renderer(options)
{
	SetOptions(options);
	m_States.SetMaxSize(256);
}

void GeoSorter::Exit(bool shutdown)
{
	Empty();
	m_States.Empty();
}

/*!
 * @fn bool GeoSorter::AddShape(const Shape* shape, const Matrix* mtx, int32 lightmask)
 * @param	shape	shape whose geometry should be added
 * @param	mtx		pointer to world matrix for shape, if NULL camera matrix is used
 * @param	lightmask bit mask for lights which illuminate this shape
 *
 * Adds the meshes in the given shape to this sorter
 * to be rendered later. This step sorts the meshes
 * into different buckets based on the appearance of each.
 */
int GeoSorter::AddShape(const Shape* shape, const Matrix* mtx, int32 lightmask)
{
	const Geometry*		geo = shape->GetGeometry();
	const Appearance*	appear;
	Appearance*			cheat;
	int					stateindex;
	RenderPrim*			newprim;

	if ((geo == NULL) || geo->IsSet(GEO_Culled))
		return -1;
	appear = shape->GetAppearance();
	cheat = (Appearance*) appear;
	if (appear)
		stateindex = appear->GetAppIndex();
	else
		stateindex = 1;
	newprim = AddPrim(shape, stateindex, mtx, lightmask);
	return stateindex;
}

GeoSorter::RenderPrim* GeoSorter::AddPrim(const Shape* shape, int stateindex, const Matrix* mtx, int32 lightmask)
{
	RenderPrim* newprim = new (&m_FrameAlloc) RenderPrim(shape);
	m_IsEmpty = false;
	if (stateindex < (int) m_States.GetSize())
	{
		RenderPrim* lastprim = GetState(stateindex);
		if (lastprim)
			newprim->Next = lastprim;
	}
	if (mtx)
		newprim->Matrix = new (&m_FrameAlloc) Matrix(*mtx);
	newprim->LightMask = lightmask;
	SetState(stateindex, newprim);
	return newprim;
}


/*!
 * @fn int GeoSorter::CompareZ(const RenderPrim* p1, const RenderPrim* p2)
 * Internal routine to compare RenderPrims based on their Z coordinate.
 * Used in Z sorting transparent primitives below. The Z coordinate
 * is computed by taking the Z of the first vertex of the primitive
 * and transforming it by the current matrix stored in the DynShape
 * property (if there is one)
 */
int GeoSorter::CompareZ(const RenderPrim* p1, const RenderPrim* p2)
{
	const Geometry*	g1 = p1->Shape->GetGeometry();
	const Geometry*	g2 = p2->Shape->GetGeometry();
	Vec3			v1 = g1->GetSortLoc();
	Vec3			v2 = g2->GetSortLoc();
	Scene::TLS*		tls = Scene::GetTLS();
	const float (*m1)[4][4];
	const float (*m2)[4][4];

	if (p1->HasMatrix())				// shape has world matrix?
		m1 = (const float (*)[4][4]) p1->Matrix->GetMatrix();
	else
		m1 = (const float (*)[4][4]) Matrix::GetIdentity();
	if (p2->HasMatrix())
		m2 = (const float (*)[4][4]) p2->Matrix->GetMatrix();
	else								// use camera matrix
		m2 = (const float (*)[4][4]) Matrix::GetIdentity();

	v1.z = v1.x * (*m1)[2][0] + v1.y * (*m1)[2][1] + v1.z * (*m1)[2][2] + (*m1)[2][3];
	v2.z = v2.x * (*m2)[2][0] + v2.y * (*m2)[2][1] + v2.z * (*m2)[2][2] + (*m2)[2][3];
	if (v1.z < v2.z)					// compare Zs
		return -1;
	if (v1.z > v2.z)
		return 1;
	return 0;
}


/*!
 * @fn void GeoSorter::Render(int opts)
 * @param scene	scene to render
 * @param opts	determines which primitives to render:
 *	- GeoSorter::Opaque render only opaque primitves
 *	- GeoSorter::Transparent render only transparent primitives
 *	- GeoSorter::All render all primitives
 */
void GeoSorter::Render(int frame, int opts)
{
	if (m_States.IsEmpty())
		return;
	if (m_States.GetSize() == 0)
		return;
	if (opts & Opaque)					// render opaque stuff?
	{
		for (int stateindex = Opaque; stateindex < (int) m_States.GetSize(); ++stateindex)
		{
			VX_ASSERT(stateindex < INT_MAX);
			RenderState(stateindex);
		}
	}
	if (opts & Transparent)			// render transparent stuff?
	{
		if (SortBin((RenderPrim**) &m_States.GetAt(0), &CompareZ))
			RenderState(Transparent);
	}
}

bool GeoSorter::SortBin(RenderPrim** listhead, CompareFunc* cmpfunc)
{
	bool	xchg = true;

	if (*listhead == NULL)
		return false;
	while (xchg)						// bubble sort the list
	{
		List** prevptr = (List**) listhead;
		List*	cur = *prevptr;
		List*	next = cur->Next;
		xchg = false;
		while (next = cur->Next)		// for each list item
		{
			if ((*cmpfunc)((RenderPrim*) cur, (RenderPrim*) next) > 0)
			{
				cur->Next = next->Next;	// swap the two
				next->Next = cur;
				*prevptr = next;
				prevptr = &(next->Next);
				xchg = true;			
			}
			else						// skip to next one
			{
				prevptr = &(cur->Next);
				cur = next;
			}
		}
	}
	return true;
}

void GeoSorter::RenderState(int32 stateindex)
{
	RenderPrim* prim = GetState(stateindex);
	while (prim)
	{
		const Shape*	shape = prim->Shape;

		m_LightList.LightsOn(prim->LightMask);
	#if _TRACE > 1
		if ((Appearance::Debug > 1) || (Scene::Debug > 1))
		{
			Core::String name(prim->Shape->GetName());
			char namebuf[128];

			name.AsMultiByte(namebuf, 128);
			VX_PRINTF(("<scene_rendershape name='%s' stateindex='%d' >\n", namebuf, stateindex));
			RenderMesh(shape->GetGeometry(), shape->GetAppearance(), prim->Matrix);
			prim = (RenderPrim*) prim->Next;
			VX_PRINTF(("</scene_rendershape>"));
		}
		else
	#endif
		{
			RenderMesh(shape->GetGeometry(), shape->GetAppearance(), prim->Matrix);
			prim = (RenderPrim*) prim->Next;
		}
		shape->SetRendered(true);
	}
}


/*!
 * @fn void GeoSorter::Reset()
 *
 * Called internally at the start of each frame to indicate
 * this primlist is now active during display traversal.
 * The default behavior is to empty this frame's state
 * sorting buckets. This function is called by the display
 * traversal (sim/cull) thread.
 *
 * @see GeoSorter::Empty SceneThread
 */
void	GeoSorter::Reset()
{
	Renderer::Reset();
	if (!m_IsEmpty)
	{
		for (int i = 0; i < (int) m_States.GetSize(); ++i)
		{
			VX_ASSERT(i < INT_MAX);
			SetState(i, NULL);
		}
		m_IsEmpty = true;
	}
	m_FrameAlloc.Empty();
}

/*!
 * @fn int32 GeoSorter::GetState(Appearance* app) const
 *
 * Computes the state index based on the information in the appearance
 *
 * @return state bucket index for this appearance
 */
int32 GeoSorter::GetState(const Appearance* app) const
{
	if (app)
	{
		const DeviceBuffer* mat = app->GetMaterial();
		int	appidx = app->GetAppIndex();

		if (app->Get(Appearance::TRANSPARENCY))
			return Transparent;
		if (mat && mat->IsKindOf(CLASS_(PhongMaterial)))
		{
			Col4 c = ((PhongMaterial*) mat)->GetDiffuse();
			if (c.a < 1.0f)
				return Transparent;
		}
		if (m_Options & NoStateSort)
			return Opaque;
		return (appidx  & StateMask) + Opaque;
	}
	return Opaque;
}


/*!
 * @fn void GeoSorter::Empty()
 *
 * The state sorting bucket elements and the entries they reference
 * are allocated from a non-locking fast allocator.
 * This routine reclaims all the memory and is called
 * at the end of each frame.
 */
void GeoSorter::Empty()
{
	Renderer::Empty();
	if (m_IsEmpty)
		return;
	for (int i = 0; i < (int) m_States.GetSize(); ++i)
	{
		VX_ASSERT(i < INT_MAX);
		SetState(i, NULL);
	}
	m_IsEmpty = true;
	m_FrameAlloc.FreeAll();
}

void GeoSorter::Print(DebugOut& dbg) const
{
	endl(dbg << "<geosorter numstates='" << m_States.GetSize() << "'>");
	for (intptr i = 0; i < m_States.GetSize(); ++i)
	{
		RenderPrim* prim = m_States.GetAt(i);
		if (prim)
			endl(dbg << "<state index='" << i << "' meshcount='" << prim->GetSize() << "'/>");
	}
	endl(dbg << "</geosorter>");
}

}	// end Vixen