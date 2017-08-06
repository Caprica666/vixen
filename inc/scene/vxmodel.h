/*!
 * @file vxmodel.h
 * @brief 3D object in display hierarchy.
 *
 * The scene graph is a tree of models which contain 3D geometry
 * and appearances.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxshape.h vxcam.h vxlight.h vxscene.h
 */
#pragma once

namespace Vixen {

class Scene;

/*!
 * @class Model
 * @brief Node in a display hierarchy (scene graph).
 *
 * Usually a model is something that can be displayed on the screen.
 * There are many ways to represent models - as triangles meshes with textures,
 * video sequences, even sounds.
 *
 * A model can also be a group of other models, or a hierarchy.
 * For example, consider a huamn figure. It is composed of a number of models
 * for individual parts like the head, arms and legs.
 * The model at the top of the hierarchy represents the figure as a whole
 * and contains the individual parts as children. 3D transformations
 * are inherited so when you move or rotate the figure, the operation
 * is applied to the individual parts as well.
 *
 * @image html scenegraph.gif
 *
 * Models may be moved, scaled and rotated in either their local
 * coordinate system or in world coordinates. Successive transformations
 * to a model are concatenated together and the sequence is preserved
 * in a single matrix within the model. There are a lot of routines
 * provided to manipulate the model's matrix in intuitive terms.
 * You can also ask a model for its center and bounding box.
 *
 * You view the models in a hierarchy by associating them with a Scene.
 * Each frame, your application repositions the models and the scene
 * displays them. You can control how models are displayed by overriding
 * the framework functions that traverse the scene graph.
 *
 * <B>Subclassing</B>
 *
 * Each frame, the simulation pass updates the models in the hierarchy
 * and they are redisplayed. Display of the scene graph involves
 * several steps, all can be overridden by a subclass. This allows you
 * to create your own application or platform specific models.
 * The game extensions like BillBoard and Switch are simple
 * examples. BillBoard overrides  CalcMatrix to produce a world
 * transformation matrix that makes the billboard always face the camera.
 * Switch replaces the default  Cull behavior of displaying all the model's
 * children with only displaying a single child based on the switch index.
 *
 * On some platforms, the scene manager does simulation, scene graph traversal and rendering
 * in separate threads. Thus the  CalcMatrix function might compute a world
 * matrix in one thread that is passed to the  Render function in another thread.
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
 *
 * @ingroup vixen
 * @see Scene Matrix Transformer
 */
class Model : public Group
{
public:
	VX_DECLARE_CLASS(Model);
	VX_GROUP_DECLARE(Model);

	/*!
	 * @brief Model bounding box transformation type
	 * @see GetBound GetCenter
	 */
	enum
	{
		WORLD = 0,		//!< model & children in world coords (top parent)
		LOCAL = -1,		//!< model & children in local coords
		NONE = -2,		//!< model & children, no matrix applied
	};

	/*!
	 * @brief optimization hints.
	 * @see SetHints
	 */
	enum
	{
		STATIC = 2,		//!< static - model geometry and matrix do not change
		MORPH =	4,		//!< morph - model geometry dynamically changes
	};
	
	/*!
	 * @brief Cull return codes
	 * @see SetCull
	 */
	enum
	{
		DISPLAY_NONE = 0,
		DISPLAY_ME = 1,
		DISPLAY_ALL = 2
	};

//	BOUNDS AND OPTIMIZATION
	Model(const Model&);
	Model();
	~Model()	{ }

	//! Return model optimization hints.
	int				GetHints() const;

	//! Set model optimization hints.
	virtual void	SetHints(int hints, bool descend = false);

	//! Return model center with respect to a parent.
	Vec3			GetCenter(int transformtype = WORLD) const;
	virtual bool	GetCenter(Vec3* OUTPUT, int transformtype = WORLD) const;

	//! Return model direction in local coordinates.
	virtual Vec3	GetDirection() const;

	//! Get the bounding volume of the model
	bool			GetBound(Sphere* OUTPUT, int transformtype = WORLD) const;
	bool			GetBound(Box3* OUTPUT, int transformtype = WORLD) const;

	//! Set bounding volume as axially aligned box.
	virtual void	SetBound(const Box3* INPUT);

	//! Set bounding volume as sphere.
	virtual void	SetBound(const Sphere* INPUT);

	//! Return view volume culling flag ( true if enabled).
	bool			IsCulling() const;

	//! Enable / Disable view volume culling for this model and children.
	void			SetCulling(bool cull, bool descend = false);

	//! Calculate model bounding volume as a sphere.
	virtual bool	CalcSphere(Sphere*) const;

	//! Calculate model bounding volume as a box.
	virtual bool	CalcBound(Box3*) const;

	//! Return  true if model has been rendered this frame
	bool			WasRendered() const;

	//! Set "rendered" flag for model
	void			SetRendered(bool flag) const;

	//! Determine if this model was hit by the given ray
	virtual bool	Hit(Ray& ray, float& distance) const;

// MATRIX OPERATIONS

	//! Make local matrix identity (no local transformation on model).
	void			Reset();

	//! Set local matrix translation factors.
	void			SetTranslation(const Vec3& INPUT);

	//! Return local matrix translation factors.
	Vec3			GetTranslation() const;

	//! Set local matrix rotation factors.
	void			SetRotation(const Quat& INPUT);

	//! Get local matrix rotation factors.
	Quat			GetRotation() const;

	//! Rotate model in local coordinate system (post multiply).
	void			Turn(const Vec3& axis, float angle);
	void			Turn(const Quat& INPUT);

	//! Rotate model in parent coordinate system (pre multiply).
	void			Rotate(const Vec3& axis, float angle);
	void			Rotate(const Quat& INPUT);

	//! Rotate model to face a point.
	void			LookAt(const Vec3& INPUT, float twist);

	//! Translate model in local coordinate system (post multiply).
	void			Move(const Vec3&);
	void			Move(float x, float y, float z);

	//! Translate model in parent coordinate system (pre multiply).
	void			Translate(const Vec3& INPUT);
	void			Translate(float x, float y, float z);

	//! Scale model in parent coordinate system (pre multiply).
	void			Scale(const Vec3&);
	void			Scale(float x, float y, float z);

	//! Scale model in local coordinate system (post multiply).
	void			Size(const Vec3& INPUT);
	void			Size(float x, float y, float z);

	//! Set local transformation matrix.
	void			SetTransform(const Matrix* src);
	void			SetTransform(const float* floatArray);

	//! Get local transformation matrix.
	const Matrix*	GetTransform() const;

	//! Compute world transformation matrix.
	void			TotalTransform(Matrix* output) const;

	//! Compute transformation matrix relative to a parent.
	void			TotalTransform(Matrix* output, const Model* parent) const;

	//! Return number of vertices in the model and its children.
	intptr			GetNumVtx() const;

	//! Enable / disable view volume culling for all models.
	static	bool	DoCulling;

	//! X, Y, Z Axes convenience members
	static const Vec3	XAXIS;
	static const Vec3	YAXIS;
	static const Vec3	ZAXIS;

	virtual	bool		Do(Messenger& s, int opcode);
	virtual	int			Save(Messenger&, int) const;
	virtual	bool		Copy(const SharedObj* src);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;


//	INTERNAL RENDERING OVERRIDES

	//! Calculate number of vertices and faces in model and children.
	virtual	void	CalcStats() const;

	//! Calculate number of vertices and faces in model only.
	virtual	void	CalcStats(intptr& numverts, intptr& numfaces) const;

	//! Determine which geometry should be culled.
	virtual	intptr	Cull(const Matrix*, Scene*);

	//! Traverse display hierarchy to determine what to render.
	virtual	void	Display(Scene*);

	//! Compute final transformation matrix for the model.
	virtual bool	CalcMatrix(Matrix*, Scene* scene) const;

	//! Prepare model geometry for rendering.
	virtual void	Render(Scene*);

	/*
	 * Model::Do opcodes (and for binary file format)
	 */
	enum Opcode
	{
		MOD_SetTransform = Group::GROUP_NextOp,
		MOD_Rotate,
		MOD_Turn,
		MOD_Translate,
		MOD_Move,
		MOD_Scale,
		MOD_Size,
		MOD_LookAt,
		MOD_SetTranslation,
		MOD_SetRotation,
		MOD_Reset,
		MOD_SetBound,
		MOD_SetHints,
		MOD_SetCulling,
		MOD_SetBoxBound,
		MOD_NextOp = Group::GROUP_NextOp + 30
	};

protected:
	//! Notification callback when model is removed from group.
	virtual	void	TakeOut(Group* parent);

	//! Notification callback when model is added to group.
	virtual	void	PutIn(Group* parent);

	//! Notification callback when model's bounding volume changes.
	virtual void	OnBoundsChange() const { };

	//! Compute axially aligned bounding box for hierarchy.
	bool			DoBounds() const;

//	DATA MEMBERS
	Ref<Matrix>		m_Transform;		// local matrix
	mutable Sphere	m_BoundVol;			// bounding sphere
	mutable Box3	m_BoundBox;			// bounding box
	mutable bits	m_Hints : 4;		// hint bits
	mutable bits	m_NoCull : 1;		// true to suppress culling
	mutable bits	m_NoBounds : 1;		// true if empty bounds
	mutable bits	m_AutoBounds : 1;	// true to automatically calc bounds
	mutable bits	m_Rendered : 1;		// true if rendered
	mutable intptr volatile m_Faces;	// number of faces in the model & children
	mutable intptr volatile m_Verts;	// number of vertices in model & children
};

} // end Vixen
