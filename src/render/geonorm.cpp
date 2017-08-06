/****
 *
 * automatic normal generation
 * This implementation is for the C binding. The C++ binding
 * is inlines on top of these functions.
 *
 ****/
#include "vixen.h"

namespace Vixen {
using namespace Core;

/*
 * Internal functions
 */
static void tri_normal(const Vec3*, const Vec3*, const Vec3*, Vec3*, Vec3*, Vec3*);
static void tri_normals(Mesh*);

/*!
 * @fn bool TriMesh::MakeNormals(bool noclear)
 * @param noclear	if true, do not clear all normals before computing them
 *					the default is false
 *
 * Automatically generates normals for all vertices based on
 * the indices for each primitive. We will only generate these
 * if there is space for them in the data structure. If no
 * primitives have been added, no normals are generated.
 *
 * If you are computing normals on a vertex pool shared among
 * multiple meshes, use the  noclear flag for meshes after
 * the first so normals of shared vertices are averaged properly.
 *
 * @return  true if normals were generated, else  false
 *
 * @see Geometry::GetBound Mesh::SetNormals Mesh::SetStyle
 */
bool TriMesh::MakeNormals(bool noclear)
{
	ObjectLock			lock(this);
	VertexArray*		verts = GetVertices();
	VertexPool::Iter	iter(verts);
	intptr				nverts = verts->GetNumVtx();

	if (!iter.HasNormals())
		return false;
	/*
	 * Clear all normals to zero - can be done in parallel.
	 */
	if (!noclear)
	{
		#pragma omp PARALLEL_FOR(nverts)
		cilk_for (intptr i = 0; i < nverts; ++i)
		{
			Vec3* nml = (Vec3*) iter.GetNormal(i);
			nml->x = 0.0f;
			nml->y = 0.0f;
			nml->z = 0.0f;
		}
	}
	/*
	 * Generate normals for each primitive, can only be parallel
	 * if the mesh is not indexed.
	 */
	tri_normals(this);
	/*
	 * Normalize the normals - can be done in parallel
	 */
	iter.Reset();
	#pragma omp PARALLEL_FOR(nverts)
	cilk_for (intptr j = 0; j < nverts; ++j)
	{
		Vec3* nml = (Vec3*) iter.GetNormal(j);
		nml->Normalize();
	}
	return true;
}



/****
 *
 * Generate the normals for a triangle list.
 *
 ****/
static void tri_normals(Mesh* geo)
{
	intptr		ind0, ind1, ind2;
	bool		indexed = (geo->GetNumIdx() > 0);
	VertexArray* verts = geo->GetVertices();
	intptr		maxind = verts->GetNumVtx();

	VertexArray::Iter iter(verts);
	if (indexed)			// cannot be parallel - vertices may be shared
		for (intptr i = 0; i < geo->GetNumIdx(); i += 3)
		{
			ind0 = geo->GetIndex(i);
			ind1 = geo->GetIndex(i + 1);
			ind2 = geo->GetIndex(i + 2);
			if ((ind0 >= maxind) || (ind1 >= maxind) || (ind2 >= maxind))
			{
				continue;				// vertex index out of range?
			}
			Vec3* p0 = iter.GetLoc(ind0);
			Vec3* p1 = iter.GetLoc(ind1);
			Vec3* p2 = iter.GetLoc(ind2);
			Vec3* n0 = iter.GetNormal(ind0);
			Vec3* n1 = iter.GetNormal(ind1);
			Vec3* n2 = iter.GetNormal(ind2);
			tri_normal(p0, p2, p1, n0, n1, n2);
		}
	else
		for (intptr j = 0; j < maxind; ++j)
		{
			iter.Next();
			Vec3* p0 = iter.GetLoc();
			Vec3* n0 = iter.GetNormal();
			iter.Next();
			Vec3* p1 = iter.GetLoc();
			Vec3* n1 = iter.GetNormal();
			iter.Next();
			Vec3* p2 = iter.GetLoc();
			Vec3* n2 = iter.GetNormal();
			tri_normal(p0, p1, p2, n0, n1, n2);
		}
}

/****
 *
 * Compute vertex normals for a triangle.
 *	v0, v1, v2	pointers to vertices of triangle
 *	n0, n1, n2	pointers to normals of triangle
 *
 ****/
void tri_normal(const Vec3* v0, const Vec3* v1, const Vec3* v2,
			   Vec3* n0, Vec3* n1, Vec3* n2)
{
	Vec3	a(*v1);
	Vec3	b(*v2);
	a -= *v0;
	b -= *v0;
	a.Normalize();
	b.Normalize();
	Vec3 c(b.Cross(a));
    c.Normalize();
	*n0 += c;
	*n1 += c;
	*n2 += c;
}

/*!
 * @fn bool TriMesh::TriIter::Next(int32& ind0, int32& ind1, int32& ind2)
 *
 * Returns the vertex indices of the three vertices of
 * the next triangle in the list, strip or fan.
 * The vertex indices returned are zero based offsets into
 * the vertex array of this iterator's mesh.
 *
 * @param ind0	32 bit integer to get first vertex index
 * @param ind1	32 bit integer to get second vertex index
 * @param ind2	32 bit integer to get third vertex index
 *
 * @return  true if another triangle,  false when done
 *
 * @see TriMesh VertexArray::Iter VertexArray
 */
bool TriMesh::TriIter::Next(intptr& ind0, intptr& ind1, intptr& ind2)
{
	const TriMesh* mesh = (const TriMesh*) ObjPtr;
	if (mesh == NULL)
		return false;
	m_PrevIndex = m_TriIndex;
	if (mesh->GetNumIdx())
	{
		if (m_TriIndex >= mesh->GetNumIdx())
			return false;
		ind0 = mesh->GetIndex(m_TriIndex);
		ind1 = mesh->GetIndex(m_TriIndex + 1);
		ind2 = mesh->GetIndex(m_TriIndex + 2);
	}
	else
	{
		if (m_TriIndex >= mesh->GetNumVtx())
			return false;
		ind0 = m_TriIndex;
		ind1 = ind0 + 1;
		ind2 = ind1 + 1;
	}
	m_TriIndex += 3;
	return true;
}

}  // end Vixen