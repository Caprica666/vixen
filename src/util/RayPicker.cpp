
#include "vixen.h"
#include "vxutil.h"
#include "scene/vxoctree.h"

namespace Vixen {

#define	PICK_ObjPicked	(OBJ_LASTUSED<<1)	// indicate object was picked

/*!
 * @class HiliteProp
 * @brief Used to hilite a shape by changing its appearance table.
 *
 * When a shape is selected, a HiliteProp is attached to save the appearances
 * table associated with the shape so a new one can be used for hiliting.
 * When an appearances table is shadowed, a HiliteProp is attached to the
 * VXAppearances object so the shadow is made only once.
 *
 * @ingroup vixenint
 */
#define	PROP_Hilite		intptr(CLASS_(HiliteProp))

class HiliteProp : public Property
{
public:
	VX_DECLARE_CLASS(HiliteProp);
	HiliteProp();
	void	Hilite(Model* mod, Appearance* hilite);
	void	UnHilite(Model* mod);
	void	HilitePicked(Model* mod);
	void	DebugPrint(Model* mod, const TCHAR* code);

	Ref<Appearance>	OldShapeAppear;
	Ref<Appearance>	HiliteAppear;
	bool			IsHilited;
};

static const TCHAR* opnames[] =
{	TEXT("SetOptions"),
	TEXT("SetRay"),
	TEXT("SetHilite"),
	TEXT("SetButtons"),
	TEXT("SetBound"),
	TEXT("SetNameFilter")
};

const TCHAR** RayPicker::DoNames = opnames;

VX_IMPLEMENT_CLASSID(Picker, MouseEngine, VX_Picker);

VX_IMPLEMENT_CLASSID(RayPicker, Picker, VX_RayPicker);

VX_IMPLEMENT_CLASSID(NamePicker, RayPicker, VX_NamePicker);

VX_IMPLEMENT_CLASS(HiliteProp, Property);

HiliteProp::HiliteProp() : Property()
{
	OldShapeAppear = (Appearance*) NULL;
	HiliteAppear = (Appearance*) NULL;
	IsHilited = false;
}

/*
 * Hilite the given model by saving its current appearance
 * and replacing it with this one.
 * If hilite appearance has texturing enabled but no texture,
 * use the shape's texture
 */
void HiliteProp::Hilite(Model* mod, Appearance* hilite)
{
	Shape*			shape = (Shape*) mod;

	if (IsHilited)
		return;
	IsHilited = true;
	if (hilite == NULL)
		return;
	if (!shape->IsClass(VX_Shape))
		return;
	DebugPrint(mod, TEXT("Hilite"));
	OldShapeAppear = shape->GetAppearance();
	if ((HiliteAppear == NULL) && !OldShapeAppear.IsNull())
	{
		Appearance* newapp = hilite;
		Sampler* newsmp = hilite->GetSampler(0);
		Sampler* oldsmp = OldShapeAppear->GetSampler(0);
		if ((!newsmp || (newsmp->GetTexture() == NULL)) &&
			oldsmp && (oldsmp->GetTexture() != NULL))
		{
			newapp = (Appearance*) hilite->Clone();
			newapp->SetSampler(0, oldsmp);
		}
		HiliteAppear = newapp;
	}
	shape->SetAppearance(HiliteAppear);
}

void HiliteProp::UnHilite(Model* mod)
{
	Shape*		shape = (Shape*) mod;

	IsHilited = false;
	if (!shape->IsClass(VX_Shape) || (shape->GetGeometry() == NULL))
		return;
	DebugPrint(mod, TEXT("UnHilite"));
	if (!OldShapeAppear.IsNull())
	{
		shape->SetAppearance(OldShapeAppear);
		OldShapeAppear = NULL;
	}
}

/*
 * HilitePicked is called when HILITE_PICKED is set requesting the
 * picked shape to be permanently hilited even though it is no longer selected.
 * This is done by replacing the shape's appearance with the hilite appearance
 */
void HiliteProp::HilitePicked(Model* mod)
{
	Shape*	shape = (Shape*) mod;
	IsHilited = false;
	if (!shape->IsClass(VX_Shape) || !HiliteAppear)
		return;
	DebugPrint(mod, TEXT("HilitePicked"));
	if (OldShapeAppear->GetMaterial())
		HiliteAppear->SetMaterial(OldShapeAppear->GetMaterial());
	shape->SetAppearance(HiliteAppear);
	HiliteAppear = (Appearance*) NULL;
}

void HiliteProp::DebugPrint(Model* mod, const TCHAR* code)
{
#ifdef _DEBUG
	Shape*	shape = (Shape*) mod;
	Texture*	image = NULL;
	Sampler*	smp = NULL;
	Appearance* app = shape->GetAppearance();
	if (app)
	{
		smp = app->GetSampler(0);
		if (smp)
			image = smp->GetTexture();
	}
	if (image)
	{
		VX_TRACE2(Picker::Debug, ("Picker::%s: %s %p\n", code, mod->GetName(),
				image->GetFileName(), image->GetBitmap()));
	}
	else
	{ VX_TRACE2(Picker::Debug, ("Picker::%s: %s\n", code, mod->GetName())); }
#endif
}

Picker::Picker() : MouseEngine()
{
	m_Nearest = (Model*) NULL;
	m_Closest = FLT_MAX;
	m_Options = HILITE_SELECTED;
	DoPick = false;
	DoHilite = false;
	m_Buttons = MouseEvent::SHIFT | MouseEvent::RIGHT;
	SetChanged(false);
	SetFlags(SharedObj::DOEVENTS);
	World3D::Get()->GetMessenger()->Observe(this, Event::MOUSE, NULL);
}

/*!
 * @fn void	Picker::SetOptions(int opts)
 * @param opts	pick options to set
 *
 * Establishes behavior for hit testing and selection. Bounding volume hit testing
 * is the default, gives the best performance but is not very accurate. Mesh
 * picking tests against the triangles in a mesh and is slower but gives better results.
 * Selection hilighting will be performed if a hilite appearance is used.
 * @code
 *	BOUNDS			pick shapes using bounding spheres and boxes
 *	MESH			pick shapes using mesh geometry
 *	PICK_MODEL		allow groups as well as shapes to be selected
 *	SELECT_PARENT	if shape is hit but can't be selected but it's parent can,
 *					hilite shape and siblings and select only the parent
 *	HILITE_SELECTED	apply hilite appearance to selected objects
 *	HILITE_PICKED	apply hilite appearance to picked objects
 *	NAME_FILTER		restrict selection of objects based on name
 * @endcode
 *
 * @see Picker::SetHilite NamePicker
 */
void	Picker::SetOptions(int opts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_RayPicker, PICK_SetOptions) << this << int32(opts);
	VX_STREAM_END()

	m_Options = opts;
}

/*!
 * @fn void Picker::SetHilite(const Appearance* hilite)
 * @param hilite	appearance to use to display selected shapes
 *
 * Provides the appearance to be used for highlighting selections on mouse over.
 * If an appearance is specified, it is applied to all selected geometry
 * as the mouse cursor moves. Objects no longer picked are returned to
 * their previous state so selection hilighting is non-destructive to the scene.
 *
 * If the shape's original appearance was multitextured, the hilite appearance
 * is merged with the original to create a new blended appearance. The first image
 * in the original that is NULL is replaced by the hilite appearance image.
 * If the original is not multitextured, the hilite appearance is used directly
 * for selection hiliting without blending.
 *
 * @see Picker::SetOptions Picker::SetNameFilter
 */
void Picker::SetHilite(const Appearance* hilite)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_RayPicker, PICK_SetHilite) << this << hilite;
	VX_STREAM_END()

	const Model* target = (const Model*) GetTarget();
	m_HiliteApp = hilite;
}

/*!
 * @fn void	Picker::SetButtons(int button)
 * @param button mouse button to use for picking, one of: MouseEvent::CONTROL,
 *				MouseEvent::SHIFT, MouseEvent::LEFT, MouseEvent::RIGHT, MouseEvent::MIDDLE
 *
 * Selection hiliting is done while the mouse button or key is down and a
 * pick event is emitted on mouse/key up. Multiple selections are not
 * currently supported.
 */
void	Picker::SetButtons(int b)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_RayPicker, PICK_SetButtons) << this << int32(b);
	VX_STREAM_END()

	m_Buttons = b;
}

bool Picker::Eval(float time)
{
	if (!HasChanged() || !DoHilite)		// don't repick unless something changed
		return true;

	Scene*	scene = GetMainScene();
	Model*	root = (Model*) GetTarget();
	Model*	parent;
	bool	result;
	bool	dopick = DoPick;
	Camera*	cam = scene->GetCamera();

	m_Nearest = (Model*) NULL;		// clear current selection
	SetChanged(false);
	m_Closest = FLT_MAX;
	if (root == NULL)					// no target, don't select
		return true;
	VX_ASSERT(root->IsClass(VX_Model));
	parent = root->Parent();
	if (parent)
		parent->TotalTransform(&m_TotalMatrix);
	else
		m_TotalMatrix.Identity();
	UpdateHilite();						// unhilite previously selected stuff
	result = Pick(root);				// pick result
	if (result)
		SetFlags(PICK_ObjPicked);
	else
		ClearFlags(PICK_ObjPicked);
	if (!dopick)						// not picking?
		return true;
/*
 * Unhilite all items and mark everything as unselected.
 * Output an event to indicate something or nothing was picked.
 */
	if (result && m_Selected.GetSize())	// ray hit something?
	{
		UnHiliteAll(true);
		OnPick();
	}
	else
	{
		UnHiliteAll(false);				// unhilite everything
		OnNoPick();						// no, notify nothing picked
	}
	DoPick = false;
	DoHilite = false;					// disable hiliting also
	return true;
}

/*
 * Called during selection hiliting to unhilite deselected objects.
 * m_Hilited contains objects currently displayed as hilited. The IsHilited flag is
 * set on the hilite propery of objects that should still be hilited next frame.
 * We unhilite the objects that are no longer selected and delete their hilite properties
 */
void Picker::UpdateHilite()
{
	for (intptr i = 0; i < m_Hilited.GetSize(); ++i)
	{
		Ref<Shape> shape = (Shape*) (SharedObj*) m_Hilited.GetAt(i);
		HiliteProp* hp = (HiliteProp*) shape->GetProp(PROP_Hilite);

		if (hp && !hp->IsHilited)
		{
			if (IsSet(SharedObj::DOEVENTS))
			{
				PickEvent* e = new PickEvent(Event::DESELECT, this);
				e->Target = shape;
				e->Log();
			}
			if (m_Options & HILITE_SELECTED)
				hp->UnHilite(shape);
			shape->DelProp(PROP_Hilite);
			m_Hilited.RemoveAt(i--);
			intptr selofs = m_Selected.Find((const Shape*) shape);
			if (selofs >= 0)
				m_Selected.RemoveAt(selofs);
		}
	}
}

/*
 * m_Selected contains objects currently selected.
 * We unhilite and unselect everything if nothing was picked.
 * If something was picked, we unhilite only if HILITE_PICKED was not enabled.
 */
void Picker::UnHiliteAll(bool didpick)
{
	for (intptr i = 0; i < m_Hilited.GetSize(); ++i)
	{
		Ref<Shape> shape = (Shape*) (SharedObj*) m_Hilited.GetAt(i);
		HiliteProp* hp = (HiliteProp*) shape->GetProp(PROP_Hilite);

		if (hp)
		{
			if (didpick)
			{
				if (m_Options & HILITE_PICKED)
					hp->HilitePicked(shape);	// permanently hilite picked objects
				if (IsSet(SharedObj::DOEVENTS))
				{
					PickEvent* e = new PickEvent(Event::DESELECT, this);
					e->Target = shape;
					e->Log();
				}
			}
			else
			{
				if (m_Options & HILITE_SELECTED)
					hp->UnHilite(shape);		// unhilite object (restore original appearance)
				shape->DelProp(PROP_Hilite);
			}
		}
	}
	m_Hilited.Empty();
	if (!didpick)							// leave selected if picked
		m_Selected.Empty();
}

/*!
 * @fn bool Picker::Pick(Model* root)
 * @param root	root of hierarchy to pick from
 *
 *	Determines which object in the given hierarchy, if any,
 *	was picked. The entire hierarchy is traversed breadth
 *	first using the Hit function to determine if an
 *	individual model was picked and how far away it is.
 *	If an object was hit and it is closer than the previously
 *	picked object, it will become the current selection.
 *
 *	You can override Pick() to control how objects are picked
 *	with respect to one another. If you want to change
 *	the mechanism used to determine whether or not an object
 *	was picked, you should override Hit() instead.
 *
 * @return  true if an object in the hierarchy was picked, else  false
 *
 * @see Picker::Hit Picker::DoSelect
 */
bool Picker::Pick(Model* root)
{
	float	closest = 0.0f;

	if (root == NULL)
		return false;
	if (root->IsClass(VX_Octree))			// octree is optimized path
	{
		if (!Hit(root, &closest))			// does not require matrix concatenation
			return false;
		return DoSelect(root, closest, m_Options);
	}

	bool	rc = false;
	bool	pickmesh = (m_Options & Picker::MESH) != 0;
	Matrix	savemtx(m_TotalMatrix);

	VX_ASSERT(root->IsClass(VX_Model));
	m_TotalMatrix.PostMul(*(root->GetTransform()));
	if (Hit(root, &closest))			// test for hit
	{
		intptr c = root->Cull(&m_TotalMatrix, GetMainScene());
		if (c == Model::DISPLAY_ALL)
		{
			GroupIter<Model>	iter(root, Group::CHILDREN);
			Model* mod;

			while (mod = iter.Next())
				if (Pick(mod))
					rc = true;
		}
		if (DoSelect(root, closest, m_Options))	// root was hit
			rc = true;
	}
	m_TotalMatrix.Copy(&savemtx);
	return rc;
}

/*!
 * @fn bool Picker::Hit(const Model* mod, float* dist)
 * @param mod	model to hit test
 * @param dist	distance of object hit is returned here
 *
 * Determines if a model was picked or not. The mechanism
 * used for picking varies depending on the type of picker.
 * For example, RayPicker implements Hit to determine if
 * a ray intersected any the model. The base implementation
 * always returns  false.
 *
 * @return  true if the model was hit, else  false
 *
 * @see Picker::Pick Picker::DoSelect
 */
bool Picker::Hit(const Model* mod, float* dist)
{
	return false;
}

/*!
 * @fn bool Picker::DoSelect(Model* mod, float closest, int opts)
 * @param mod		model to test for hit against
 * @param closest	closest point on model that was hit
 * @param opts		selection options (one or more of MULTI_SELECT, SELECT_PARENT)
 *
 * Adds the input model to a list of selected objects (all objects
 * which were picked this frame). Also tracks the closest
 * selected object. If the Picker::PICK_MODEL option is selected, models
 * without geometry are selectable. By default, this options is
 * not enabled and only shapes which have geometry may be picked.
 *
 * @return  true if model selected, else  false
 *
 * @see Picker::Pick Picker::Hit Picker::SetOptions
 */
bool Picker::DoSelect(Model* mod, float closest, int opts)
{
	int	rc = CanSelect(mod, closest);
	if (rc > 0)						// a shape was hit?
	{
		if ((opts & MULTI_SELECT) == 0)
			UnHiliteAll();
		if (HiliteModel(mod))		// hilite if new
			m_Selected.Append(mod);	// add to selection
		return true;
	}
	if (!(opts & SELECT_PARENT))	// select the group, hilite children?
		return false;				// shape not picked
/*
 * If Picker::SELECT_PARENT is enabled, check to see if the parent is selectable.
 * If so, hilite all of the children (even those not hit) but select only
 * the parent group. No locking is necessary because the caller has
 * already locked the hierarchy.
 */
	Model* parent = mod->Parent();
	if (rc == 0)					// a group was hit?
		parent = mod;
	else							// a shape was hit?
	{
		parent = mod->Parent();
		if ((closest == 0) || !parent || (CanSelect(parent, closest) < 0))
			return false;
	}
	if (opts & MULTI_SELECT)
	{
		if (HiliteModel(parent))		// hilite if new
			m_Selected.Append(parent);	// add to selection
		Model* tmp = parent->First();	// hilite the children
		while (tmp)						// for each child
		{
			HiliteModel(tmp);			// is object already selected?
			tmp = (Model*) tmp->Next();
		}
	}
	else
	{
		UnHiliteAll();
		if (HiliteModel(parent))		// hilite if new
			m_Selected.Append(parent);	// add to selection
	}
	return true;
}

/*!
 * @fn int Picker::CanSelect(const Model* mod, float dist)
 * @param mod	model to check for selectibility
 * @param dist	distance of geometry from viewer
 *
 * Determines if this model can be selected based on the picking options.
 * If geometry was actually hit, the  dist argument will be nonzero.
 * If a bounding volume of a group was hit,  dist will be zero.
 * The Picker::PICK_MODEL option controls whether or not groups may be selected.
 * If it is not set, only shapes with geometry that was actually hit will
 * pass the selection test.
 *
 * This routine also chooses the nearest object to the viewer.
 * Only shapes whose geometry was hit can be returned as the nearest object.
 * Even in the bounding volume picking case, a tight
 * axially aligned bounding box in model space is used so the distance
 * is reasonable. Groups use bounding spheres which do not give accurate
 * distances and cannot be returned as nearest object.
 *
 * @return positive if model can be selected, else zero
 *
 * @see Picker::DoSelect
 */
int Picker::CanSelect(const Model* mod, float dist)
{
	if (mod->IsClass(VX_Shape) &&
		(((Shape*) mod)->GetGeometry() != NULL))
	{
		if (dist == 0.0f)			// mesh picking failed, don't select
			return 0;
	}
	else if (!(m_Options & Picker::PICK_MODEL))
		return 0;					// groups not allowed
	if (dist < m_Closest)			// closest shape so far?
	{
		m_Closest = dist;			// track nearest shape
		m_Nearest = (Model*) mod;
	}
	return 1;
}

bool Picker::HiliteModel(Model* mod)
{
	HiliteProp*		hp = (HiliteProp*) mod->GetProp(PROP_Hilite);
	Appearance*		hilite = m_HiliteApp;
	const TCHAR*	name = mod->GetName();
	bool			rc = false;
	bool			ishilited = false;

	if (name == NULL)				// cannot hilite unnamed things
		return false;
	if (hp == NULL)					// no hilite property?
	{
		hp = new HiliteProp();		// attach hilite property to shape
		mod->AddProp(hp);
		m_Hilited.Append(mod);
	}
	else ishilited = hp->IsHilited;
	if (!ishilited && IsSet(SharedObj::DOEVENTS))
	{
		PickEvent* e = new PickEvent(Event::SELECT, this);
		e->Target = mod;
		e->Log();
	}
	if (m_Options & HILITE_SELECTED)
		hp->Hilite(mod, hilite);
	else
		hp->IsHilited = true;
	return !ishilited;
}

/*!
 * @fn void Picker::OnPick()
 *
 * Called when an object has been picked. Usually, your
 * Picker subclasses override this routine to handle
 * a pick event. Only one pick event will be generated
 * per frame for each active picker. If no objects were
 * picked, OnNoPick is called. The default behavior is
 * to log a generic pick event (Event::PICK) and
 * print the name of the nearest object (in the debug version).
 *
 * @see Picker::OnNoPick Picker::Pick Picker::Hit
 */
void Picker::OnPick()
{
	if (IsSet(SharedObj::DOEVENTS))
	{
		PickEvent* e = new PickEvent(Event::PICK, this);
		e->Target = GetNearest();
		e->Log();
	}

#ifdef _DEBUG
	if (Debug)
	{
		const TCHAR* name = m_Nearest.IsNull() ? NULL : m_Nearest->GetName();
		if (name == NULL)
			name = TEXT("NONE");
		VX_TRACE(Debug, ("Picker::OnPick Nearest = %s\n", name));
	}
#endif
}

/*!
 * @fn void Picker::OnNoPick()
 *
 * Called if nothing was picked after the mouse was released.
 * Picker subclasses override this routine to handle
 * a pick event. Only one no pick event will be generated
 * per frame for each active picker. If objects were
 * picked, OnPick is called. The default behavior is
 * to do nothing.
 *
 * @see Picker::OnPick Picker::Pick Picker::Hit
 */
void Picker::OnNoPick()
{
	PickEvent* e = new PickEvent(Event::NOPICK, this);
	e->Log();
	VX_TRACE(Debug, ("Picker::OnNoPick\n"));
}

Picker::~Picker()
{
	SetTarget(NULL);
}


int Picker::Save(Messenger& s, int op) const
{
	int32 h = Engine::Save(s, op);
	if (h < 0)
		return h;
	if (!m_HiliteApp.IsNull() && (m_HiliteApp->Save(s, op) >= 0) && h)
		s << OP(VX_RayPicker, PICK_SetHilite) << h << (SharedObj*) m_HiliteApp;
	if (h <= 0)
		return h;
	s << OP(VX_RayPicker, PICK_SetOptions) << h << int32(m_Options);
	s << OP(VX_RayPicker, PICK_SetButtons) << h << int32(m_Buttons);
	return h;
}

bool Picker::Do(Messenger& s, int opcode)
{
	Vec3	start, direction;
	int32	n;
	SharedObj*	obj;
	Box2	b;

	switch (opcode)
	{
		case PICK_SetOptions:
		s >> n;
		SetOptions(n);
		break;

		case PICK_SetButtons:
		s >> n;
		SetButtons(n);
		break;

		case PICK_SetHilite:
		s >> obj;
		if (obj)
			{ VX_ASSERT(obj->IsClass(VX_Appearance)); }
		SetHilite((Appearance*) obj);
		break;

		default:
		return Engine::Do(s, opcode);
	 }
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << RayPicker::DoNames[opcode - PICK_SetOptions] << " " << this);
#endif
	return true;
}

bool Picker::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Engine::Copy(src_obj))
		return false;
    if (src_obj->IsClass(VX_Picker))
	{
		Picker* src = (Picker*) src_obj;
		SetOptions(src->GetOptions());
		SetButtons(src->GetButtons());
		SetHilite(src->GetHilite());
		m_Selected.Copy(&(src->m_Selected));
		m_Hilited.Copy(&(src->m_Hilited));
		m_Nearest = src->m_Nearest;
		m_Closest = src->m_Closest;
	}
	return true;
}


RayPicker::RayPicker() : Picker()
{
	Ray ray(Vec3(0, 0, 0), Vec3(0, 0, -1), 1000);
	SetRay(ray);
	SetChanged(false);
}

/*!
 * @fn void RayPicker::SetPickShape(const Model* shape)
 *
 * Establishes the shape to use for picking. The depth of this
 * shape is the length of the pick ray, it's orientation is the ray direction.
 * Each object in the target hierarchy is tested against this ray.
 * The objects this ray intersects are saved in a selection array during picking.
 *
 * @see Picker::SetOptions Picker::SetRay
 */
void RayPicker::SetPickShape(const Model* shape)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_RayPicker, PICK_SetPickShape) << this << shape;
	VX_STREAM_END()

	m_PickShape = shape;
	SetChanged(true);
	m_HitInfo.Distance = FLT_MAX;
	m_HitInfo.Target = (Mesh*) NULL;
	m_HitInfo.TriIndex = -1;
}

/*!
 * @fn void RayPicker::SetRay(const Ray& r)
 *
 * Establishes the ray to use for picking.	Each object in the target
 * hierarchy is tested against this ray. The objects this ray intersects
 * are saved in a selection array during picking.
 *
 * @see Picker::SetOptions Picker::SetPickShape
 */
void RayPicker::SetRay(const Ray& r)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_RayPicker, PICK_SetRay) << this << r.start << r.direction << r.length;
	VX_STREAM_END()

	SetChanged(true);
	if (m_Ray == r)
		return;
	m_Ray = r;
	DoHilite = true;
	m_HitInfo.Distance = FLT_MAX;
	m_HitInfo.Target = (Mesh*) NULL;
	m_HitInfo.TriIndex = -1;
}

/*!
 * @fn bool RayPicker::Hit(const Model* mod, float* dist)
 * @param mod	model to hit test
 * @param dist	distance of object hit is returned here
 *
 * Determines if a model was picked or not by testing if
 * a ray intersected the model's geometry or bounds.
 *
 * The RayPicker supports the Picker::MESH option which will
 * cast rays against the geometry of shape objects.
 * The input ray is in world coordinates. To avoid transforming
 * the geometry, we get the transformation matrix of the shape
 * that owns the surface and use its inverse to map the ray back
 * into model space. Then, we try to determine which part of
 * the geometry was hit and where the point of intersection was.
 *
 * Returns true if bounding sphere was intersected. dist will
 * only be set to a nonzero value if geometry was hit.
 *
 * @return  true if the model was hit, else  false
 *
 * @see Picker::Pick Picker::DoSelect
 */
bool RayPicker::Hit(const Model* mod, float* dist)
{
	Sphere	bsp;
	Box3	box;
	Ray		ray(m_WorldRay.start, m_WorldRay.end);
	const Geometry* surf;

	if (!mod->IsActive())
		return false;
/*
 * Octree is optimized path for picking
 * Does not require any transformations because all meshes
 * are already in world coordinates and nodes are spatially sorted
 */
	if (mod->IsClass(VX_Octree))
	{
		if (!mod->Hit(m_WorldRay, *dist))
			return false;
		surf = ((Shape*) mod)->GetGeometry();
		if (surf == NULL)
			return true;
	}
/*
 * Normal scene graph requires transforming ray by total transformation matrix
 */
	else
	{
		const TCHAR* name;

		if (!mod->GetBound(&bsp, Model::NONE))
			return false;
		name = mod->GetName();
		if (name)
		{	VX_TRACE2(Picker::Debug, ("Check: %s(%f,%f,%f) r=%f ", name, bsp.Center.x, bsp.Center.y, bsp.Center.z, bsp.Radius)); }
		bsp *= m_TotalMatrix;
		if (!bsp.Hit(m_WorldRay, *dist, NULL))
			return false;
		if (name)
		{	VX_TRACE2(Picker::Debug, ("Hit: %s(%f,%f,%f) dist=%f\n", name, bsp.Center.x, bsp.Center.y, bsp.Center.z, *dist)); }
		*dist = 0.0f;
		if (!mod->IsClass(VX_Shape))
			return true;
		surf = ((Shape*) mod)->GetGeometry();
		if (surf == NULL)
			return true;
/*
 * Invert the total transformation matrix and use it to map the ray
 * from world coordinates (where it starts at 0,0,0) to model coordinates
 * (to be consistent with untransformed coordinates)
 * If the ray actually hit the surface geometry, dist will be
 * nonzero. We return true always to indicate that the bounding
 * sphere test passed and children of this model should be examined
 */
		Matrix local;
		Vec3	start, end;
		Vec3	dir;

		local.Invert(m_TotalMatrix);
		local.Transform(m_WorldRay.start, start);
		local.Transform(m_WorldRay.end, end);
		ray.Set(start, end);
	}
/*
 * If mesh picking is enabled, provide hit test event to capture
 * the mesh and triangle hit by the ray
 */
	if (m_Options & MESH)
	{
		m_HitInfo.Code = Event::TRI_HIT;
		if (surf->Hit(ray, &m_HitInfo) && (m_HitInfo.TriIndex >= 0))
			*dist = m_HitInfo.Distance;
	}
	else
	{
		m_HitInfo.Code = Event::NONE;
		if (surf->Hit(ray, &m_HitInfo))
			*dist = m_HitInfo.Distance;
	}
	return true;
}

bool RayPicker::OnStart()
{
	Messenger* mess = GetMessenger();
	if (m_PickShape == (const Model*) NULL)
	{
		mess->Observe(this, Event::MOUSE, NULL);
		mess->Observe(this, Event::NAVINPUT, NULL);
	}
	return true;
}

bool RayPicker::Eval(float time)
{
	Scene*	scene = GetMainScene();
	Camera*	cam = scene->GetCamera();
	Vec3	ctr, start, end;
	Matrix	total;

	if (!m_PickShape.IsNull())			// pick shape being used?
	{
		const Matrix* viewmtx = cam->GetViewTrans();
		Model* shape = m_PickShape;

		if (!IsSet(PICK_ObjPicked))
			DoPick = true;
		shape->TotalTransform(&total);
		total.PreMul(*viewmtx);
		total.GetTranslation(ctr);
		end = ctr;
		end.Normalize();
		start.Set(0.0f, 0.0f, 0.0f);
		if (!end.IsEmpty())
		{
			Ray	r(start, end * cam->GetYon());
			SetRay(r);
		}
	}
	if (HasChanged())
	{
		cam->TotalTransform(&total);
		total.Transform(m_Ray.start, start);
		total.Transform(m_Ray.end, end);
		m_WorldRay.Set(start, end);
	}
	return Picker::Eval(time);
}

/*!
 * @fn bool RayPicker::OnEvent(Event* e)
 *
 * Handles mouse events for ray picker. The mouse position
 * is converted from window coordinates into world coordinates.
 * The mouse is assumed to lie on the hither plane in Z.
 * A ray is cast from the mouse position in world coords down
 * thru the camera position. The object this ray hits that is
 * closest to the viewer is picked.
 */
bool RayPicker::OnEvent(Event* e)
{
	Scene*	scene = GetMainScene();
	if (scene == NULL)
		return false;

	Vec3			pick;
	int32			flags;
	int32			oldflags;
	Camera*			cam = scene->GetCamera();
	Box3			vv = cam->GetViewVol();
	Box2			vport = scene->GetViewport();
	MouseEvent*		me = (MouseEvent*) e;
	NavInputEvent*	ie = (NavInputEvent*) e;
	Vec3			p;
	Vec3			ctr(vv.Center());
	Vec2			tmp;
	bool			button_was_pressed;
	bool			button_is_pressed; 

	oldflags = MouseFlags;
	pick.z = -cam->GetHither();
/*
 * Mouse position is in a coordinate space with (0,0) in the upper left
 * corner and (W, H) in lower right where (W,H) is the width and height
 * of the window in pixels. NavInput position is in a coordinate space with
 * (0,0) at the center and (-1, 1) as upper left corner. These are converted
 * to world coordinates (defined by the camera view volume)
 */
	switch (e->Code)
	{
		case Event::MOUSE:
		tmp.x = me->MousePos.x / vport.Width();
		tmp.y = me->MousePos.y / vport.Height();
		pick.x = vv.min.x + tmp.x * vv.Width();
		pick.y = vv.max.y - tmp.y * vv.Height();
		flags = me->MouseFlags;
		OnMouse(me->MousePos, me->MouseFlags);
		break;

		case Event::NAVINPUT:
		tmp.x = ie->Pos.x;
		tmp.y = ie->Pos.y;
		pick.x = ctr.x + ie->Pos.x * vv.Width() / 2.0f;
		pick.y = ctr.y + ie->Pos.y * vv.Height() / 2.0f;
		MouseFlags = flags = ie->Buttons;
		break;

		default:
		return false;
	}
/*
 * If the button is not down or was not down previously, ignore this event.
 * If the picker has established bounds (CHECK_PARENT_BOUND option)
 * and the input position is not within them, ignore the event
 */
	button_was_pressed = (oldflags & m_Buttons) != 0;
	button_is_pressed = (flags & m_Buttons) != 0;
	DoPick = false;
	if (!button_is_pressed)
		if (button_was_pressed)
			DoPick = true;
		else
			return true;

	if ((m_Options & CHECK_PARENT_BOUND) && button_is_pressed)
	{
		Box3	bound;
		Matrix trans;
		Shape* target = (Shape*) GetTarget();

		cam->TotalTransform(&trans);
		p = pick;
		p *= trans;

		if (target && target->IsClass(VX_Shape) &&
			target->GetBound(&bound, Model::WORLD) &&
			Box2(bound.min.x, bound.max.x, bound.min.y, bound.max.y).Contains(Vec2(p.x, p.y)))
			e->Code = Event::NONE;
		else
		{
			DoPick = false;
			return true;
		}
	}
	if (pick.IsEmpty())
		pick.z = -1;
	else
		pick.Normalize();
	/*
	 * Construct a ray from the origin thru the pick position to the yon plane.
	 * if the button is going up, indicate this should be a PICK as opposed to a SELECT.
	 */
	Ray	r(Vec3(0,0,0), pick * cam->GetYon());
	SetRay(r);
	VX_TRACE(DoPick && Picker::Debug > 1, ("RayPicker: DoPick (%f, %f) -> (%f,%f,%f)\n", tmp.x, tmp.y, pick.x, pick.y, pick.z));
	return true;
}

void RayPicker::OnPick()
{
	Picker::OnPick();
	DoPick = false;
	if (IsSet(SharedObj::DOEVENTS) &&
		(m_HitInfo.Target != (const Mesh*) NULL) &&
		(m_HitInfo.TriIndex >= 0))
	{
		TriHitEvent* e = new TriHitEvent(m_HitInfo);
		e->Log();
	}
}


bool RayPicker::Do(Messenger& s, int opcode)
{
	Vec3	start, direction;
	float	length;
	int32	n;
	SharedObj*	obj;
	Box2	b;

	switch (opcode)
	{
		case PICK_SetOptions:
		s >> n;
		SetOptions(n);
		break;

		case PICK_SetButtons:
		s >> n;
		SetButtons(n);
		break;

		case PICK_SetHilite:
		s >> obj;
		if (obj)
			{ VX_ASSERT(obj->IsClass(VX_Appearance)); }
		SetHilite((Appearance*) obj);
		break;

		case PICK_SetRay:
		{
			s >> start >> direction >> length;
			Ray ray( start, direction * length );
			SetRay(ray);
			break;
		}

		default:
		return Engine::Do(s, opcode);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::" << RayPicker::DoNames[opcode - PICK_SetOptions] << " " << this);
#endif
	return true;
}

int RayPicker::Save(Messenger& s, int opts) const
{
	int32 h = Picker::Save(s, opts);
	if (h > 0)
		s << OP(VX_RayPicker, PICK_SetRay) << h << m_Ray.start << m_Ray.direction << m_Ray.length;
	return h;
}

bool RayPicker::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Picker::Copy(src_obj))
		return false;
    if (src_obj->IsClass(VX_RayPicker))
	{
		RayPicker* src = (RayPicker*) src_obj;
		SetRay(src->m_Ray);
		m_WorldRay = src->m_WorldRay;
	}
	return true;
}

NamePicker::NamePicker() : RayPicker()
{
}

/*!
 *
 * @fn void NamePicker::SetNameFilter(const TCHAR* str)
 * @param str	substring required in shape name to be selected
 *
 * Sets name filter to be used to restrict the shapes picked. Shapes and groups
 * whose names	contain the input string are the only ones which will be picked.
 * The name filter is ignored unless the  NAME_FILTER option is enabled.
 *
 * This allows you to use a naming convention in the content to control which
 * items can be selected. For example, a virtual tourism scene might name all the
 * hotels with "hotel_" and restrict the user to selecting only hotels.
 *
 * @see Picker::SetOptions Picker::SetHilite
 */
void NamePicker::SetNameFilter(const TCHAR* str)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_RayPicker, PICK_SetNameFilter) << this << str;
	VX_STREAM_END()

	m_NameFilter = str;
}

/*!
 *
 * @fn int NamePicker::CanSelect(const Model* mod, float dist)
 * @param mod	model to check for selectibility
 * @param dist	distance of geometry from viewer
 *
 * Determines if this model can be selected by checking whether
 * or not its name contains the name filter string. Also checks
 * the other picking options.
 *
 * @return	negative if name test fails, positive if geometry was hit,
 *			zero if bounding volume hit
 *
 * @see Picker::CanSelect Picker::SetOptions NamePicker::SetNameFilter
 */
int NamePicker::CanSelect(const Model* mod, float closest)
{
	if (!IsNameValid(mod))
		return -1;
	return RayPicker::CanSelect(mod, closest);
}

bool NamePicker::IsNameValid(const Model* mod)
{
	const TCHAR*	filter = m_NameFilter;
	const TCHAR*	objname = mod->GetName();

	if ((m_Options & NAME_FILTER) && filter)
	{
		const TCHAR* p = objname;
		size_t n = STRLEN(filter);

		while (objname)
		{
			p = STRCHR(objname, *filter);
			if (p == NULL)
				return false;
			objname = p;
			if (STRNCMP(objname, filter, n) == 0)
				return true;
			++objname;
		}
		return false;
	}
	VX_TRACE2(Picker::Debug, ("Picker::IsNameValid %s\n", objname));
	return true;
}

int NamePicker::Save(Messenger& s, int opts) const
{
	int32 h = RayPicker::Save(s, opts);
	if ((h > 0) && GetNameFilter())
		s << OP(VX_RayPicker, PICK_SetNameFilter) << h << GetNameFilter();
	return h;
}

bool NamePicker::Do(Messenger& s, int opcode)
{
	Vec3	start, direction;
	Box2	b;
	TCHAR	name[VX_MaxName];

	switch (opcode)
	{
		case PICK_SetNameFilter:
		s >> name;
		SetNameFilter(Core::String(name));
		break;

		default:
		return RayPicker::Do(s, opcode);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << RayPicker::DoNames[opcode - PICK_SetOptions] << " " << this);
#endif
	return true;
}

bool NamePicker::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!RayPicker::Copy(src_obj))
		return false;
    if (src_obj->IsClass(VX_NamePicker))
	{
		NamePicker* src = (NamePicker*) src_obj;
		SetNameFilter(src->GetNameFilter());
	}
	return true;
}

}	// end Vixen