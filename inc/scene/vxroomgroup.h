/*!
 * @file vxroomgroup.h
 * @brief Set of connected rooms with visibility information.
 *
 * Rooms are displayed based on their visibility from the current viewpoint.
 *
 * @author Nola Donato
 * @author algorithm from University of North Carolina Computer Graphics Lab
 * @ingroup vixen
 *
 * @see vxportal.h vxwall.h vxroom.h
 */
#pragma once

namespace Vixen {

/*!
 * @class RoomGroup
 * @brief represents a set of connected rooms connected by visibility portals.
 *
 * The RoomGroup serves a gatekeeper to the set of rooms, as drawing a room
 * group causes the currently visible parts of the set of rooms to be drawn.
 * The room group also automatically draws the shell object that represents
 * the exterior of the set of rooms (if such an exterior exists) if the camera
 * is not contained in any of the rooms in the group.
 *
 * A room group is really a visibility graph giving a high-level representation
 * of an interior scene. This visibility graph describes the topology
 * of the rooms within the scene. If drawn as a graph, nodes of the graph
 * are the rooms and the edges (directed) are the portals. 
 * This represents the idea that the only way of seeing out of a room
 * (and into another) is through a doorway, window, or opening in the wall
 * of the room. 
 *
 * The room group manages the rooms in its interior scene,
 * and directs the rendering of the rooms. Its main purpose is to start 
 * the rendering of the graph of rooms from the correct room. In this case,
 * the correct room is the room (if any) that contains the current camera location.
 * When a room group is drawn, it determines the room (if any) that contains 
 * the camera, and recursively draws that room. If no room contains the
 * camera’s location, then the room group assumes that the camera is currently
 * located outside the interior scene, and draws the shell or exterior
 * representation of the scene, if one is supplied.
 *
 * @see Wall Room Portal
 */
class RoomGroup : public Model
{
public:
	VX_DECLARE_CLASS(RoomGroup);
    RoomGroup();									//!< Make empty room group.

    // representation of room group when viewed from the outside
    void			AttachShell(Model* shell);	//!< attach a model as the shell object.
    Model*		DetachShell();					//!< detach the shell model.
    const Model*	GetShell() const;				//!< return shell model.

    // rooms (viewed from inside)
    void			AttachRoom(Room* room);		//!< Attach a room to this group.
    Room*			DetachRoom(Room* room);		//!< Detach a room from this group.
    void			SetLastRoom(Room* room);		//!< Set last room which contains the camera.
    Room*			GetLastRoom() const;			//!< Get last room which contains the camera.
    const RefArray<Model>& GetRooms() const;	//!< Return list of rooms.

    //! Determine which room contains the camera.
    void			WhichRoom(const Matrix* mv, Scene* scene);

	void			RemoveMatrix();					// remove matrices from portal framework

    // overrides
    virtual void	Display (Scene* pScene);
	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger& s, int opcode);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		ROOMGROUP_AttachShell = Model::MOD_NextOp,
		ROOMGROUP_AttachRoom,
		ROOMGROUP_NextOp = Model::MOD_NextOp + 10,
	};

protected:
    // repacks the children (order = shell, rooms)
    void UpdateChildArray();

	void LogEvent(Room* enter, Room* exit, Camera* cam);

    Ref<Model>		m_Shell;		// outside representation of room group
    RefArray<Model>	m_Rooms;		// list of managed rooms
    Ref<Room>			m_LastRoom;		// last room containing the camera
	Matrix			m_lastMatrix;
};

inline RoomGroup::RoomGroup() : Model() { SetChanged(true); }


inline const Model* RoomGroup::GetShell () const
{
    return m_Shell;
}

inline const RefArray<Model>& RoomGroup::GetRooms() const
{
    return m_Rooms;
}


/*!
 * @fn Room* RoomGroup::GetLastRoom() const
 *
 * Returns the room containing the camera center at the time of the last Click
 * that drew the room group. This function returns null if the last camera
 * drawing the room group was found to be outside of all of the rooms.
 * This will be the first room to be checked against the camera position
 * the next time the room group is drawn.
 *
 * @see RoomGroup::SetLastRoom RoomGroup::WhichRoom
 */
inline Room* RoomGroup::GetLastRoom() const
{
    return m_LastRoom;
}

/*!
 * @fn void* RoomGroup::SetLastRoom(Room* room)
 *
 * SetLastRoom allows the application to set the last room pointer manually.
 * This can increase performance if the application has prior knowledge of
 * the room that contains the camera. The portals system will set the last
 * room pointer for the next click at the time of the previous click,
 * allowing applications to overwrite this value (if desired) at any time
 * between the two clicks.
 *
 * @see RoomGroup::GetLastRoom RoomGroup::WhichRoom
 */
inline void RoomGroup::SetLastRoom(Room* room)
{
    m_LastRoom = room;
}

} // end Vixen