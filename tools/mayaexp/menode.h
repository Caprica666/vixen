#pragma once

#include "meconv.h"

/*!
 * @class meConvNode
 * @brief Base class to convert between Maya and Vixen data structures.
 *
 * The base implementation handles hierarchy linkage and conversion of the
 * transformation matrix. Subclasses deal with different types of geometry
 * and rendering state. Converters are reference counted so they can be
 * shared in the global Maya name dictionary.
 *
 * @see NameTable vixExporter
 */
class meConvNode  : public meConvObj
{
public:
    meConvNode(const MDagPath& mpath);

    //! Make a node converter of the appropriate type for the Maya node referenced by the DAG path.
    static meConvObj*	MakeConverter(const MDagPath& mpath);

    //! Make a Vixen node of the appropriate type.
    virtual SharedObj*	Make();

    //! Convert the Maya object into one or more Vixen nodes.
    virtual int		Convert(meConvObj* = NULL);

    //! Link the Performer node in the the Vixen scene graph.
    virtual int		Link(meConvObj* convparent = NULL);

    virtual	bool	GetMatrix(Matrix&) const;

    //! Get total transform from root to this node.
    bool			GetTotalTransform(Matrix&) const;

    //! Get inverse of total transform from root to this node.
    bool			GetTotalInverse(Matrix&) const;

	//! Link animation controllers
	bool			LinkAnimation();
	
	bool			ParentIsJoint;
	Transformer*	Animation;
};

/*!
 * @class meConvCam
 * @brief Node converter for cameras.
 *
 * Converts a camera from Maya into a Vixen camera.
 *
 * @see meConvNode
 */
class meConvCam : public meConvNode
{
public:
    meConvCam(const MDagPath& mpath) : meConvNode(mpath) { }

    virtual int			Convert(meConvObj* = NULL);
    virtual SharedObj*	Make();
};

/*!
 * @class meConvLight
 * @brief Node converter for lights.
 *
 * Converts a light from Maya into a Vixen light.
 *
 * @see meConvNode
 */
class meConvLight : public meConvNode
{
public:
    meConvLight(const MDagPath& mpath) : meConvNode(mpath) { }

    virtual int			Convert(meConvObj* = NULL);
    virtual SharedObj*	Make();
};
