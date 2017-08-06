#pragma once

namespace Vixen {

class VertexCache;

class VertexCacheEntry
{
public:
	VertexCacheEntry(int locid = -1, intptr vindex = -1) : LocID(locid), VtxIndex(vindex) { }

	int		LocID;		// index of unique location
	intptr	VtxIndex;	// real index of vertex

	static float MaxDiff;
};

namespace Core
{
template <> inline bool Core::Dict<Vec3, VertexCacheEntry, Vixen::BaseDict>::CompareKeys(const Vec3& knew, const Vec3& kdict)
{
	return knew.DistanceSquared(kdict) <= VertexCacheEntry::MaxDiff * VertexCacheEntry::MaxDiff;
//	return knew == kdict;
}

/*
 * Hash so that vertices that are closer go to the same bucket
 */
template <> inline uint32 Core::Dict<Vec3, VertexCacheEntry, Vixen::BaseDict>::HashKey(const Vec3& v) const
{
	int		exp;
	float	fx = fabs(frexp(v.x, &exp));
	float	fy = fabs(frexp(v.y, &exp));
	float	fz = fabs(frexp(v.z, &exp));
	uint32	x = (uint32) (fx * (2 << 16)) & 0XFFFF;
	uint32	y = (uint32) (fy * (2 << 16)) & 0XFFFF;
	uint32	z = (uint32) (fz * (2 << 16)) & 0XFFFF;
	
	return x + y + z;

}

}	// end Core

/*!
 * @class VertexCache
 * @brief A subclass of VertexArray for working with animated characters.
 *
 * Authoring tools and run-time engines for animation often want to maintain
 * their vertex data in terms of unique locations, each which may have multiple
 * normals or texture coordinates. The VertexArray format only permits a single
 * normal, color or texture coordinate per location. Consequently, when converting
 * geometry to this format, vertices must be duplicated and rearranged. Any
 * animation information which relies on vertex offsets in the original content will
 * not be applicable to the resulting scene manager vertex list.
 *
 * VertexCache bridges this gap by maintaining a mapping between the original
 * unique locations and the real vertex array, where that location
 * may be used by several different vertices. These mapping tables are
 * pre-computed when content is exported to a .VIX file and preserved with
 * the vertex data, allowing you to determine which vertices in the PSM
 * list correspond to a specific vertex offset in the original content.
 *
 * VertexCache currently relies on the programmer or art exporter to designate
 * which locations are shared. It currently cannot scan the geometry and automatically
 * detect duplicates.
 *
 * Internally, two tables are constructed to describe the mapping of duplicates.
 * Each entry in the vertex use table indicates the offset in the vertex map table
 * that tells how many times a location is shared. The vertex map table keeps
 * the offsets of the vertices which use each unique location. These tables are
 * constructed and maintained by VertexCache but you can ask for the set of duplicates
 * for a particular location.
 *
 * While constructing the vertex array (usually done during art conversion time),
 * vertices are actually kept in a set of vertex arrays based on the
 * current vertex group. This is to handle the case where the input mesh uses
 * multiple appearances. Vixen only allows a single appearance per mesh.
 * which is critical for performance. After all vertices have been added, the
 * individual vertex lists are compressed to become a single list which has the
 * vertices sorted based on which group they came from. This ordering allows
 * the vertices used by primitives of the same appearance to be kept together so
 * that the minimum number of vertices will be transformed, lit and clipped
 * for a particular primitive.
 *
 * @see VertexArray Mesh TriMesh
 */
class VertexCache : public Dictionary< Vec3, VertexCacheEntry >
{
public:
	VX_DECLARE_CLASS(VertexCache);

	VertexCache(intptr nuniq = 0);
	~VertexCache() { }
//! Return vertex array we are managing
	VertexArray*	GetVertices()					{ return m_Verts; }
	const VertexArray* GetVertices() const			{ return m_Verts; }
//! Designate original mapping of unique locations
	void		SetLocations(VertexArray* verts);
	void		SetLocations(FloatArray* locs);
//!	Designate the vertex array to manage
	void		SetVertices(VertexArray* verts);
//! Return vertex mapping table.
	IntArray*	GetVertexMap() const				{ return m_VertexMap; }
//! Add a vertex and track duplicates.
	intptr		AddVertex(const float* vertex);
	/*
	 * VtxAnim::Do opcodes (also for binary file format)
	 */
	enum Opcodes
	{
		VTX_SetLocations = VertexArray::VTX_NextOp,
		VTX_SetVertices,
		VTX_SetVertexMap,
		VTX_NextOp = VertexArray::VTX_NextOp + 30
	};

//	Overrides
	bool		Copy(const SharedObj*);
	bool		Do(Messenger&, int);
	int			Save(Messenger&, int) const;
	virtual		DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

protected:

	//! Regenerate faces after compression.
	bool		RegenFaces(Mesh* mesh, intptr firstvtx);

	//! Add new vertex to list
	intptr		NewVertex(int locid, const float* vtxptr);

	//! Find the given vertex in the list if possible
	intptr		FindVertex(const float* vtxptr);

//	Data members
	Ref<VertexArray>	m_Verts;			// vertex array to manage
	Ref<VertexArray>	m_UniqLocs;			// uniq locations
	Ref<IntArray>		m_VertexMap;		// maps duplicates to unique verts
};

} // end Vixen