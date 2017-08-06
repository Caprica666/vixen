#pragma once

inline bool VDict<VXVec3, int, VXBaseDict>::CompareKeys(const VXVec3& knew, const VXVec3& kdict)
{
	return	(knew.x == kdict.x) &&
			(knew.y == kdict.y) &&
			(knew.z == kdict.z);
}

inline uint32 VDict<VXVec3, int, VXBaseDict>::HashKey(const VXVec3& v) const
{
	float t = v.x + v.y + v.z;
	return (uint32) t;
}


/*!
 * @class VXVtxCache
 * @brief Dictionary that maintains a set of unique vertices for a mesh.
 *
 * Vertices are hashed on their location so that all vertices with the same
 * location but different normals or texcoords are kept in the same hash bucket.
 * The mesh associated with the vertex cache is directly modified as new
 * vertices are encountered.
 */
class VXVtxCache : protected VXDict< VXVec3, int >
{
public:
    VXVtxCache(VXMesh* mesh = NULL);
	void	SetMesh(VXMesh*);
    VXMesh*	GetMesh()			{ return m_Mesh; }
	VXVtxArray* GetVertices()	{ return m_Mesh.IsNull() ? NULL : m_Mesh->GetVertices(); }
    int		GetVtxSize()		{ return m_Mesh.IsNull() ? 0 : m_Mesh->GetVtxSize(); }
    int		GetNumVtx()			{ return m_Mesh.IsNull() ? 0 : m_Mesh->GetNumVtx(); }
    int		FindVertex(const float* vtx);

protected:
    bool	CompareVertex(const float* vtx, int index);
    int		AddVertex(const float* vtx);

    Ref<VXMesh>		m_Mesh;
	VXVtxPool::Iter	m_Iter;
};


inline VXVtxCache::VXVtxCache(VXMesh* mesh) :  VXDict< VXVec3, int >()
{
	if (mesh)
		SetMesh(mesh);
}

inline void VXVtxCache::SetMesh(VXMesh* mesh)
{
	m_Mesh = mesh;
	m_Iter.Init(mesh->GetVertices());
}
