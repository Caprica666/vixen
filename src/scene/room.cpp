#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Room, Model, VX_Room);

static const TCHAR* opnames[] = {
	TEXT("AttachWall"), TEXT("AttachOutgoing"), TEXT("AttachIncoming"), TEXT("AttachFixture"),
	TEXT("DetachWall") , TEXT("DetachOutging"), TEXT("DetachIncoming"), TEXT("DetachFixture") };

const TCHAR** Room::DoNames = opnames;

/*!
 * @fn void Room::AttachWall(Wall* wall)
 * @param wall new wall to attach
 *
 * Attach the given wall to this room.
 *  Will not attach more than one pointer to a given wall at once.
 *
 * @see Room::GetWalls Room
 */
void Room::AttachWall(Wall* pWall)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Room, ROOM_AttachWall) << this << pWall;
	VX_STREAM_END();

    if (m_Wall.Find(pWall) >= 0)
        return;
    m_Wall.Append(pWall);
	m_ComputeNormals = true;
    UpdateChildArray();
}

/*!
 * @fn Wall* Room::DetachWall(Wall* wall)
 * @param wall wall to detach
 *
 * Detaches the given wall from this room.
 * 
 * @return  wall detached, NULL if wall not ofund
 *
 * @see Room::GetWalls Room
 */
Wall* Room::DetachWall(Wall* pWall)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Room, ROOM_DetachWall) << this << pWall;
	VX_STREAM_END();

	int i = m_Wall.Find(pWall);
    if (i < 0)
        return (Wall*) NULL;

    pWall = (Wall*) m_Wall.GetAt(i);
	if (pWall == NULL)
		return NULL;
	VX_ASSERT(pWall->IsClass(VX_Wall));
	m_Wall.RemoveAt(i);
	if (pWall->Remove(Group::UNLINK_NOFREE))
		return pWall;
	return NULL;
}

/*!
 * @fn void Room::AttachOutgoingPortal(Portal* portal)
 * @param portal new portal to attach
 *
 * Attach and detach the given portal as a way of seeing out of the room.
 * Will not attach more that one pointer to a given portal at once.
 * DetachOutgoingPortal returns the sent portal unless it was
 * not found in the room, in which case it returns null.
 *
 * @see Room Room::GetOutgoingPortals Room::AttachIncomingPortal
 */
void Room::AttachOutgoingPortal(Portal* portal)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Room, ROOM_AttachOutgoing) << this << portal;
	VX_STREAM_END();

    if (m_OutPortal.Find(portal) >= 0)
        return;
    m_OutPortal.Append(portal);
    UpdateChildArray();
}

Portal* Room::DetachOutgoingPortal(Portal* p)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Room, ROOM_DetachOutgoing) << this << p;
	VX_STREAM_END();

	int i = m_OutPortal.Find(p);
    if (i < 0)
        return (Portal*) NULL;
    p = (Portal*) m_OutPortal.GetAt(i);
	if (p == NULL)
		return NULL;
	m_OutPortal.RemoveAt(i);
	if (p->Remove(Group::UNLINK_NOFREE))
		return p;
	return NULL;
}

/*!
 * @fn void Room::AttachIncomingPortal(Portal* portal)
 * @param portal new portal to attach
 *
 * Attach and detach the given portal as a way of seeing into the room.
 * Will not attach more that one pointer to a given portal at once.
 * DetachIncomingPortal returns the sent portal unless it was
 * not found in the room, in which case it returns null.
 *
 * The incoming portals for a room are designed as an efficiency enhancement.
 * When a room draws its outgoing portals, it disables its incoming portals
 * before doing so. This works under the assumption that once the view rays
 * leave a room through a portal, they cannot reenter. This is based on
 * the simple geometric observation that if one assumes rooms do not overlap,
 * then a point leaving a convex region in a constant linear direction
 * cannot ever reenter the region. As a result, there is no need to do any
 * processing involving the incoming portals. Any portals added to a rooms
 * incoming portal list will be disabled while that rooms outgoing
 * portals are being drawn.
 *
 * @see Room Room::GetIncomingPortals Room::AttachOutgoingPortal
 */
void Room::AttachIncomingPortal(Portal* pPortal)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Room, ROOM_AttachIncoming) << this << pPortal;
	VX_STREAM_END();

    if (m_InPortal.Find(pPortal) >= 0)
        return;
    m_InPortal.Append(pPortal);
}

Portal* Room::DetachIncomingPortal(Portal* p)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Room, ROOM_DetachIncoming) << this << p;
	VX_STREAM_END();

	int i = m_InPortal.Find(p);
    if (i < 0)
        return (Portal*) NULL;
    p = (Portal*) m_InPortal.GetAt(i);
	if (p == NULL)
		return NULL;
	m_InPortal.RemoveAt(i);
	if (p->Remove(Group::UNLINK_NOFREE))
		return p;
	return NULL;
}

/*!
 * @fn void Room::AttachFixture(Model* mod)
 * @param mod	model to use for fixtures (contents of room)
 *
 * Attach the given model from the interior of the room.
 * Will not attach more than one fixture model.
 *
 * @see Room::GetFixture Room
 */
void Room::AttachFixture(Model* pFixture)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Room, ROOM_AttachFixture) << this << pFixture;
	VX_STREAM_END();

	if (m_Fixture == (const Model*) pFixture)
		return;
	if (m_Fixture != (const Model*) NULL)
		m_Fixture->Remove(UNLINK_FREE);
    m_Fixture = pFixture;
	if (pFixture)
		Append(pFixture);
}

Model* Room::DetachFixture()
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Room, ROOM_DetachFixture) << this;
	VX_STREAM_END();

	Model* fixture = m_Fixture;
	if (fixture != NULL)
	{
		m_Fixture = (Model*) NULL;
		fixture->Remove(Group::UNLINK_NOFREE);
		return fixture;
	}
	return NULL;
}

void Room::UpdateChildArray()
{
    // Repack the children so that the outgoing portals occur first, walls
    // occur second, and fixtures occur third.
	Empty();				// dereference all children

	ObjArray::Iter iter(m_OutPortal);
	Model* mod;
    while (mod = (Model*) iter.Next())
		Append(mod);		// attach outgoing portals first

	Wall* wall;
	Box3 b;
	m_WallBound.Empty();
	iter.Reset(&m_Wall);    // attach walls second
    while (wall = (Wall*) iter.Next())
        Append(wall);
	if (m_Fixture != (const Model*) NULL)
		Append(m_Fixture);    // attach fixtures third
}

/*!
 * @fn void Room::UpdateWalls(const Matrix* mv)
 * @param mv	model/view matrix
 *
 * Traverse the list of walls and tell each one to compute its world
 * plane from its model plane.
 * Guarantees that the wall normals point to the inside of their rooms.
 * Any point inside the room may be used.
 */
void Room::UpdateWalls(const Matrix* mv)
{
	Vec3 roomctr = m_WallBound.Center();
	ObjArray::Iter witer(m_Wall);
	Wall* pWall;

	while (pWall =(Wall*) witer.Next())
	{
		if (m_ComputeNormals)
			pWall->ComputeCorrectNormal(roomctr);
		pWall->UpdateWorldPlane(mv);
    }
	m_ComputeNormals = false;
}

/*!
 * @fn bool Room::ContainsOrigin
 *
 * The room is assumed to be convex.  Traverse the list of walls to determine
 * on which side of each wall is the origin.  If the origin is on the
 * 'positive' side of all walls, then it is in the room.
 * Requires that the walls world/ planes are current.
 * (The scene manager puts the camera at the origin and
 * transforms all geometry to camera coordinate system.)
 */
bool Room::ContainsOrigin()
{

	ObjArray::Iter witer(m_Wall);
	Wall* pWall;
	while (pWall =(Wall*) witer.Next())
    {
        if (pWall->GetWorldPlane().w < 0.0f)
		{
			VX_TRACE(Wall::Debug, ("Room::ContainsOrigin[%s] outside wall %s)", GetName(), pWall->GetName()));
            return false;
		}
		VX_TRACE(Wall::Debug, ("Room::ContainsOrigin[%s] inside wall %s)", GetName(), pWall->GetName()));
    }
    return true;
}

void Room::RemoveMatrix()
{
	const Matrix*			roommtx = GetTransform();
	GroupIter<Model>	iter(this, Group::CHILDREN);
	Model*				child;
	Box3					b;

	VX_TRACE(Wall::Debug, ("Room::RemoveMatrix %s)", GetName()));
	while (child = iter.Next())
	{
		const Matrix* childmtx = child->GetTransform();
		Matrix mtx(*roommtx);

		if (!childmtx->IsIdentity())
			mtx.PostMul(*childmtx);
		child->SetTransform(&mtx);
		if (child->IsClass(VX_Wall))
		{
			((Wall*) child)->RemoveMatrix();
			if (child->GetBound(&b, NONE))
				m_WallBound.Extend(b);
		}
//		if (child->IsClass(VX_Portal))
//			((Portal*) child)->RemoveMatrix();
	}
	SetTransform((Matrix*) NULL);
}

void Room::Display(Scene* pScene)
{
    // Once you are in a room to display it, further portal graph traversal
    // will not contain a loop that exits current room then enters it again.
    // This is a consequence of having convex rooms.  Turn off all incoming
    // portals to prevent the graph traversal from reentering the room.
	ObjArray::Iter iter(m_InPortal);
	Portal* portal;
    int i = 0;
    while (portal = (Portal*) iter.Next())
    {
		m_InPortalActive.SetAt(i, portal->IsActive());	// save portal's active state
		portal->SetFlags(INACTIVE);					// turn off incoming portal
 		VX_TRACE(Portal::Debug, ("Portal::Disable %s", portal->GetName()));
	   i++;
    }

    // Draw the portal system.  The outgoing portals are processed first and
    // result in traversing to the 'last' visible portal from the camera.
    // The corresponding room adjoining the portal has its walls drawn first,
    // its fixtures drawn last.
	VX_TRACE(Portal::Debug, ("Room::Display %s", GetName()));
    Model::Display(pScene);

    iter.Reset();
    i = 0;
    while (portal = (Portal*) iter.Next())	// restore the portals' active states
    {
		if (m_InPortalActive.GetAt(i))			// restore incoming portal
			portal->ClearFlags(INACTIVE);
  		VX_TRACE(Portal::Debug, ("Portal::Enable %s", portal->GetName()));
       i++;
    }
}

/****
 *
 * class Room override for SharedObj::Do
 *
 ****/
bool Room::Do(Messenger& s, int op)
{
	SharedObj*		obj;
	Wall*		pWall;
	Model*	pFixture;
	Portal*	pPortal;
	
	switch (op)
	{
		case ROOM_AttachWall:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Wall));
		pWall = (Wall*) obj;
		AttachWall(pWall);
		break;

		case ROOM_AttachOutgoing:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Portal));
		pPortal = (Portal*) obj;
		AttachOutgoingPortal(pPortal);
		break;

		case ROOM_AttachIncoming:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Portal));
		pPortal = (Portal*) obj;
		AttachIncomingPortal(pPortal);
		break;

		case ROOM_AttachFixture:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Model));
		pFixture = (Model*) obj;
		AttachFixture(pFixture);
		break;

		case ROOM_DetachOutgoing:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Portal));
		pPortal = (Portal*) obj;
		DetachOutgoingPortal(pPortal);
		break;

		case ROOM_DetachIncoming:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Portal));
		pPortal = (Portal*) obj;
		DetachIncomingPortal(pPortal);
		break;

		case ROOM_DetachFixture:
		DetachFixture();
		break;

		default:
		return Model::Do(s, op);
	}
	return true;
}

/****
 *
 * class Room override for SharedObj::Save
 *
 ****/
int Room::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	ObjArray::Iter iter(m_InPortal);
	Portal* portal;
	Wall* wall;

    while (portal = (Portal*) iter.Next())
    {
        if ((portal->Save(s, opts) >= 0) && h)
		{
			s << OP(VX_Room, ROOM_AttachIncoming) << h << portal;
			s << OP(VX_Portal, Portal::PORTAL_SetAdjoiner) << portal << h;
		}
    }
	if (h <= 0)
		return h;
	iter.Reset(&m_OutPortal);
    while (portal = (Portal*) iter.Next())
		s << OP(VX_Room, ROOM_AttachOutgoing) << h << portal;
	iter.Reset(&m_Wall);
    while (wall = (Wall*) iter.Next())
		s << OP(VX_Room, ROOM_AttachWall) << h << wall;
	if (m_Fixture != (const Model*) NULL)
		s << OP(VX_Room, ROOM_AttachFixture) << h << (const Model*) m_Fixture;
	return h;
}

/****
 *
 * class Room override for SharedObj::Copy
 *
 ****/
bool	Room::Copy(const SharedObj* src_obj)
{
	const Room*	src = (const Room*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Model::Copy(src_obj))
	  return false;
	if (src_obj->IsClass(VX_Room))
	{
		m_Wall = src->m_Wall;
		m_Fixture = src->m_Fixture;
		m_InPortal = src->m_InPortal;
		m_OutPortal = src->m_OutPortal;
	}
	return true;
}

/****
 *
 * class Room override for SharedObj::Print
 *
 ****/
DebugOut&	Room::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Model::Print(dbg, opts & ~PRINT_Trailer);
	ObjArray::Iter iter(m_InPortal);
	SharedObj* obj;

	endl(dbg << "\t<attr name='incoming'>");
    while (obj = iter.Next())
		obj->Print(dbg, PRINT_Summary);
	endl(dbg << "</attr>");
	iter.Reset(&m_OutPortal);
	endl(dbg << "\t<attr name='outgoing'>");
    while (obj = iter.Next())
		obj->Print(dbg, PRINT_Summary);
	endl(dbg << "</attr>");
	iter.Reset(&m_Wall);
	endl(dbg << "\t<attr name='wall'>");
    while (obj = iter.Next())
		obj->Print(dbg, PRINT_Summary);
	endl(dbg << "</attr>");
	Model::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

}	// end Vixen