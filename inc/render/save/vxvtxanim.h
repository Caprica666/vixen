#pragma once

#define	VTX_MaxDups	8

/*!
 * @class VXVtxAnim
 * @brief A subclass of VXVtxArray for working with animated characters.
 *
 * Authoring tools and run-time engines for animation often want to maintain
 * their vertex data in terms of unique locations, each which may have multiple
 * normals or texture coordinates. The VXVtxArray format only permits a single
 * normal, color or texture coordinate per location. Consequently, when converting
 * geometry to this format, vertices must be duplicated and rearranged. Any
 * animation information which relies on vertex offsets in the original content will
 * not be applicable to the resulting scene manager vertex list.
 *
 * VXVtxAnim bridges this gap by maintaining a mapping between the original
 * unique locations and the real vertex array, where that location
 * may be used by several different vertices. These mapping tables are
 * pre-computed when content is exported to a .VIX file and preserved with
 * the vertex data, allowing you to determine which vertices in the PSM
 * list correspond to a specific vertex offset in the original content.
 *
 * VXVtxAnim currently relies on the programmer or art exporter to designate
 * which locations are shared. It currently cannot scan the geometry and automatically
 * detect duplicates.
 *
 * Internally, two tables are constructed to describe the mapping of duplicates.
 * Each entry in the vertex use table indicates the offset in the vertex map table
 * that tells how many times a location is shared. The vertex map table keeps
 * the offsets of the vertices which use each unique location. These tables are
 * constructed and maintained by VXVtxAnim but you can ask for the set of duplicates
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
 * @see VXVtxArray VXMesh VXTriMesh
 */
class VXVtxAnim : public VXVtxArray
{
public:
	VX_DECLARE_CLASS(VXVtxAnim);

	VXVtxAnim(int style = VTX_Normals, int nuniq = 0);
	VXVtxAnim(const VXVtxArray&);

//! Return number of unique vertices in this pool.
	int			GetNumUniq() const		{ return m_NumUniq; }
//! Set the number of unique vertices in this pool.
	void		SetNumUniq(int n);
//! Get vertex array containing vertices for the given group.
	VXVtxArray* GetVtxGroup(int group) const;
//! Set entries in vertex use table.
	bool		SetVtxUse(int32* usedata);
//! Set entries in vertex mapping table.
	bool		SetVtxMap(int32* mapdata, int n);
//! Get vertex mapping data for the given vertex.
	int			GetVtxMap(int uniqvtx, int32* mapdata) const;
//! Get the number of times the given vertex is used.
	int			GetVtxUse(int uniqvtx) const;
//! Return vertex mapping table.
	const VXIntArray* GetVtxMap() const	{ return m_VtxMap; }
//! Return vertex usage table.
	const VXIntArray* GetVtxUse() const	{ return m_VtxUse; }
//! Get index of first vertex in the given group.
	int			GetFirstVtx(int group) const;
//! Set index of first vertex in the given troup.
	void		SetFirstVtx(int group, int vtx);
//! Mark a vertex as shared
	int			ShareVtx(int uniqvtx, int dupvtx, int group, VXVec3* loc = NULL);
//! Compress the vertices from meshes in the list into a single vertex array.
	bool		Compress();
//! Set unique vertex locations in the given range.
	void		SetUniqLocs(const VXVec3*, int size, int ofs = 0);
//! Get unique vertex locations in the given range.
	void		GetUniqLocs(VXVec3*, int size, int ofs = 0) const;
//! Remap the vertices with the input index array.
	bool		Remap(VXIndexArray*);
//! Replaces the normals
	void		SetNormals(const VXArray<VXVec3>& normals);
//!	Sets the mesh for a given group indexs
	void		SetMesh(int group, VXMesh* mesh)	{ m_Meshes.SetAt(group, mesh); }

	/*
	 * VtxAnim::Do opcodes (also for binary file format)
	 */
	enum Opcodes
	{
		VTX_SetNumUniq = VXVtxArray::VTX_NextOp, 
		VTX_SetFirstVtx,
		VTX_SetUniqLocs,
		VTX_SetVtxUse,
		VTX_SetVtxMap,
		VTXANIM_NextOp = VXVtxArray::VTX_NextOp + 30
	};

//	Overrides
	bool		Copy(const VXObj*);
	bool		Do(VXMessenger&, int);
	int			Save(VXMessenger&, int) const;

protected:
//! Regenerate indices after compression.
	bool		RegenIndex(VXMesh* mesh, int firstvtx);

//	Data members
	vint32			m_NumUniq;		// number of unique locations
	VXIndexArray	m_FirstVtx;		// offset of first vertex in each group
	Ref<VXIntArray>	m_VtxUse;		// vertex use tables for each group
	Ref<VXIntArray>	m_VtxMap;		// vertex mapping tables for each group
	VXObjArray		m_Meshes;		// table of meshes
};

/*!
 * @param int VXVtxAnim::GetFirstVtx(int g) const
 * @param group	number of group to get vertex offset for
 *
 * If this object has not been compressed, the vertex offset
 * returned will be zero because the vertices for each group 
 * are in separate vertex arrays. After compression, the
 * vertices from all groups are combined in a single vertex
 * array and the offsets returned will not always be zero.
 *	
 * @return -> offset of first vertex in group, zero if not compressed
 *
 * @see VXVtxAnim::Compress VXVtxAnim::GetVtxGroup VXVtxAnim::ShareVtx
 */
inline int VXVtxAnim::GetFirstVtx(int g) const
{
	if (m_FirstVtx.GetSize() <= g)
		return 0;
	else return m_FirstVtx[g];
}
