/*!
 * @file vxtrigger.h
 * @brief Simple collision detection engine.
 *
 * Triggers allow you to specify spherical, rectilinear or mesh collision
 * zones with a list of models to check for collision against.
 *
 * @author Nola Donato
 * @ingroup vixen
 */
#pragma once

namespace Vixen {

/*!
 * @class Trigger
 * @brief Simple collision detection engine.
 *
 * Triggers collision events when a scene object enters or
 * leaves the area enclosed by the collision geometry.
 * Several collision methods are supported for different
 * speed / accuracy tradeoffs. The base implementation supports
 * spheres or boxes for bounding geometry. Generation of collision
 * events can be suppressed by disabling SharedObj::DOEVENTS for the trigger.
 *
 * If the target of the trigger engine is a model, the position,
 * orientation and size of the trigger area will be affected by
 * the transformation associated with the model. The trigger zone
 * will move, scale and rotate with the model in the scene.
 * If no target is given, the trigger geometry is assumed to be
 * in world coordinates.
 *
 * Trigger engines are normally used in conjunction with a higher
 * level collision manager that determines which scene objects
 * to test against the trigger. Simple collision detection can
 * be implemented by directly supplying each trigger with the objects
 * to test. If you specify a collider model, the trigger engine tests this
 * object against the trigger area every frame. If the collider is an
 * array of objects, all of the models in the array are collision tested.
 * This approach is not efficient if you supply large collider lists or
 * if the collision geometry is complex.
 *
 * @see TriggerEvent Animator Engine::SetTarget Engine::SetControl
 */
class Trigger : public Engine
{
public:
	VX_DECLARE_CLASS(Trigger);

	Trigger();
	Trigger(const Trigger&);

	SharedObj*			GetColliders();					//!< Return objects to test for collision.
	const SharedObj*	GetColliders() const;
	SharedObj*			GetGeometry();					//!< Return object with collision geometry.
	const SharedObj*	GetGeometry() const;
	bool				SetColliders(const SharedObj*);	//!< Set array of collision objects.
	bool				AddCollider(const Model*);		//!< Add model to list of collision objects.
	bool				RemoveCollider(const Model*);	//!< Remove model from collision list.
	bool				SetGeometry(const SharedObj*);	//!< Use given object as collision geometry.
	bool				SetGeoSphere(const Sphere&);	//!< Use input sphere as collision geometry.
	bool				SetGeoBox(const Box3&);			//!< Use input box as collision geometry.
	bool				GetGeoSphere(Sphere&) const;	//!< Get collision sphere.
	bool				GetGeoBox(Box3&) const;			//!< Get collision box.
	int					GetOptions() const;				//!< Get collision options.
	void				SetOptions(int opts);			//!< Set collision options.

//	Overrides
	virtual bool		Do(Messenger& s, int opcode);
	virtual int			Save(Messenger&, int) const;
	virtual bool		Copy(const SharedObj*);
	virtual bool		Eval(float t);
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*!
	 * @brief Trigger testing and reporting options.
	 * @see Trigger::SetOptions
	 */
	enum
	{
		SPHERE =	1,	//!< do sphere test
		BOX =		2,	//!< do box test
		FIT =		4,	//!< fit trigger box to target

		OUTSIDE =	1,	//!< outside collision area
		INSIDE =	2,	//!< inside collision area
		ENTER =		4,	//!< entering collision area
		LEAVE =		8	//!< leaving collision area
	};

	/*
	 * Trigger::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		TRIGGER_SetColliders = Engine::ENG_NextOp,
		TRIGGER_SetOptions,
		TRIGGER_SetGeoSphere,
		TRIGGER_SetGeoBox,
		TRIGGER_SetGeoObj,
		TRIGGER_AddCollider,
		TRIGGER_RemoveCollider,
		TRIGGER_NextOp = Engine::ENG_NextOp + 20
	};

protected:
	//! Callback to perform collision detection with a model.
	virtual bool	Hit(const Model*, int32&, TriggerEvent&);

	int32			m_Options;		// collision testing options, type, etc.
	Ref<IntArray>	m_TriggerFlags;	// state flags, matched with m_Collider array (if array)
	ObjRef			m_Collider;		// objects to collision test
	Box3			m_GeoBox;		// box geometry
	Sphere			m_GeoSphere;	// sphere geometry
};

inline SharedObj* Trigger::GetColliders()
{	return m_Collider; }

inline const SharedObj* Trigger::GetColliders() const
{	return m_Collider; }

inline int Trigger::GetOptions() const
{	return m_Options; }

inline SharedObj* Trigger::GetGeometry()
{	return NULL; }

inline const SharedObj* Trigger::GetGeometry() const
{	return NULL; }

} // end Vixen
	
 
