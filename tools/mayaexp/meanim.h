#pragma once

#include "meconv.h"

/*!
 * @class meConvEng
 * @brief Base class to convert between Maya and Vixen data structures.
 *
 * The base implementation handles hierarchy linkage and conversion of the
 * transformation matrix. Subclasses deal with different types of geometry
 * and rendering state. Converters are reference counted so they can be
 * shared in the global Maya name dictionary.
 *
 * @see NameTable vixExporter
 */
class meConvEng  : public meConvObj
{
public:
    meConvEng(MObject& mpath);
	meConvEng(const MDagPath& mpath);

	virtual	SharedObj*	Make();

	//! Link the engine into the Vixen simulation tree
    virtual int			Link(meConvObj* convparent = NULL);

	//! Link this engine to it's Vixen target Model or Engine
	virtual SharedObj*	LinkTarget(meConvObj*);

   //! Make a node converter of the appropriate type for the Maya node referenced by the DAG path.
    static meConvEng*	MakeConverter(MObject&);


protected:
	static bool			GatherCurves(MPlug& plug);
};

/*!
 * @class meConvJoint
 * @brief Node converter for joints.
 *
 * Converts a joint from Maya into a Vixen Transformer.
 *
 * @see meConvNode
 */
class meConvSkin;
class meConvSkeleton;
class meConvJoint : public meConvEng
{
public:
	enum
	{
		ROTATION = 1,
		TRANSLATION = 2,
		MAYA = 8
	};

	meConvJoint(MObject& mayaObj) : meConvEng(mayaObj) { HasAnimation = 0; HasIK = 0; m_JointIndex = -1; }
    meConvJoint(const MDagPath& dagNode) : meConvEng(dagNode) { HasAnimation = 0; HasIK = 0; m_JointIndex = -1; }

    virtual	int			Convert(meConvObj* = NULL);
    virtual	SharedObj*	Make();
	virtual int			Link(meConvObj* convparent = NULL);
	virtual SharedObj*	LinkTarget(meConvObj* conv);
	virtual	bool		GetMatrix(Matrix&) const;
	int					GetJointIndex() const		{ return m_JointIndex; }
	Skeleton*			MakeSkeleton();
	int					LinkAnimation(MObject& mayaObj, meConvJoint* parentconv);
	void				CalcJointPosRot(Vec3& pos, Quat& rot);
	void				CalcJointMatrix(Matrix& worldmtx);
	int					MakeJointMap(const MDagPathArray& influences, int32* jointmap);

	Ref<Interpolator>	Position;
	Ref<Interpolator>	Rotation;
	int					HasIK;			// 0, MFn::kIkHandle, MFn::kIKEffector, MFn::kIKJoint
	int					HasAnimation;	// 0, 1 = rotation, 2 = translation


protected:
	int					m_JointIndex;

	Ref<Skeleton>		m_Skeleton;
};


/*!
 * @class meConvSkin
 * @brief Node converter for skin bind.
 *
 * Converts a skinbind node from Maya into a Vixen Skin engine.
 *
 * @see meConvEng meConvBone meConvJoint
 */
class meConvSkin : public meConvEng
{
public:
    meConvSkin(MObject& mayaObj) : meConvEng(mayaObj) { }

    virtual	int			Convert(meConvObj* = NULL);
    virtual	SharedObj*	Make();
	virtual	int			Link(meConvObj* convparent = NULL);
	virtual SharedObj*	LinkTarget(meConvObj*);
	meConvJoint*		FindRootJoint(const MDagPathArray& influences);
	static void			LinkSkeleton(meConvJoint* rootjoint);	
	static const int	MAX_BONES_PER_VERTEX;

protected:
	meConvPoly*			m_OutMesh;
};

/*!
 * @class meConvDeform
 * @brief Node converter for deformer.
 *
 * Converts a blend shape node from Maya into a Vixen Deformer engine.
 *
 * @see meConvEng meConvBone meConvJoint
 */
class meConvDeform : public meConvEng
{
public:
    meConvDeform(MObject& mayaObj) : meConvEng(mayaObj) { }

    virtual	SharedObj*	Make();
	virtual int			Link(meConvObj* convparent = NULL);
	virtual	int			Convert(meConvObj* = NULL);
	virtual	SharedObj*	LinkTarget(meConvObj* conv);

protected:
	void				LinkBlendShapes();
	VertexArray*		MakeBlendSource(Shape* source);
	MObject				m_OutMesh;
	VertexArray*		m_TargetVerts;
};


/*!
 * @class meConvCloth
 * @brief Node converter for cloth simulation.
 *
 * Converts a cloth simulation node from Maya into a Vixen MeshAnimator engine.
 *
 * @see meConvEng meConvDeform
 */
class meConvCloth : public meConvEng
{
public:
    meConvCloth(MObject& mayaObj) : meConvEng(mayaObj) { }
    meConvCloth(const MDagPath& dagNode) : meConvEng(dagNode) { }

    virtual	SharedObj*	Make();
	virtual int			Link(meConvObj* convparent = NULL);

protected:
	VertexArray*		MakeRestVerts();

	MObject				m_OutMesh;
	MObject				m_RestMesh;
	VertexArray*		m_TargetVerts;
};