/****
 *
 * VertexCache implementation of PSVtxArray that shares a single
 * list of unique locations among multiple vertex groups, each
 * group being maintained as a separate mesh. The relationship between
 * the master list of locations and the individual groups is
 * specified in the form of vertex mapping tables for each group.
 *
 ****/
#include "vixen.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(VertexCache, SharedObj, VX_VtxCache);

static const TCHAR* opnames[] =
	{ TEXT("SetLocations"), TEXT("SetFirstVtx"), TEXT("SetUniqLocs"), TEXT("SetVtxUse"), TEXT("SetVtxMap"), };

const TCHAR** VertexCache::DoNames = opnames;


float VertexCacheEntry::MaxDiff = VX_EPSILON;

VertexCache::VertexCache(intptr nuniq)
{
	if (nuniq > 0)
		m_VertexMap = new IntArray(nuniq);
}


/*!
 * @fn void VertexCache::SetLocations(VertexArray* verts)
 * @param verts	vertex array containing unique vertex locations
 *
 * This function adds all the unique locations to this dictionary.
 * The internal vertex map will map vertices added with AddVertex
 * to these unique locations.
 */
void VertexCache::SetLocations(VertexArray* verts)
{

	if (verts == NULL)
		return;
	VertexArray::ConstIter iter(verts);
	const Vec3* vptr;
	int			vindex = 0;
	intptr		nverts = verts->GetNumVtx();

	if (m_VertexMap.IsNull())
		m_VertexMap = new IntArray(nverts);
	while (vptr = (Vec3*) iter.Next())
	{
		VertexCacheEntry e(vindex++);

		Set(*vptr, e);
	}
}

void VertexCache::SetLocations(FloatArray* locs)
{
	if (locs == NULL)
		return;
	const float* vdata = locs->GetData();
	intptr		nverts = locs->GetSize() / 3;

	if (m_VertexMap.IsNull())
		m_VertexMap = new IntArray(nverts);
	for (int vindex = 0; vindex < nverts; ++vindex)
	{
		VertexCacheEntry e(vindex);
		const Vec3* vptr = (const Vec3*) (vdata + vindex * 3);

		Set(*vptr, e);
	}
}

bool VertexCache::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!SharedObj::Copy(src_obj))
		return false;
	VertexCache* src = (VertexCache*) src_obj;
	if (src->IsClass(VX_VtxCache))
	{
		m_Verts = src->m_Verts;
		m_VertexMap = src->m_VertexMap;
		return true;
	}
	return false;
}

void VertexCache::SetVertices(VertexArray* verts)
{
	if (verts == m_Verts)
		return;
	m_VertexMap->Empty();
	m_Verts = verts;
}


/*!
 * @fn int VertexCache::AddVertex(const float* vertex)
 * @param vertex	vertex to add
 *
 * Adds a unique location the list. Two vertices with the same location
 * are considered duplicates if they have different normals or texture coordinates.
 * This function updates the vertex map that keeps track
 * of the duplicates for each vertex.
 *
 * @return unique offset of vertex or -1 on failure
 *
 * @see VertexCache::Compress VertexCache::GetVtxUse VertexCache::SetVtxMap
 */
intptr VertexCache::AddVertex(const float* vertex)
{
	Vec3				loc(vertex[0], vertex[1], vertex[2]);
	VertexCacheEntry*	uniqptr;
	int					uniqvtx = -1;
	intptr				vtxofs;

	if (GetVertices() == NULL)
		VX_ERROR(("ERROR: VtxCache::SetVertex vertex array is missing\n"), -1);
/*
 * Determine if we have seen this location before. If not, add it to the vertex array.
 */
	uniqptr = Find(loc);				// see if this vertex has been added
	if (uniqptr == NULL)				// vertex not found
		VX_ERROR(("VertexCache::AddVertex no matching vertex for %f, %f, %f\n", loc.x, loc.y, loc.z), -1);
	uniqvtx = uniqptr->LocID;			// location ID for all verts sharing same position
	VX_ASSERT(uniqvtx >= 0);
	vtxofs = FindVertex(vertex);		// find this exact vertex (including normal, texcoord)
	if (vtxofs >= 0)					// already have this vertex
		return vtxofs;					// don't need to add it, we are done
	vtxofs = NewVertex(uniqvtx, vertex);
	return vtxofs;
}


/*
 * int VertexCache::FindVertex(const float* vtxptr)
 * @param vtxptr	-> vertex data (location, normal, texcoords, ...)
 *
 * Find the offset of this vertex in the vertex list.
 *
 * @return offset of vertex or -1 if not found
 */
intptr VertexCache::FindVertex(const float* vtxptr)
{
    Entry*	entry;
    Vec3	key(vtxptr[0], vtxptr[1], vtxptr[2]);
    int		index = HashKey(key) & m_Mask;
	VertexArray::Iter iter(m_Verts);

    for (entry = (Entry*) m_Buckets[index];     // search all entries
         entry != NULL;                         // in our bucket
         entry = (Entry*) entry->Next)
    {
		const VertexCacheEntry& vc = entry->Value;
        
		if (vc.VtxIndex < 0)					// is a reference vertex?
			continue;
        if (iter.CompareVtx(vc.VtxIndex, vtxptr))
			return vc.VtxIndex;					// found it, return its index
    }
    return -1;
}

/*
 * intptr VertexCache::NewVertex(intptr locid, const float* vtxptr)
 * @param locid		location ID for vertex
 * @param vtxptr	-> vertex data for vertex to add
 *
 * Add this vertex at the end of the vertex list
 * @return offset of new vertex or -1 if no more memory
 */
intptr VertexCache::NewVertex(int locid, const float* vtxptr)
{
    Vec3				key(vtxptr[0], vtxptr[1], vtxptr[2]);
	VertexArray*		verts = GetVertices();
    intptr				nverts = verts->GetNumVtx();
	VertexCacheEntry	e(locid, nverts);

	if (verts->AddVertices(vtxptr, 1) == nverts)
	{
		Set(key, e);
		m_VertexMap->SetAt(nverts, locid);
		return nverts;
	}
	VX_ERROR(("ERROR: VertexCache out of memory, cannot add vertex %d\n", nverts), -1);
}

/*!
 * @fn bool VertexCache::RegenFaces(Mesh* mesh, int firstvtx)
 * Remaps the index tables for this mesh
 * based on the vertex mapping tables for this VertexCache.
 *
 * The vertices in the shared array have already been
 * compressed so that the vertices are in the order they 
 * are used by the faces. The index tables are regenerated
 * to match the vertex order.
 *
 * @see VertexCache::ShareVtx VertexCache::Compress
 */
bool VertexCache::RegenFaces(Mesh* mesh, intptr firstvtx)
{
	VertexArray*	verts = GetVertices();
	IndexArray*		faces;
	intptr				numidx;

	if ((mesh == NULL) || !mesh->IsClass(VX_Mesh))
		return false;
	if (verts == NULL)
		return false;
	numidx = mesh->GetNumIdx();
	if (numidx == 0)
		return false;
	faces = new IndexArray(numidx);
	for (intptr i = 0; i < numidx; ++i)
	{
		int32 n = (int) (mesh->GetIndex(i) + firstvtx);
		VX_ASSERT(n < INT_MAX);
		faces->Append(n);
	}
	mesh->SetIndices(faces);
	return true;
}

/****
 *
 * class VtxArray for SharedObj::Do
 *	VTX_SetVtxUse	<IntArray>
 *	VTX_SetVtxMap	<IntArray>
 *
 ****/
bool VertexCache::Do(Messenger& s, int op)
{
	bool		ret = false;
	SharedObj*	obj;

	switch (op)
	{
		case VTX_SetVertices:
		s >> obj;
		VX_ASSERT(obj && obj->IsClass(VX_VtxArray));
		SetVertices((VertexArray*) obj);
		break;

		case VTX_SetVertexMap:
		s >> obj;
		VX_ASSERT(obj && obj->IsClass(VX_IntArray));
		m_VertexMap = (IntArray*) obj;
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << VertexCache::DoNames[op - VTX_SetLocations]);
#endif
	return true;
}

/****
 *
 * class VtxCache override for SharedObj::Save()
 * The base implementation just saves the vertex maximum
 * and style. It is left to the derived class to save
 * the vertex data of the return value is non-zero.
 *
 ****/
int VertexCache::Save(Messenger& s, int opts) const
{
	int32 h = SharedObj::Save(s, opts);
	if (h < 0)
		return h;
	if (m_Verts->GetNumVtx() && (m_Verts->Save(s, opts) >= 0) && h)
		s << OP(VX_VtxCache, VTX_SetVertices) << h << m_Verts;
	if (m_VertexMap->GetSize() && (m_VertexMap->Save(s, opts) >= 0) && h)
		s << OP(VX_VtxCache, VTX_SetVertexMap) << h << m_VertexMap;
	if (h <= 0)
		return h;
	return h;
}

/****
 *
 * class VtxArray override for SharedObj::Print()
 *
 ****/
DebugOut& VertexCache::Print(DebugOut& dbg, int opts) const
{
	if ((opts & (PRINT_Data | PRINT_Children)) == 0)
		return SharedObj::Print(dbg);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
/*
 * Print vertex mapping information: For each unique location
 * print the vertex offsets associated with it in the map table
 * and their groups in the format:
 *	<uniqloc>	<group>:<vtx offset> <group>:<vtx offset> ...
 */
	if (opts & PRINT_Data)
	{
		endl(dbg << "<attr name='VertexMap'>");
		m_VertexMap->Print(dbg, opts & ~PRINT_Trailer);
		endl(dbg << "</attr>");
	}
	return SharedObj::Print(dbg, opts & PRINT_Trailer);
}


}	// end Vixen