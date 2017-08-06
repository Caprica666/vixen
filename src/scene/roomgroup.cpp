#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(RoomGroup, Model, VX_RoomGroup);

/****
 *
 * Debug = 0	disables all debug printout for roomgroups
 * Debug = 1	prints which room you are in each frame
 *
 ****/

/*!
 * @fn void RoomGroup::AttachShell(Model* shell)
 * @param shell model to add as shell
 *
 * The  shell object represents the exterior of the set of rooms 
 * to be drawn if the camera is not contained in any of the rooms in this group.
 *
 * @see RoomGroup::DetachShell RoomGroup::GetShell
 */
void RoomGroup::AttachShell(Model* pShell)
{
	if (!m_Shell.IsNull() && ((Model*) m_Shell != pShell))
		m_Shell->Remove();
	m_Shell = pShell;
	if (pShell && (pShell->Parent() != this))
		PutFirst(pShell);
}

Model* RoomGroup::DetachShell()
{
	Model* shell = m_Shell;
	m_Shell = (Model*) NULL;
    if (shell->Remove(Group::UNLINK_NOFREE))
		return shell;
	return NULL;
}

/*!
 * @fn void RoomGroup::AttachRoom(Room* room)
 * @param room new room to attach
 *
 * Attaches the given room to the room group.
 * AttachRoom will not reattach a room that is already in the group.
 *
 * @see RoomGroup::DetachRoom Room
 */
void RoomGroup::AttachRoom(Room* room)
{
	if (m_Rooms.Find(room) >= 0)
		return;
	m_Rooms.Append(room);
	if (room->Parent() != this)
		Append(room);
}

Room* RoomGroup::DetachRoom(Room* room)
{
	int i = m_Rooms.Find(room);
	if (i < 0)
		return NULL;
	m_Rooms.RemoveAt(i);
	if (room->Remove(Group::UNLINK_NOFREE))
		return room;
	return NULL;
}

/*!
 * @fn void RoomGroup::WhichRoom(const Matrix* mv, Scene* scene)
 *
 * Sets the last room to a room in the room group that contains the origin.
 * If more than one room in the room group contains the origin,
 * then the library does not guarantee any particular method for choosing
 * between them. The second version of the function will always check
 * the given room first if the origin is contained in the given room, 
 * then that room is returned; otherwise all rooms in the room
 * group are checked until an enclosing room is found. 
 *
 *@see RoomGroup::GetLastRoom RoomGroup::SetLastRoom
 */
void RoomGroup::WhichRoom(const Matrix* mv, Scene* scene)
{
    // The camera in its own coordinate system is located at the origin.
    // Room walls must be updated first by the scene matrix so that the
    // point location routines have access to the current world planes.
    // The point to test will always be the origin.
	Camera*	cam = scene->GetCamera();
	Sphere	bound;
	bool		MatrixChanged = (*mv != m_lastMatrix);

	m_lastMatrix.Copy(mv);
    if (!m_LastRoom.IsNull())
    {
		m_LastRoom->GetBound(&bound, NONE);
		bound *= *mv;						// transform by input matrix
        if (MatrixChanged)
			m_LastRoom->UpdateWalls(mv);
        if (cam->IsVisible(bound) && m_LastRoom->ContainsOrigin())
			return;
   }
    // Camera is not in the same room it was last frame.  Search for the
    // room which contains it.  (OPTIMIZATION:  Maintain a graph of rooms
    // rather than a list.  The search for the room containing the camera
    // should be a breadth-first traversal of the graph.)
	ObjArray::Iter iter(m_Rooms);
	Room* last = m_LastRoom;
	Room* r;
    while (r = (Room*) iter.Next())
    {
		m_LastRoom = r;
        if (r->IsActive())
        {
			r->GetBound(&bound, NONE);
			bound *= *mv;					// transform by input matrix
            if (MatrixChanged)
				r->UpdateWalls(mv);
            if (cam->IsVisible(bound) && r->ContainsOrigin())
			{
				if (last != r)
					LogEvent(r, last, cam);
                return;
			}
        }
    }
    m_LastRoom = (Room*) NULL;
	LogEvent(NULL, last, cam);
}

void RoomGroup::LogEvent(Room* enter, Room* exit, Camera* cam)
{
	TriggerEvent*	event;
	const TCHAR*	name = GetName();

	if (name == NULL)
		name = TEXT("UNKNOWN");
	if (exit)
	{
		if (IsSet(SharedObj::DOEVENTS) || exit->IsSet(SharedObj::DOEVENTS))
		{
			event = new TriggerEvent(Event::LEAVE);
			event->Sender = exit;
			event->Target = exit;
			event->Collider = cam;
			event->CollidePos = Vec3(0,0,0);
			event->Log();
		}
		VX_TRACE(RoomGroup::Debug, ("RoomGroup::WhichRoom(%s) exiting room %s", name, exit->GetName()));
	}
	if (enter)
	{
		if (IsSet(SharedObj::DOEVENTS) || enter->IsSet(SharedObj::DOEVENTS))
		{
			event = new TriggerEvent(Event::ENTER);
			event->Sender = enter;
			event->Target = enter;
			event->Collider = cam;
			event->CollidePos = Vec3(0,0,0);
			event->Log();
		}
		VX_TRACE(RoomGroup::Debug, ("RoomGroup::WhichRoom(%s) entering room %s", name, enter->GetName()));
	}
}

void RoomGroup::Display (Scene* scene)
{
    if (!IsActive())				// don't display if not active
		return;

	Matrix*	mtx = scene->GetWorldMatrix();
	Matrix	save_mtx(*mtx);

	if (HasChanged())
		RemoveMatrix();
	CalcMatrix(mtx, scene);
    WhichRoom(mtx, scene);			// determine which room contains the camera
    if (!m_LastRoom.IsNull())		// start the portal system from this interior room
        m_LastRoom->Display(scene);
    else if (!m_Shell.IsNull())
        m_Shell->Display(scene);	// draw the exterior representation of the room group
	mtx->Copy(save_mtx);
}

/****
 *
 * class RoomGroup override for SharedObj::Do
 *
 ****/
bool RoomGroup::Do(Messenger& s, int op)
{
	SharedObj*		obj;
	Model*	pShell;
	Room*		pRoom;
	
	switch (op)
	{
		case ROOMGROUP_AttachShell:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Model));
		pShell = (Model*) obj;
		AttachShell(pShell);
		break;

		case ROOMGROUP_AttachRoom:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Room));
		pRoom = (Room*) obj;
		AttachRoom(pRoom);
		break;

		default:
		return Model::Do(s, op);
	}
	return true;
}

/****
 *
 * class RoomGroup override for SharedObj::Save
 *
 ****/
int RoomGroup::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	if (h <= 0)
		return h;
	if (!m_Shell.IsNull())
		s << OP(VX_RoomGroup, ROOMGROUP_AttachShell) << h << (Group *) m_Shell;

	ObjArray::Iter iter(m_Rooms);
	Room* room;
	while (room = (Room*) iter.Next())
		s << OP(VX_RoomGroup, ROOMGROUP_AttachRoom) << h << room;
	return h;
}

void RoomGroup::RemoveMatrix()
{
	GroupIter<Model> iter(this, Group::CHILDREN);
	Model* room;
	while (room = (Model*) iter.Next())
	{
		Matrix roommtx(*GetTransform());
		roommtx.PostMul(*(room->GetTransform()));
		room->SetTransform(&roommtx);
		if (room->IsClass(VX_Room))
			((Room*) room)->RemoveMatrix();
	}
	SetTransform((Matrix*) NULL);
	SetChanged(false);
}

/****
 *
 * class RoomGroup override for SharedObj::Print
 *
 ****/
DebugOut&	RoomGroup::Print(DebugOut& dbg, int opts) const
{
	return Model::Print(dbg, opts);
}

}	// end Vixen