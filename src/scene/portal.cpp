#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Portal, Shape, VX_Portal);

/****
 *
 * Debug = 0 disables all debug printout
 * Debug = 1 prints messages when a portal is enabled, disabled, pushed or popped
 * Debug = 2 creates and displays geometry for portals so you can see them in the scene\
 *			 and inhibits any portal culling
 *
 ****/

/*!
 * @fn Portal::Portal
 *
 * Default constructor makes an empty but active portal with no adjoiner.
 */
Portal::Portal() : Shape()
{
    m_usVertices = 0;
    m_usPlanes = 0;
    m_pAdjoiner = NULL;
    m_bVisited = false;
	m_ReverseVerts = false;
	m_matrixChanged = true;
}

/*!
 * @fn void Portal::SetVertices(VertexArray* verts)
 *
 * Sets the vertices for the portal polygon.
 * The portal polygon must be counterclockwise as veiwed
 * from the front and must be a convex polygon.
 * Portal polygons should not be degenerate - that is,
 * no two verticess should be equal, and no three vertices should be collinear.
 *
 * @see Portal Portal::GetVertices
 */
void Portal::SetVertices(VertexArray* verts)
{
	if (verts == NULL)
	{
		m_usVertices = m_usPlanes = 0;
		m_ModelVertex = (VertexArray*) NULL;
		return;
	}

    m_usVertices = verts->GetNumVtx();
    m_usPlanes = m_usVertices;
	m_ModelVertex = verts;

    // copy the input vertices
    m_pWorldVertex.SetSize(m_usVertices);
    m_pWorldPlane.SetSize(m_usPlanes);
	VertexPool::ConstIter iter(*verts);
	int i = 0;
	while (iter.Next())
        m_pWorldVertex.SetAt(i++, *iter.GetLoc());
    // Use the model bounding sphere of the portal polygon for culling.
    // (Portals should not have children.)
	m_matrixChanged = true;
	SetGeometry(NULL);
	NotifyParents(MOD_BVinvalid);
}

void Portal::SetVertices(const Vec3* verts, int n)
{
    m_usVertices = n;
    m_usPlanes = n;

    // copy the input vertices
    m_ModelVertex = new VertexArray(0, n);
    m_pWorldVertex.SetSize(n);
    m_pWorldPlane.SetSize(m_usPlanes);
	VertexPool::Iter iter(m_ModelVertex);
	int i = 0;
	while (iter.Next())
    {
        *iter.GetLoc() = verts[i];
        m_pWorldVertex.SetAt(i, verts[i]);
		++i;
    }
	m_matrixChanged = true;
	SetGeometry(NULL);
	NotifyParents(MOD_BVinvalid);
}

/*!
 * @fn  void Portal::SetAdjoiner (Model* pAdjoiner)
 *
 * The adjoiner object is the root of the scene graph
 * that is to be seen through the portal.
 * This is not a parent-child relationship,
 * and thus no transforms are inherited by the adjoiner from the portal.
 *
 * @see Portal::SetVertices
 */
 void Portal::SetAdjoiner (Model* pAdjoiner)
{
    m_pAdjoiner = pAdjoiner;
}


/****
 *
 * class Portal override for Model::CalcBound
 * Calculate the bounding box of the geometry represented by the portal.
 *
 ****/
bool Portal::CalcBound(Box3* b) const
{
	if (m_ModelVertex.IsNull())
		return false;
	return m_ModelVertex->GetBound(b);
}

bool Portal::CalcSphere(Sphere* sp) const
{
    float fRadiusSqr = 0.0f;
	Box3 bound;

	if (m_ModelVertex.IsNull() || !CalcBound(&bound))
		return false;

    // radius is maximum distance from center to vertices
	VertexPool::ConstIter iter(m_ModelVertex);
	while (iter.Next())
	{
        Vec3 diff = *iter.GetLoc() - bound.Center();
        float fLengthSqr = diff.Dot(diff);
        if ( fLengthSqr > fRadiusSqr )
            fRadiusSqr = fLengthSqr;
    }
	sp->Set(bound.Center(), (float) sqrt(fRadiusSqr));
	return true;
}

void Portal::OnBoundsChange() const
{
	Portal* cheat = (Portal*) this;
	cheat->m_matrixChanged = true;
}

void Portal::UpdateWorldVertices(const Matrix& mv)
{
    // Portal is assumed not to be transformed with respect to its parent,
    // so only the parent's transform is applied to get the world portal
    // polygon from the model one.
    
	if (m_ModelVertex.IsNull() || (m_usVertices < 3))
		return;
	VertexPool::ConstIter iter(*m_ModelVertex);
	int i = 0;
	while (iter.Next())
	{
		mv.Transform(*iter.GetLoc(), m_pWorldVertex[i++]);
	}
//
// If the portal face normal does not point into the room, reverse
// the order of the vertices.
//
 	if (m_matrixChanged)
	{
		Vec3	room_ctr;
		Vec3	edge1(m_pWorldVertex[1] -  m_pWorldVertex[0]);
		Vec3	edge2(m_pWorldVertex[2] -  m_pWorldVertex[1]);
		Vec3	normal = edge1.Cross(edge2);

		normal.Normalize();
		mv.Transform(Parent()->GetCenter(LOCAL), room_ctr);
		Plane plane(normal.x, normal.y, normal.z, -normal.Dot(m_pWorldVertex[0]));
		VX_TRACE(Debug, ("Portal::Update %s  plane = { %f %f %f %f }", GetName(), plane.x, plane.y, plane.z, plane.w));
		m_ReverseVerts = plane.Distance(room_ctr) < 0;
	}
	m_matrixChanged = false;
}

void Portal::PushPlanes(Camera* pCamera)
{
    // World planes are in the camera coordinate system, so the camera
    // location is the origin.  Compute the planes of the frustum formed by
    // the origin and each edge of the portal polygon.

    Vec3 normal, portalEdge;
    unsigned short i;
	if (m_usVertices < 3)
		return;
	for (i = 0; i < m_usVertices - 1; i++)
	{
		if (m_ReverseVerts)
			portalEdge = m_pWorldVertex[i] - m_pWorldVertex[i+1];
		else
			portalEdge = m_pWorldVertex[i+1] - m_pWorldVertex[i];
		normal = m_pWorldVertex[i].Cross(portalEdge);
		if ((normal.x != 0) || (normal.y != 0) || (normal.z != 0))
			normal.Normalize();
		m_pWorldPlane[i].x = normal.x;
		m_pWorldPlane[i].y = normal.y;
		m_pWorldPlane[i].z = normal.z;
		m_pWorldPlane[i].w = 0.0f;
        pCamera->PushClipPlane(m_pWorldPlane[i]);
    }

    // compute plane for portal edge between vertices m_usVertices-1 and 0
	if (m_ReverseVerts)
		portalEdge = m_pWorldVertex[i] - m_pWorldVertex[0];
	else
		portalEdge = m_pWorldVertex[0] - m_pWorldVertex[i];
    normal = m_pWorldVertex[i].Cross(portalEdge);
    normal.Normalize();
    m_pWorldPlane[i].x = normal.x;
    m_pWorldPlane[i].y = normal.y;
    m_pWorldPlane[i].z = normal.z;
	m_pWorldPlane[i].w = 0.0f;
    pCamera->PushClipPlane(m_pWorldPlane[i]);
}

void Portal::PopPlanes(Camera* pCamera)
{
    for (unsigned short i = 0; i < m_usPlanes; i++)
        pCamera->PopClipPlane();
}

/*!
 * @fn void Portal::RemoveMatrix()
 * Applies the local matrix for the portal model to the
 * vertices of the portal polygon. This is NOT the concatenated
 * matrix from the root of the scene graph - just the local matrix.
 * Sets the local portal matrix to identity.
 *
 * The roomgroup and room matrices are  never applied to portals
 * because they may be displayed after a sequence of rooms.
 * Instead, the Room::RemoveMatrix is called beforehand to apply
 * the room matrix to the geometry for the walls and fixtures of the room
 * (but not the portals polygon vertices). The roomgroup and room matrices
 * are instead concatenated into the local matrix of the portal.
 *
 * This routine multiplies the vertices of the portal polygon by the local matrix.
 * It must be called from the room in which the portal is first displayed.
 * It will cause the world coordinates of the portal polygon to be
 * recalculated when the portal is next displayed.
 * 
 * @see Portal::Display
 */
void Portal::RemoveMatrix()
{
	const Matrix* mtx = GetTransform();
	VertexArray* verts = GetVertices();

	if (mtx->IsIdentity())
		return;
	*verts *= *mtx;						// apply to vertices
	VX_TRACE(Portal::Debug, ("Portal::RemoveMatrix %s)", GetName()));
	SetTransform((Matrix*) NULL);		// remove portal matrix
	NotifyParents(MOD_BVinvalid);
	m_matrixChanged = true;
}


/*!
 * @fn void Portal::Display(Scene* scene)
 * Called during scene graph traversal to "display" the portal.
 * This routine will be invoked  after a room is displayed.
 * If this portal is active, it is an outgoing portal of the
 * most recently displayed room. This routine adds the clip
 * planes of the portal polygon to the current camera,
 * causing the objects outside this portal to be culled.
 * It also displays the adjoining room of this portal.
 *
 * Note that the roomgroup and room matrices are not applied
 * to portals. Before any portal is displayed, Room::RemoveMatrix
 * is called to apply the room matrix to the walls and fixtures
 * (but not to portal polgons). This is because a portal is displayed after a
 * sequence of adjoining rooms have already been displayed.
 * It is this concatenated matrix that must be used to transform
 * the portal polygon to world coordinates.
 *
 * @see Room::Display Portal::RemoveMatrix
 */
void Portal::Display(Scene* pScene)
{
	const Matrix*	room_mtx = pScene->GetWorldMatrix();

	if (m_bVisited || !IsActive())
		return;
	if (!GetTransform()->IsIdentity())	// if portal has a matrix
		RemoveMatrix();
#ifdef _DEBUG 		
	if (Portal::Debug > 1)			// don't cull or apply in debug mode
	{
		if (GetGeometry() == NULL)
			MakeRenderGeometry();		// make mesh from portal verts
		Shape::Render(pScene);		// display portal polygon
  		VX_TRACE(Debug, ("Portal::Push %s", GetName()));
		if (m_pAdjoiner)
			m_pAdjoiner->Display(pScene);
		VX_TRACE(Debug, ("Portal::Pop %s", GetName()));
		return;
	}
#endif
    m_bVisited = true;
	//
	// If portal is culled, this usually means it is not visible
	// But if it is hither clipped but still in front of the camera
	// we can see thru it and should still display the adjoiner

	UpdateWorldVertices(*room_mtx);
	if (Cull(room_mtx, pScene) != DISPLAY_NONE)
    {
       // ensure world portal polygon is current
        // traverse the portal graph and draw adjoiners
        Camera* pCamera = pScene->GetCamera();
		PushPlanes(pCamera);
  		VX_TRACE(Debug, ("Portal::Push %s", GetName()));
      
        //*** If you implement enable/disable of cull planes, then the
        //*** following pseudocode needs to be implemented.
        //
        // bool bNearOff = false, bFarOff = false;
        //
        // if ( near plane is disabled )
        // {
        //     bNearOff = true;
        //     enable near plane;
        // }
        // if ( far plane is disabled )
        // {
        //     bFarOff = true;
        //     enable far plane;
        // }
        //***
        //*** End pseudocode

        if (m_pAdjoiner)
		{
			Room* room = (Room*) m_pAdjoiner;
			VX_ASSERT(room->IsClass(VX_Room));
			room->UpdateWalls(room_mtx);
            m_pAdjoiner->Display(pScene);
		}

        //*** If you implement enable/disable of cull planes, then the
        //*** following pseudocode needs to be implemented.
        //
        // if ( bNearOff )
        // {
        //     disable near plane;
        // }
        // if ( bFarOff )
        // {
        //     disable far plane;
        // }
        //***
        //*** End pseudocode
        
		VX_TRACE(Debug, ("Portal::Pop %s", GetName()));
		PopPlanes(pCamera);
    }
	else if (m_pAdjoiner)
	{
		Room* room = (Room*) m_pAdjoiner;
		VX_ASSERT(room->IsClass(VX_Room));
		room->UpdateWalls(room_mtx);
		if (room->ContainsOrigin())
			room->Display(pScene);
	}
	 m_bVisited = false;
}


/****
 *
 * class Portal override for SharedObj::Do
 *
 ****/
bool Portal::Do(Messenger& s, int op)
{
	Vec3*		pVertex = NULL;
	SharedObj*		obj;
	Model*	pAdjoiner;
	
	switch (op)
	{
		case PORTAL_SetVertices:
		s >> obj;
		VX_ASSERT((obj == NULL) || obj->IsClass(VX_VtxArray));
		SetVertices((VertexArray*) obj);
		break;

		case PORTAL_SetAdjoiner:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Model));
		pAdjoiner = (Model*) obj;
		SetAdjoiner(pAdjoiner);
		break;

		default:
		return Shape::Do(s, op);
	}
	return true;
}

/****
 *
 * class Portal override for SharedObj::Save
 *
 ****/
int Portal::Save(Messenger& s, int opts) const
{
	int32 h = Model::Save(s, opts);
	if (h < 0)
		return h;
	if (!m_ModelVertex.IsNull() && (m_ModelVertex->Save(s, opts) >= 0) && h)
		s << OP(VX_Portal, PORTAL_SetVertices) << h << (const VertexArray*) m_ModelVertex;
	return h;
}

/****
 *
 * class Portal override for SharedObj::Copy
 *
 ****/
bool	Portal::Copy(const SharedObj* src_obj)
{
	const Portal*	src = (const Portal*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Shape::Copy(src_obj))
	  return false;
	if (src_obj->IsClass(VX_Portal))
	{
		m_ReverseVerts = src->m_ReverseVerts;
		m_usVertices = src->m_usVertices;
		m_ModelVertex = src->m_ModelVertex;
		m_pWorldVertex.Copy(&(src->m_pWorldVertex));
		m_pWorldPlane.Copy(&(src->m_pWorldPlane));
		m_pAdjoiner = src->m_pAdjoiner;
		m_bVisited = false;
		m_matrixChanged = true;
	}
	return true;
}

/****
 *
 * class Portal override for SharedObj::Print
 *
 ****/
DebugOut&	Portal::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Model::Print(dbg, opts & ~PRINT_Trailer);
	if (m_pAdjoiner)
		m_pAdjoiner->Print(dbg, PRINT_Summary);
	if (opts & PRINT_Data)
	{
		endl(dbg << "<attr name='Vertices'>");
			for (int i = 0; i < m_usVertices; ++i)
				endl(dbg << "\t" << *(((Vec3*) m_pWorldVertex.GetData()) + i));
		endl(dbg << "</attr>");
	}
	Model::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

void Portal::MakeRenderGeometry()
{
	TriMesh* mesh = new TriMesh(VertexPool::LOCATIONS, m_usVertices);

	if (GetGeometry() || m_ModelVertex.IsNull())
		return;
	mesh = new TriMesh(VertexPool::LOCATIONS, m_usVertices);
	SetGeometry(mesh);
	mesh->SetVertices(m_ModelVertex);
}

/*
 * @fn Model* Portal::Cull(const Matrix* trans, Scene* scene)
 * If the portal is not visible (culled), it does not restrict visibility.
 * We must test against the entire camera view volume, not the cull volume,
 * so that portals will work correctly on tiled displays.
 *
 * @see Camera::SetCullVol Camera::SetViewVol
 */
int Portal::Cull(const Matrix* trans, Scene* scene)
{
	Sphere	bound = m_BoundVol;			// bounding sphere
	Plane		planes[6];
	uint32		vis = 0;

	if (bound.Radius <= 0.0f)				// empty bounds?
		return DISPLAY_NONE;
	if (trans)								// in camera coordinates
		bound *= *trans;
	Camera::CalcViewPlanes(scene->GetCamera()->GetViewVol(), planes);
	for (int i = 0; i < 4; i++)
	{
		float dist = planes[i].Distance(bound.Center);
		if (dist >= bound.Radius)			// cull the portal
			return DISPLAY_NONE;
		if ((dist < 0) &&
			(dist <= bound.Radius))
			vis = (vis << 1) | 1;
	}
	return DISPLAY_ALL;						// do not cull
}

}	// end Vixen