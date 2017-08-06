#include "vixen.h"

namespace Vixen {

#ifndef BAD_TRIGGER_TYPE
#define BAD_TRIGGER_TYPE 0
#endif

VX_IMPLEMENT_CLASSID(Trigger, Engine, VX_Trigger);

static const TCHAR* opnames[] =
{	TEXT("SetColliders"), TEXT("SetOptions"),
	TEXT("SetGeoSphere"), TEXT("SetGeoBox"), TEXT("SetGeometry"),
	TEXT("AddCollider"), TEXT("RemoveCollider")
};

const TCHAR** Trigger::DoNames = opnames;

Trigger::Trigger() : Engine()
{
Debug = 1;
	m_Options = SPHERE;
	m_TriggerFlags = new IntArray();
	m_GeoSphere.Empty();
	m_GeoBox.Empty();
	SetFlags(SharedObj::DOEVENTS);
}

Trigger::Trigger(const Trigger& src) : Engine(src)
{
	m_Options = src.m_Options;
	m_TriggerFlags = src.m_TriggerFlags;
	m_Collider = src.m_Collider;
	m_GeoSphere = src.m_GeoSphere;
	m_GeoBox = src.m_GeoBox;
}

/*!
 * @fn void Trigger::SetOptions(int opts)
 * @param opts	collision detection options
 * - Trigger::SPHERE	use sphere geometry
 * - Trigger::BOX		use box geometry
 *
 * Determines which collision detection method to use and
 * defines the type of collision geometry provided.
 *
 * @see Trigger::SetColliders
 */
void Trigger::SetOptions(int opts)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Trigger, TRIGGER_SetOptions) << this << int32(opts);
	VX_STREAM_END(  )

	m_Options = opts;
}

/*!
 * @fn bool Trigger::SetColliders(const SharedObj* obj)
 * @param obj	model or array of models to test for collision
 *
 * Establishes which objects will be collision tested against
 * this trigger when the engine is evaluated. If the input object
 * is a single model, this model is hit-tested against the collision
 * geometry. If an array of models is supplied, all the models
 * in the array are collision tested.
 *
 * @return  true if collider object acceptable, else  false
 *
 * @see Trigger::SetOptions Trigger::SetGeometry Trigger::AddCollider Trigger::RemoveCollider
 */
bool Trigger::SetColliders(const SharedObj* obj)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Trigger, TRIGGER_SetColliders) << this << obj;
	VX_STREAM_END( )

	if (obj == NULL || obj->IsClass(VX_Model))
	{
		m_Collider = obj;
		m_TriggerFlags->SetAt(0, OUTSIDE);
		return true;
	}
	else if (obj->IsClass(VX_ObjArray))
	{
		m_Collider = obj;
		intptr sz = ((ObjArray*)obj)->GetSize();
		for (intptr i=0; i<sz; i++) 
			m_TriggerFlags->SetAt(i, OUTSIDE);
		return true;
	}
	return false;
}

/*!
 * @fn bool Trigger::AddCollider(const Model* mod)
 * @param mod	model to add to collision list
 *
 * Adds an object to the list of those which are hit tested
 * against this trigger. An object cannot be added twice.
 *
 * @return  true if collider object acceptable, else  false
 *
 * @see Trigger::SetGeometry Trigger::SetColliders Trigger::RemoveCollider
 */
bool Trigger::AddCollider(const Model* mod)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Trigger, TRIGGER_AddCollider) << this << mod;
	VX_STREAM_END( )

	ObjArray* oa;
	SharedObj* obj = m_Collider;

	if ((mod == NULL) || !mod->IsClass(VX_Model))
		return false;
	if (obj == NULL)
		return SetColliders(mod);
	if (obj->IsClass(VX_Model))			// single model collider?
	{
		oa	= new ObjArray();			// make collision set
		oa->Append(obj);
		oa->Append(mod);				// add the collider
		m_TriggerFlags->SetAt(1, OUTSIDE);
		m_Collider = oa;
		return true;
	}
	if (obj->IsClass(VX_ObjArray))		// multiple colliders?
	{
		oa = (ObjArray*) obj;
		intptr n = oa->GetSize();
		if (!oa->Append(mod))			// add the new model
			return false;
		m_TriggerFlags->SetAt(n, OUTSIDE);
		return true;
	}
	return false;
}

/*!
 * @fn bool Trigger::RemoveCollider(const Model* mod)
 * @param mod	model to be removed
 *
 * The collision list is searched and, if the model is found,
 * it is removed. Collision detection will no longer be performed
 * against this model.
 *
 * @return true if model removed, else  false
 *
 * @see Trigger::SetColliders Trigger::AddCollider
 */
bool Trigger::RemoveCollider(const Model* mod)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Trigger, TRIGGER_RemoveCollider) << this << mod;
	VX_STREAM_END( )

	const SharedObj* obj = m_Collider;

	if ((mod == NULL) || !mod->IsClass(VX_Model))
		return false;
	if (obj == NULL)
		return SetColliders(mod);
	if (obj->IsClass(VX_Model))		// single model collider?
	{
		if (mod == obj)
		{
			SetColliders(NULL);
			return true;
		}
		return false;
	}
	if (obj->IsClass(VX_ObjArray))		// multiple colliders?
	{
		ObjArray* oa = (ObjArray*) obj;
		intptr i = oa->Find(mod);		// is it in the list?
		if (i < 0)						// no, guess not
			return false;
		oa->RemoveAt(i);				// remove the one found
		return true;
	}
	return false;
}

/*!
 * @fn bool Trigger::Hit(const Model* col, int32& op, TriggerEvent& event)
 * @param col	model to test for collision
 * @param op	current collision results
 * @param event	event to indicate trigger collision
 *
 * Determines if the input model intersected the collision
 * geometry area and if it is entering or leaving.
 * If there was a collision, the trigger event describing the
 * collision is initialized with the event code (Event::ENTER
 * or Event::LEAVE), the collider and the point of collision
 * (in the coordinate space of the trigger collision geometry)
 *
 * The collision code parameter describes collision results. It is
 * used on input and updated on return.
 * @code
 *	Trigger::OUTSIDE	model outside collision area
 *	Trigger::INSIDE		model intersects collision area
 *	Trigger::ENTER		model is entering collision area
 *	Trigger::LEAVE		model is leaving collision area
 * @endcode
 *
 * If no collision geometry is supplied but there is a target model,
 * the bounding sphere or box of that model is used as the collision geometry.
 *
 * @return  true if there was a collision, else  false
 *
 * @see Model::Hit Trigger::SetColliders TriggerEvent
 */
bool Trigger::Hit(const Model* col, int32& op, TriggerEvent& event)
{
	const SharedObj*	targ = GetTarget();
	Vec3				colPos, dV;
	Box3				bbox;
	Vec3				center(0, 0, 0);
	Sphere				bsp;
	float				dist;
	const Model*		mod = NULL;
	const Transformer*	bone = NULL;
	Matrix				wmtx;

	if (col->IsSet(INACTIVE))
		return false;
	colPos = col->GetCenter(Model::WORLD);
	if (targ)
		if (targ->IsClass(VX_Model))			// have a target model?
		{
			mod = (const Model*) targ;
			mod->GetBound(&bsp, Model::WORLD);	// get bounding sphere
			center = bsp.Center;
		}
		else if (targ->IsClass(VX_Transformer))
		{
			bone = ((const Transformer*) targ);
			bone->GetTotalTransform()->GetTranslation(center);
		}
	switch (m_Options)
	{
		case SPHERE:
		if (mod != NULL)						// attached to model?
		{
			if (m_GeoSphere.Radius != 0)		// center comes from model
				bsp.Radius = m_GeoSphere.Radius;
		}
		else if (bone)							// attached to bone?
			bsp.Radius = m_GeoSphere.Radius;	// radius comes from trigger
		else
			bsp = m_GeoSphere;					// no target, radius and center come from trigger
		dV = colPos - bsp.Center;
		dist = dV.Length();
		VX_TRACE(Debug > 1, ("Trigger: %s(%f, %f, %f) ? %s(%f, %f, %f) %f\n",
				col->GetName(), colPos.x, colPos.y, colPos.z, GetName(), bsp.Center.x, bsp.Center.y, bsp.Center.z, dist));
		if (dist < bsp.Radius)					// collision?
		{
			if (op & OUTSIDE)					// collider outside trigger
			{
				op &= ~OUTSIDE;
				op |= INSIDE;					// indicate collider inside
				event.Collider = col;
				event.Code = Event::ENTER;
				event.CollidePos = colPos;
				event.Target = mod;
				return true;
			}
			else								// collider inside trigger
			{
				VX_ASSERT(op & INSIDE);
				return false;
			}
		}
		else									// no collision
		{
			if (op & INSIDE)					// collider inside trigger
			{
				op &= ~INSIDE;
				op |= OUTSIDE;
				event.Collider = col;
				event.Code = Event::LEAVE;
				event.CollidePos = colPos;
				event.Target = mod;
				return true;
			}
			else								// collider outside trigger
			{
				op |= OUTSIDE;
				return false;
			}
		}
		break;

		case BOX:
		bbox = m_GeoBox;					// assume collision geometry in world coords
		if (mod != NULL)					// have a target model?
		{
			mod->GetBound(&bbox, Model::NONE);	// get it's bounding box
			mod->TotalTransform(&wmtx);			// get collider position in
			wmtx.Invert();						// local coordinate space of model
		}
		else if (bone != NULL)
		{
			const Matrix* bonemtx = bone->GetTotalTransform();
			bonemtx->GetTranslation(center);
			wmtx.Invert(*bonemtx);
		}
		wmtx.Transform(colPos, colPos);		// where box is axially aligned
		center = bbox.Center();
		if (bbox.IsEmpty())
			return false;
		dV = colPos - center;
		dist = dV.Length();
		VX_TRACE(Debug > 1, ("Trigger: %s(%f, %f, %f) ? %s(%f, %f, %f) %f\n",
				col->GetName(), colPos.x, colPos.y, colPos.z, GetName(), center.x, center.y, center.z, dist));
		if (bbox.Contains(colPos))				// collider center within collision box?
		{
			if (op & OUTSIDE)
			{
				op &= ~OUTSIDE;
				op |= INSIDE;
				event.Collider = col;
				event.Code = Event::ENTER;
				event.CollidePos = colPos;
				VX_TRACE(Trigger::Debug, ("Trigger: Enter %s\n", col->GetName()));
				return true;
			}
			else
			{
				VX_ASSERT(op & INSIDE);
				return false;
			}
		}
		else
		{
			if (op & INSIDE)
			{
				op &= ~INSIDE;
				op |= OUTSIDE;
				event.Collider = col;
				event.Code = Event::LEAVE;
				event.CollidePos = colPos;
				VX_TRACE(Trigger::Debug, ("Trigger: Leave %s\n", col->GetName()));
				return true;
			}
			else
			{
				VX_ASSERT(op & OUTSIDE);
				return false;
			}
		}
		break;

		default:
		VX_ASSERT(BAD_TRIGGER_TYPE);
		break;
	}

	return false;
}

/*!
 * @fn bool Trigger::SetGeometry(const SharedObj* obj)
 * @param obj	model with collision geometry
 *
 * Updates the collision geometry associated with the trigger.
 * A trigger may simultaneously have sphere, box and object geometries.
 * The collision options determine which one is used for collision.
 *
 * If the input object is a model, the transformation of the model
 * is used to position the collision geometry. As the model moves or
 * rotates, the collision zone follows. The base class only supports 
 * box and sphere geometry - it does not do model to model collisions.
 * Hooks are provided for subclasses to use their own geometry objects.
 *
 * @return  true if geometry object supported, else  false
 *
 * @see Trigger::SetOptions Trigger::SetColliders
 */
bool Trigger::SetGeometry(const SharedObj* obj)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Trigger, TRIGGER_SetGeoObj) << this << obj;
	VX_STREAM_END( )

	if (obj->IsClass(VX_Model))
	{
		const Model* mod = (const Model*) obj;
		Sphere bsp;

		mod->GetBound(&bsp, Model::NONE);
		mod->GetBound(&m_GeoBox, Model::NONE);
		m_GeoBox.min -= bsp.Center;
		m_GeoBox.max -= bsp.Center;
		m_GeoSphere.Center.Set(0,0,0);
		m_GeoSphere.Radius = bsp.Radius;
		return true;
	}
	return false;
}

/*!
 * @fn bool Trigger::SetGeoSphere(const Sphere& sphere)
 * @param sphere	spherical geometry
 *
 * Sets the sphere collision geometry. The sphere is in world coordinates.
 * If the TRIGGER_Sphere option is enabled, collision events are
 * triggered for models in the collision list if their centers enter or leave the sphere.
 * This is the simplest and fastest form of collision detection.
 *
 * @return  true if sphere collision supported, else  false
 *
 * @see Trigger::SetOptions Trigger::SetColliders
 */
bool Trigger::SetGeoSphere(const Sphere& sphere)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Trigger, TRIGGER_SetGeoSphere) << this << sphere.Center << sphere.Radius;
	VX_STREAM_END(  )

	m_GeoSphere = sphere;
	return true;
}

bool Trigger::GetGeoSphere(Sphere& sphere) const
{
	sphere = m_GeoSphere;
	return true;
}

/*!
 * @fn bool Trigger::SetGeoBox(const Box3& box)
 * @param box		axially-aligned box to use for collision
 *
 * Sets the box collision geometry. It is an axially-aligned bounding
 * box in world coordinates. If the TRIGGER_Box option is enabled,
 * collision events are triggered for models in the collision list if
 * their centers enter or leave the box.
 *
 * @return  true if geometry object supported, else  false
 *
 * @see Trigger::SetOptions Trigger::SetColliders
 */
bool Trigger::SetGeoBox(const Box3& box)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Trigger, TRIGGER_SetGeoBox) << this << box.min << box.max;
	VX_STREAM_END(  )

	m_GeoBox = box;
	return true;
}

bool Trigger::GetGeoBox(Box3& box) const
{
	box = m_GeoBox;
	return true;
}

bool Trigger::Eval(float t) 
{ 
	TriggerEvent	te;
	int32			op, hit;
	const Model*	collider = (const Model*)  (const SharedObj*) m_Collider;

	if (!IsSet(SharedObj::DOEVENTS))
		return false;

	if (collider == NULL)
	{
		collider = GetMainScene()->GetCamera();
		if (m_TriggerFlags->GetSize() < 1)
			m_TriggerFlags->SetAt(0, OUTSIDE);
	}

	te.Sender = this;
	te.Target = this->GetTarget();

	if (collider->IsClass(VX_ObjArray))
	{
		//array of object colliders
		ObjArray* oa = (ObjArray*)(*m_Collider);
		intptr sz = oa->GetSize();
		for (intptr i=0; i<sz; i++)
		{
			const Model* mod = (const Model*) (const SharedObj*) oa->GetAt(i);
			op = m_TriggerFlags->GetAt(i);
			hit = Hit(mod, op, te);
			m_TriggerFlags->SetAt(i, op);
			if (hit)
			{
				*GetMessenger() << te;
				return true;
			}
		}
	}
	else if (collider->IsClass(VX_Model))
	{
		op = m_TriggerFlags->GetAt(0);
		hit = Hit(collider, op, te);
		m_TriggerFlags->SetAt(0, op);
		if (hit)
		{
			*GetMessenger() << te;
			return true;
		}
	}
	return false; 
}

DebugOut& Trigger::Print(DebugOut& dbg, int opts) const
{
	return Engine::Print(dbg);
}

bool Trigger::Copy(const SharedObj* src_obj)
{
	const Trigger* src = (const Trigger*) src_obj;
	
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Engine::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_Trigger))
	{
		m_Collider = src->m_Collider;
		m_GeoBox = src->m_GeoBox;
		m_GeoSphere = src->m_GeoSphere;
		m_Options = src->m_Options;
	}
	return true;
}


/****
 *
 * class Trigger override for SharedObj::Do
 *
 ****/
bool Trigger::Do(Messenger& s, int op)
{
	SharedObj*	obj;
	Vec3		v1, v2;
	Box3		b;
	Sphere		sp;
	float		r;
	int32		n;
	Matrix		trans;

	switch (op)
	{
		case TRIGGER_SetColliders:
		s >> obj;
		SetColliders(obj);
		break;

		case TRIGGER_AddCollider:
		s >> obj;
		AddCollider((const Model*) obj);
		break;

		case TRIGGER_RemoveCollider:
		s >> obj;
		RemoveCollider((const Model*) obj);
		break;

		case TRIGGER_SetGeoSphere:
		s >> v1 >> r;
		sp.Set(v1, r);
		SetGeoSphere(sp);
		break;

		case TRIGGER_SetGeoBox:
		s >> v1 >> v2;
		b.Set(v1, v2);
		SetGeoBox(b);
		break;

		case TRIGGER_SetGeoObj:
		s >> obj;
		SetGeometry(obj);
		break;

		case TRIGGER_SetOptions:
		s >> n;
		SetOptions(n);
		break;
		
		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Trigger::DoNames[op - TRIGGER_SetColliders]
					   << " " << this);
#endif
	return true;
}


/****
 *
 * class Trigger override for SharedObj::Save
 *
 ****/
int Trigger::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	if (h < 0) return h;

	const SharedObj* obj = GetColliders();	//take care of collider
	if (obj &&	(obj->Save(s, opts) >= 0))
		s << OP(VX_Trigger, TRIGGER_SetColliders) << h << obj;
	obj = GetGeometry();
	if (obj && (obj->Save(s, opts) >= 0))	//take care of geometry
		s << OP(VX_Trigger, TRIGGER_SetGeoObj) << h << obj;
	if (h <= 0)
		return h;

	if (!m_GeoSphere.IsEmpty())		//take care of sphere
		s << OP(VX_Trigger, TRIGGER_SetGeoSphere) << h << m_GeoSphere.Center << m_GeoSphere.Radius;
	if (!m_GeoBox.IsEmpty())		// take care of box
		s << OP(VX_Trigger, TRIGGER_SetGeoBox) << h << m_GeoBox.min << m_GeoBox.max;
	if (GetOptions())				// take care of options
		s << OP(VX_Trigger, TRIGGER_SetOptions) << h << int32(GetOptions());
	return h;
}
}	// end Vixen