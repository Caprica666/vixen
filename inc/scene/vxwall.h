#pragma once

namespace Vixen {

/*!
 * @class Wall
 * @brief Planar walls in a room used with portals.
 *
 * In order to achieve optimal performance, a wall containing a portal must have
 * a  hole cut out of the geometry of the wall where the portal is to go
 * The portal does not cut a hole in the wall, it just connects the rooms
 * The geometry must be cut out by the application. 
 *
 * @image html wall.gif
 * 
 * Fixtures are not special objects in and of themselves. Fixtures is a general 
 * term applied to the objects in a room that are not part of the convex walls. 
 * Objects such as furniture, columns, light fixtures are all examples of fixtures.
 * The Fixtures in a given room are drawn after the portals and walls of a room are 
 * drawn, but before the room returns from its drawing routine. In this way, 
 * fixtures added as children of the room that contains them are drawn before 
 * objects inside rooms that are closer to the viewer.
 *
 * If a room has fixtures, then the rendered scene will tend to have a depth
 * complexity of greater than one, because the fixture will probably obscure a 
 * room wall. However, fixtures are culled and clipped to the portals through 
 * which the room containing them is seen. Fixtures must be sorted or zbuffered 
 * in order to make sure thatr they are drawn correctly with respect to the other
 * fixtures in that room (The scene manager takes care of drawing the fixtures of different rooms
 * in the correct order.)
 *
 * The @htmlonly <A><HREF="oview/portaloview.htm">Portals Overview</A> @endhtmlonly
 * describes how walls, fixtures and rooms are related.
 *
 * @see Portal Room RoomGroup
 */
class Wall : public Model
{
public:
	VX_DECLARE_CLASS(Wall);

    // construction and destruction
    Wall();
    virtual ~Wall ();

    //! Get plane of wall in local (model) coordinates.
    const Plane& GetModelPlane () const;

    //! Get plane of wall in world coordinates.
    const Plane& GetWorldPlane () const;

	//! Establish plane of the wall.
    void		SetModelPlane (const Plane& plane);
    void		UpdateWorldPlane (const Matrix* mv);
	void		RemoveMatrix();

    // Called by ISMRoom::ComputeCorrectNormal for each wall of the room.
    // The input point must be a point inside the room.  The model plane
    // normal is corrected (if necessary) to point to the correct side of
    // the wall.  Return value is 'true' iff the normal was corrected.
    bool		ComputeCorrectNormal (const Vec3& inside);

	// Overrides
	virtual bool		Copy(const SharedObj* src);
	virtual int			Save(Messenger&, int) const;
	virtual bool		Do(Messenger& s, int opcode);
	virtual bool		CalcBound(Box3* box) const;
	virtual bool		CalcSphere(Sphere* bsp) const;
	virtual int			Cull(const Matrix* trans, Scene* scene);
	virtual DebugOut&	Print(DebugOut& deb = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		WALL_SetModelPlane = Model::MOD_NextOp,
		WALL_NextOp = Model::MOD_NextOp + 10,
	};

protected:
	void		MakeRenderGeometry();

    // planes of the wall
    Plane	m_modelPlane;
    Plane	m_worldPlane;
};

inline Wall::Wall () : Model()
{
}

inline Wall::~Wall ()
{
}

/*!
 * @fn const Plane& Wall::GetModelPlane () const
 *
 * Retrieve the model space plane defining the wall.
 * A wall’s plane defines the plane containing all of the geometry for that wall,
 * as well as the half-space that is the inside side of the wall.
 * The plane should have its positive side face into the room.
 * Applications using these functions to modify a wall’s model-space plane
 * must ensue that Update is called upon the wall or an ancestor of the wall
 * before the world data is used (e.g. for rendering, querying point-in-room, etc).
 *
 * @see Wall::SetModelPlane
 */
inline const Plane& Wall::GetModelPlane () const
{
    return m_modelPlane;
}

inline const Plane& Wall::GetWorldPlane () const
{
    return m_worldPlane;
}

} // end Vixen