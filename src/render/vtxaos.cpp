/****
 *
 * AOS (array of structures) implementation of VertexArray
 * This implementation keeps the vertex data in a single
 * contiguous area of memory. The data for each vertex (location,
 * normal, color, texcoord) is also contiguous and kept in this
 * order (opposite order from OpenGL). The AOS implementation
 * of VertexArray supports dynamic growth of vertex pools.
 *
 ****/
#include "vixen.h"


namespace Vixen {
Core::Allocator*	VertexArray::VertexAlloc = NULL;

VX_IMPLEMENT_CLASSID(VertexArray, SharedObj, VX_VtxArray);

/*!
 * @fn VertexArray::VertexArray(int style, int nvtx)
 * @param nvtx	Number of vertices to make room for initially.
 * @param style Determines what components a vertex has:
 * @code
 *	VertexPool::NORMALS		vertices have normals
 *	VertexPool::COLORS		vertices have colors
 *	VertexPool::TEXCOORDS	vertices have texture coordinates
 * @endcode
 *
 * Constructs a vertex array for vertices of the given style.
 * This implementation of vertex array permits dynamic growth of
 * the vertex data area. Note that modifying the vertex pool at run time
 * will incur a performance penalty because the scene manager caches
 * vertex data on the display card if possible.
 *
 * @see VertexPool::SetStyle VertexArray::SetMaxVtx VertexPool::SetNumVtx
 */
VertexArray::VertexArray(int style, intptr nvtx) :	VertexPool(style)
{
	DevHandle = NULL;
	if (VertexAlloc)
		m_Data.SetElemAllocator(VertexAlloc);
	if (nvtx > 0)
		SetMaxVtx(nvtx);
}

VertexArray::VertexArray(const TCHAR* layout_desc, intptr nvtx) : VertexPool(layout_desc)
{
	DevHandle = NULL;
	if (VertexAlloc)
		m_Data.SetElemAllocator(VertexAlloc);
	if (nvtx > 0)
		SetMaxVtx(nvtx);
}

/*!
 * @fn VertexArray::VertexArray(const VertexArray& src)
 * @param src	vertex array to copy
 *
 * Clones the input vertex array, duplicating all of the vertices
 * in this array. The initial size of this array is the same as
 * the source.
 *
 * @see VertexPool::Merge VertexArray::SetMaxVtx
 */
VertexArray::VertexArray(const VertexArray& src)
  :	VertexPool(src.GetStyle())
{
	DevHandle = NULL;
	if (VertexAlloc)
		m_Data.SetElemAllocator(VertexAlloc);
	SetMaxVtx(src.GetNumVtx());
	Copy(&src);
}

/*!
 * @fn bool VertexArray::SetMaxVtx(intptr n)
 *
 * Establishes the maximum number of vertices 
 * this vertex pool currently has room for. If this size
 * is increased, the vertex array is enlarged if
 * possible.  Making the array smaller with this call
 * may destroy data. This implementation supports
 * dynamic growth so the array may be enlarge to an arbitrary size.
 *
 * @return \b true if array was successfully resized, else \b false
 *
 * @see VertexPool::SetNumVtx VertexPool::Grow
 */
bool	VertexArray::SetMaxVtx(intptr n)
{
	if (!m_Data.SetMaxSize(n * GetVtxSize()))
		return false;
	m_MaxVtx = n;
	return true;
}

/*!
 * @fn bool VertexArray::SetNumVtx(intptr n)
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
bool VertexArray::SetNumVtx(intptr n)
{
	if (!m_Data.SetSize(n * GetVtxSize()))
		return false;
	Core::InterlockSet(&m_NumVtx, n);		// remember new size
	SetChanged(true);
	return true;
}
/*!
 * @fn float* VertexArray::GetData() const
 *
 * Returns a pointer to the vertex data area.
 * This is an implementation dependent value because vertices are
 * stored in a device-specific format.
 *
 * @returns pointer to vertex data, NULL if no vertices
 *
 * @see VertexPool::GetLocs VertexPool::GetAt VertexPool::SetVertex
 */
float* VertexArray::GetData()
	{ return m_Data.GetData(); }

const float* VertexArray::GetData() const
	{ return (const float*) m_Data.GetData(); }

/*!
 * @fn int VertexArray::AddVertices(const float* vtx, intptr n)
 * @param vtx	-> first vertex in the list, if NULL space is allocated
 *				 for vertices but no data is copied
 * @param n		number of vertices to add
 *
 * Adds vertices to a vertex array, enlarging it if necessary.
 * The number of floats in a vertex is determined by the vertex style.
 * The vertex array is enlarged to 1 1/2 times the original size
 * or the new size, whichever is bigger.
 *
 * This call assumes the incoming vertex data is in array of
 * structures format: All vertices are passed in a single
 * contiguous chunk of memory.  Components of the vertex, if present,
 * are stored in the order location (Vec3 or Vec4), normal (Vec3),
 * color (Color), texcoord (Vec2). Note that this is
 * not necessarily the way vertices are stored internally in
 * the device vertex pools.
 *
 * @return offset of first vertex added, negative on error
 *
 * @see VertexArray::SetMaxVtx VertexPool::SetStyle Mesh::AddVertices
 */
intptr VertexArray::AddVertices(const float* vtx, intptr n)
{
	intptr ofs = VertexPool::AddVertices(vtx, n);
	int vtxsize = GetVtxSize();

	if (ofs < 0)
		return ofs;
	n += ofs;								// new max vertex index
	n *= vtxsize;
	if (vtx == NULL)						// no vertex data
		return ofs;
	for (intptr i = ofs * vtxsize; i < n; i++)	// copy the vertex data
		m_Data.SetAt(i, *vtx++);
	return ofs;
}

static int32	PadLayout(DataLayout& padded_layout, const DataLayout& src_layout, int vecsize)
{
	int		vtxsize = vecsize;
	int		i;

	/*
	 * Determine the size of the padded vertex
	 */
	padded_layout = src_layout;
	padded_layout.Slot[0].Size = vecsize;
	for (i = 1; i < src_layout.NumSlots; ++i)
	{
		const LayoutSlot& slot = src_layout.Slot[i];
		padded_layout.Slot[i].Offset = vtxsize;
		if (slot.Style & VertexArray::NORMALS)
		{
			padded_layout.Slot[i].Size = vecsize;
			vtxsize += vecsize;
			VX_ASSERT(vecsize >= slot.Size);
		}
		else
			vtxsize += slot.Size;
	}
	vtxsize = (vtxsize + 3) & ~3;
	padded_layout.Size = vtxsize * sizeof(float);
	padded_layout.MakeDescriptor(padded_layout.Descriptor);
	return vtxsize;
}

static int32	ComputeSrcVtxSize(const DataLayout& src_layout, int vecsize)
{
	int		vtxsize = vecsize;

	for (int i = 1; i < src_layout.NumSlots; ++i)
	{
		const LayoutSlot& slot = src_layout.Slot[i];
		if (slot.Style & VertexArray::NORMALS)
			vtxsize += vecsize;
		else
			vtxsize += slot.Size;
	}
	return vtxsize;
}

float*	VertexArray::PadVertices(const float* inputVerts, intptr nverts, intptr srcstride)
{
	const DataLayout*	layout(m_Layout);
	int					vtxsize = layout->Size;
	DataLayout			padded_layout;

	/*
	 * Determine the size of the padded vertex
	 */
	if (layout->Slot[0].Size != GetMessenger()->SysVecSize)
	{
		Core::String	newdesc;

		vtxsize = PadLayout(padded_layout, *layout, GetMessenger()->SysVecSize);
		if (vtxsize == srcstride)		// no copy necessary
			return NULL;
		layout = SetLayout(padded_layout.Descriptor);
		m_VtxSize = vtxsize;
	}
	/*
	 * Positions and normals will need padding. Copy each vertex component separately.
	 * Update the vertex layout too.
	 */
	float* newverts = (float*) Core::ThreadAllocator::Get()->Alloc(sizeof(float) * nverts * vtxsize);
	for (intptr v = 0; v < nverts; v++)
	{
		float*			dst = newverts + v * vtxsize;
		const float*	src = inputVerts + v * srcstride;

		*dst++ = *src++; *dst++ = *src++; *dst++ = *src++; *dst++ = 1.0f;
		for (int i = 1; i < layout->NumSlots; ++i)
		{
			const LayoutSlot&	slot = layout->Slot[i];

			if (slot.Style & (VertexArray::NORMALS | VertexArray::TANGENTS))
			{
				*dst++ = *src++; *dst++ = *src++; *dst++ = *src++; *dst++ = 0.0f;
			}
			else
			{
				for (int j = 0; j < slot.Size; ++j)	// copy the input vertex
					*dst++ = *src++;
			}
		}
	}
	return newverts;
}

/****
 *
 * class VtxArray override for SharedObj::Copy
 *
 ****/
bool VertexArray::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	const VertexArray*	src = (const VertexArray*) src_obj;

	if (!VertexPool::Copy(src_obj))
		return false;
	return m_Data.Copy(&(src->m_Data));
}

}	// end Vixen