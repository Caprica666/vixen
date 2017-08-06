/*!
 * @file vxportal.h
 * @brief Portal which restricts clipping in visibility graph.
 *
 * Portals define openings thru which the viewer can see from
 * one room to another. They restrict culling of objects outside the portal.
 *
 * @author Nola Donato
 * @author algorithm from University of North Carolina Computer Graphics Lab
 * @ingroup vixen
 *
 * @see vxroom.h vxwall.h vxroomgroup.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Portal
 *
 * @brief A portal represents a way of seeing a part of a scene graph  through a convex polygon.
 *
 * This represents the idea of  seeing a scene  through a doorway or window.
 * Any part of the adjoining scene (called the Adjoiner) that cannot be seen from the 
 * eyepoint through the polygonal portal will be culled or clipped out. Note that 
 * Portals are not bidirectional; that is, they represent the ability to see through
 * a door in one direction only. Two portals are required to represent both directions
 * of an open doorway.
 *
 * Portals may be used independently of the RoomGroups,
 * as they may be attached in any scene to represent the ability
 * to see a scene through a hole of some sort. Note that the adjoiner
 * relationship is not a parent-child relationship. A portal is a child
 * of whatever scene graph sees through it, but the adjoiner
 * (the scene graph that is seen through the portal) is not a child
 * of the portal. The adjoiner itself should not have multiple instances
 * (although instances within the adjoiner subtree are allowed). 
 *
 * @image html roomgroup.gif
 *
 *
 * @see Wall RoomGroup Room
 */
class Portal : public Shape
{
public:
    // construction and destruction (points are copied by constructor)
	VX_DECLARE_CLASS(Portal);
	Portal();

    void			SetAdjoiner(Model* pAdjoiner);	//!< Set portal adjoiner.
    Model*			GetAdjoiner() const;			//!< Return portal adjoiner.

	void			SetVertices(VertexArray* verts);			//!< Set portal polygon vertices
	void			SetVertices(const Vec3* vertx, int n);		//!< Set portal polygon vertices
    int				GetNumVtx() const;							//!< Return number of vertices in portal geometry.
    VertexArray*	GetVertices() const;						//!< Return portal polygon vertices.
	void			RemoveMatrix();

    // overrides
	virtual	void		OnBoundsChange() const;
    virtual	void		Display (Scene* pScene);
	virtual	int			Save(Messenger&, int) const;
	virtual	bool		Do(Messenger& s, int opcode);
	virtual	bool		Copy(const SharedObj* src);
	virtual	int			Cull(const Matrix*, Scene*);
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	enum Opcode
	{
		PORTAL_SetVertices = Model::MOD_NextOp,
		PORTAL_SetAdjoiner,
		PORTAL_NextOp = Model::MOD_NextOp + 10,
	};

protected:
    void		PushPlanes(Camera* pCamera);	//!< Give camera extra planes to use for culling.
    void		PopPlanes(Camera* pCamera);	//!<Remove extra cull planes from camera.
	bool		CalcSphere(Sphere*) const;	// calculate bounding sphere
	bool		CalcBound(Box3* b) const;		// calculate bounding box
    void		UpdateWorldVertices (const Matrix& mv);
	void		MakeRenderGeometry();

    // portal geometry (must be a planar convex polygon)
    int32				m_usVertices;
    int32				m_usPlanes;
	Ref<VertexArray>	m_ModelVertex;
    Array<Vec3>			m_pWorldVertex;

    // array of planes defined by the portal geometry and camera location
    Array<Plane>		m_pWorldPlane;

    // portal adjoiner (do not ref-count to avoid ref-cycles)
    Model*				m_pAdjoiner;

    // indicates if portal has already been visited in graph traversal
    bool m_bVisited;
	bool m_ReverseVerts;
	bool m_matrixChanged;
};

inline Model* Portal::GetAdjoiner () const
{
    return m_pAdjoiner;
}

inline int Portal::GetNumVtx () const
{
    return m_usVertices;
}

inline VertexArray* Portal::GetVertices() const
{
    return m_ModelVertex;
}

} // end Vixen