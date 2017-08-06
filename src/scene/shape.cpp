/****
 *
 * Shape Class
 * Shapes are models described as a geometric mesh with
 * textures applied to the faces.
 *
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Shape, Model, VX_Shape);

static	const TCHAR* opnames[] =
	{ TEXT("SetAppearance"), TEXT("SetGeometry") };

const TCHAR** Shape::DoNames = opnames;

/****
 *
 * Print shape description on standard output.
 *
 ****/
DebugOut& Shape::Print(DebugOut& dbg, int opts) const
{
	const Geometry*	geo = m_Geometry;
	const Appearance*	app = m_Appearance;

	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Model::Print(dbg, opts & ~PRINT_Trailer);
	if (app)
		app->Print(dbg, opts | PRINT_Trailer);
	if (geo)
		geo->Print(dbg, opts | PRINT_Trailer);
	return Model::Print(dbg, opts & PRINT_Trailer);
}

/****
 *
 * class Shape override for Model::CalcBound
 * Calculate the bounding box of the geometry represented by the shape.
 *
 ****/
bool Shape::CalcBound(Box3* b) const
{
	VX_ASSERT(b);
	if (m_Geometry.IsNull())					// no surface?
		return false;
	return (m_Geometry->GetBound(b));		// calculate bounds
}

/****
 *
 * class Shape override for Model::CalcSphere
 * Calculate the bounding sphere of the geometry represented by the shape.
 *
 ****/
bool Shape::CalcSphere(Sphere* sp) const
{
	VX_ASSERT(sp);
	if (m_Geometry.IsNull())					// no surface?
		return false;
	Box3 b;
	if (!m_Geometry->GetBound(&b))			// no bounds?
		return false;
	*sp = b;
	return true;
}

void Shape::SetHints(int hints, bool descend)
{
	const Mesh*			mesh = (const Mesh*) GetGeometry();
	const VertexArray*	verts = NULL;

	if (mesh && mesh->IsClass(VX_Mesh))
		verts = mesh->GetVertices();
	if (hints & MORPH)
	{
		m_AutoBounds = true;
		if (verts)
			verts->SetFlags(VertexPool::MORPH);
	}
	return Model::SetHints(hints, descend);
}

/****
 *
 * class Shape override for SharedObj::Copy
 * Copy the contents of one shape into another. Copies are not made
 * of allocated attributes - we just copy the pointers. If the
 * source class is a subclass of Shape, we copy the shape attributes.
 * Otherwise, these are left alone.
 *
 ****/
bool Shape::Copy(const SharedObj* src_obj)
{
	const Shape*	src = (const Shape*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Model::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_Shape))
	{
		m_Geometry = src->m_Geometry;
		m_Appearance = src->m_Appearance;
	}
	return true;
}


/*!
 * @fn void Shape::SetAppearance(const Appearance* app)
 * @param app	appearance object
 *
 * Changes the appearance used by the primitives in the mesh.
 *
 * @see Appearance Shape::SetAppearance
 */
void Shape::SetAppearance(const Appearance* app)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Shape, SHAPE_SetAppearance) << this << app;
	VX_STREAM_END( )

	m_Appearance = app;
}


/*!
 * @fn void Shape::SetGeometry(const Geometry* surf)
 * @param surf	surface geometry to associate with model.
 *				If NULL, model will have no geometry (will not be visible)
 *
 * The surface is the geometric description for the model.
 * It describes the overall form in terms of triangles,
 * patches or other geometric constructs.
 *
 * @see Shape::SetAppearance Shape::Render
 */
void Shape::SetGeometry(const Geometry* geo)
{
	const VertexArray*	verts = NULL;

	if (geo)
	{
		if (IsGlobal())
			GetMessenger()->Distribute(geo, GLOBAL);
		if (geo->IsClass(VX_Mesh))
		{
			verts = ((const Mesh*) geo)->GetVertices();
			if (m_Hints & Model::MORPH)
				verts->SetFlags(VertexPool::MORPH);
		}
	}

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Shape, SHAPE_SetGeometry) << this << geo;
	VX_STREAM_END( )

	m_Geometry = geo;
	NotifyParents(MOD_BVinvalid | MOD_STinvalid); // mark bounding volume as invalid
	m_Faces = 0;
	m_Verts = 0;
}

/*!
 * @fn void Shape::CalcStats() const
 *
 * This function can compute the number of vertices and faces in this shape's
 * geometry and its children.
 * Once computed, the vertex and face counts are cached until the geometry changes.
 */
void Shape::CalcStats() const
{
	const Mesh* geo = (const Mesh*) GetGeometry();

	if (!IsSet(MOD_STinvalid))
		return;
 	if (geo && geo->IsClass(VX_Mesh))
	{
		Core::InterlockAdd(&m_Verts, geo->GetNumVtx());
		Core::InterlockSet(&m_Faces, geo->GetNumFaces());
	}
	else
	{
		Core::InterlockSet(&m_Faces, 0);
		Core::InterlockSet(&m_Verts, 0);
	}
	Model::CalcStats();
}

void Shape::CalcStats(intptr& numverts, intptr& numfaces) const
{
	const Mesh* geo = (const Mesh*) GetGeometry();

 	if (geo && geo->IsClass(VX_Triangles))
	{
		numverts += geo->GetNumVtx();
		numfaces += geo->GetNumFaces();
	}
}

/*!
 * @fn void Shape::Render(Scene* scene)
 * @param scene	scene shape is rendering in
 *
 * Displays the geometry in the Shape using
 * the Shape's Appearance. This process may
 * change the geometry or update internal information
 * in the shape so this routine is not  const.
 *
 * @see Shape::Cull Mesh::Render Model::Render
 */
void Shape::Render(Scene* scene)
{
/*
 * If this scene is multi-threaded or doing state sorting,
 * find the appropriate ShapeInfo property to update. In this case,
 * real rendering is done later in another thread and only the
 * ShapeInfo properties are updated.
 */
	Renderer*		render = (Renderer*) scene->GetRenderer();
	uint32			lightmask = LIGHT_All;
	LightList*		lights = render->GetLights();
	const Matrix*	mv;

	if (m_Geometry.IsNull())
		return;
	if (m_Hints & STATIC)					// static geometry - no matrix
		mv = NULL;
	else
		mv = scene->GetWorldMatrix();		// get world matrix from scene
	if (lights)
		lightmask = lights->NearLights(this, mv, LIGHT_All);
/*
 * State sorting enabled. Add each mesh in the surface
 * to a state-sorting bucket depending on its appearance index
 */
	render->AddShape(this, mv, lightmask);
}

/*!
 * @fn const Model* Shape::Cull(const Matrix* trans, Scene* scene)
 * @param	trans	local to world coordinate matrix
 * @param	scene	scene displaying the shape
 *
 * This function is called during scene traversal to determine
 * whether this shape should be culled or not. The default
 * implementation culls based on the bounding box of the Surface.
 *
 * @return DISPLAY_NONE if model should be culled, DISPLAY_ALL to display all children
 *
 * @see Model::Cull
 */
intptr Shape::Cull(const Matrix* trans, Scene* scene)
{
	Geometry*	geo = GetGeometry();
	Scene::TLS*	tls = Scene::GetTLS();
	SceneStats*	s = scene->GetStats();
	intptr		c;

	m_Rendered = false;
	if (m_NoCull || !DoCulling)				// do not cull this one
	{
		if (geo)							// mark all as not culled
			geo->Cull(NULL, NULL);
		Core::InterlockAdd(&(s->TotalVerts), (int) m_Verts);
		Core::InterlockInc(&(s->TotalModels));
		return DISPLAY_ALL;
	}
	c = Model::Cull(trans, scene);
	if (c == DISPLAY_NONE)
		return DISPLAY_NONE;
	if (m_Hints & STATIC)
		trans = NULL;
	if (geo)
	{
		intptr nculled = geo->Cull(trans, scene);
		intptr numvtx = geo->GetNumVtx();
		Core::InterlockAdd(&(s->CulledVerts), (int) nculled);
		Core::InterlockAdd(&(s->TotalVerts), (int) numvtx);
	}
	return c;
}

/****
 *
 * class Shape override for SharedObj::Do
 *		Shape::SetAppearance	<Appearance*>
 *		Shape::SetGeometry	<Geometry*>
 *
 ****/
bool Shape::Do(Messenger& s, int op)
{
	SharedObj*			obj;
	const Appearance*	app;
	const Geometry*		geo;
	Opcode				o = Opcode(op);

	switch (op)
	{
		case SHAPE_SetAppearance:
		s >> obj;
		if (obj) { VX_ASSERT(obj->IsClass(VX_Appearance)); }
		app = (const Appearance*) obj;
		SetAppearance(app);
		break;

		case SHAPE_SetGeometry:
		s >> obj;
		if (obj) { VX_ASSERT(obj->IsClass(VX_Geometry)); }
		geo = (const Geometry*) obj;
		SetGeometry(geo);
		break;

		default:
		return Model::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
	{
		vixen_debug << ClassName() << "::"
				  << Shape::DoNames[op - SHAPE_SetAppearance]
				   << " " << this;
		endl(vixen_debug);
	}
#endif
	return true;
}

/****
 *
 * class Shape override for SharedObj::Save
 *
 ****/
int Shape::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	if (h < 0)
		return h;
	const Appearance* app = GetAppearance();
	const Geometry* surf = GetGeometry();
	if (opts != Messenger::MESS_ClearGlobal)
	{
		if (app && (app->Save(s, opts) >= 0) && h)
			s << OP(VX_Shape, SHAPE_SetAppearance) << h << app;
	}
	if (surf && (surf->Save(s, opts) >= 0) && h)
		s << OP(VX_Shape, SHAPE_SetGeometry) << h << surf;
	return h;
}

}	// end Vixen