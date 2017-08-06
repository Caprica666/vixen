#pragma once

namespace Vixen {

inline int Mesh::GetStyle() const
	{ return m_Verts->GetStyle(); }

inline const IndexArray* Mesh::GetIndices() const
	{ return m_VtxIndex; }

inline IndexArray* Mesh::GetIndices()
	{ return m_VtxIndex; }

inline intptr Mesh::GetNumIdx() const
{
	if (m_VtxIndex.IsNull())
		return 0;
	return m_VtxIndex->GetSize();
}

inline intptr Mesh::GetIndex(intptr i) const
{
	const IndexArray* idx = m_VtxIndex;
	if (idx == NULL)
		return -1;
	return  idx->GetAt(i);
}


/*!
 * @fn int Mesh::GetVtxSize() const
 *
 * Returns the number of floats used in a single vertex.
 *
 * @see Mesh::SetStyle VertexArray::GetVtxSize
 */
inline int Mesh::GetVtxSize() const
	{ return m_Verts.IsNull() ? 0 : m_Verts->GetVtxSize(); }

FORCE_INLINE intptr Mesh::GetStartVtx() const
	{ return m_StartVtx; }

FORCE_INLINE intptr Mesh::GetEndVtx() const
	{ return m_EndVtx; }

/*!
 * @fn bool	Mesh::SetNumVtx(int n)
 * @param n	new vertex count desired
 *
 * If the new vertex count exceeds the maximum
 * number of vertices we currently have room for,
 * the vertex array is enlarged if possible.
 * Making the array with this call does not disturb the data
 * (whereas SetMaxVtx does).
 *
 * @return  true if array was successfully resized, else  false
 *
 * @see Mesh::SetMaxVtx VertexArray::SetNumVtx
 */
inline bool	Mesh::SetNumVtx(intptr n)
	{ return m_Verts->SetNumVtx(n); }

inline intptr Mesh::GetNumVtx() const
	{ return m_Verts.IsNull() ? 0 : m_Verts->GetNumVtx(); }

/*!
 * @fn bool	Mesh::SetMaxVtx(intptr n)
 *
 * If the maximum size is increased, the vertex array is enlarged if possible.
 * Resources used by the vertex array may be reclaimed.
 * Making the array smaller with this call may destroy data (whereas
 * SetNumVtx will not)
 *
 * @return  true if array was successfully resized, else  false
 *
 * @see Mesh::SetNumVtx VertexArray::SetMaxVtx
 */
inline bool	Mesh::SetMaxVtx(intptr n)
	{ return m_Verts->SetMaxVtx(n); }



FORCE_INLINE const VertexArray* Mesh::GetVertices() const
	{ return m_Verts; }

FORCE_INLINE VertexArray* Mesh::GetVertices()
	{ return m_Verts; }

/*!
 * @fn bool Mesh::GetBound(Box3* bound) const
 * @param bound	where to store bounding volume
 *
 * The axially aligned bounding box for the vertices is computed
 * by examining the vertices in the vertex array of the mesh.
 *
 * @return  true if bounding volume computed,  false if empty
 *
 * @see VertexArray::GetBound
 */
inline bool Mesh::GetBound(Box3* bound) const
{
	if (m_Bound.IsEmpty())
	{
		if (m_Verts.IsNull() || !m_Verts->GetBound(&m_Bound))
			return false;
	}
	*bound = m_Bound;
	return true;
}

/*!
 * @fn Geometry& Mesh::operator*=(const Matrix& trans)
 *
 * The vertices of the mesh are transformed as points by
 * the given matrix. If normals are present, they are transformed
 * as vectors. This changes the vertex array of the mesh but
 * not the index array.
 *
 * @see Matrix VertexPool::Transform
 */
inline Geometry& Mesh::operator*=(const Matrix& trans)
	{ *((VertexArray*) m_Verts) *= trans; Touch(); m_Bound.Empty(); return *this; }

/*!
 * @fn Vec3 Geometry::GetSortLoc() const
 *
 * Returns the location for Z comparison of transparent objects.
 * This function is called when Z sorting and should return
 * an absolute position in the coordinate space of the geometry.
 *
 * @see  Geometry::Render
 */
inline Vec3 Geometry::GetSortLoc() const
{
	return Vec3(0,0,0);
}

inline Vec3 Mesh::GetSortLoc() const
{
	const VertexArray* verts = GetVertices();
	Box3 bound;

	if (verts && verts->GetBound(&bound))
		return bound.Center();
	return Vec3(0,0,0);
}

} // end Vixen