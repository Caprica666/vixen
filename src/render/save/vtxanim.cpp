/****
 *
 * PSVtxAnim implementation of PSVtxArray that shares a single
 * list of unique locations among multiple vertex groups, each
 * group being maintained as a separate mesh. The relationship between
 * the master list of locations and the individual groups is
 * specified in the form of vertex mapping tables for each group.
 *
 ****/
#include "vixen.h"

VX_IMPLEMENT_CLASSID(PSVtxAnim, PSVtxArray, PSM_VtxAnim);

#ifdef _DEBUG
static const TCHAR* opnames[] =
	{ TEXT("SetNumUniq"), TEXT("SetVtxUseOLD"), TEXT("SetVtxMapOLD"),
	   TEXT("SetFirstVtx"), TEXT("SetUniqLocs"), TEXT("SetVtxUse"), TEXT("SetVtxMap"), };

const TCHAR** PSVtxAnim::DoNames = opnames;
#endif

/*
 * Vertex use table entry format.
 * There is one entry for each unique location.
 */
union VtxUseEntry
{
	int32	All;
	struct	UseEntry
	   {
		unsigned int	UseCount : 8;	// number of times vertex is used
		unsigned int	MapOfs : 24;	// offset in vertex map
	   } Use;
};

/*
 * Vertex map table entry format.
 * There are one or more entries for each unique location.
 * The total number of entries is the same as the number of vertices.
 */
union VtxMapEntry
{
	int32	All;
	struct MapEntry
	   {
		unsigned int	Group : 8;		// group duplicate is in
		unsigned int	VtxOfs : 24;	// vertex offset
	   } Map;
};	

PSVtxAnim::PSVtxAnim(int style, int nuniq)
  :	PSVtxArray(style)
{
	m_NumUniq = nuniq;
}


PSVtxArray* PSVtxAnim::GetVtxGroup(PSSurface* surf, int appidx) const
{
	VXMesh* mesh = (VXMesh*) surf->Find(appidx);
	if (mesh == NULL)
		return false;
	return mesh->GetVertices();
}

/*!
 * @fn void PSVtxAnim::SetNumUniq(int n)
 * @param n number of unique locations
 *
 * Establishes the current number of unique locations in this list.
 * If you change this number after you have establish vertex mappings,
 * unpredictable results will follow unless your program updates
 * the vertex maps.
 *	
 * @see PSVtxAnim::SetNumGroups
 */
void PSVtxAnim::SetNumUniq(int n)
{
	VX_STREAM_BEGIN(s);
		*s << OP(PSM_VtxAnim, VTX_SetNumUniq) << int32(n);
	VX_STREAM_END();

	VInterlockSet(&m_NumUniq, n);
}

/*!
 * @fn bool PSVtxAnim::SetVtxUse(int32* usedata)
 * @param usedata vertex use table data, zero-based offsets into vertex group
 *
 * Establishes the vertex mappings for the duplicate vertices.
 * The  vertex  use table has an entry for each unique location in
 * the PSVtxAnim (as established by SetNumUniq).
 * The entry designates the number of times this location has been
 * shared and its offset in the vertex map. The  vertex  map is an
 * array with offsets into the real vertex list for each shared location.
 *
 * Since the format of these tables is not public, this routine is
 * primarily intended for restoring the state of the VtxAnim from a
 * file and not for users to create their own use and mapping tables.
 *	
 * @see PSVtxAnim::SetNumGroups PSVtxAnim::SetNumUniq PSVtxAnim::SetVtxMap
 */
bool PSVtxAnim::SetVtxUse(int32* usedata)
{
	Lock();
	for (int i = 0; i < m_NumUniq; ++i)
		m_VtxUse->SetAt(i, usedata[i]);
	Unlock();
	return true;
}

/*!
 * @fn int PSVtxAnim::GetVtxUse(int uniqvtx) const
 * @param usedata	vertex use table data, zero-based offsets into vertex group
 * @param uniqvtx	offset of unique vertex to look up usage for
 *
 * Returns information about the sharing of all unique locations
 * or for a particular one. The  vertex  use table has an entry
 * 	for each unique location in the PSVtxAnim (as established by SetNumUniq).
 * The entry contains the number of times this location has been shared
 * and its offset in the vertex map. The  vertex  map table is an array
 * that has the offsets into the real vertex list for all of the shared locations.
 *	
 * @return Number of times this location has been shared (will be at least 1)
 *
 * @see PSVtxAnim::SetNumUniq PSVtxAnim::GetVtxMap
 */
int PSVtxAnim::GetVtxUse(int uniqvtx) const
{
	VtxUseEntry		e;
	e.All = m_VtxUse->GetAt(uniqvtx);
	return e.Use.UseCount;
}

/*!
 * @fn bool PSVtxAnim::SetVtxMap(int32* mapdata, int n)
 * @param mapdata	vertex map table data, zero-based offsets into vertex group
 *				if NULL, vertex map deleted
 * @param n		number of vertex map entries
 *
 * The  vertex  map table is an index array that contains offsets
 * into the vertex list for all copies of each shared location.
 * The  vertex  use table tells which vertex map entries apply
 * to each unique location.
 *	
 * Since the format of these tables is not public, this routine is
 * primarily intended for restoring the state of the VtxAnim from a
 * file and not for users to create their own use and mapping tables.
 *	
 * @see PSVtxAnim::SetNumGroups PSVtxAnim::SetNumUniq PSVtxAnim::SetVtxUse PSVtxAnim::GetVtxMap
 */
bool PSVtxAnim::SetVtxMap(int32* mapdata, int n)
{
	Lock();
	if (m_VtxMap.IsNull())
		m_VtxMap = new VXIntArray(n);
	for (int i = 0; i < n; ++i)
		m_VtxMap->SetAt(i, mapdata[i]);
	Unlock();
	return true;
}

/*!
 * @fn int PSVtxAnim::GetVtxMap(int uniqvtx, int32* vtxmap) const
 * @param uniqvtx	offset of unique vertex to look up map entries for
 * @param mapdata	where to store vertex map entries for a location,
 *					cannot be NULL and must be large enough for entries
 *
 * Returns information about the sharing of all unique locations
 * or a particular one. The  vertex  map table is an index array
 * that contains offsets into the vertex list for all copies
 * of each shared location.
 *	
 * If you supply the offset of a unique location, GetVtxMap will
 * copy the vertex map entries for that location into your input array.
 * You can call GetVtxUse to determine how many entries there are
 * for a location. No checks are made on the size of the input array.
 *	
 * @return Number of vertex map entries copied
 *
 * @see PSVtxAnim::SetNumUniq PSVtxAnim::GetVtxUse PSVtxAnim::SetVtxMap
 */
int PSVtxAnim::GetVtxMap(int uniqvtx, int32* vtxmap) const
{
	VtxUseEntry		e;
	VtxMapEntry		m;

	e.All = m_VtxUse->GetAt(uniqvtx);
	for (uint32 i = 0; i < e.Use.UseCount; ++i)
	   {
		m.All = m_VtxMap->GetAt(e.Use.MapOfs + i);
		*vtxmap++ = m.Map.VtxOfs;
	   }
	return e.Use.UseCount;
}

bool PSVtxAnim::Copy(const VXObj* src_obj)
{
	VXObjLock dlock(this);
	VXObjLock slock(src_obj);
	if (!PSVtxArray::Copy(src_obj))
		return false;
	PSVtxAnim* src = (PSVtxAnim*) src_obj;
	if (src->IsClass(PSM_VtxAnim))
	   {
		m_FirstVtx.Copy(&(src->m_FirstVtx));
		m_VtxUse = src->m_VtxUse;
		m_VtxMap = src->m_VtxMap;
		return true;
	   }
	return false;
}

void PSVtxAnim::SetFirstVtx(int group, int vtx)
{
	VX_STREAM_BEGIN(s);
		*s << OP(PSM_VtxAnim, VTX_SetFirstVtx) << int32(group) << int32(vtx);
	VX_STREAM_END(  );

	m_FirstVtx.SetAt(group, vtx);
}

/*!
 * @fn int PSVtxAnim::ShareVtx(int uniqvtx, int dupvtx, PSSurface* surf, int appidx, VXVec3* loc)
 * @param uniqvtx	offset of unique location to be shared
 * @param dupvtx	offset of duplicate in vertex list, if -1
 *					duplicate added to end of group
 * @param surf		surface containing vertex lists for each group
 * @param appidx	appearance index (selects the specific mesh)
 * @param loc		location to add or NULL
 *
 * Designates a vertex as being duplicated in the vertex list.
 * Updates the vertex use table and vertex map that keep track
 * of the duplicates for each vertex.
 *
 * This function initially allocates room for four duplicates
 * for each vertex in the mapping tables. After all sharing calls
 * have been performed, the vertex mapping tables can be compressed
 * by the Compress call
 *
 * @return number of duplicate vertex or -1 on failure
 *
 * @see PSVtxAnim::Compress PSVtxAnim::GetVtxUse PSVtxAnim::SetVtxMap
 */
int PSVtxAnim::ShareVtx(int uniqvtx, int dupvtx, PSSurface* surf, int appidx, VXVec3* loc)
{
	VtxUseEntry		e;
	VtxMapEntry		m;
	PSVtxArray*		verts = NULL;
	VXMesh*			mesh;

/*
 * We know a vertex can have at most VTX_MaxDups duplicates.
 * We only allocate enough room for each vertex to be shared
 * this many times. Later on, we will compress this array.
 */
	if (m_VtxUse.IsNull())
		m_VtxUse = new VXIntArray(m_NumUniq);
	m_VtxUse->SetSize(m_NumUniq);
	if (m_VtxMap.IsNull())
		m_VtxMap = new VXIntArray(VTX_MaxDups * m_NumUniq);
/*
 * Add to appropriate vertex map if requested.
 * There is one mesh for each appearance index used.
 * Each mesh initially gets its own vertex array which only
 * contains vertices used by that mesh. Later one, all meshes
 * will share a single vertex array.
 */
	if (loc || (dupvtx < 0))
	{
		mesh = (VXMesh*) surf->Find(appidx);
		if ((mesh == NULL) || !mesh->IsClass(PSM_Mesh))
		{
			const PSVtxLayout* layout = GetLayout();
			mesh = new VXTriMesh(layout->Descriptor);
			mesh->SetAppIndex(appidx);
			surf->Append(mesh);
		}
		verts = mesh->GetVertices();
		VX_ASSERT(verts);
		VX_ASSERT(verts->GetLayout() == GetLayout());
		if (dupvtx < 0)			// add it at the end?
		{
			dupvtx = verts->GetNumVtx();
			verts->SetNumVtx(dupvtx + 1);
		}
		PSVtxPool::Iter iter(verts);
		VXVec3* vptr = iter.GetLoc(dupvtx);
		VX_ASSERT(vptr != NULL);
		*vptr = *loc;
	}
/*
 * The offset of each vertex in the vertex map is its unique location
 * offset * 4
 */
	m.Map.Group = appidx;					// group for vertex
	m.Map.VtxOfs = dupvtx;					// offset for vertex
	if ((uniqvtx < 0) || (uniqvtx >= m_VtxUse->GetSize()))
		e.All = 0;
	else
		e.All = m_VtxUse->GetAt(uniqvtx);	// get vertex use table entry
	int32 mapofs = uniqvtx * VTX_MaxDups;	// first vertex map offset
	if (e.All == 0)							// no duplicates?
	{
		e.Use.UseCount = 1;					// one duplicate so far
		e.Use.MapOfs = mapofs;				// use next map table entry
	}
/*
 * We allow a vertex to have at most VTX_MaxDups duplicates
 */
	else										// add to dup list
	{
		mapofs += e.Use.UseCount;				// offset of next map entry
		if (e.Use.UseCount++ >= VTX_MaxDups)	// exceeded max # of duplicates
		{
			m_VtxUse->SetAt(uniqvtx, e.All);	// update vertex use table
			VX_ERROR(("VtxAnim::Share - vertex cannot be shared more than %d times\n", VTX_MaxDups), dupvtx);
		}
	}
	m_VtxUse->SetAt(uniqvtx, e.All);			// update vertex use table
	m_VtxMap->SetAt(mapofs, m.All);				// update vertex map
	return dupvtx;				
}

/*!
 * @fn bool PSVtxAnim::Compress(PSSurface* surf)
 *
 * Compresses the vertex mapping tables and vertex lists
 * to occupy the minimum amount of storage. The vertex list
 * for all meshes in the surface are combined into a single
 * vertex list shared by the meshes.
 *
 * The empty space in the vertex mapping tables is removed
 * and they are compressed.
 *
 * Compression can change the behavior of GetFirstVtx.
 * Before compression, GetFirstVtx always returns zero.
 * After compression, GetFirstVtx will return non-zero
 * offsets for some vertex groups.
 *
 * @see PSVtxAnim::ShareVtx PSVtxAnim::SetVtxMap PSVtxAnim::GetFirstVtx PSVtxAnim::RegenIndex
 */
bool PSVtxAnim::Compress(PSSurface* surf)
{
	if (m_NumUniq == 0)
		return false;
	if ((surf == NULL) || (surf->GetSize() < 1))
		return false;
	if (m_VtxUse->GetSize() != m_NumUniq)
		return false;
	if (m_VtxMap->GetSize() < 1)
		return false;

	VtxUseEntry		u1, u2;
	VtxMapEntry		m;
	PSVtxIndex		newofs = 0;
	VXIntArray		remap;
	VXIntArray*		new_vmap = new VXIntArray;
	int 			i;
	PSSurface::Iter	iter(surf);
	VXTriMesh*	mesh;

/*
 * Combine all the individual mesh vertices into a single
 * contiguous vertex array. Save the offsets of the first
 * vertex in each mesh. Upon return, all meshes use this vertex array.
 */
	VX_ASSERT(GetNumVtx() == 0);
	newofs = 0;
	while (mesh = (VXTriMesh*) iter.Next())
		if (mesh->IsClass(PSM_Triangles))
		{
			PSVtxArray* verts = mesh->GetVertices();
			m_FirstVtx.SetAt(iter.GetIndex(), newofs);
			VX_ASSERT(verts);
			AddVertices(verts->GetData(), verts->GetNumVtx());
			RegenIndex(mesh, newofs);
			newofs += verts->GetNumVtx();
			mesh->SetVertices(this);
		}
/*
 * Remove unused space in the vertex mapping tables by compressing
 * the data and remapping the offsets in the vertex use table
 * Remap the vertex map offsets to reference new vertex array
 */
	newofs = 0;
	for (i = 0; i < m_NumUniq; ++i)
	 {
		u1.All = u2.All = m_VtxUse->GetAt(i);
		if (u1.Use.UseCount > VTX_MaxDups)
			u1.Use.UseCount = VTX_MaxDups;	// clamp use counts to table size
		u2.Use.MapOfs = newofs;
		newofs += u1.Use.UseCount;
		m_VtxUse->SetAt(i, u2.All);
		for (uint32 j = 0; j < u1.Use.UseCount; ++j)
		{
			m.All = m_VtxMap->GetAt(u1.Use.MapOfs + j);
			m.Map.VtxOfs += m_FirstVtx[m.Map.Group];
			m.Map.Group = 0;
			new_vmap->Append(m.All);
		}			
	}
	m_VtxMap = new_vmap;
	return true;		
}

/*!
 * @fn bool PSVtxAnim::RegenIndex(VXTriMesh* mesh, int firstvtx)
 * Remaps the index tables for this mesh
 * based on the vertex mapping tables for this PSVtxAnim.
 *
 * m_FirstVtx gives the first vertex in the shared array
 * for each mesh. The vertices in the shared array have already 
 * been compressed so that the vertices are in the order they 
 * are used by the primitives. The index tables are regenerated
 * to match the vertex order.
 *
 * @see PSVtxAnim::ShareVtx PSVtxAnim::Compress
 */
bool PSVtxAnim::RegenIndex(VXTriMesh* mesh, int firstvtx)
{
	if (mesh->GetNumIdx() == 0)
		return false;

	PSIndexArray*	inds = new PSIndexArray(mesh->GetNumIdx());
	PSVtxIndex		vtxofs = 0;
	const PSTriPrim* prim;
	VXTriMesh::PrimIter piter(mesh);
	while (prim = piter.Next())
	{
		int32		idxofs = prim->VtxIndex;
		((PSTriPrim*) prim)->VtxIndex = vtxofs;
		for (int j = 0; j < prim->Size; ++j)
		{
			int32 n = mesh->GetIndex(idxofs + j);
			inds->SetAt(vtxofs++, n + firstvtx);
		}
	}
	VX_ASSERT(vtxofs == mesh->GetNumIdx());
	mesh->SetIndices(inds);
	return true;
}

/****
 *
 * class VtxArray for VXObj::Do
 *	VTX_SetVtxUse	<VXIntArray>
 *	VTX_SetVtxMap	<VXIntArray>
 *
 ****/
bool PSVtxAnim::Do(VXMessenger& s, int op)
{
	int32		n, ofs, f;
	VXObj*		arr;
	VXIntArray*	data;
	VXVec3*		locs;
	bool		ret = false;

	if (s.Version < 5)
		switch (op)
		{
			case VTX_SetVtxUseOLD:
			ret = true;
			if (m_NumUniq <= 0)
				break;
			data = new VXIntArray;
			data->SetMaxSize(m_NumUniq);
			s.Input((int32*) data->GetData(), m_NumUniq);
			m_VtxUse = data;		
			break;

			case VTX_SetVtxMapOLD:
			s >> n;
			ret = true;
			if (n <= 0)
				break;
			data = new VXIntArray;
			data->SetMaxSize(n);
			s.Input((int32*) data->GetData(), n);
			m_VtxMap = data;		
			break;
		}

	if (!ret) switch (op)
	{
		case VTX_SetUniqLocs:
		s >> ofs >> n;
		locs = new VXVec3[n];
		s.Input((float*) locs, n * 3);
		SetUniqLocs(locs, n, ofs);
		delete [] locs;
		break;

		case VTX_SetVtxMap:
		s >> arr;
		if (arr)
		{
			VX_ASSERT(arr->IsClass(PSM_Array));
			m_VtxMap = (VXIntArray*) arr;
		}
		break;

		case VTX_SetVtxUse:
		s >> arr;
		if (arr)
		{
			VX_ASSERT(arr->IsClass(PSM_Array));
			m_VtxUse = (VXIntArray*) arr;
		}
		break;

		case VTX_SetNumUniq:
		s >> n;
		SetNumUniq(n);
		break;

		case VTX_SetFirstVtx:
		s >> n >> f;
		SetFirstVtx(n, f);
		break;

		default:
		return PSVtxArray::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(psm_debug << ClassName() << "::"
					   << PSVtxAnim::DoNames[op - VTX_SetNumUniq]);
#endif
	return true;
}

/****
 *
 * class VtxAnim override for VXObj::Save()
 * The base implementation just saves the vertex maximum
 * and style. It is left to the derived class to save
 * the vertex data of the return value is non-zero.
 *
 ****/
int PSVtxAnim::Save(VXMessenger& s, int opts) const
{
	int32 h = PSVtxArray::Save(s, opts);
	int32 i;
	if (h < 0)
		return h;
	if (m_NumUniq == 0)			// treat like PSVtxArray
		return h;
	if (!m_VtxUse.IsNull() && m_VtxUse->GetSize() && (m_VtxUse->Save(s, opts) >= 0) && h)
		s << OP(PSM_VtxAnim, VTX_SetVtxUse) << h << (const VXIntArray *)m_VtxUse;
	if (!m_VtxMap.IsNull() && m_VtxMap->GetSize() && (m_VtxMap->Save(s, opts) >= 0) && h)
		s << OP(PSM_VtxAnim, VTX_SetVtxMap) << h << (const VXIntArray *)m_VtxMap;
	if (h <= 0)
		return h;
	s << OP(PSM_VtxAnim, VTX_SetNumUniq) << h << m_NumUniq;
	for (i = 0; i < m_FirstVtx.GetSize(); ++i)
	{
		int32 n = m_FirstVtx.GetAt(i);
		s << OP(PSM_VtxAnim, VTX_SetFirstVtx) << h << i << n;
	}
	return h;
}

/****
 *
 * class VtxArray override for VXObj::Print()
 *
 ****/
VDebug& PSVtxAnim::Print(VDebug& dbg) const
{
	PSVtxArray::Print(dbg);
/*
 * Print vertex mapping information: For each unique location
 * print the vertex offsets associated with it in the map table
 * and their groups in the format:
 *	<uniqloc>	<group>:<vtx offset> <group>:<vtx offset> ...
 */
	endl(dbg << ClassName() << "::SetVtxMap " << this);
	for (int i = 0; i < m_NumUniq; ++i)
	{
		VtxMapEntry m;
		VtxUseEntry u;
		int32		numdups;

		dbg << "\t" << i << "\t";
		u.All = m_VtxUse->GetAt(i);
		numdups = u.Use.UseCount;
		for (int j = 0; j < numdups; ++j)
		   {
			m.All = m_VtxMap->GetAt(u.Use.MapOfs + j);
			dbg << " " << m.Map.Group << ":" << m.Map.VtxOfs;
		   }
		endl(dbg);
	}
	return dbg;
}

void PSVtxAnim::SetUniqLocs(const VXVec3* vert_data, int size, int ofs)
{
	int i;
	VXVec3* vptr;

	VX_STREAM_BEGIN(s);
		PSVtxArray::Output(s, OP(PSM_VtxAnim, VTX_SetUniqLocs), (const float*) vert_data, ofs, size, 3);
	VX_STREAM_END( );

	VX_ASSERT(this->GetNumUniq() >= ofs + size);
	SetChanged(true);
	PSVtxPool::Iter iter(this);
	if (GetVtxMap() == NULL)
		for (i = ofs; i < size; ++i)
		{
			vptr = iter.GetLoc(i);
			VX_ASSERT(vptr);
			*vptr = vert_data[i];
		}
	else
		for (i = ofs; i < size ; i++)
		{
			int identical_vert_ids[20]; // Is 20 is big enough to accommodate the shared vertex???
			int num_of_identical_verts = this->GetVtxMap(i, identical_vert_ids);
			VX_ASSERT(num_of_identical_verts <= 20);
			for (int j = 0; j < num_of_identical_verts; j++)
			{
				vptr = iter.GetLoc(identical_vert_ids[j]);
				VX_ASSERT(vptr);
				*vptr = vert_data[i];
			}
		}
}

void PSVtxAnim::GetUniqLocs(VXVec3* vert_data, int size, int ofs) const
{
	VtxUseEntry		e;
	VtxMapEntry		m;

	VX_ASSERT(GetNumUniq() >= ofs + size);
	SetChanged(true);
	if (GetVtxMap() == NULL)
		return;
	PSVtxPool::ConstIter citer(this);
	for (int i = ofs; i < size; i++)
	{
		e.All = m_VtxUse->GetAt(i);
		m.All = m_VtxMap->GetAt(e.Use.MapOfs);
		const VXVec3* vptr = citer.GetLoc(m.Map.VtxOfs);
		VX_ASSERT(vptr);
		vert_data[i] = *vptr;
	}
}

bool PSVtxAnim::Remap(PSIndexArray* remap)
{
	if (m_NumUniq == 0)
		return false;
	if (m_VtxMap->GetSize() < 1)
		return false;

	int32			i = 0, t;
	PSVtxArray		vtxcopy(*this);
	VXIntArray		vtxmap(*((VXIntArray*) m_VtxMap));
	VtxMapEntry		m;
	ConstIter		citer(this);
	Iter			iter(vtxcopy);
	VXObjLock		lock(this);
	VXVec3*			vptr;

	while (vptr = (VXVec3*) iter.Next())
	{
		if (i >= remap->GetSize())
			continue;
		t = remap->GetAt(i);
		iter.CopyVtx(citer.GetVtx(t));
	}
	for (i = 0; i < m_VtxMap->GetSize(); ++i)
	{
		m.All = vtxmap.GetAt(i);
		if (int32(m.Map.VtxOfs) >= remap->GetSize())
			continue;
		int32 r = remap->GetAt(m.Map.VtxOfs);
		m.Map.VtxOfs = r;
		m.Map.Group = 0;
		m_VtxMap->SetAt(i, m.All);
	}
	return true;
}

void PSVtxAnim::SetNormals(const VXArray<VXVec3>& normals)
{
	PSVtxPool::Iter iter(this);
	int i = 0;
	int s = normals.GetSize();
	VX_ASSERT(normals.GetSize() == GetNumVtx());
	while (iter.Next())
	{
		VXVec3* nml = iter.GetNormal();
		VX_ASSERT(nml);
		*nml = normals.GetAt(i++);
	}
}
