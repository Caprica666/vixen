#include "vixen.h"

namespace Vixen {

static const TCHAR*	opnames[] = {
	TEXT("MakeNormals"),
};

const TCHAR** TriMesh::DoNames = opnames;

VX_IMPLEMENT_CLASSID(TriMesh, Mesh, VX_TriMesh);


/*!
 * @fn TriMesh::TriMesh(int style, int nvtx)
 * @param style			determines which vertex components are present
 *						(one or more of VertexPool::NORMALS, VertexPool::COLORS, VertexPool::TEXCOORDS)
 * @param nvtx			Initially make room for this number of vertices. The 
 *						vertex area is dynamically enlarged as necessary.
 *
 * Allocates and initializes a triangle list with room
 * for the given number of vertices. The copy constructor
 * shares the vertices and indices of the source.
 *
 * @see VertexArray::AddVertices Mesh
 */
TriMesh::TriMesh(int style, intptr nvtx)
	: Mesh(style, nvtx)
{
	if (nvtx)
		SetMaxVtx(nvtx);
}

/*!
 * @fn TriMesh::TriMesh(const TCHAR* layout_desc, int nvtx)
 * @param layout_desc	string describing the layout of vertices
 * @param nvtx			Initially make room for this number of vertices. The 
 *						vertex area is dynamically enlarged as necessary.
 *
 * Allocates and initializes a triangle list with room
 * for the given number of vertices. The copy constructor
 * shares the vertices and indices of the source.
 *
 * @see VertexArray::AddVertices Mesh VertexPool::FindLayout VertexLayout
 */
TriMesh::TriMesh(const TCHAR* layout_desc, intptr nvtx)
	: Mesh(layout_desc, nvtx)
{
	if (nvtx)
		SetMaxVtx(nvtx);
}

TriMesh::TriMesh(const TriMesh& src) : Mesh(src)
{
}


intptr TriMesh::GetNumFaces() const
{
	intptr n = GetNumIdx();
	if (n > 3)
		return n / 3;
	else return 0;
}
		
/*!
 * @fn bool TriMesh::Hit(const Ray& ray, float* distance, TriHitEvent* hitinfo) const
 * @param ray		ray to hit test against
 * @param hitinfo	returns triangle hit information here
 *
 * Determines whether a ray hit the geometry of the mesh,
 * and where the intersection point was
 *
 * @return  true if ray intersected any of the triangles in mesh, else  false
 *
 * @see Ray Model::Hit TriHitEvent
 */
bool TriMesh::Hit(const Ray& ray, TriHitEvent* hitinfo) const
{
	TriHitEvent	boxhit;

	if (!Mesh::Hit(ray, &boxhit))				// hit test against bounding box
		return false;
	if (hitinfo == NULL)
		return true;
	if (hitinfo->Code != Event::TRI_HIT)
	{
		hitinfo->Distance = boxhit.Distance;
		return true;
	}

	const VertexArray* verts = GetVertices();
	const float* vptr = verts->GetData();
	intptr		i0, i1, i2;
	float		dist = FLT_MAX;
	Vec3		intersect(0,0,0);				// minimum intersection distance
	int			vtxsize = verts->GetVtxSize();
	TriMesh::TriIter	triter(this);			// test all the triangles

	hitinfo->Target = this;
	while (triter.Next(i0, i1, i2))				// for each triangle
	{
		const Vec3* v0 = (const Vec3*) (vptr + i0 * vtxsize);	// get vertex locations
		const Vec3* v1 = (const Vec3*) (vptr + i1 * vtxsize);
		const Vec3* v2 = (const Vec3*) (vptr + i2 * vtxsize);
		if (TriHit(ray, *v0, *v1, *v2, &intersect))
		{
			float d = intersect.Distance(ray.start);
			if (d < dist)						// closest one so far?
			{
				dist = d;
				if (d > hitinfo->Distance)
					continue;
				hitinfo->TriIndex = triter.GetIndex();
				hitinfo->Distance = d;
				hitinfo->Intersect = intersect;
			}
		}
	}
	return (dist < FLT_MAX);
}


/*
 * @fn bool TriMesh::Do(Messenger& s, int op)
 * @param mess		input messenger
 * @param opcode	initial opcode
 *
 * Loads the next Mesh opcode from the input messenger
 * and updates the mesh.
 *
 * @code
 *	MESH_MakeNormals
 * @endcode
 *
 * @return  true if operation was successful, else  false
 *
 * @see Mesh::Do
 */
bool TriMesh::Do(Messenger& s, int op)
{
	Vec3			vec;
	const TCHAR*	dbgstr = TriMesh::DoNames[op - TRIMESH_MakeNormals];

	switch (op)
	{
		case TRIMESH_MakeNormals:
		MakeNormals();
		break;

		default:
		return Mesh::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::" << dbgstr << " " << this);
#endif
	return true;
}


} // end Vixen