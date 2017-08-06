/*!
 * @file vxroom.h
 * @brief Room in portal-based visibility graph.
 *
 * Rooms are displayed based on their visibility from the current viewpoint.
 *
 * @author Nola Donato
 * @author algorithm from University of North Carolina Computer Graphics Lab
 * @ingroup vixen
 *
 * @see vxportal.h vxwall.h vxroomgroup.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Room
 * @brief nodes of a room group visibility graph.
 *
 * A Room represents a subset of the interior scene that is
 * defined by a set of infinite planes (the walls). As a result,
 * a Room represents a convex subset of 3-space. Note that this does not
 * mean that every physical room in the interior scene must be convex.
 * Non-convex rooms can be formed as the union of two or more convex rooms,
 * joined by portals.
 * 
 * Rooms contain three components; Walls, Fixtures, and Portals.
 * Walls are instances of ISMWall, and these represent planar walls in the room.
 * Fixtures may be any type of scene graph represent any piece of geometry
 * in the room that is not entirely within the plane of one of the room’s walls.
 * Finally, portals represent ways of seeing out of a room into other rooms.
 *
 * @image html cvxroom.gif
 *
 * Convex rooms are the heart of the drawing process for an interior scene.
 * The rooms implement the recursive traversal of the interior scene graph
 * to correctly draw it back-to front (although the clipping\par to doorways
 * is handled by the portals). The basic algorithm for drawing a convex room
 * is as follows:
 *
 * @code
 *	if the current room is being visited
 *	    then return to calling room
 *	else
 *	  mark the current room as being visited
 *	 for each portal in the room
 *	    test the portal against the current clipping planes
 *	    if the portal is visible
 *	       then push the portal’s clipping plans
 *	       and recursively draw the room
 *	 endfor
 *	 draw the current room’s walls (unsorted)
 *	 draw the current room’s fixtures (sorted)
 *	 mark the current room as not being visited
 *	 return to calling room
 * @endcode
 *
 * @see Portal RoomGroup Wall
 */
class Room : public Model
{
public:
	VX_DECLARE_CLASS(Room);

    // construction and destruction
    Room();

	//! Attach another wall to this room.
    void AttachWall(Wall* pWall);

	//! Detach given wall from room.
    Wall* DetachWall(Wall* pWall);

 	//! Get list of walls for this room.
    const RefArray<Model>& GetWalls() const;

    // Attach another outgoing portal to this room.
    void AttachOutgoingPortal(Portal* pPortal);

	//! Detach given portal from outgoing list.
    Portal* DetachOutgoingPortal(Portal* pPortal);

	//! Get list of outgoing portals (those leaving this room).
    const RefArray<Model>& GetOutgoingPortals() const;

    // Attach another incoming portal to this room.
    void AttachIncomingPortal(Portal* pPortal);

	//! Detach given portal from incoming list.
	Portal* DetachIncomingPortal(Portal* pPortal);

	//! Get list of incoming portals (those entering this room).
    const RefArray<Model>& GetIncomingPortals() const;

	//! Attach another fixture to room.
    void AttachFixture(Model* pFixture);

	//! Detach the fixture model.
    Model* DetachFixture();

    //!< Get hierarchy of fixtures (objects to display in the room).
    Model* GetFixture() const;

    //! Compute world planes of walls from from model planes
    void UpdateWalls(const Matrix* room_mtx);

	//! Determine if room contains the origin.
	bool ContainsOrigin();

	//! Remove matrices from room, walls and portals
 	void RemoveMatrix();

    // overrides
	virtual void	Display (Scene* pScene);
	virtual bool	Copy(const SharedObj* src);
	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger& s, int opcode);
	virtual DebugOut&	Print(DebugOut& deb = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		ROOM_AttachWall = Model::MOD_NextOp,
		ROOM_AttachOutgoing,
		ROOM_AttachIncoming,
		ROOM_AttachFixture,
		ROOM_DetachWall,
		ROOM_DetachOutgoing,
		ROOM_DetachIncoming,
		ROOM_DetachFixture,
		ROOM_NextOp = Model::MOD_NextOp + 10,
	};

protected:
    // repacks the children (order = outgoing portals, walls, fixtures)
    void		UpdateChildArray();

    // incoming portals are used for graph traversal and are not 'drawn'
    RefArray<Model> m_InPortal;
    RefArray<Model> m_OutPortal;	// outgoing portals are 'drawn' if visible
    RefArray<Model> m_Wall;			// walls of the room
    IntArray			m_InPortalActive;
    Ref<Model>		m_Fixture;		// objects in the room
	Box3				m_WallBound;
	bool				m_ComputeNormals;	// force recalculation of wall normals

};

inline Room::Room() : Model() { m_WallBound.Empty(); }

inline const RefArray<Model>& Room::GetWalls() const
{
    return m_Wall;
}

inline const RefArray<Model>& Room::GetOutgoingPortals() const
{
    return m_OutPortal;
}

inline const RefArray<Model>& Room::GetIncomingPortals() const
{
    return m_InPortal;
}

inline Model* Room::GetFixture() const
{
    return m_Fixture;
}

} // end Vixen