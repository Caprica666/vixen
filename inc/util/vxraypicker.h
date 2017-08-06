#pragma once

namespace Vixen {

/*!
 * @class Picker
 * @brief Selects one or more models from its target hierarchy.
 *
 * Picker is a base class from which different types of pickers can
 * be derived. You must supply a picking algorithm if you override this class.
 * It will use your algorithm to select the closest objects in the target hierarchy.
 *
 * You can customize your picker by overriding its callback functions. If the 
 * picker is active, it will be evaluated once per frame and will call either
 *  OnPick, if an object was picked or  OnNoPick, if nothing was picked. For each 
 * model picked  DoSelect is called. The  Event::PICK event is logged when an
 * object is picked.
 *
 * @see RayPicker Event
 */
class Picker : public MouseEngine
{
public:
	VX_DECLARE_CLASS(Picker);

	Picker();
	~Picker();

	//!	Get distance to closest object picked.
	float			GetNearDist() const			{ return m_Closest; }

	//! Return list of objects which were picked.
	const ObjArray*	GetSelected() const;

	//! Return nearest picked object.
	Model*			GetNearest() const;

	//! Return picking options.
	uint32			GetOptions() const			{ return m_Options; }

	//! Return mouse buttons that initiate picking
	uint32			GetButtons() const			{ return m_Buttons; }

	//! Return appearance to use for hiliting objects.
	Appearance*		GetHilite() const			{ return m_HiliteApp; }

	//! Set picking options.
	void			SetOptions(int opts);

	//! Establish mouse buttons to use for picking.
	void			SetButtons(int mbutton);

	//! Set appearance to use for hiliting selected objects.
	void			SetHilite(const Appearance* hilite);

	//! Unhilite and unselect all objects previously hilited by this picker.
	void			UnHiliteAll(bool didpick = false);

	//! Called when something is picked.
	virtual void	OnPick();

	//! Called when nothing has been picked.
	virtual void	OnNoPick();

	virtual bool	Copy(const SharedObj* src);
	virtual bool	Eval(float t);
	virtual bool	Do(Messenger& s, int opcode);
	virtual int		Save(Messenger& s, int opts) const;
//	DebugOut&		Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*!
	 * @brief Picking Options.
	 * @see Picker::SetOptions
	 */
	enum
	{
		BOUNDS = 0,					//!< pick using bounding spheres and boxes
		MESH = 1,					//!< pick using geometry
		PICK_MODEL = 2,				//!< allow models to be picked, default is only shapes
		SELECT_PARENT = 4,			//!< hilite shape & siblings, select parent model
		HILITE_SELECTED	= 8	,		//!< temporarily hilite selected objects
		HILITE_PICKED = 16,			//!< permanently hilite picked objects
		NAME_FILTER = 32,			//!< apply name selection filter
		MULTI_SELECT = 64,			//!< allow multiple selection
		CHECK_PARENT_BOUND = 128,	//!< only pick within parent bounds
	};

	/*
	 * RayPicker::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		PICK_SetOptions = Engine::ENG_NextOp,
		PICK_SetRay,
		PICK_SetHilite,
		PICK_SetButtons,
		PICK_SetBound,
		PICK_SetPickShape,
		PICK_SetNameFilter,
		PICK_NextOp = Engine::ENG_NextOp + 10
	};

protected:
	//! Called to hit-test a model and compute its distance.
	virtual bool	Hit(const Model*, float* dist);

	//! Called to hit-test against a hierarchy.
	virtual bool	Pick(Model*);

	//! Called to select a model which has been hit.
	virtual bool	DoSelect(Model*, float closest, int opts);

	//! Called to hilight a model which has been selected.
	virtual	bool	HiliteModel(Model* mod);

	//! Called to determine if a model can be selected.
	virtual	int		CanSelect(const Model* mod, float closest);

	void			UpdateHilite();

	uint32			m_Options;
	uint32			m_Buttons;
	float			m_Closest;
	ObjArray		m_Selected;
	ObjArray		m_Hilited;
	Ref<Appearance>	m_HiliteApp;
	Ref<Model>		m_Nearest;
	Matrix			m_TotalMatrix;

public:
	bool	DoHilite;		//!<  tru to enable selection hiliting
	bool	DoPick;			//!<  true to enable picking
};

/*!
 * @fn Model* Picker::GetNearest() const
 *
 * Returns the selected model that was determined to be nearest
 * to the viewer during the last pick operation.
 *
 * @see Picker::GetSelected RayPicker::SetRay Picker::SetOptions
 */
inline Model* Picker::GetNearest() const
	{ return m_Nearest; }

/*!
 * @fn const ObjArray* Picker::GetSelected() const	
 *
 * Returns the array of objects selected during the last pick operation.
 *
 * @see Picker::GetNearest RayPicker::SetRay Picker::SetOptions
 */
inline const ObjArray* Picker::GetSelected() const	
	{ return &m_Selected; }

/*!
 * @class RayPicker
 * @brief Selects shapes from a hierarchy that a given ray intersects.
 *
 * Hit testing the ray against the objects is done
 * by bounding spheres or optionally, using object geometry.
 *
 * @see Picker Ray Model::Hit
 */
class RayPicker : public Picker
{
public:
	VX_DECLARE_CLASS(RayPicker);
	RayPicker();

	//! Set shape to derive picking ray from.
	void			SetPickShape(const Model* mod);

	//! Get shape used to derive picking ray.
	const Model*	GetPickShape() const	{ return m_PickShape; }

	//! Set ray to pick with.
	void			SetRay(const Ray& r);

	//! Get ray being used for picking.
	const Ray&		GetRay() const			{ return m_Ray; }

// Overrides
	virtual bool	Do(Messenger& s, int opcode);
	virtual int		Save(Messenger& s, int opts) const;
	virtual bool	Copy(const SharedObj* src);
	virtual bool	OnEvent(Event*);
	virtual bool	Eval(float t);
	virtual void	OnPick();


protected:
	virtual bool	OnStart();
	virtual bool	Hit(const Model*, float* dist);

	Ray			m_Ray;			// ray in camera coordinates
	Ray			m_WorldRay;		// ray in world coordinates
	TriHitEvent	m_HitInfo;		// mesh picking hit info
	Ref<Model>	m_PickShape;
};

/*!
 * @class NamePicker
 * @brief Selects one or more shapes from a hierarchy if their names
 * match a given search string.
 *
 * Intersection of the object with a ray is used to determine which objects to check.
 *
 * @see Picker RayPicker
 */
class NamePicker : public RayPicker
{
public:
//	Constructors
	VX_DECLARE_CLASS(NamePicker);
	NamePicker();

	//! Get object name filter.
	const TCHAR*	GetNameFilter() const;

	//! Set string to constrain objects selected based on their name.
	void			SetNameFilter(const TCHAR* str);

	virtual bool	Do(Messenger& s, int opcode);
	virtual int		Save(Messenger& s, int opts) const;
	virtual bool	Copy(const SharedObj* src);

protected:
	int			CanSelect(const Model* mod, float closest);
	bool		IsNameValid(const Model* mod);

	Core::String	m_NameFilter;
};

inline const TCHAR* NamePicker::GetNameFilter() const
{
	if (m_NameFilter.GetLength() == 0)
		return NULL;
	return (const TCHAR*) m_NameFilter;
}

} // end Vixen