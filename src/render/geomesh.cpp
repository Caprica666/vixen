#include "vixen.h"

namespace Vixen {

/****
 *
 * prim_name returns the name of the primitive as an ascii string
 * using the primitive type as an index. These must follow the
 * GEO_xxx ordering
 *
 ****/
static const TCHAR* geonames[] =
{	TEXT("Transform"),	TEXT("SetBound") };

static const TCHAR* meshnames[] =
{	TEXT("SetVertices"),TEXT("SetIndices"),	TEXT("SetStyle"),	TEXT("AddVertices"),
	TEXT("SetStartVtx"),TEXT("SetEndVtx")
};

const TCHAR** Mesh::DoNames = meshnames;

const TCHAR** Geometry::DoNames = geonames;



VX_IMPLEMENT_CLASSID(Geometry, SharedObj, VX_Geometry);

VX_IMPLEMENT_CLASSID(Mesh, Geometry, VX_Mesh);

Geometry::Geometry(const Geometry& src) : SharedObj(src)
{
	DevHandle = NULL;
	Copy(&src);
}

/*!
 * @fn bool Geometry::Hit(const Ray& ray, float* dist, TriHitEvent* hitinfo) const
 * @param ray		ray to hit test against
 * @param distance	returns distance between ray and intersection point here
 * @param hitinfo	returns triangle hit information here
 *
 * Determines whether a ray hit the geometry,
 * and where the intersection point was.
 * The base implementation just tests intersection against the
 * axially aligned bounding volume of the geometry.
 *
 * @return  true if ray intersected any geometry, else  false
 *
 * @see Ray Model::Hit TriHitEvent
 */
bool Geometry::Hit(const Ray& ray, TriHitEvent* hitinfo) const
{
	Box3	bvol;
	float	distance;
	Vec3	intersect;

	if (!GetBound(&bvol))
		return false;
	if (hitinfo == NULL)
		return bvol.Hit(ray, distance, NULL);
	if (bvol.Hit(ray, distance, &intersect) && (distance < hitinfo->Distance))
	{
		hitinfo->Target = (Mesh*) this;
		hitinfo->Distance = distance;
		hitinfo->Intersect = intersect;
		hitinfo->TriIndex = -1;
		return true;
	}
	return false;
}


/*!
 * @fn void Geometry::Touch()
 *
 * Notifies the system that this geometry was modified.
 * Invalidates local cached information. You should
 * always call this function after you have changed
 * vertex or index data.
 */
void Geometry::Touch()
{
	SetChanged(true);
}



/*!
 * @fn intptr Geometry::Cull(const Matrix* trans, Scene* scene)
 * @param scene	scene the geometry is being rendered to, NULL suppresses culling
 * @param trans	transformation matrix to map from local coordinates to camera coordinates
 *
 * Determines whether or not this geometry is culled during rendering.
 * The default implementation turns the bounding box of the geometry
 * into a bounding sphere and asks the scene's camera if it is visible.
 * If you are building your own geometry class, you can override this
 * routine to implement a geometry-specific culling algorithm or just to
 * visit the geometry prior to rendering.
 *
 * If the scene argument is NULL, the mesh should be marked as not culled and
 * no culling tests should be done. This form of the routine is used when the
 * shape which owns the geometry is marked as "never cull".
 *
 * @return number of vertices culled or 0 if nothing culled
 *
 * @see Model::Cull Camera Model::SetCulling
 */
intptr Geometry::Cull(const Matrix* trans, Scene* scene)
{
	Box3		box;
	Sphere	bsp;

	if (!IsActive())						// user marked it as culled?
	{
		SetFlags(GEO_Culled);
		return (int) GetNumVtx();
	}
	ClearFlags(GEO_Culled);					// mark mesh as not culled
	if (scene == NULL)						// don't need to check culling
		return 0;
	if (!GetBound(&box))					// convert bounding box to sphere
		return 0;
	bsp = box;
	if (trans)
		bsp *= *trans;						// transform to camera coords
	if (scene->GetCamera()->IsVisible(bsp))	// is it visible?
		return 0;							// return no verts culled
	return GetNumVtx();						// return number of verts culled
}

/*
 * @fn bool Geometry::Do(Messenger& s, int op)
 * Loads the next Geometry opcode from the input messenger
 * and updates the mesh.
 *
 * @code
 *	GEO_Transform		<Transform*>
 * @endcode
 *
 * @return  true if operation was successful, else  false
 *
 * @see SharedObj::Do
 */
bool Geometry::Do(Messenger& s, int op)
{
	SharedObj*			obj;
	const Matrix*	trans;
	Opcode			o = Opcode(op);	// for debugging

	switch (op)
	{
		case GEO_Transform:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Matrix));
		trans = (const Matrix*) obj;
		*this *= *trans;
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Geometry::DoNames[op - GEO_Transform]
					   << " " << this);
#endif
	return true;
}



/*!
 * @fn Mesh::Mesh(int style, intptr nvtx)
  * @param style			style of vertices
 * @param nvtx			number of vertices to make room for initially
 *
 * Constructs an non-indexed mesh with a vertex pool of the
 * given size and style. Initially the mesh will have
 * no index table. If you add primitives with index data
 * the index table is automatically created for you.
 *
 * @see VertexArray Geometry Mesh::SetIndices
 */
Mesh::Mesh()
	: Geometry()
{
	m_Bound.Empty();
	m_Verts = new VertexArray();
	m_StartVtx = m_EndVtx = 0;
}

Mesh::Mesh(int style, intptr nvtx)
	: Geometry()
{
	m_Bound.Empty();
	m_Verts = new VertexArray(style, nvtx);
	m_StartVtx = m_EndVtx = 0;
}

/*!
 * @fn Mesh::Mesh(const TCHAR* layout_desc, intptr nvtx)
 * @param layout_desc	string describing layout of vertices
 * @param nvtx			number of vertices to make room for initially
 *
 * Constructs an non-indexed mesh with a vertex pool of the
 * given size and layout. Initially the mesh will have
 * no index table. If you add primitives with index data
 * the index table is automatically created for you.
 *
 * @see VertexArray Geometry Mesh::SetIndices
 */
Mesh::Mesh(const TCHAR* layout_desc, intptr nvtx)
	: Geometry()
{
	m_Bound.Empty();
	m_Verts = new VertexArray(layout_desc, nvtx);
	m_StartVtx = m_EndVtx = 0;
}

/*!
 * @fn Mesh::Mesh(const Mesh& src)
 *
 * Constructs a new mesh which shares the vertices and index table,
 * of the source mesh. This is different from the
 * behavior of Mesh::Copy which duplicates the vertices and indices.
 *
 * @see VertexArray Geometry Mesh::SetIndices
 */
Mesh::Mesh(const Mesh& src) : Geometry(src)
{
	m_Verts = src.m_Verts;
	m_StartVtx = src.m_StartVtx;
	m_EndVtx = src.m_EndVtx;
	m_Bound = src.m_Bound;
	if (src.GetNumIdx())
		m_VtxIndex = src.m_VtxIndex;
}

void Mesh::Empty()
{
// TODO: add stream logging here?
	Lock();
	m_Bound.Empty();
	m_Verts->SetNumVtx(0);
	m_VtxIndex = (IndexArray*) NULL;
	m_StartVtx = 0;
	m_EndVtx =0;
	Touch();
	Unlock();
}


/*!
 * @fn bool Mesh::SetStartVtx(long svtx)
 *
 * The start vertex is the minimum vertex index used by this mesh.
 * If all the vertices in the vertex array are used by this mesh,
 * 0 is returned.
 *
 * @see Mesh::GetNumVtx Mesh::SetEndVtx
 */
bool Mesh::SetStartVtx(intptr svtx)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Mesh, MESH_SetStartVtx) << this << int32(svtx);
	VX_STREAM_END( )

	m_StartVtx = svtx;
	return true;
}

/*!
 * @fn bool Mesh::SetEndtVtx(intptr svtx)
 *
 * The start vertex is the maximum vertex index used by this mesh.
 * If all the vertices in the vertex array are used by this mesh,
 * the index of the last vertex is returned.
 *
 * @see Mesh::GetNumVtx Mesh::SetEndVtx
 */
bool Mesh::SetEndVtx(intptr e)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Mesh, MESH_SetEndVtx) << this << int32(e);
	VX_STREAM_END( )

	if (e < 0)
		return false;
	m_EndVtx = e;
	return true;
}

/*!
 * @fn bool Mesh::AddIndex(intptr vtxidx)
 * @param vtxidx	vertex index to add to index table
 *
 * Adds an index to the index array for this mesh.
 * An add operation will cause the creation of an index array if
 * one does not exist.
 *
 * @return  true if vertex successfully added, else  <false
 *
 * @see Mesh::SetIndex
 */
bool Mesh::AddIndex(intptr v)
{
	IndexArray* idx = (IndexArray*) m_VtxIndex;
	intptr		n;

	if (idx)
		n = idx->GetSize();
	else
		n = 0;
	return SetIndex(n,  v);
}

/*!
 * @fn bool Mesh::SetIndex(intptr i, intptr vtxidx)
 * @param i			zero based offset into index array
 * @param vtxidx	zero based index into vertex array
 *
 * Updates one of the indices in the index array for this mesh.
 * A set operation will cause the creation of an index array if
 * one does not exist and will fail (return false) if one
 * cannot be created.
 *
 * @see Mesh::SetIndices Mesh::AddIndex
 */
bool Mesh::SetIndex(intptr i, intptr v)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Mesh, MESH_SetIndex) << this << int32(i) << int32(v);
	VX_STREAM_END( )

	IndexArray* idx = (IndexArray*) m_VtxIndex;
	if (idx == NULL)
		if ((idx = new IndexArray(i + 1)) == NULL)
			return false;
	m_VtxIndex = idx;
	SetChanged(true);
	VX_ASSERT(v < INT_MAX);
	return idx->SetAt(i, (int32) v);
}

/*!
 * @fn void Mesh::SetIndices(IndexArray* inds)
 *
 * Indices are stored in a contiguous block of integers.
 * They are zero-based vertex numbers which reference individual vertices
 * in the vertex array. The index array is enlarged dynamically as primitives
 * are added. Multiple meshes may share the same index array.
 *
 * @note index values may be either short or long depending on
 * the operating system and display device.
 *
 * @see Mesh::SetStyle Mesh::SetVertices
 */
void Mesh::SetIndices(IndexArray* inds)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Mesh, MESH_SetIndices) << this << inds;
	VX_STREAM_END( )

	m_VtxIndex = inds;
}


/*!
 * @fn int Mesh::AddIndices(const int32* idx, long n)
 * @param idx	-> first index in the list, if NULL space is allocated
 *				for indices but no data is copied
 * @param n		number of indices to add
 *
 * Adds indices to the index array of a mesh.
 * The index array is enlarged or allocated if necessary.
 * Does not affect primitive or vertex data. The internal
 * organization of the index array may not be as
 * a contiguous area of 32 bit integers. On some platforms,
 * the indices may be 16 bits and not contiguous.
 *
 *
 * @return offset of first index added, negative on error
 *
 * @see Mesh::AddVertices Mesh::SetIndex
 */
intptr Mesh::AddIndices(const int32* idx, intptr n)
{
	intptr	ofs;

	VX_STREAM_BEGIN(s)
	if (idx)
	{
		int32	maxinds = (BufMessenger::MaxBufSize - 3 * sizeof(int32)) / sizeof(int32);
		intptr	m = n;
		while (m > 0)
		{
			if ((int32) m < maxinds)
			{
				VX_ASSERT(m < INT_MAX);
				maxinds = (int32) m;
			}
			*s << OP(VX_Mesh, MESH_AddIndices) << this << maxinds;
			s->Output(idx, maxinds);
			idx += maxinds;
			m -= maxinds;
		}
	}
	VX_STREAM_END( )

	if (m_VtxIndex.IsNull())
	{
		m_VtxIndex = new IndexArray(n);
		ofs = 0;
	}
	else ofs = m_VtxIndex->GetSize();
	if (!m_VtxIndex->SetSize(n + ofs))
		return -1;
	if (idx == NULL)
		return ofs;
	VertexIndex*	iptr = (VertexIndex*) m_VtxIndex->GetData();
	iptr += ofs;
	#pragma omp PARALLEL_FOR(n)
	cilk_for (intptr i = 0; i < n; ++i)
		iptr[i] = VertexIndex(idx[i]);
	return ofs;
}

/*!
 *
 * @fn Mesh::SetVertices(VertexArray* vtx)
 * @param vtx	new vertex array to use
 *
 * Replaces the vertex array for this mesh with the input
 * vertex array. The original vertex data is garbage collected
 * if it is no longer used. A single vertex array may be shared
 * among multiple meshes.
 *
 * @see Mesh::AddVertices Mesh::SetIndices
 */
void Mesh::SetVertices(VertexArray* vtx)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_Mesh, MESH_SetVertices) << this << vtx;
	VX_STREAM_END( )

	m_Verts = vtx;
	vtx->SetChanged(true);
}

/*!
 * @fn long Mesh::AddVertices(const float* v, long n)
 * @param v	vertex data to add
 * @param n	number of vertices in data area
 *
 * The vertex style determines the format of the
 * input vertex data, i.e. which components of the vertex are updated.
 * The order of vertex components in the input array is location,
 * normal, color, texture coordinates. Only the components designated
 * by the vertex style should be in the input data array. For example,
 * for a mesh with vertex style VertexPool::NORMALS | VertexPool::TEXCOORDS, the
 * input data should store 8 floats for each vertex - the location (3),
 * the normal (3) and the texture coordinates (2) in that order.
 *
 * If there is not enough room for the new data and the vertex array
 * cannot be enlarged, this call will fail.
 *
 * @return zero based index of first vertex added or -1 on failure
 *
 * @see Mesh::Merge Mesh::SetVertices Mesh::SetStyle
 */
intptr Mesh::AddVertices(const float* v, intptr n)
{
	return m_Verts->AddVertices(v, n);
}

/*
 * @fn bool Mesh::Do(Messenger& s, int op)
 *
 * Loads the next Mesh opcode from the input messenger
 * and updates the mesh.
 *
 * @code
 *	MESH_Transform		<Transform*>
 *	MESH_AddVertices	<int32 n> <float [ ]>
 *	MESH_AddIndices		<int32 n> <int32 [ ]>
 *	MESH_SetIndex		<int32>
 * @endcode
 *
 * @return  true if operation was successful, else  false
 *
 * @see SharedObj::Do
 */
bool Mesh::Do(Messenger& s, int op)
{
	SharedObj*		obj;
	int32			vs, n, v;
	int32*			inds;
	float*			vtx;
	Opcode			o = Opcode(op);	// for debugging

	switch (op)
	{
		case MESH_SetStartVtx:
		s >> n;
		SetStartVtx(n);
		break;

		case MESH_SetIndex:
		s >> n >> v;
		SetIndex(n, v);
		break;

		case MESH_SetEndVtx:
		s >> n;
		SetEndVtx(n);
		break;

		case MESH_AddIndices:
		s >> n;
		inds = (int32*) Core::ThreadAllocator::Get()->Alloc(sizeof(int32) * n);
		s.Input((int32*) inds, n);
		AddIndices(inds, n);
		Core::ThreadAllocator::Get()->Free(inds);
		break;

		case MESH_AddVertices:
		vs = GetVtxSize();
		s >> n;
		vtx = (float*) Core::ThreadAllocator::Get()->Alloc(vs * n * sizeof(float));
		s.Input(vtx, n * vs);
		VX_ASSERT(0);					// deprecated
		AddVertices(vtx, n);
		Core::ThreadAllocator::Get()->Free(vtx);
		break;

		case MESH_SetVertices:
		s >> obj;
		if (obj)
		   { VX_ASSERT(obj->IsClass(VX_VtxArray)); }
		SetVertices((VertexArray*) obj);
		break;

		default:
		return Geometry::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << Mesh::DoNames[op - MESH_SetVertices]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class Mesh override for SharedObj::Save
 *
 ****/
int Mesh::Save(Messenger& s, int opts) const
{
	intptr	n = GetNumVtx();
	int32	h = Geometry::Save(s, opts);
	const VertexArray*	verts = GetVertices();

	if (h < 0)
		return h;
	//if (h)
	//	s << OP(VX_Mesh, MESH_SetStyle) << h << int32(GetStyle());
	if (n && verts && (verts->Save(s, opts) >= 0) && h)
		s << OP(VX_Mesh, MESH_SetVertices) << h << verts;
	if (h <= 0)
		return h;
	if (!m_VtxIndex.IsNull())
	{
		int32 maxinds = (BufMessenger::MaxBufSize - 3 * sizeof(int32)) / sizeof(int32);
		intptr startidx = 0;
		n = m_VtxIndex->GetSize();
		while (n > 0)
		{
			if ((int32) n < maxinds)
			{
				VX_ASSERT(n < INT_MAX);
				maxinds = (int) n;
			}
			s << OP(VX_Mesh, MESH_AddIndices) << h << maxinds;
			for (int i = 0; i < maxinds; ++i)
				s << int32(GetIndex(startidx++));
			n -= maxinds;
		}
	}
	if (m_StartVtx > 0)
		s << OP(VX_Mesh, MESH_SetStartVtx) << h << int32(m_StartVtx);
	if (m_EndVtx < VertexIndex(GetNumVtx() - 1))
		s << OP(VX_Mesh, MESH_SetEndVtx) << h << int32(m_EndVtx);
	return h;
}

/****
 *
 * class Mesh override for SharedObj::Print
 *
 ****/
DebugOut& Mesh::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return Geometry::Print(dbg, opts);
	Geometry::Print(dbg, opts & ~PRINT_Trailer);
	intptr	n = GetNumVtx();
	if (n > 0)
		m_Verts->Print(dbg, opts);			// print vertex list
	if (m_StartVtx != 0)
		endl(dbg << "<attr='StartVtx'>" << m_StartVtx << "</attr>");
	if (m_EndVtx != 0)
		endl(dbg << "<attr='EndVtx'>" << m_EndVtx << "</attr>");
	n = GetNumIdx();
	if (n > 0)
	{
		endl(dbg << "<attr='Indices' size='" << n << "'>");
		if (opts & PRINT_Data)
			for (intptr i = 0; i < n; ++i)				/* for each index */
			{
				dbg << GetIndex(i) << " ";
				if ((i % 16) == 15)					/* new line every 16 indices */
					endl(dbg);
			}
		endl(dbg << "</attr>");
	}
	Geometry::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

/****
 *
 * Override for SharedObj::Copy
 *
 ****/
bool Mesh::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Geometry::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_Mesh))
	{
		const Mesh*	src = (const Mesh*) src_obj;

		m_Verts = (VertexArray*) src->m_Verts->Clone();
		if (!src->m_VtxIndex.IsNull())
			m_VtxIndex = (IndexArray*) src->m_VtxIndex->Clone();
	}
	return true;
}

/*!
 * @fn bool TriMesh::TriHit(const Ray& ray, const Vec3& V0, const Vec3& V1, const Vec3& V2, Vec3* intersect)
 * @param ray	ray to hit test with
 * @param V0	first vertex of triangle to test
 * @param V1	second vertex of triangle to test
 * @param V2	third vertex of triangle to test
 * @param intersect pointer to where to store point of intersection
 *
 * Determines whether or not the input triangle was hit by the
 * specified ray and calculates the intersection point. This algorithm
 * was taken from Graphics Gems I p 393
 */
bool TriMesh::TriHit(const Ray& ray, const Vec3& V0, const Vec3& V1, const Vec3& V2, Vec3* intersect)
{
//
// determine where ray intersects plane of triangle
// RAY(t) = ray.start + t * ray.direction
// triangle plane equation: N . P - d = 0
// intersection of ray with plane at t = -(d + N . ray.start) / (N . ray.direction)
//
	Vec3	V0V1(V1 - V0);
	Vec3	V0V2(V2 - V0);
	Vec3	n(V0V1.Cross(V0V2));
	Vec3	inter(ray.start);	// intersection point
	float	d = -V0.Dot(n);		// distance of triangle plane from origin
	float	p = n.Dot(ray.direction);
	float	t;

	if ((p <= VX_EPSILON) && (p >= -VX_EPSILON))
		return false;			// ray parallel to plane, no intersection
	t = -(d + n.Dot(ray.start));
	t /= p;
	if (t <= 0)					// intersection not on ray
		return false;
	inter += ray.direction * t;	// point of intersection
//
// figure out primary projection plane
//
	float	nmax = fabs(n.x);
	int		i1 = 1;
	int		i2 = 2;
	if (fabs(n.y) > nmax)
		{ i1 = 2; i2 = 0; nmax = fabs(n.y); }
	if (fabs(n.z) > nmax)
		{ i1 = 0; i2 = 1; }

//
// Compute A and B interpolation parameters
// vectors (V1 - V0) and (V2 - V0) define the plane, we try to describe
// the intersection point as a linear combination of these 2 vectors:
// inter = A * (V1 - V0) + B * (V2 - V0)
//
	float u0 = inter[i1] - V0[i1];
	float v0 = inter[i2] - V0[i2];
	float u1 = V1[i1] - V0[i1];
	float u2 = V2[i1] - V0[i1];
	float v1 = V1[i2] - V0[i2];
	float v2 = V2[i2] - V0[i2];
	float A = -1, B = -1;
	if (u1 == 0)
	{
		B = u0 / u2;
		if ((B >= 0) && (B <= 1))
			A = (v0 - B * v2) / v1;
	}
	else
	{
		B = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
		if ((B >= 0) && (B <= 1))
			A = (u0 - B * u2) / u1;
	}
//
// Determine if there was an intersection and
// save intersection point if there way
//
	if ((A >= 0) && (B >= 0) && ((A + B) <= 1))
	{
		if (intersect)
			*intersect = inter;
		return true;
	}
	return false;
}

}	// end Vixen
