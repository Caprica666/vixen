#include "vixen.h"

#ifndef VX_NOTHREAD
#include "computethread.h"
#endif

namespace Vixen {

VX_IMPLEMENT_CLASSID(Deformer, Engine, VX_Deformer);

Deformer::Deformer() : Engine()
{
}

/*
 * Copy the source vertices (neutral positions & normals) into the active verts.
 */
bool Deformer::Reset()
{
	VertexArray::ConstIter	srciter(m_RestLocs);
	intptr			n = m_RestLocs->GetNumVtx();
	const Vec3*		srcloc;
	float*			dst;

	if (m_ActiveLocs == NULL)
		return false;
	dst = (float*) m_ActiveLocs->GetData();
	VX_ASSERT(m_ActiveLocs->GetSize() == n * 3);
	while (srcloc = (const Vec3*) srciter.Next())
	{
		*dst++ = srcloc->x;
		*dst++ = srcloc->y;
		*dst++ = srcloc->z;
	}
	if (!m_ActiveNormals.IsNull())
		if (!m_RestNormals.IsNull())
		{
			const float* src = m_RestNormals->GetData();
			dst = (float*) m_ActiveNormals->GetData();

			n = m_ActiveNormals->GetSize();
			memcpy(dst, src, n * sizeof(float));
		}
		else if (srciter.HasNormals())
		{
			srciter.Reset();
			dst = (float*) m_ActiveNormals->GetData();
			VX_ASSERT(m_ActiveNormals->GetSize() == m_RestLocs->GetNumVtx() * 3);
			while (srciter.Next())
			{
				srcloc = srciter.GetNormal();
				*dst++ = srcloc->x;
				*dst++ = srcloc->y;
				*dst++ = srcloc->z;
			}
		}
	return true;
}

bool Deformer::GetBound(Box3& b) const
{
	b.Set(0, 0, 0, 0, 0, 0);
	if (m_ActiveLocs.IsNull())
		return false;
	if (m_ActiveLocs->GetData())
	{
		b.Extend((const Vec3*) m_ActiveLocs->GetData(), m_ActiveLocs->GetSize() / 3);
		return true;
	}
	return false;
}

/*
 * Save the normals from the target vertex array as the rest normals.
 */
int Deformer::InitRestNormals()
{
	VertexArray::ConstIter	iter(m_TargetVerts);
	intptr	n = m_TargetVerts->GetNumVtx();
	int		i = 0;
	float*	dstdata;

	if ((m_TargetVerts->GetStyle() & VertexPool::NORMALS) == 0)
	{
		m_RestNormals = NULL;
		return 0;
	}
	if (m_RestNormals.IsNull())
		m_RestNormals = new FloatArray(n * 3);
	m_RestNormals->SetSize(n * 3);
	dstdata = m_RestNormals->GetData();
	while (iter.Next())
	{
		const Vec3* srcnml = iter.GetNormal();
		Vec3* dstnml = (Vec3*) (dstdata + i * 3);
		*dstnml = *srcnml;
		++i;
	}
	return i;
}

/*!
 * @fn void Deformer::SetRestLocs(VertexArray* locs)
 * @param locs	vertex array with locations to use as input to deformation.
 *				represents the rest positions before deformation.
 *				Also contains bone indices and weights for each vertex.
 *
 * This array contains a set of rest locations. It does not necessarily
 * have the same ordering as the target mesh. If no rest locations are supplied,
 * the rest pose is initialized with the locations of the target mesh and they
 * will have the same ordering. Otherwise, a mapping table is computed between the rest
 * locations and the target mesh vertices.
 *
 * The rest pose locations define the neutral shape  with no deformations.
 * For each vertex there is a fixed set of bones that may influence that vertex
 * along with a weight for each bone. These weights should add up to 1.0.
 * Deformers compute their results relative to this neutral shape and use the
 * bone weights to determine how the bone movement influences the final deformation.
 *
 * @see Deformer::MakeVertexMap
 */
void Deformer::SetRestLocs(VertexArray* locs)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Deformer, DEFORM_SetRestLocs) << this << locs;
	VX_STREAM_END(  )

	m_RestLocs = locs;
}

/*!
 * @fn void Deformer::SetRestNormals(FloatArray* nmls)
 * @param nmls	array of normals to use as input to deformation.
 *				represents the rest normals before deformation.
 *
 * This array contains a set of rest normals. It does has the same
 * ordering as the target mesh. If no rest normals are supplied,
 * the rest pose is initialized with the normals of the target mesh.
 *
 * The rest pose normals define the neutral shape  with no deformations.
 * Deformers compute their results relative to this neutral shape.
 * The entries in the rest normals array are used as input by the
 * Deformer to update the vertex array each frame.
 *
 * Unlike the rest locations, the rest normals correspond exactly to the
 * normals in the target mesh and do not require the vertex mapping table.
 *
 * @see Deformer::SetRestLocs
 */
void Deformer::SetRestNormals(FloatArray* nmls)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Deformer, DEFORM_SetRestNormals) << this << nmls;
	VX_STREAM_END(  )

	m_RestNormals = nmls;
}

/*!
 * @fn void Deformer::SetVertexMap(const IntArray* vmap)
 * @param vmap	vertex mapping array
 *
 * If you have a set of unique locations for deformation but would like multiple texture
 * coordinates for a single location, these vertices must be duplicated in the mesh for rendering.
 * To deform the rest pose locations but render with a different vertex order
 * or number of vertices, you can provide a vertex mapping table that indicates which rest
 * location corresponds to which vertex in the target mesh.
 *
 * If this table is not supplied, it is assumed the rest locations directly
 * correspond to the target mesh vertices. Rest normals ALWAYS match the target mesh -
 * the vertex map only applies to locations.
 *
 * @see Deformer::MakeVertexMap Deformer::SetRestLocs Deformer::SetTarget
 */
void Deformer::SetVertexMap(IntArray* vmap)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Deformer, DEFORM_SetVertexMap) << this << vmap;
	VX_STREAM_END(  )

	m_VertexMap = vmap;
}

/*!
 * @fn bool Deformer::MakeVertexMap(const VertexArray* verts)
 * @param verts vertices to make vertex map from
 *
 * This function allows you to deform a mesh whose vertices are in a different
 * order than the rest locations. For example, if you have a set of unique locations
 * for deformation but would like multiple texture coordinates for a single location,
 * these vertices must be duplicated in the mesh for rendering. This function constructs
 * a mapping table between the rest locations and the vertices in the mesh
 * so the rest locations and be deformed and rendering can have duplicates.
 * Once the vertex map is computed, the deformation logic uses this map to update
 * the locations in the target vertex array.
 *
 * This function will fail if it does not find a corresponding position in the
 * rest locations for each vertex in the mesh.

 * @see Deformer::SetTarget
 */
bool Deformer::MakeVertexMap(const VertexArray* verts)
{
	IntArray*		vertexmap = NULL;
	intptr			nverts;
	VertexCache*	vdict = m_VertexCache;

	if (vdict == NULL)
	{
		VertexCacheEntry::MaxDiff = 0.001f;
		m_VertexCache = vdict = MakeVertexCache();
	}
	m_VertexMap = NULL;
	if ((verts == NULL) || m_RestLocs.IsNull())
		return false;
	nverts = m_RestLocs->GetNumVtx();
	if ((verts == NULL) || (nverts == 0))
		return false;
	if (verts->GetNumVtx() == 0)
		return false;
	vertexmap = ComputeVertexMap(m_RestLocs, verts, vdict);
	if (vertexmap != NULL)
	{
		m_VertexMap = vertexmap;
		return true;
	}
	return false;
}

/*!
 * @fn IntArray* Deformer::ComputeVertexMap(const VertexArray* srclocs, const VertexArray* dstverts)
 * Constructs a mapping from destination to source locations.
 * @param srcverts	source vertices to map from
 * @param dstverts	destination vertices to map to
 *
 * This function allows you to deform a mesh whose vertices are in a different
 * order than the rest pose locations. For example, if you have a set of unique control points
 * for skinning but would like multiple texture coordinates for a single location,
 * these vertices must be duplicated in the mesh for rendering. It constructs a mapping
 * table between the neutral locations in the skin and the vertices in the mesh
 * so skinning can use the unique vertices and rendering can have duplicates.
 * Once the vertex map is computed, the skinning logic uses this map to update
 * the locations in the target vertex array.
 *
 * This function will fail if it does not find a corresponding location in the
 * source array for each destination vertex.

 * @see Deformer::SetTarget 
 */
IntArray* Deformer::ComputeVertexMap(const VertexArray* srclocs, const VertexArray* dstverts, VertexCache* vcache)
{
	intptr		nsrcverts;

	if ((dstverts == NULL) || (srclocs == NULL))
		return NULL;
	nsrcverts = srclocs->GetNumVtx();
	if (nsrcverts == 0)
		return NULL;

	ObjectLock				srclock(srclocs);
	ObjectLock				dstlock(dstverts);
	VertexPool::ConstIter	dst_iter(dstverts);
	intptr					ndstverts = dstverts->GetNumVtx();
	IntArray*				vertexmap = new IntArray(nsrcverts);;

	VX_ASSERT(ndstverts >= nsrcverts);
	vertexmap->SetMaxSize(nsrcverts);
	cilk_for (intptr vindex = 0; vindex < ndstverts; ++vindex)
	{
		const Vec3*	vptr = (const Vec3*) dst_iter.Next();
		int			imin = 0;
		float		mindist = FLT_MAX;

		if (vcache && vcache->Find(*vptr))
		{
			Core::Dict<Vec3, VertexCacheEntry, Vixen::BaseDict>::Entry&	entry = (*vcache)[*vptr];
			mindist = vptr->Distance(entry.Key);
			imin = entry.Value.LocID;
		}
		else
		{
			VertexPool::ConstIter	src_iter(srclocs);
			const Vec3* v;
			int i = 0;
			while (v = (const Vec3*) src_iter.Next())
			{
				float d = vptr->DistanceSquared(*v);
				if (d < mindist)
				{
					imin = (int32) i;
					mindist = d;
				}
				++i;
			}
		}
		if (mindist < 0.005f)
		{
			vertexmap->SetAt(vindex, imin);
		}
		else
			VX_WARNING(("Deformer::ComputeVertexMap: vertex %d (%f, %f, %f) not found in rest locations\n", vindex, vptr->x, vptr->y,vptr->z));
	}
	return vertexmap;
}


VertexCache* Deformer::MakeVertexCache()
{
	VertexArray*	uniqlocs = m_RestLocs;
	VertexCache*	vcache;
	intptr			nverts;

	if (uniqlocs == NULL)
		return NULL;
	nverts = uniqlocs->GetNumVtx();
	if (nverts == 0)
		return NULL;
	vcache = new VertexCache(nverts);
	vcache->SetLocations(uniqlocs);
	m_VertexCache = vcache;
	return vcache;
}

void Deformer::SetTarget(SharedObj* obj)
{
	ObjectLock		lock(this);
	SharedObj*		oldtarg = GetTarget();

	Engine::SetTarget(obj);
	if (obj == NULL)
	{
		m_TargetVerts = NULL;
		m_TargetMesh = NULL;
		m_VertexMap = NULL;
		return;
	}
	if (oldtarg != obj)
	{
		m_TargetVerts = NULL;
		m_TargetMesh = NULL;
	}
	Init(obj);
}

int Deformer::Save(Messenger& s, int opts) const
{
	int32 h = Engine::Save(s, opts);
	const SharedObj* obj;

	if (h < 0)
		return h;
	obj = m_RestLocs;
	if (obj && (obj->Save(s, opts) >= 0) && h)
		s << OP(VX_Deformer, DEFORM_SetRestLocs) << h << obj;
	obj = m_RestNormals;
	if (obj && (obj->Save(s, opts) >= 0) && h)
		s << OP(VX_Deformer, DEFORM_SetRestNormals) << h << obj;
	obj = m_VertexMap;
	if (obj && (obj->Save(s, opts) >= 0) && h)
		s << OP(VX_Deformer, DEFORM_SetVertexMap) << h << obj;
	return h;
}

bool Deformer::Do(Messenger& s, int op)
{
	SharedObj*	obj;

	switch (op)
	{
		case DEFORM_SetRestLocs:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_VtxArray));
		SetRestLocs((VertexArray*) obj);
		break;

		case DEFORM_SetRestNormals:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_FloatArray));
		SetRestNormals((FloatArray*) obj);
		break;

		case DEFORM_SetVertexMap:
		s >> obj;
		SetVertexMap((IntArray*) obj);
		break;

		default:
		return Engine::Do(s, op);
	}

	return true;
}

DebugOut& Deformer::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	if (opts & PRINT_Data)
	{
		if (!m_VertexMap.IsNull())
		{
			endl(dbg << "<attr name='VertexMap'>");
			m_VertexMap->Print(dbg, opts & ~PRINT_Trailer);
			endl(dbg << "</attr>");
		}
		if (!m_RestLocs.IsNull())
		{
			endl(dbg << "<attr name='RestLocs'>");
			m_RestLocs->Print(dbg, opts & ~PRINT_Trailer);
			endl(dbg << "</attr>");
		}
	}
	return Engine::Print(dbg, opts & PRINT_Trailer);
}

VertexArray* Deformer::ValidateTarget(SharedObj* target)
{
	VertexArray*	dstlocs = m_TargetVerts;

	if (target == NULL)
	{
		target = GetTarget();
		if (target == NULL)
			target = Parent();		// Use target vertices from parent skin
	}
	if (target == NULL)
		return NULL;
	if (target->IsClass(VX_Deformer))
		dstlocs = ((Deformer*) target)->GetRestLocs();
	else if (target->IsClass(VX_VtxArray))
		dstlocs = (VertexArray*) target;
	else if (target->IsClass(VX_Mesh))
	{
		m_TargetMesh = (TriMesh*) target;
		dstlocs = ((Mesh*) target)->GetVertices();
	}
	else if (target->IsClass(VX_Shape))
	{
		Geometry* geo = ((Shape*) target)->GetGeometry();
		if (geo && geo->IsClass(VX_Mesh))
		{
			((Shape*) target)->SetHints(Model::MORPH);
			m_TargetMesh = geo;
			dstlocs = ((Mesh*) geo)->GetVertices();
		}
	}
	if (dstlocs == NULL)
		return NULL;
	if (dstlocs->GetNumVtx() == 0)
		return NULL;
	dstlocs->SetFlags(VertexPool::MORPH);
	m_TargetVerts = dstlocs;
	dstlocs->MakeLock();
	return dstlocs;
}

/*
 * @fn bool Deformer::Init(SharedObj* target, VertexCache* vcache)
 * Initializes the deformer internal state.
 * @param	target	target Shape or Mesh to defor
 * @param	vcache	vertex cache containing unique vertices in target vertedx array
 *
 * If the number of vertices in the target mesh does not match the amount
 * in the rest locations (if any have been supplied), this function computes
 * an internal array that maps a vertex in the target mesh onto a vertex in the
 * rest locations. The vertex cache, if supplied, is used to optimize the
 * construction of the vertex map.
 *
 * If no rest locations are supplied, they are copied from the target vertex array
 * and no vertex map is used.
 *
 * @return true if initialization successful, else false
 */
bool Deformer::Init(SharedObj* target, VertexCache* vcache)
{
	VertexArray*		dstlocs = m_TargetVerts;
	const VertexArray*	srclocs = (const VertexArray*) m_RestLocs;
	intptr				nverts;

	if (dstlocs == NULL)
		dstlocs = ValidateTarget(target);	
	if (dstlocs == NULL)
		return false;
	nverts = dstlocs->GetNumVtx();
	if (vcache)
		m_VertexCache = vcache;
	if (srclocs)
	{
		if (m_VertexMap.IsNull() && (srclocs->GetNumVtx() != nverts))
			MakeVertexMap(dstlocs);
	}
	else
		m_RestLocs = dstlocs;
	return Reset();
}

bool Deformer::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Engine::Copy(src_obj))
		return false;
	const Deformer* src = (const Deformer*) src_obj;
	if (src->IsClass(VX_Deformer))
    {
		m_VertexMap = src->m_VertexMap;
		m_RestLocs = src->m_RestLocs;
		m_RestNormals = src->m_RestNormals;
		if (src->m_ActiveLocs.IsNull())
			m_ActiveLocs = NULL;
		else
			m_ActiveLocs = src->m_ActiveLocs->Clone();
		if (src->m_ActiveNormals.IsNull())
			m_ActiveNormals = NULL;
		else
			m_ActiveNormals = src->m_ActiveNormals->Clone();
   }
    return true;
}

float	Deformer::ComputeTime(float t)
{
	float f = Engine::ComputeTime(t);

	if (f < 0.0f)
		return f;
	if (!Init(m_Target))
		return -1.0f;
	return f;
}

bool Deformer::Eval(float t)
{
	if (!m_ActiveLocs.IsNull() && !m_TargetVerts.IsNull())
	{
		if (!UpdateVertices(m_ActiveLocs, m_TargetVerts, m_VertexMap, m_ActiveNormals))
		{
			TriMesh* mesh = (TriMesh*) (Mesh*) m_TargetMesh;

			if (mesh && mesh->IsClass(VX_TriMesh))
				mesh->MakeNormals();
		}
	}
	return true;
}

/*!
 * @fn bool Deformer::UpdateVertices(const FloatArray* srclocs, VertexArray* dstverts, const IntArray* vmap, const FloatArray* srcnmls)
 * Copy the source vertices computed from skinning into the target vertex array using the vertex map.
 * @param	srclocs		vertex array with source locations
 * @param	dstverts	destination vertex array
 * @param	vmap		maps destination locations to source locations
 *						if NULL they are assumed to directly correspond
 * @param	srcnlms		array with source normals which directly correspond to destination normals
 */
bool Deformer::UpdateVertices(const FloatArray* srclocs, VertexArray* dstverts, const IntArray* vmap, const FloatArray* srcnmls)
{
	if (dstverts == NULL)
		return true;

	ObjectLock			dstlock(dstverts);
	VertexPool::Iter	dstiter(dstverts);
	const float*		srcdata = srclocs->GetData();
	bool				copynormals = dstiter.HasNormals();
	bool				calcnormals = dstiter.HasNormals();
	Vec3*				dst;
	const Vec3*			src;
	int					i = 0;
 
	if (srcnmls)
		calcnormals = false;			// we will copy the normals
	else
		copynormals = false;			// caller should calculate normals
	if (vmap)							// do we have a vertex map?
	{
		while (dst = (Vec3*) dstiter.Next())
		{
			int		vindex = vmap->GetAt(i++);

			src = (const Vec3*) (srcdata + vindex * 3);
			*dst = *src;
		}
	}
	else								// copy positions directly
	{
		while (dst = (Vec3*) dstiter.Next())
		{
			src = (const Vec3*) (srcdata + i * 3);
			++i;
			*dst = *src;
		}
	}
	if (copynormals)					// copy normals?
	{
		const float* nmldata = srcnmls->GetData();
		
		i = 0;
		dstiter.Reset();
		VX_ASSERT(srcnmls->GetSize() == dstverts->GetNumVtx() * 3);
		while (dstiter.Next())
		{
			src = (const Vec3*) (nmldata + i * 3);
			dst = dstiter.GetNormal();
			*dst = *src;
			++i;
		}
	}
	dstverts->SetChanged(true);
	return !calcnormals;
}
}	// end Vixen