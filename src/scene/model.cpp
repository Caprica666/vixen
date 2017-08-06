/****
 *
 * Model
 * A Model is a node in a scene graph - a hierarchy of displayable
 * objects that can be animated in 3D space. All models can have
 * children that inherit their movements and rotations.
 * Specific types of models like 3D geometric figures, animated
 * models and 3D sounds can all be kept in the same hierarchy.
 *
 * All models have a transformation matrix that positions them in
 * 3 space with respect to their parent. There are a lot of routines
 * provided to manipulate the model's matrix in intuitive terms.
 *
 * Each frame, the simulation pass updates the models in the hierarchy
 * and they are redisplayed. Display of a model involves these steps:
 *	1. CalcMatrix: compute final transformation matrix
 *	2. Cull: determine if model is visible this frame
 *	3. Render: transform, light and clip model geometry
 *	5. Display: recursively display selected children
 *
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Model, Group, VX_Model);

const Vec3	Model::XAXIS(1.0f, 0.0f, 0.0f);
const Vec3	Model::YAXIS(0.0f, 1.0f, 0.0f);
const Vec3	Model::ZAXIS(0.0f, 0.0f, 1.0f);

static const TCHAR* opnames[] =
{	TEXT("SetTransform"), TEXT("Rotate"), TEXT("Turn"), TEXT("Translate"),
	TEXT("Move"), TEXT("Scale"), TEXT("Size"), TEXT("LookAt"), TEXT("SetTranslation"),
	TEXT("SetRotation"), TEXT("Reset"), TEXT("SetBound"), TEXT("SetHints"),
	TEXT("SetCulling"), TEXT("SetBoxBound"),
};

const TCHAR** Model::DoNames = opnames;

bool Model::DoCulling = true;


Model::Model() : Group()
{
	SetFlags(MOD_BVinvalid | MOD_STinvalid);
	m_Verts = m_Faces = 0;
	m_BoundVol.Empty();
	m_BoundBox.Empty();
	m_NoBounds = m_AutoBounds = true;
	m_NoCull = false;
	m_Rendered = false;
	m_Hints = 0;
}

Model::Model(const Model& src) : Group(src)
{
	SetFlags(MOD_BVinvalid | MOD_STinvalid);
	m_Verts = m_Faces = 0;
	m_BoundVol.Empty();
	m_BoundBox.Empty();
	m_NoBounds = m_AutoBounds = true;
	m_NoCull = false;
	m_Rendered = false;
	m_Hints = 0;
}

/*!
 * @fn void Model::Display(Scene* scene)
 * @param scene	scene which is displaying the model
 *
 * This is the function which traverses the display hierarchy every frame.
 * It calls several methods which subclasses can override to alter
 * traversal behavior for specific types of nodes. It also calls some
 * methods on the input Scene which can alter overall traversal behavior.
 *
 *	In the single-threaded case, Display calls its framework
 *	functions sequentially:
 *	- Call CalcMatrix to compute world transformation matrix
 *	- Call Cull to determine if this model should be displayed
 *	- If not culled
 *	-    call Render to display the mesh associated with the model
 *	-    call Display for each child
 *
 * On some platforms, the scene manager does simulation, scene graph traversal and rendering
 * in separate threads. Thus the CalcMatrix function might compute a world
 * matrix in one thread that is passed to the Render function in another thread.
 * If your subclass calculates information in one override that is used by
 * another, it is responsible for the thread safety of that information.
 *
 * For example, a dynamic terrain generation subclass could override Cull
 * to produce a list of triangles to draw and Render to display those triangles.
 * The geometry need not be represented in Vixen format if the application
 * renders it directly. However, such a subclass is responsible for ensuring
 * that both Cull and Render can be called simultaneously on different
 * processors, one to compute the triangle list for the next frame and
 * the other to render it for this frame. Usually this requires keeping
 * two copies of the data, one for each frame. If your application does
 * not require the performance benefit of distributing traversal and rendering,
 * your subclass could work in the multi-threaded environment by performing
 * your culling in the Render step and not overriding Cull.
 *
 * @see Model::Cull Model::Render Model::CalcMatrix
 */
void Model::Display(Scene* scene)
{
	Model*	m;
	Matrix	save_mtx;
	Matrix*	mv = (Matrix*) scene->GetWorldMatrix();
	Matrix*	save = NULL;

	if (!IsActive())			// don't display if not active
		return;
/*
 * Push current matrix and compute new transformation matrix
 * for this character. The default behavior for CalcMatrix is to
 * concatenate the model's local transformation matrix onto
 * the current ModelView.
 */
	Lock();
	save_mtx.SetMatrix(mv->GetMatrix());
	if (CalcMatrix(mv, scene))
		save = &save_mtx;
/*
 * Check whether the model and its children should be culled.
 * If not, render the model and traverse its children
 */
	Core::InterlockInc(&(scene->GetStats()->TotalModels));
	switch (Cull(mv, scene))
	{
		case DISPLAY_NONE:
		Unlock();
		break;

		case DISPLAY_ME:
		Render(scene);				// render this model
		Unlock();
		break;

		default:
		Render(scene);				// render this model
		Unlock();
		m = First();				// get first child
		while (m)
		{
			m->Display(scene);		// display each child
			m = m->Next();
		}
		break;
	}
	if (save)
		mv->Copy(save);
}

/*!
 * @fn void Model::Render(Scene* scene)
 * @param scene	scene which is rendering the model
 *
 * On entry to this function, the total transformation matrix to
 * bring the model from local to camera coordinates has been calculated
 * and loaded into the geometry engine. Render is responsible for
 * preprocessing  the geometry associated with the model so it can
 * be rendered efficiently by the render thread. The default implementation
 * does nothing. Shape overrides Render and sorts meshes based on
 * their matrix and appearance.
 *
 * If your subclass overrides Render, you can use the input scene
 * to access global state needed to render your model, like camera and environment.
 * The render function is only called for models which have not been culled.
 *
 * @see Model::Display Model::CalcMatrix Shape Model::Cull
 */
void Model::Render(Scene* scene)
{	}


/*!
 * @fn intptr Model::Cull(const Matrix* trans, Scene* scene)
 * @param trans	world transformation matrix for model
 *			If Cull is not called during traversal, this
 *			will be NULL. Otherwise it is the matrix to
 *			bring the model from its local coordinates into
 *			world coordinates.
 * @param scene	scene which is currently displaying the model
 *
 * This function is called during scene traversal to determine
 * which, if the node should be rendered.
 *
 * The default implementation culls based on the bounding sphere
 * stored in the model. If culling is disabled globally by setting
 * Model::DoCulling to  false, Cull is never called and all
 * models are assumed to be visible. Culling can be disabled for
 * a single model only by calling Model::SetCulling;
 *
 * @returns code indicating which children to display
 * @code
 *	DISPLAY_ALL		indicates display node and all children
 *	DISPLAY_NONE	indicates display nothing, node culled
 *	DISPLAY_ME		indicates display this node but not its children
 * @endcode
 *
 * @see Model::Display ModelSwitch Model::Render Model::CalcMatrix Camera::IsVisible Model::CalcBound
 */
intptr Model::Cull(const Matrix* trans, Scene* scene)
{
	Sphere	bound = m_BoundVol;				// bounding sphere
	SceneStats* s = scene->GetStats();

	m_Rendered = false;
	Core::InterlockInc(&(s->TotalModels));
	Core::InterlockAdd(&(s->TotalVerts), (int) m_Verts);
	if (m_NoCull || !DoCulling)				// do not cull this one
		return DISPLAY_ALL;
	if (bound.Radius <= 0.0f)				// empty bounds?
		return DISPLAY_ALL;
	if (Parent() == NULL)					// never cull the root
		return DISPLAY_ALL;
	if (trans)								// in camera coordinates
		bound *= *trans;
	if (scene->GetCamera()->IsVisible(bound))
		return DISPLAY_ALL;					// model is visible
	Core::InterlockAdd(&(s->CulledVerts), (int) m_Verts);
	Core::InterlockInc(&(s->CulledModels));
	return DISPLAY_NONE;					// model culled
}

/*!
 * @fn void Model::CalcStats() const
 *
 * This function can compute the number of vertices and faces in this shape's
 * geometry and, if specified, the counts for its children
 * as well. Once computed, the vertex and face counts are cached until the
 * geometry changes.
 */
void Model::CalcStats() const
{
	if (!IsSet(MOD_STinvalid))
		return;
	ClearFlags(MOD_STinvalid);
	GroupIter<Model> iter(this, Group::CHILDREN);
	Model* mod;
	while (mod = iter.Next())
	{
		mod->CalcStats();
		Core::InterlockAdd(&(((Model*) this)->m_Faces), mod->m_Faces);
		Core::InterlockAdd(&(((Model*) this)->m_Verts), mod->m_Verts);
	}
}

void Model::CalcStats(intptr& numverts, intptr& numfaces) const
{
	numverts = 0;
	numfaces = 0;
}


/*!
 * @fn bool Model::CalcMatrix(Matrix* mtx, Scene* scene) const
 * @param mtx	On input, this has total matrix for parent.
 *				On output, should get total matrix for model.
 * @param scene	Scene the matrix should be calculated for
 *
 * This function is called during scene traversal to calculate
 * the total matrix for this node. The input matrix has
 * the parent's total matrix. The default implementation
 * of  CalcMatrix concatenates the model's local matrix
 * to the input matrix using a post-multiply operation.
 *
 * You can override CalcMatrix to replace or augment the
 * matrix that will be used to render this model. The
 * matrix computed here is loaded into the geometry engine
 * before Select or Render is called for the model.
 *
 * @returns  true if the input matrix was changed and a new world
 *	matrix must be loaded. If  false is returned, the parent's
 *	matrix is used.
 *
 * @see Model::SetTransform Model::Render Model::Display Model_Cull
 */
bool Model::CalcMatrix(Matrix* trans, Scene* scene) const
{
	if (m_Transform.IsNull())
		return false;
	trans->PostMul(*((const Matrix*) m_Transform));
	return true;
}

/*!
 * @fn bool Model::CalcBound(Box3* box) const
 * @param box	calculated bounding box upon return
 *
 * Called internally by Model::GetBound to calculate the bounding
 * box of this model. The scene manager uses bounding volumes for
 * first-level hit testing. If you subclass model, you must override
 * this function to provide an accurate bounding volume. The default
 * implementation fails and returns an empty box. The bounding volume
 * returned should  not include the model's children.
 *
 * @return  false if model has no volume,  true if successfully computed
 *
 * @see Model::GetBound Shape::CalcBound
 */
bool Model::CalcBound(Box3* box) const
{
	return false;
}


/*!
 * @fn bool Model::CalcSphere(Sphere* sph) const
 * @param sph	calculated bounding sphere upon return
 *
 * Called internally by Model::DoBound to calculate the bounding
 * sphere of a model. The scene manager uses bounding spheres for culling those
 * models that are outside the view volume of the camera. If you
 * subclass model, you must override this function to provide an
 * accurate bounding volume for your geometry or it will not cull
 * correctly. The default implementation fails and returns an empty sphere.
 *
 * @return  false if model has no volume,  true if successfully computed
 *
 * @see Model::GetBOund Shape::CalcSphere
 */
bool Model::CalcSphere(Sphere*) const
{
	return false;
}

/*!
 * @fn void Model::SetCulling(bool cull, bool descend)
 * @param cull		true to enable culling, false to disable
 * @param descend	if true, set culling for model's descendants also
 *
 * View volume culling discards models that are outside
 * the culling volume of the current Camera. This routine
 * enables or disables it for this model possibly its children.
 * You can set Model::DoCulling to  false to disable
 * culling for all models.
 *
 * @see Model::Cull Model::SetBound Camera::SetCullVol Camera::SetViewVol
 */
void Model::SetCulling(bool cull, bool descend)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_SetCulling) << this << int32(cull) << int32(descend);
	VX_STREAM_END( )

	m_NoCull = !cull;
	if (!descend)
		return;

	GroupIter<Model> iter(this);
	Model*			mod;

	while (mod = iter.Next())
		mod->m_NoCull = !cull;
}

/*!
 * @fn void Model::SetHints(int hints, bool descend)
 * @param hints		hints about how the model is being used
 * @param descend	true to descend the hierarchy and set hints on descendants
 *
 * Optimization hints that tell what you plan to do with this model.
 * If you do not plan to move it at all, the scene manager can optimize
 * for this case. If you are going to morph vertices or move the model,
 * the scene manager must recalculate internally cached information.
 *
 * No hints are enabled by default. Associating a model with an animation
 * controller marks it as dynamic.
 *
 * Models which do not move should be put in the  static scene
 * graph, which is optimized internally to remove all embedded
 * matrices.
 *
 * @code
 *	STATIC		says model matrix will not change
 *	MORPH		says model vertices will change
 * @endcode
 *
 * @see Model::SetBound Model::SetCulling
 */
void Model::SetHints(int hints, bool descend)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_SetHints) << this << int32(hints);
	VX_STREAM_END( )

	m_Hints = hints;
	if ((hints & MORPH) || !(hints & STATIC))
		m_AutoBounds = true;
	if (!descend)
		return;

	GroupIter<Model> iter(this);
	Model*			mod;

	while (mod = iter.Next())
		SetHints(hints, false);
}

/*!
 * @fn void Model::SetBound(const Sphere* bsp)
 * @param bsp	new bounding sphere for model
 *
 * The bounding volume is the volume enclosed by the model and its
 * descendants in the local coordinate space of the model.
 * If you set it to a non-NULL value, the system will not recompute the
 * bounds when the geometry or transforms of the model or its children change.
 * If the bounding volume is set to NULL, automatic bounding volume
 * computation is reinstated (this is the default condition).
 * Setting a non-null bounding volume will clear the MOD_Morph hint.
 *
 * @see Model::GetBound Model::Cull Model::SetCulling Model::SetHints
 */
void Model::SetBound(const Sphere* bsp)
{
	VX_STREAM_BEGIN(s)
	if (bsp)
		*s << OP(VX_Model, MOD_SetBound) << this << bsp->Center << bsp->Radius;
	VX_STREAM_END( )

	if (bsp == NULL)					/* no bounds provided? */
		m_AutoBounds = true;			/* reinstate automatic calcs */
	else
	{
		m_BoundVol = *bsp;				/* new bounding sphere */
		m_NoBounds = bsp->IsEmpty();	/* is it empty? */
		m_AutoBounds = false;			/* suppress automatic calcs */
	}
	Group* parent = Parent();
	if (parent)
		parent->NotifyParents(MOD_BVinvalid); /* notify parents bbox changed */
	ClearFlags(MOD_BVinvalid);
}

/*!
 * @fn void Model::SetBound(const Box3* box)
 * @param box	new bounding box for model
 *
 * The bounding box is the smallest axially-aligned box which
 * encloses the model and its children in local coordinate space.
 * If you set it to a non-NULL value, the system will not recompute the
 * bounds when the geometry or transforms of the model or its children change.
 * Setting a non-null bounding volume will clear the MOD_MorphBounds hint.
 *
 * @see Model::GetBound Model::Cull Model::SetCulling Model::SetHints
 */
void Model::SetBound(const Box3* box)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_SetBoxBound) << this << box->min << box->max;
	VX_STREAM_END( )

	if (box == NULL)
	{
		m_AutoBounds = true;
		SetBound((Sphere*) NULL);
		m_BoundBox.Empty();
		return;
	}

	Sphere	bsp(*box);
	m_BoundBox = *box;
	SetBound(&bsp);
}


/*!
 * @fn bool Model::DoBounds() const
 *
 * Helper function that does the work to compute the bounding box of a
 * model and its children. Inactive models also report empty bounds.
 *
 * This routine is not used internally unless Model::GetBound is called.
 *
 * @internal
 * @return  true if non-empty bounds computed, else  false
 *
 * @see Model::CalcBound Model::GetBound
 */
bool Model::DoBounds() const
{
	bool	rc = false;
	bool	nobounds = true;
	Matrix	totalmtx;

/*
 * If the bounding sphere is already valid, just return it to the
 * caller, transformed if requested
 */
	if (IsSet(MOD_BVinvalid))				// bounds invalid?
		OnBoundsChange();					// let subclasses know
/*
 * if the bounding volume is valid or it has been set by user,
 * do not recalculate the bounds here.
 * If we are morphing, we always recalculate since the geometry is changing.
 */
	if ((!IsSet(MOD_BVinvalid) || !m_AutoBounds) && !(m_Hints & MORPH))
		return true;
/*
 * Compute the bounding sphere from the bounding box of this model
 * and the bounding spheres of all children. Start with just
 * the bounds of the geometry for this model only.
 */
	Sphere	boundvol(Vec3(0,0,0), 0);
	Box3	boundbox(0,0,0,0,0,0);

	if (IsActive() && CalcSphere(&boundvol) && CalcBound(&boundbox))
	{
		nobounds = false;
		rc = true;
	}
/*
 * Calculate bounding volumes for all children and add that in too.
 */
	const Model* mod;
	Sphere sp;
	for (mod = First(); mod != NULL; mod = mod->Next())	// for each child
	{
		VX_ASSERT(mod != this);
		if (mod->DoBounds())				// get bounding volume
		{
			Sphere	bsp(mod->m_BoundVol);
			Box3	bbox(mod->m_BoundBox);
			const Matrix* trans = mod->GetTransform();

			if (bsp.Radius == 0)			// don't count empty models
				continue;
			if (!trans->IsIdentity())		// have a local matrix?
			{
				bsp *= *trans;				// apply child's local matrix
				bbox *= *trans;
			}
			boundvol.Extend(bsp);			// extend overall model bounds
			boundbox.Extend(bbox);
			rc = true;
			nobounds = false;
		}
	}
	Lock();
	ClearFlags(MOD_BVinvalid);		// mark bounds as valid
	m_NoBounds = nobounds;
	m_BoundVol = boundvol;			// update model bounding volume
	m_BoundBox = boundbox;
	Unlock();
	return rc;
}

/*!
 *
 * @fn bool Model::GetCenter(Vec3* ctr, int opts) const
 * @param ctr	where to store the center point
 * @param opts	determines how to transform the center point.
 * @code
 *	Model::WORLD	transform relative to the top of hierarchy (center in world coordinates)
 *	Model::LOCAL	only apply model's local transform (center in local coordinates)
 *	Model::NONE		do not transform at all (center of the geometry)
 * @endcode
 *
 * Returns the center of the bounding volume of a model transformed
 * by the total, concatenated matrix beginning at a given parent.
 * The  parent argument works in the same manner as for Model::GetBound.
 *
 * @return  true for successful center computation,
 *		 false  for empty bounding volume, returns (0,0,0) as center
 *
 * @see Model::GetBound Model::TotalTransform
 */
bool Model::GetCenter(Vec3* p, int opts) const
{
	Sphere	sp;
	Vec3	tmp;
	Matrix	mtx;

    VX_ASSERT(p);
	if (!DoBounds())				// empty bounds?
	{
		p->Set(0, 0, 0);			// (0,0,0) is center
		return false;
	}
	*p = m_BoundVol.Center;			// get center of bounding sphere
	switch (opts)
	{
		case WORLD:					// model & children in world coords (top parent)
		TotalTransform(&mtx);
		mtx.Transform(m_BoundVol.Center, *p);
		break;

		case LOCAL:					// model & children in local coords
		m_Transform->Transform(m_BoundVol.Center, *p);
		break;
	}
	return true;
}

Vec3 Model::GetCenter(int flags) const
    { Vec3 p; GetCenter(&p, flags); return p; }

/*!
 * @fn Vec3 Model::GetDirection() const
 *
 * Returns the local direction of a model. This is the vector
 * (0, 0, -1) (the default direction of a model) transformed
 * by its local matrix.
 *
 * @return direction vector for model
 *
 * @see Model::GetTranslation Model::GetRotation Model::SetRotation
 */
Vec3 Model::GetDirection() const
{
	Vec3		v;
	Matrix	tmp;

	TotalTransform(&tmp);
	tmp.TransformVector(Vec3(0, 0, -1), v);
	return v;
}

/*!
 * @fn bool Model::GetBound(Sphere* bsp, int opts) const
 * @param bsp	Where to store the bounding sphere.
 * @param opts	Bounds calculation options
 *
 * Calculates the local bounding sphere of a model and optionally
 * its children. The bounds can be returned in one of several coordinate spaces
 * as indicated by the input options.
 *
 * @code
 *	Model::NONE		no transformation applied
 *	Model::WORLD	compute bounds for model & children,
 *					transform relative to the top of hierarchy
 *					(center in world coordinates)
 *	Model::LOCAL	compute bounds for model & children,
 *					only apply Model's local transform.
 *					(center in local coordinates)
 * @endcode
 *
 * @return  true for successful computation,  false for empty bounds
 *
 * @see Model::GetCenter Model::TotalTransform Model::CalcSphere
 */
bool Model::GetBound(Sphere* sphere, int flags) const
{
	Sphere	sp;
	Matrix	totalmtx;

	VX_ASSERT(sphere);
	if (!DoBounds())		// calculate bounding volumes
	{
		sphere->Empty();
		return false;		// bounds are empty
	}
	*sphere = m_BoundVol;
	switch (flags)
	{
		case WORLD:			// model & children in world coords (top parent)
		TotalTransform(&totalmtx);
		*sphere *= totalmtx;
		break;

		case LOCAL:			// model & children in local coords
		*sphere *= *GetTransform();
		break;
	}
	return true;
}

/*!
 * @fn bool Model::GetBound(Box3* bbox, int opts) const
 * @param bbox	Where to store the bounding box.
 * @param opts	Bounds calculation options
 *
 * Calculates an axially aligned bounding box of a model and optionally
 * its children. The bounds can be returned in one of several coordinate spaces
 * as indicated by the input options.
 *
 * @code
 *	Model::NONE		no transformation applied
 *	Model::WORLD	compute bounds for model & children,
 *					transform relative to the top of hierarchy
 *					(center in world coordinates)
 *	Model::LOCAL	compute bounds for model & children,
 *					only apply Model's local transform.
 *					(center in local coordinates)
 * @endcode
 *
 * @return  true for successful computation,  false for empty bounds
 *
 * @see Model::GetCenter Model::TotalTransform Model::CalcBound
 */
bool Model::GetBound(Box3* bbox, int flags) const
{
	if (!DoBounds())
	{
		if (bbox)
			bbox->Empty();
		return false;
	}
	if (!bbox)
		return true;
/*
 * If we just want a bounding box with the local transform applied,
 * this is cached as a property on the object
 */
	*bbox = m_BoundBox;
	if (flags == LOCAL)
	{
		if (!m_Transform.IsNull())
			*bbox *= *((const Matrix*) m_Transform);	// transform by local matrix
	}
	else if (flags == WORLD)				// bounding box in world coordinates
	{
		Matrix totalmtx;

		TotalTransform(&totalmtx);
		*bbox *= totalmtx;
	}
	return true;
}

/*!
 * @fn bool Model::Hit(Ray& ray, float& distance) const
 * @param ray		ray to intersect
 * @param distance	-> where to store distance of intersection
 *
 * Determines whether the input ray hit this model and where.
 * This routine uses the model's bounding sphere. It does not
 * report which parts of the geometry were actually hit.
 *
 * @return  true> if ray hit model, else  false
 *
 * @see Sphere::Hit Geometry::Hit RayPicker
 */
bool Model::Hit(Ray& ray, float& distance) const
{
	Sphere wbsp;
	if (!GetBound(&wbsp))
		return false;
	 return wbsp.Hit(ray, distance, NULL);
}


/*!
 * @fn void Model::TotalTransform(Matrix* trans, const Model* parent) const
 * @param trans		Where to store the total transform
 * @param parent	Determines how far up the hierarchy to concatenate
 * @code
 *	Model::WORLD	transform from the top of hierarchy
 *	Model::LOCAL	only Model's local transform.
 *	other			concatenated transforms from this parent
 * @endcode
 *
 * The total matrix is the concatenation of all the transformations
 * which apply to this model in the hierarchy it is a part of.
 * The matrix is computed up to and including the input parent.
 *
 * @see Model::GetCenter Model::GetBound
 */
void Model::TotalTransform(Matrix* trans, const Model* parent) const
{
	const Model* c = this;
	if (m_Transform.IsNull())					// no local matrix
		trans->Identity();
	else										// copy local matrix
		trans->Copy((Matrix *) m_Transform);
	if (parent == NULL)
		return;
	while (((c = (const Model*) c->Parent()) != NULL) && (c != parent))
		if (!c->m_Transform.IsNull())
			trans->PreMul(*((Matrix*) c->m_Transform));
}

/*!
 * @fn void Model::TotalTransform(Matrix* trans) const
 * @param trans			Where to store the total transform
 *
 * The total matrix is the concatenation of all the transformations
 * which apply to this model in the hierarchy it is a part of.
 *
 * @see Model::GetCenter Model::GetBound
 */
void Model::TotalTransform(Matrix* trans) const
{
	const Model* c = this;
	if (m_Transform.IsNull())					// no local matrix
		trans->Identity();
	else										// copy local matrix
		trans->Copy((Matrix *) m_Transform);
	while ((c = (const Model*) c->Parent()) != NULL)
		if (!c->m_Transform.IsNull())
			trans->PreMul(*((Matrix*) c->m_Transform));
}

void Model::Reset()
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_Reset) << this;
	VX_STREAM_END( )

	if (!m_Transform.IsNull())
		m_Transform->Identity();
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::Size(const Vec3& v)
 * @param v	vector with new scale factors
 *
 * Resizes the model relative to the local coordinate system.
 * This post-multiplies the model's local matrix by a scale matrix.
 *
 * @see Model::Move Model::Scale Matrix::Scale
 */
void Model::Size(const Vec3& v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_Size) << this << v;
	VX_STREAM_END( )

	if (m_Transform.IsNull())
	{
		m_Transform = new Matrix();
		m_Transform->ScaleMatrix(v);
	}
	else
	{
		Matrix	tmp;
		tmp.ScaleMatrix(v);
		m_Transform->PostMul(tmp);
	}
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::Size(float x, float y, float z)
 * @param x	X scale factor (1.0 does nothing)
 * @param y	Y scale factor (1.0 does nothing)
 * @param z	Z scale factor (1.0 does nothing)
 *
 * Resizes the model relative to the local coordinate system.
 * This post-multiplies the model's local matrix by a scale matrix.
 *
 * @see Model::Move Model::Scale Matrix::Scale
 */
void Model::Size(float x, float y, float z)
	{ Size(Vec3(x, y, z)); }

/*!
 * @fn void Model::Scale(const Vec3& v)
 * @param v	vector with new scale factors
 *
 * Resizes the model relative to the world coordinate system.
 * This pre-multiplies the model's local matrix by a scale matrix.
 *
 * @see Model::Translate Model::Size Matrix::Scale
 */
void Model::Scale(const Vec3& v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_Scale) << this << v;
	VX_STREAM_END( )

	NotifyParents(MOD_BVinvalid);
	if (m_Transform.IsNull())
	{
		m_Transform = new Matrix();
		m_Transform->ScaleMatrix(v);
	}
	else m_Transform->Scale(v);
}

/*!
 * @fn void Model::Scale(float x, float y, float z)
 * @param x	X scale factor (1.0 does nothing)
 * @param y	Y scale factor (1.0 does nothing)
 * @param z	Z scale factor (1.0 does nothing)
 *
 * Resizes the model relative to the world coordinate system.
 * This pre-multiplies the model's local matrix by a scale matrix.
 *
 * @see Model::Translate Model::Size Matrix::Scale
 */
void Model::Scale(float x, float y, float z)
	{ Scale(Vec3(x, y, z)); }

/*!
 * @fn void Model::Move(const Vec3& )
 * @param v	vector with new translation factors
 *
 * Translates the model relative to the local coordinate system.
 * This pre-multiplies the model's local matrix by a translation matrix.
 *
 * @see Model::Translate Model::Size Matrix::Translate
 */
void Model::Move(const Vec3& v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_Move) << this << v;
	VX_STREAM_END( )

	if (m_Transform.IsNull())
	{
		m_Transform = new Matrix();
		m_Transform->TranslationMatrix(v);
	}
	else
	{
		Matrix	tmp;
		tmp.TranslationMatrix(v);
		m_Transform->PostMul(tmp);
	}
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::Move(float x, float y, float z)
 * @param x	X translation factor (0 does nothing)
 * @param y	Y translation factor (0 does nothing)
 * @param z	Z translation factor (0 does nothing)
 *
 * Translates the model relative to the local coordinate system.
 * This pre-multiplies the model's local matrix by a translation matrix.
 *
 * @see Model::Translate Model::Size Matrix::Translate
 */
void Model::Move(float x, float y, float z)
	{ Move(Vec3(x, y, z)); }

/*!
 * @fn void Model::Translate(const Vec3& v)
 * @param v	vector with new translation factors
 *
 * Translates the model relative to the world coordinate system.
 * This pre-multiplies the model's local matrix by a translation matrix.
 *
 * @see Model::Move Model::Scale Matrix::Translate
 */
void Model::Translate(const Vec3& v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_Translate) << this << v;
	VX_STREAM_END( )

	if (m_Transform.IsNull())
	{
		m_Transform = new Matrix();
		m_Transform->TranslationMatrix(v);
	}
	else m_Transform->Translate(v);
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::Translate(float x, float y, float z)
 * @param x	X translation factor (0 does nothing)
 * @param y	Y translation factor (0 does nothing)
 * @param z	Z translation factor (0 does nothing)
 *
 * Translates the model relative to the world coordinate system.
 * This pre-multiplies the model's local matrix by a translation matrix.
 *
 * @see Model::Move Model::Scale Matrix::Translate
 */
void Model::Translate(float x, float y, float z)
	{ Translate(Vec3(x, y, z)); }

/*!
 * @fn void Model::Turn(const Vec3& axis, float angle)
 * @param axis		Axis of rotation
 * @param angle		Angle to rotate about the axis (specified in radians)
 *
 * Rotates the model about its geometric center in
 * its local coordinate system.  This pre-multiplies the
 * model's local matrix by a rotation matrix about one
 * of the coordinate axes. The translation components of the
 * matrix are not affected so the model remains in the same position.
 *
 * @see Model::Rotate Model::SetRotation Matrix::Rotate
 */
void Model::Turn(const Vec3& axis, float angle)
{
	VX_STREAM_BEGIN(s)
			*s << OP(VX_Model, MOD_Turn) << this << axis << angle;
	VX_STREAM_END( )

	if (angle == 0.0)
		return;

	if (m_Transform.IsNull())
	{
		m_Transform = new Matrix();
		m_Transform->RotationMatrix(axis, angle);
	}
	else
	{
		Matrix tmp;
		tmp.RotationMatrix(axis, angle);
		m_Transform->PostMul(tmp);
	}
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::Turn(const Quat& q)
 * @param q	Quaternion specifying angle & axis
 *
 * Rotates the model about its geometric center in
 * its local coordinate system.  This pre-multiplies the
 * model's local matrix by a rotation matrix about one
 * of the coordinate axes. The translation components of the
 * matrix are not affected so the model remains in the same position.
 *
 * @see Model::Rotate Model::SetRotation Matrix::Rotate Quat
 */
void Model::Turn(const Quat& q)
{
//	VX_STREAM_BEGIN(s)
//			*s << OP(VX_Model, MOD_Turn) << this << axis << angle;
//	VX_STREAM_END( )

	if (q.w == 0.0)
		return;
	if (m_Transform.IsNull())
		m_Transform = new Matrix(q);
	else
	{
		Matrix tmp(q);
		m_Transform->PostMul(tmp);
	}
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::Rotate(const Vec3& axis, float angle)
 * @param axis		Axis of rotation
 * @param angle		Angle to rotate about the axis (specified in radians)
 *
 * Rotates the model about its geometric center in
 * the world coordinate system.  This post-multiplies the
 * model's local matrix by a rotation matrix about one
 * of the coordinate axes. If it is not located at 0,0,0
 * the model will change position.
 *
 * @see Model::Rotate Model::SetRotation Matrix::Rotate
 */
void Model::Rotate(const Vec3& axis, float angle)
{
	if (angle == 0.0)
		return;

	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_Rotate) << this << axis << angle;
	VX_STREAM_END( )

	if (m_Transform.IsNull())
	{
		m_Transform = new Matrix();
		m_Transform->RotationMatrix(axis, angle);
	}
	else m_Transform->Rotate(axis, angle);
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::Rotate(const Quat& q)
 * @param q	Quaternion specifying angle & axis
 *
 * Rotates the model about its geometric center in
 * the world coordinate system.  This post-multiplies the
 * model's local matrix by a rotation matrix about one
 * of the coordinate axes. If it is not located at 0,0,0
 * the model will change position.
 *
 * @see Model::Rotate Model::SetRotation Matrix::Rotate Quat
 */
void Model::Rotate(const Quat& q)
{
//	VX_STREAM_BEGIN(s)
//			*s << OP(VX_Model, MOD_Rotate) << this << axis << angle;
//	VX_STREAM_END( )
// TODO: put stream logging here
	if (q.w == 0.0)
		return;
	if (m_Transform.IsNull())
		m_Transform = new Matrix(q);
	else
	{
		Matrix tmp(q);
		m_Transform->PreMul(tmp);
	}
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::LookAt(const Vec3& lookat, float twist)
 * @param lookat	Point to look at. This point must be in the coordinate
 *					system of the model's parent
 * @param twist		Angle to rotate about the Z axis (in radians)
 *
 * Modifies the model's local transform to make the model
 * face a given point. The front of the model is considered
 * to be aligned along the Z axis. Note: this function does  not
 * preserve scale or rotation factors. It does preserve translation.
 *
 * @see Model::Rotate Matrix::LookAt Matrix::Rotate
 */
void Model::LookAt(const Vec3& lookat, float twist)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_LookAt) << this << lookat << twist;
	VX_STREAM_END( )

    Vec3	trans, p;

	if (m_Transform.IsNull())
		m_Transform = new Matrix();
	m_Transform->GetTranslation(trans);
    p = lookat;
    p -= trans;
	m_Transform->LookAt(p, twist);
    m_Transform->Translate(trans);
 	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn void Model::SetTranslation(const Vec3& p)
 * @param p	new translation factors
 *
 * Replaces the translation factors in the Model's
 * transformation matrix. The position of the model
 * will be affected but its orientation will remain the same.
 *
 * @see Model::Translate Model::Move Matrix::Translate Model::SetRotation
 */
void Model::SetTranslation(const Vec3& p)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_SetTranslation) << this << p;
	VX_STREAM_END( )

	if (m_Transform.IsNull())
	{
		m_Transform = new Matrix();
		m_Transform->TranslationMatrix(p);
	}
	else
		m_Transform->SetTranslation(p);

	NotifyParents(MOD_BVinvalid);
}

Vec3 Model::GetTranslation() const
{
	Vec3 p;

	if (m_Transform.IsNull())
		return Vec3(0,0,0);
	m_Transform->GetTranslation(p);
	return p;
}

/*!
 * @fn void Model::SetRotation(const Quat& q)
 * @param q	new rotation, specified as a Quaternion
 *
 * Replaces the rotation factors in the Model's
 * transformation matrix. Note: this routine
 * destroys any scale factors in the matrix.
 *
 * @see Model::Rotate Model::Turn Quat Matrix::Rotate
 */
void Model::SetRotation(const Quat& q)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_SetRotation) << this << q;
	VX_STREAM_END( )

	if (m_Transform.IsNull())
	{
		m_Transform = new Matrix();
		m_Transform->Set(q);
		return;
	}
	Vec3 p = GetTranslation();
	m_Transform->Set(q);
	SetTranslation(p);
}

Quat Model::GetRotation() const
{
	if (m_Transform.IsNull())
		return Quat(0,0,0,1);
	return Quat(*((Matrix*) m_Transform));
}

/****
 *
 * Override for SharedObj::Print
 * Print model description on standard output.
 *
 ****/
DebugOut& Model::Print(DebugOut& dbg, int opts) const
{
	if ((opts & (PRINT_Attributes | PRINT_Children)) == 0)
		return SharedObj::Print(dbg, opts);
	Group::Print(dbg, opts & ~PRINT_Trailer);
	if (!m_NoBounds)
		endl(dbg << "\t<attr name='Bound'>" << m_BoundBox.min << m_BoundBox.max << "</attr>");
	if (!m_Transform.IsNull())
		m_Transform->Print(dbg, opts | PRINT_Trailer);
	return Group::Print(dbg, opts & PRINT_Trailer);
}

/*!
 * @fn void Model::SetTransform(const Matrix* mtx)
 * @param mtx	new local matrix for model
 *
 * Copies the given matrix into the transformation matrix of the
 * model. This replaces the previous matrix.
 *
 * @see Matrix Model::GetTransform Model::TotalTransform
 */
void Model::SetTransform(const Matrix* trans)
{
	VX_STREAM_BEGIN(s)
		if (trans == NULL)
			*s << OP(VX_Model, MOD_Reset) << this;
		else
		{
			*s << OP(VX_Model, MOD_SetTransform) << this;
			s->Output(trans->GetMatrix(), 16);
		}
	VX_STREAM_END( )

	NotifyParents(MOD_BVinvalid);
	if ((trans == NULL) || trans->IsIdentity())
	{
		m_Transform = (Matrix*) NULL;
		return;
	}
	if (m_Transform.IsNull())
		m_Transform = new Matrix(*trans);
	else m_Transform->Copy(trans);
}

/*!
 * @fn void Model::SetTransform(const float* mtx)
 * @param mtx	16 floating numbers of matrix data
 *
 * Copies the given matrix into the transformation matrix of the
 * model. This replaces the previous matrix.
 *
 * @see Matrix Matrix::SetMatrix Model::TotalTransform
 */
void Model::SetTransform(const float* mtx)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Model, MOD_SetTransform) << this;
		s->Output(mtx, 16);
	VX_STREAM_END( )

	NotifyParents(MOD_BVinvalid);
	if (mtx == NULL)
	{
		m_Transform = (Matrix*) NULL;
		return;
	}
	if (m_Transform.IsNull())
		m_Transform = new Matrix(mtx);
	else m_Transform->SetMatrix(mtx);
}

void Model::TakeOut(Group* parent)
{
	NotifyParents(MOD_BVinvalid | CHANGED);
}

void Model::PutIn(Group* parent)
{
	NotifyParents(MOD_BVinvalid | CHANGED);
}

/****
 *
 * Override for SharedObj::Copy
 * Copies the attributes of one model into another. All of the
 * model's children are copied but not their visual descriptions.
 * Copy should multiply reference attributes rather than make copies.
 *
 ****/
bool Model::Copy(const SharedObj* src_obj)
{
	const Model* src = (const Model*) src_obj;

	VX_ASSERT(src);
    VX_ASSERT(src_obj->IsClass(VX_Model));
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Group::Copy(src_obj))
		return false;
	m_BoundVol = src->m_BoundVol;
	m_NoBounds = src->m_NoBounds;
	if (src->m_Transform.IsNull())
		m_Transform = (Matrix*) NULL;
	else if (m_Transform.IsNull())
		m_Transform = new Matrix(*((Matrix*) src->m_Transform));
	else m_Transform->Copy((const Matrix*) src->m_Transform);
	m_AutoBounds = src->m_AutoBounds;
	m_NoCull = src->m_NoCull;
	m_Hints = src->m_Hints;
	m_Faces = src->m_Faces;
	m_Verts = src->m_Verts;
	return true;
}

/****
 *
 * class Model override for SharedObj::Do
 *		MOD_SetTransform	<Matrix>
 *		MOD_Rotate			<Vec3 axis>  <float angle>
 *		MOD_Turn			<Vec3 axis>  <float angle>
 *		MOD_Translate		<Vec3>
 *		MOD_Move			<Vec3>
 *		MOD_Scale			<Vec3>
 *		MOD_Size			<Vec3>
 *		MOD_LookAt			<Vec3 lookat>  <float twist>
 *		MOD_SetBound		<Vec3 center> <float radius>
 *
 ****/
bool Model::Do(Messenger& s, int op)
{
	Matrix	trans;
	float	md[16];
	Vec3	v;
	float	f;
	int32	n, d;
	Sphere	bsp;
	Box3	box;
	Opcode	o = Opcode(op);

	switch (op)
	{
		case MOD_SetTransform:
		s.Input(md, 16);
		trans.SetMatrix(md);
		SetTransform(&trans);
		break;

		case MOD_Rotate:
		s >> v >> f;
		Rotate(v, f);
		break;

		case MOD_Turn:
		s >> v >> f;
		Turn(v, f);
		break;

		case MOD_LookAt:
		s >> v >> f;
		LookAt(v, f);
		break;

		case MOD_Translate:
		s >> v;
		Translate(v);
		break;

		case MOD_SetTranslation:
		s >> v;
		SetTranslation(v);
		break;

		case MOD_Move:
		s >> v;
		Move(v);
		break;

		case MOD_Scale:
		s >> v;
		Scale(v);
		break;

		case MOD_Size:
		s >> v;
		Size(v);
		break;

		case MOD_Reset:
		Reset();
		break;

		case MOD_SetBound:
		s >> bsp.Center >> bsp.Radius;
		if (bsp.Radius == 0.0f)
			SetBound((Sphere*) NULL);
		else SetBound(&bsp);
		break;

		case MOD_SetBoxBound:
		s >> box.min >> box.max;
		SetBound(&box);
		break;

		case MOD_SetHints:
		s >> n;
		SetHints(n, false);
		break;

		case MOD_SetCulling:
		s >> n >> d;
		SetCulling(n != 0, d != 0);
		break;

		default:
		return Group::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Model::DoNames[op - MOD_SetTransform]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Model override for SharedObj::Save
 *
 ****/
int Model::Save(Messenger& s, int opts) const
{
	int32 h = Group::Save(s, opts);

	if (h <= 0)
		return h;
	if (m_Hints)
		s << OP(VX_Model, MOD_SetHints) << h << int32(m_Hints);
	if (!IsCulling())
		s << OP(VX_Model, MOD_SetCulling) << this << int32(0) << int32(0);
	else if (!(m_AutoBounds || m_NoBounds))
		s << OP(VX_Model, MOD_SetBoxBound) << h << m_BoundBox.min << m_BoundBox.max;
	if (!m_Transform.IsNull())
	{
		s << OP(VX_Model, MOD_SetTransform) << h;
		s.Output(m_Transform->GetMatrix(), 16);
	}
	return h;
}

}	// end Vixen