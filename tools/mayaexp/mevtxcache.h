#pragma once

#if 1
inline bool Core::Dict<Vec3, int, BaseDict>::CompareKeys(const Vec3& knew, const Vec3& kdict)
{
	return	(knew.x == kdict.x) &&
			(knew.y == kdict.y) &&
			(knew.z == kdict.z);
}

inline uint32 Core::Dict<Vec3, int, BaseDict>::HashKey(const Vec3& v) const
{
	float t = v.x + v.y + v.z;
	return (uint32) t;
}
#endif

/*!
 * @class meVtxCache
 * @brief Dictionary that maintains a set of unique vertices for a mesh.
 *
 * Vertices are hashed on their location so that all vertices with the same
 * location but different normals or texcoords are kept in the same hash bucket.
 * The GeoArray associated with the vertex cache is directly modified as new
 * vertices are encountered.
 */
class meVtxCache : protected Dictionary< Vec3, int >
{
public:
    meVtxCache(Mesh* mesh = NULL);
	void	SetMesh(Mesh*);
    Mesh*	GetMesh()			{ return m_Mesh; }
	VertexArray* GetVertices()	{ return m_Mesh.IsNull() ? NULL : m_Mesh->GetVertices(); }
    int		GetVtxSize()		{ return m_Mesh.IsNull() ? 0 : m_Mesh->GetVtxSize(); }
    int		GetNumVtx()			{ return m_Mesh.IsNull() ? 0 : m_Mesh->GetNumVtx(); }
    int		FindVertex(const float* vtx);
    int		AddVertex(const float* vtx);
	int		SetCompareSize(int nfloats);
	int		GetCompareSize() const	{ return m_CompareSize; }

protected:
    bool	CompareVertex(const float* vtx, int index);

    Ref<Mesh>			m_Mesh;
	VertexPool::Iter	m_Iter;
	int					m_CompareSize;
};

