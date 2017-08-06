/****
 *
 * VertexPool is the base class for all vertex array derivatives.
 * It maintains a vertex data array with locations, normals,
 * colors and texcoords using a set of internal member functions
 * which base classes must implement.
 *
 * VertexPool does not know HOW the vertex data is stored but provides basic
 * infrastructure for the VertexArray class. Each scene manager port may store
 * vertex data in a different way, optimized for display on that platform.
 *
 * The base class implementations of routines which change the vertex data
 * should be called because they will dynamically grow the array if
 * your implementation supports it. They also keep the internal information
 * about the vertex data consistent. Derived classes only need to actually
 * manipulate the vertex data.
 *
 ****/
#include "vixen.h"

namespace Vixen {

/****
 *
 * vtx_layout_desc returns the vertex layout descriptor for that style as a string
 *
 ****/
static const TCHAR* vtx_layout_desc3[] = 
{
	TEXT("float3 position"),
	TEXT("float3 position, int color"),
	TEXT("float3 position, float3 normal"),
	TEXT("float3 position, float3 normal, int color"),
	TEXT("float3 position, float2 texcoord"),
	TEXT("float3 position, int color, float2 texcoord"),
	TEXT("float3 position, float3 normal, float2 texcoord"),
	TEXT("float3 position, float3 normal, int color, float2 texcoord"),
};

static const TCHAR* vtx_layout_desc4[] = 
{
	TEXT("float4 position"),
	TEXT("float4 position, int color"),
	TEXT("float4 position, float4 normal"),
	TEXT("float4 position, float4 normal, int color"),
	TEXT("float4 position, float2 texcoord"),
	TEXT("float4 position, int color, float2 texcoord"),
	TEXT("float4 position, float4 normal, float2 texcoord"),
	TEXT("float4 position, float4 normal, int color, float2 texcoord"),
};

static const TCHAR* opnames[] =
	{ TEXT("Transform"), TEXT("AddVertices"), TEXT("SetStyle"), TEXT("SetAt"),
	  TEXT("SetLocs"),TEXT( "SetNormals"), TEXT("SetColors4"), TEXT("SetTexCoords"),
	  TEXT("Merge"), TEXT("DupVtx"), TEXT("Append4"), TEXT("ReflectMap"), TEXT("SetMaxVtx"),
	  TEXT("SelectTexCoords"), TEXT("SetNumTexCoords"), TEXT("SetColor"), TEXT("Append"), TEXT("SetLayout") };

const TCHAR** VertexArray::DoNames = opnames;


/*!
 * @fn VertexPool::VertexPool(int style)
 * @param style Determines what components a vertex has:
 * @code
 *	VertexPool::NORMALS		vertices have normals
 *	VertexPool::COLORS		vertices have colors
 *	VertexPool::TEXCOORDS	vertices have texture coordinates
 * @endcode
 *
 * Constructs a pool with vertices of the given style.
 * Depending on the underlying display platform,
 * vertex data is stored in different internal formats.
 * On some implementations, vertex arrays may be dynamically enlarged and on
 * others, the first non-zero maximum specifies the largest size for all time.
 *
 * Although vertex arrays can't always be enlarged, they can always
 * be made smaller. You can also change the individual vertices
 * within them. Note that modifying the vertex pool at run time
 * may incur a performance penalty on some systems which may cache
 * internal information about mesh structure for optimization purposes.
 *
 * @see VertexPool::SetStyle VertexPool::SetMaxVtx VertexPool::SetNumVtx VertexPool::SetLayout
 */
VertexPool::VertexPool(int style) : SharedObj()
{
	VX_ASSERT(style <= 0x0F);
	m_NumVtx = 0;				// current number of vertices stored
	m_MaxVtx = 0;
	m_NumTexCoords = 0;
	m_Layout = NULL;
	m_Style = 0;
	if (style)
		SetStyle(style);			// vertex style (which components we need)
}


VertexPool::VertexPool(const TCHAR* layout_desc)
{
	m_NumVtx = 0;				// current number of vertices stored
	m_MaxVtx = 0;
	m_Style = 0;
	m_NumTexCoords = 0;
	m_Layout = NULL;
	SetLayout(layout_desc);
}

VertexPool::~VertexPool()
{
}

/*!
 * @fn void VertexPool::SetStyle(int style)
 * @param style Determines what components a vertex has:
 * @code
 *	VertexPool::NORMALS		vertices have normals
 *	VertexPool::COLORS		vertices have colors
 *	VertexPool::TEXCOORDS	vertices have texture coordinates
 * @endcode
 *
 * Establishes the vertex style which determines
 * he format of each vertex. Vertices are stored internally in
 * one of three formats:
 * @li AOS (array of structures) The entire vertex pool is kept in a
 *		single contiguous chunk of memory. All components of a
 *		vertex (location, normal, color, texcoord) are together.
 *		Components, if preset are stored in the order location
 *		(Vec3), normal (Vec3), color (Col4),
 *		texcoord (Vec2)
 * @li SOA (structure of arrays) Each floating point value of each
 *		vertex component is kept in a separate array (location
 *		X values, location Y values, location Z values,
 *		normal X values, normal Y values, etc.)
 *	Hybrid (structure of array of structures) Each vertex component
 *		(locations, colors, normals, texcoords) is kept in a separate
 *		array. All locations are kept in an array of Vec3, colors
 *		are kept in an array of Col4, etc.
 *
 * Individual style bits determine which components are present
 * in the vertices in this pool.  Setting a style for an existing
 * vertex array with data in it may destroy or reformat the data.
 *
 * For more complex vertex format, a vertex layout should be used.
 * The vertex layout lets you name the individual vertex components
 * and indicate where they are in the vertex and how big they are.
 * If the vertex layout has already been established, setting the
 * vertex style will not override the previous layout.
 *
 * @see VertexPool::SetNumVtx VertexPool::SetMaxVtx VertexPool::SetLayout
 */
void VertexPool::SetStyle(int style)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VtxArray, VTX_SetStyle) << this << int32(style);
	VX_STREAM_END( )

	if (!m_Layout || (style != m_Style))
	{
		const TCHAR* desc = vtx_layout_desc3[style];

		if (GetMessenger()->SysVecSize == 4)
			desc = vtx_layout_desc4[style];
		m_Layout = FindLayout(desc);
		m_Style = style;
		VX_ASSERT(style == m_Layout->Style);
		m_VtxSize = m_Layout->Size;
	}
	if (style & VertexPool::TEXCOORDS)
	{
		if (m_NumTexCoords <= 0)
			m_NumTexCoords = 2;
		m_VtxSize += m_NumTexCoords - 2;
	}
	VX_ASSERT(m_VtxSize <= MAX_VTX_SIZE);
	SetChanged(true);
}


/*!
 * @fn const DataLayout* VertexPool::SetLayout(const TCHAR* layout_desc)
 * @param layout_desc	string describing layout
 *
 * Establishes the layout of the vertex components.
 * Each component has a name, a type, and a size indicating
 * the number of 32 bit words it occupies.
 * The layout description is a string that looks like:
 *	slotname1 type1 size1, slotname2 type2 size2,
 *
 * For example, a vertex with locations, normals and one set
 * of 2D texture coordinates might be described as:
 *	location float 3, normal float 3, color int 1, texture float 2
 *
 * The layout descriptor is used as a hash key to uniquely
 * identify a particular vertex layout. The system attempts
 * to reuse vertex layouts whenever possible.
 *
 * @see VertexPool::SetStyle VertexPool::FindLayout
 */
const DataLayout* VertexPool::SetLayout(const TCHAR* layout_desc)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VtxArray, VTX_SetLayout) << this << layout_desc;
	VX_STREAM_END( )

	m_Layout = FindLayout(layout_desc);
	m_Style = m_Layout->Style;
	m_VtxSize = m_Layout->Size;
	SetChanged(true);
	return m_Layout;
}

/*!
 * @fn const DataLayout* VertexPool::FindLayout(const TCHAR* layout_desc)
 * @param layout_desc	string describing layout
 *
 * Finds a vertex layout based on its descriptor.
 * The layout descriptor is a string that
 * Establishes the layout of the vertex components.
 * Each component has a name, a type and a size
 * indicating the number of 32 bit words it occupies.
 *
 * For example, a vertex with locations, normals and two sets
 * of 2D texture coordinates might be described as:
 *	position float 3, normal float 3, texcoord0 float 2, texcoord1 float 2
 *
 * The layout descriptor is used as a hash key to uniquely
 * identify a particular vertex layout. If the layout has already
 * been used, this function returns the existing one.
 * Otherwise, it makes a new layout descriptor and returns that one.
 *
 * @see VertexPool::SetStyle VertexPool::SetLayout
 */
const DataLayout* VertexPool::FindLayout(const TCHAR* layout_desc)
{
	DataLayout* layout = (DataLayout*) DataLayout::FindLayout(layout_desc);

	layout->Style = 0;
	layout->NumTex = 0;
	for (int i = 0; i < layout->NumSlots; ++i)
	{
		LayoutSlot&	slot = layout->Slot[i];

		if (slot.Name == TEXT("normal"))
		{
			layout->Style |= VertexPool::NORMALS;
			slot.Style |= VertexPool::NORMALS;
			if (layout->NormalOfs < 0)
				layout->NormalOfs = slot.Offset;
		}
		else if ((slot.Name.Find(TEXT("texcoord")) == 0) || (slot.Name.Find(TEXT("texture")) == 0))
		{
			slot.TexIndex = layout->NumTex;
			slot.Style |= VertexPool::TEXCOORDS;
			++(layout->NumTex);
			layout->Style |= VertexPool::TEXCOORDS;
		}
		else if (slot.Name.Find(TEXT("color")) == 0)
		{
			slot.Style |= VertexPool::COLORS | DataLayout::INTEGER;
			layout->Style |= VertexPool::COLORS;
		}
		else if (slot.Name.Find(TEXT("tangent")) >= 0)
		{
			slot.Style |= VertexPool::TANGENTS;
			layout->Style |= VertexPool::TANGENTS;
		}
	}
	if (GetMessenger()->SysVecSize == 4)	// Round size to 16 byte boundary
		layout->Size = (layout->Size + 3) & ~3;
	return layout;
}



/*!
 * @fn bool VertexPool::SetNumVtx(intptr n)
 *
 * Establishes the current number of vertices used in this pool.
 * If this number exceeds the maximum number of vertices we
 * currently have room for, the vertex array is enlarged if possible.
 * Some implementations do not support dynamically enlarging the
 * vertex pool and will report failure in this case. Making the
 * array smaller with this call does not destroy data.
 *
 * @return \b true if array was successfully resized, else \b false
 *
 * @see VertexPool::SetMaxVtx VertexPool::SetStyle
 */
bool VertexPool::SetNumVtx(intptr n)
{
	if (n > GetMaxVtx())				// need to enlarge?
		if (!SetMaxVtx(n))				// enlarge failed?
			return false;
	Core::InterlockSet(&m_NumVtx, n);	// remember new size
	SetChanged(true);
	return true;
}

/*!
 * @fn bool VertexPool::SetMaxVtx(intptr n)
 *
 * Establishes the maximum number of vertices 
 * this vertex pool currently has room for. If this size
 * is increased, the vertex array is enlarged if
 * possible.  Making the array smaller with this call
 * may destroy data. On some platforms, you may only
 * establish a maximum size once and the vertex array
 * cannot be further enlarged.
 *
 * @return \b true if array was successfully resized, else \b false
 *
 * @see VertexPool::SetNumVtx VertexPool::Grow
 */
bool VertexPool::SetMaxVtx(intptr n)
{
	if (n == 0)
		return true;
	VX_ERROR(("VtxArray::SetMaxVtx ERROR failed to enlarge\n"), false);
}

/*!
 * @fn int VertexPool::Find(const float *vtx) const
 * @param vtx	float array with vertex to search for
 *
 * Finds the first vertex which matches the input vertex.
 *
 * @return index of the vertex which matched or -1 if not found
 *
 * @see VertexPool::Append VertexPool::SetVertex
 */
intptr VertexPool::Find(const float* vtx) const
{
	ObjectLock	lock(this);
	VertexPool::ConstIter iter(this);
	const float* vptr;
	intptr	i = 0;
	while (vptr = iter.Next())
	{
		bool found = true;
		for (int j = 0; j < m_VtxSize; ++j)
			if (vtx[j] != *vptr++)
				found = false;
		if (found)
			return i;
		++i;
	}
	return -1;
}

/*!
 * @fn bool VertexPool::Grow(int n)
 * @param n number of vertices to make room for
 *
 * Enlarges the vertex array to make room for at least the
 * given number of vertices. On implementations that can
 * enlarge the array, it is made 1 1/2 times the original size
 * or the new size, whichever is bigger. On some platforms,
 * vertex arrays cannot be enlarged and this call will fail
 * if there is no more room.
 *
 * @return new vertex array size, -1 on error
 *
 * @see VertexPool::SetMaxVtx VertexPool::AddVertices VertexPool::Append
 */
bool VertexPool::Grow(intptr n)
{
	ObjectLock	lock(this);
	intptr		old = GetNumVtx();			// index of new vertices
	if (n > GetMaxVtx())					// out of room?
	{
		intptr grow = old + (old >> 2);		// 1.5 times old size
		if (n > grow)						// they asked for more?
			grow = n;						// give them what they want
		if (!SetMaxVtx(grow) &&				// can enlarge 1 1/2?
			!SetMaxVtx(n))					// can't make exact room?
			return false;
	}
	return true;
}

/*!
 * @fn intptr VertexPool::AddVertices(const float* vtx, intptr n)
 * @param vtx	-> first vertex in the list, if NULL space is allocated
 *				 for vertices but no data is copied
 * @param n		number of vertices to add
 *
 * Adds vertices to a vertex array, enlarging it if necessary.
 * The number of floats in a vertex is determined by the vertex style.
 * The vertex array is enlarged to 1 1/2 times the original size
 * or the new size, whichever is bigger. On some platforms,
 * vertex arrays cannot be enlarged and this call will fail
 * if there is no more room.
 *
 * This call assumes the incoming vertex data is in array of
 * structures format: All vertices are passed in a single
 * contiguous chunk of memory.  Components of the vertex, if present,
 * are stored in the order location (Vec3), normal (Vec3),
 * color (Color), texcoord (Vec2). Note that this is
 * not necessarily the way vertices are stored internally in
 * the device vertex pools.
 *
 * @return offset of first vertex added, negative on error
 *
 * @see VertexPool::SetMaxVtx VertexPool::SetStyle Mesh::AddVertices
 */
intptr VertexPool::AddVertices(const float* vtx, intptr n)
{
	intptr ofs = GetNumVtx();				// index of new vertices

	VX_STREAM_BEGIN(s)
		Output(s, OP(VX_VtxArray, VTX_AddVertices), vtx, -1, (long) n, m_Layout->Size); 
	VX_STREAM_END(  )

	n += ofs;
	if (!Grow(n))
		return -1;
	if (n >= m_NumVtx)
		m_NumVtx = n;
	SetChanged(true);
	return ofs;
}

float*	VertexPool::PadVertices(const float* floatArray, intptr nverts, intptr srcstride)
{
	return NULL;
}

/*!
 * @fn VertexPool& VertexPool::operator*=(const Matrix& trans)
 * @param trans transformation matrix to use
 *
 * The vertex locations are multiplied by the input matrix
 * as points - translation components are included in the multiplication.
 * If there are vertex normals, they are transformed as vectors
 * and translation is ignored. Only the first set of locations
 * and normals are affected.
 *
 * @see TriMesh::MakeNormals
 */
VertexPool& VertexPool::operator*=(const Matrix& trans)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_VtxArray, VTX_Transform) << this << &trans;
	VX_STREAM_END( )

	if (trans.IsIdentity())
		return *this;
	VertexPool::Iter iter(this);
	while (iter.Next())
	{
		Vec3* p = iter.GetLoc();
		Vec3* n = iter.GetNormal();
	
		*p *= trans;
		if (n)
		{
			Vec3	t(*n);
			trans.TransformVector(t, *n);
			n->Normalize();
		}
	}
	SetChanged(true);
	return *this;
}

/*!
 * @fn bool VertexPool::GetBound(Box3* bound) const
 * @param bound	where to store bounding box, may not be NULL
 *
 * Computes  the smallest axially-aligned bounding box which
 * encloses all the vertices)
 *
 * @return \b true if bounds computed successfully.
 * \b false if no vertex data or empty bounds
 *
 * @see Geometry::GetBound Box3::Around
 */
bool VertexPool::GetBound(Box3* bound) const
{
	VertexPool::ConstIter iter(this);
	const Vec3* p = (const Vec3*) iter.Next();

	if (p == NULL)
		return false;
	bound->Around(*p, *p);
	while ((p = (const Vec3*) iter.Next()))
		bound->Extend(p);
	return (bound->Width() || bound->Height() || bound->Depth());
}

/****
 *
 * class VtxArray override for SharedObj::Copy
 *
 ****/
bool VertexPool::Copy(const SharedObj* src_obj)
{
	const VertexPool*	src = (const VertexPool*) src_obj;

	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	VX_ASSERT(src_obj->IsClass(VX_VtxArray));
	if (!SharedObj::Copy(src_obj))
		return false;
	SetStyle(src->GetStyle());
	m_NumTexCoords = src->m_NumTexCoords;
	SetNumVtx(src->GetNumVtx());
	m_Layout = src->m_Layout;
	return true;
}


/****
 *
 * class VtxArray for SharedObj::Do
 *	VTX_Transform	<Transform*>
 *	VTX_AddVertices	<int32 n> <float [ ]>
 *	VTX_SetStyle	<int32>
 *	VTX_SetAt		<int32 i> <float [ ]>
 *
 ****/
bool VertexPool::Do(Messenger& s, int op)
{
	const Matrix*	trans;
	SharedObj*		obj;
	int32			n, vs;
	float*			vtx;
	Core::String	layout;
	TCHAR			layout_desc[1024];
	Opcode			o = Opcode(op);

	switch (op)
	{
		case VTX_SetMaxVtx:
		s >> n;
		SetMaxVtx(n);
		break;

		case VTX_Transform:
		s >> obj;
		VX_ASSERT(obj->IsClass(VX_Matrix));
		trans = (const Matrix*) obj;
		*this *= *trans;
		break;

		case VTX_AddVertices:
		s >> n;
		if (s.Version <= 6)
		{
			vs = s.FileVecSize;
			for (int i = 1; i < m_Layout->NumSlots; ++i)
			{
				const LayoutSlot& slot = m_Layout->Slot[i];
				if (slot.Style & VertexArray::NORMALS)
					vs += s.FileVecSize;
				else
					vs += slot.Size;
			}
		}
		else
			s >> vs;
		vtx = (float*) Core::ThreadAllocator::Get()->Alloc(vs * n * sizeof(float));
		s.Input(vtx, n * vs);
		if (s.FileVecSize != s.SysVecSize)
		{
			float* padded = PadVertices(vtx, n, vs);
			if (padded)
			{
				Core::ThreadAllocator::Get()->Free(vtx);
				vtx = padded;
			}
		}
		AddVertices(vtx, n);
		Core::ThreadAllocator::Get()->Free(vtx);
		break;

		case VTX_SetLayout:
		s >> layout_desc;
		SetLayout(layout_desc);
		break;

		default:
		return SharedObj::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << VertexArray::DoNames[op - VTX_Transform]
					   << " " << this);
#endif
	return true;
}

/****
 *
 * class VtxArray override for SharedObj::Save()
 * The base implementation just saves the vertex maximum
 * and style. It is left to the derived class to save
 * the vertex data of the return value is non-zero.
 *
 ****/
int VertexPool::Save(Messenger& s, int opts) const
{
	int32	vtxcmdsize = 3 * sizeof(int32);
	intptr	n = GetNumVtx();
	int32	vs = GetVtxSize();
	int32	maxverts = (BufMessenger::MaxBufSize - vtxcmdsize) / (vs * sizeof(float));
	const float* p = GetData();
	int32 h = SharedObj::Save(s, opts);
	if (h <= 0)
		return h;
	VX_ASSERT(vs > 0);
	s << OP(VX_VtxArray, VTX_SetLayout) << h << m_Layout->Descriptor;
	s << OP(VX_VtxArray, VTX_SetMaxVtx) << h << int32(n);
	
	while (n > 0)
	{
		if ((int32) n < maxverts)
		{
			VX_ASSERT(n < INT_MAX);
			maxverts = (int) n;
		}
		s << OP(VX_VtxArray, VTX_AddVertices) << h << maxverts << vs;
		s.Output(p, maxverts * vs);
		p += maxverts * vs;
		n -= maxverts;
	}
	return h;
}

void VertexPool::Output(Messenger* s, VXOpcode op, const float* vtx, long ofs, long n, int vsize) const
{
	int32 vtxcmdsize = ((ofs >= 0) ? 3 : 4) * sizeof(int32);
	long maxverts = (BufMessenger::MaxBufSize - vtxcmdsize) / (vsize * sizeof(float));
	long startvtx = 0;

	while (n > 0)
	{
		if (n < maxverts)
			maxverts = n;
		if (ofs < 0)
			*s << op << this << maxverts << vsize;
		else
			*s << op << this << int32(ofs) << maxverts << vsize;
		s->Output(vtx, maxverts * vsize);
		n -= maxverts;
		ofs += maxverts;
	}
}

/****
 *
 * class VtxArray override for SharedObj::Print()
 *
 ****/
DebugOut& VertexPool::Print(DebugOut& dbg, int opts) const
{
	if ((opts & PRINT_Attributes) == 0)
		return SharedObj::Print(dbg, opts);
	SharedObj::Print(dbg, opts & ~PRINT_Trailer);
	endl(dbg << "\t<attr name='Layout'>" << (const TCHAR*) m_Layout->Descriptor << "</attr>");
	endl(dbg << "\t<attr name='NumVtx'>" << GetNumVtx() << "</attr>");
	if (opts & PRINT_Data)
	{
		const DataLayout* layout = GetLayout();
		VertexPool::ConstIter iter(this);
		const float* fptr;
		const int32* iptr;
	
		VX_ASSERT(layout);
		endl(dbg << "<attr name='Data'>");
		while (fptr = iter.Next())
		{
			iptr = (const int32*) fptr;
			for (int i = 0; i < layout->NumSlots; ++i)
			{
				const LayoutSlot& slot = layout->Slot[i];
				bool	isint = (slot.Style & DataLayout::INTEGER) != 0;

				if (isint)
				{
					for (int j = slot.Offset; j < slot.Offset + slot.Size; ++j)
						dbg << " " << iptr[j];
				}
				else
				{
					for (int j = slot.Offset; j < slot.Offset + slot.Size; ++j)
						dbg << " " << fptr[j];
				}
			}
			endl(dbg);
		}
		endl(dbg << "</attr>");
	}
	SharedObj::Print(dbg, opts & PRINT_Trailer);
	return dbg;
}

} // end Vixen