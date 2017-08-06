/****
 *
 * Inlines for class VertexPool and VertexPool::Iter
 *
 ****/
 
namespace Vixen {


/*!
 * @fn int VertexPool::GetVtxSize() const
 *
 * The size of a vertex is based on its style. Vector components
 * like locations and normals are 3 floats. Texture coordinates
 * are 2 floats. A color is a 32 integer (same size a single-precision float0.
 *
 * @see VertexPool::SetStyle VertexPool::GetVtxSize
 */
inline int VertexPool::GetVtxSize() const
	{ return m_VtxSize; }

inline int VertexPool::GetStyle() const
	{ return m_Style; }

inline intptr VertexPool::GetNumVtx() const
	{ return m_NumVtx; }

inline intptr VertexPool::GetMaxVtx() const
	{ return m_MaxVtx; }

inline const DataLayout* VertexPool::GetLayout() const
	{ return m_Layout; }

} // end Vixen

