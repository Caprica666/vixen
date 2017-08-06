/*!
 * @file vxvtxpool.h
 * @brief Base class for vertex pool storage.
 *
 * A vertex pool is a device-independent base class for vertex storage.
 * It is not useful by itself but is a parent class for the
 * device-dependent vertex array class VertexArray.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxvtxaos.h
 */

#pragma once
namespace Vixen {

#define VTX_MAX_SLOTS	8

#ifndef VX_VEC3_SIZE
#define	VX_VEC3_SIZE	3
#endif

struct VXOpcode;


/*!
 * @class VertexPool
 * @brief Base class for device-dependent vertex pool.
 *
 * The organization of a vertex array is abstracted to allow for
 * different internal representations. Vertex iterators are provided as well as
 * access to an individual vertex based on its index.
 *
 * A vertex is a user-defined set of 32-bit float or integer components
 * used by the vertex shader during rendering to compute the position
 * and lighting of meshes. The layout of a vertex pool is defined with
 * a string and is the same for all vertices in the pool.
 * Typical vertex components include location (Vec3), normal (Vec3),
 * color (Color) and texture coordinates (Vec2).
 *
 * The format of the vertex data in the array maps directly to
 * what is required by the underlying renderer so that vertices
 * may be quickly copied without reformatting.
 *
 * @ingroup vixen
 * @see Geometry TriMesh Mesh VertexArray DataLayout
 */
class VertexPool : public SharedObj
{
public:

	/*!
	 * @brief Vertex pool const iterator.
	 *
	 * This class knows how to walk the vertices in a pool quickly in consecutive order.
	 * The iterator can also update a vertex in place. Do not delete
	 * the vertex array while you are iterating.
	 *
	 * @ingroup vixen
	 * @see VertexPool VertexArray
	 */
	class ConstIter
	{
	protected:
		const float*	m_CurVtx;
		const float*	m_StartVtx;
		const float*	m_EndVtx;
		int				m_VtxSize;
		int				m_NormalOfs;

	public:
		ConstIter()	: m_CurVtx(NULL), m_StartVtx(NULL), m_EndVtx(NULL), m_VtxSize(0), m_NormalOfs(-1) { };
		ConstIter(const VertexPool& verts)	{ Init(&verts); }

		ConstIter(const VertexPool* verts)	{ Init(verts); }

		ConstIter(const float* vtxdata, intptr nverts, int vtxsize) :
			m_StartVtx(vtxdata),
			m_VtxSize(vtxsize),
			m_NormalOfs(-1),
			m_CurVtx(NULL),
			m_EndVtx(vtxdata + vtxsize * nverts) { }

		//! Advance iterator to next vertex
		const float* Next()
		{
			if (!m_CurVtx)
				m_CurVtx = m_StartVtx;
			else m_CurVtx += m_VtxSize;
			if (m_CurVtx >= m_EndVtx)
				return NULL;
			return m_CurVtx;
		}

		//! Initialize iterator to a specific vertex array
		void Init(const VertexPool* verts)
		{
			VX_ASSERT(verts);
			m_NormalOfs = verts->GetLayout()->NormalOfs;
			m_VtxSize = verts->GetVtxSize();
			m_StartVtx = verts->GetData();
			m_CurVtx = NULL;
			m_EndVtx = m_StartVtx + verts->GetNumVtx() * m_VtxSize;
		}

		//! Reset iterator to start at first vertex.
		void		Reset()					{ m_CurVtx = NULL; }

		const float* GetVtx(intptr i) const	{ const float* p = m_StartVtx + i * m_VtxSize ; return (p < m_EndVtx) ? p : NULL; }

		//! Get normal of current vertex
		const Vec3*	GetNormal()	 const		{ return (m_NormalOfs >= 0) ? (Vec3*) (m_CurVtx + m_NormalOfs) : NULL; }

		//! Get normal of Ith vertex
		const Vec3*	GetNormal(intptr i) const { return (m_NormalOfs >= 0) ? (const Vec3*)  (GetVtx(i) + m_NormalOfs) : NULL; }

		//! Get the location of the Ith vertex
		const Vec3*	GetLoc(intptr i) const	{ const float* p = m_StartVtx + i * m_VtxSize; return (p < m_EndVtx) ? (const Vec3*) p : NULL; }

		//! Get location of current vertex
		const Vec3*	GetLoc() const			{ return (const Vec3*) m_CurVtx; }

		//! Return true if the vertex pool being iterated
		bool		HasNormals() const		{ return m_NormalOfs >= 0; }

		//! Gets the offset (in floats) of the normal at the current vertex, -1 if no normal
		int			GetNormalOfs() const	{ return m_NormalOfs; }

		//! Gets the vertex size (in floats)
		int			GetVtxSize() const		{ return m_VtxSize; }

		bool		CompareVtx(intptr i, const float* invtx)
		{
			const float* outvtx = GetVtx(i);

			if (outvtx == NULL)
				return false;
			for (int i = 0; i < m_VtxSize; ++i)
				if (*outvtx++ != *invtx++)
					return false;
			return true;
		}
	};

	/*!
	 * @brief Vertex pool iterator.
	 *
	 * This class knows how to walk the vertices in a pool quickly in consecutive order.
	 * The iterator can also update a vertex in place. Do not delete
	 * the vertex array while you are iterating.
	 *
	 * @ingroup vixen
	 * @see VertexPool VertexArray
	 */
	class Iter : public ConstIter
	{
	public:
		Iter()	: ConstIter() { };
		Iter(VertexPool& verts)	{ Init(&verts); }
		Iter(VertexPool* verts)	{ Init(verts); }
		Iter(float* vtxdata, intptr nverts, int vtxsize) : ConstIter((const float*) vtxdata, nverts, vtxsize) { }

		float*	Next()						{ return (float*) ConstIter::Next(); }
		float*	GetVtx(intptr i) const		{ return (float*) ConstIter::GetVtx(i); }
		Vec3*	GetNormal(intptr i) const	{ return (Vec3*) ConstIter::GetNormal(i); }
		Vec3*	GetLoc(intptr i) const		{ return (Vec3*) ConstIter::GetLoc(i); }
		Vec3*	GetLoc() const				{ return (Vec3*) ConstIter::GetLoc(); }
		Vec3*	GetNormal() const			{ return (Vec3*) ConstIter::GetNormal(); }

		//! Initialize iterator to a specific vertex array
		void Init(const VertexPool* verts)
		{
			VX_ASSERT(verts);
			m_NormalOfs = verts->GetLayout()->NormalOfs;
			m_VtxSize = verts->GetVtxSize();
			m_StartVtx = verts->GetData();
			m_CurVtx = NULL;
			m_EndVtx = m_StartVtx + verts->GetMaxVtx() * m_VtxSize;
		}

		bool	CopyVtx(const float* invtx)
		{
			float* outvtx = (float*) m_CurVtx;

			if (outvtx == NULL)
				return false;
			for (int i = 0; i < m_VtxSize; ++i)
				*outvtx++ = *invtx++;
			return true;
		}

		bool	CopyVtx(intptr i, const float* invtx)
		{
			float* outvtx = GetVtx(i);

			if (outvtx == NULL)
				return false;
			for (int j = 0; j < m_VtxSize; ++j)
				*outvtx++ = *invtx++;
			return true;
		}
	};

	/*!
	 * @brief vertex style values, indicates components in vertex layout.
	 * @see VertexPool::SetLayout VertexPool::SetStyle
	 */
	enum
	{
		MORPH =			8,				//!< Object flag to indicate vertices change

		LOCATIONS =		0,				//!< vertex component is an X,Y,Z location
		COLORS =		1,				//!< vertex component is an RGBA color
		NORMALS =		2,				//!< vertex component is an X,Y,Z vector
		TEXCOORDS =		4,				//!< vertex component is a U,V texture coordinate
		TANGENTS =		8,				//!< vertex component is X,Y,Z tangent or bitangent
		INTEGER	= DataLayout::INTEGER,	//!< vertex component is an integer
		MAX_TEXCOORDS =	4,				//!< maximum # of texcoord sets
		MAX_SLOTS =		VTX_MAX_SLOTS,	//!< maximum number of vertex layout slots
		MAX_VTX_SIZE =	64,				//!< maximum # floats/vertex
	};

public:
	VertexPool(int style = LOCATIONS);
	VertexPool(const TCHAR* layout_desc);
	virtual	~VertexPool();

//!	Retrieve device-dependent pointer to vertex data information
	virtual	float*			GetData()		{ return NULL; }
	virtual	const float*	GetData() const	{ return NULL; }

//! Get the style of vertices in the pool (which components they have).
	int				GetStyle() const;
	void			SetStyle(int);		// DEPRECATED

//! Return the number of floats in a vertex.
	int				GetVtxSize() const;

//! Return the number of vertices currently in this pool.
	intptr			GetNumVtx() const;

//! Return the maximum number of vertices this pool can contain without growing.
	intptr			GetMaxVtx() const;

//! Set the number of vertices currently used.
	virtual	bool	SetNumVtx(intptr size);

//! Set the maximum number of vertices the pool can contain.
	virtual	bool	SetMaxVtx(intptr size);

//! Enlarge the vertex pool to make room for the given number of vertices.
	virtual	bool	Grow(intptr size);

//! Get the bounding box for the vertices in the pool
	virtual	bool	GetBound(Box3*) const;

//! Return the index of the first vertex found which matches the input vertex.
	virtual intptr	Find(const float* floatArray) const;

//! Add vertices to the end of the pool from a float array.
	virtual	intptr	AddVertices(const float* floatArray, intptr n);

//! Pad a vertex array to make locations & normals 4 floats
	virtual	float*	PadVertices(const float* floatArray, intptr size, intptr srcstride);

//! Find the vertex layout described by the given string
	static const DataLayout* FindLayout(const TCHAR* layout_desc);

//! Get vertex layout information for all components
	const DataLayout* GetLayout() const;

//! Set vertex component layout
	const DataLayout* SetLayout(const TCHAR* layout_desc);

//! Transform the vertex locations and normals by the given matrix.
	virtual	VertexPool&	operator*=(const Matrix&);

//	Internal overrides
	virtual	bool		Copy(const SharedObj*);
	virtual	bool		Do(Messenger&, int);
	virtual	int			Save(Messenger&, int) const;
	virtual	DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * VtxArray::Do opcodes (also for binary file format)
	 */
	enum Opcode
	{
 		VTX_Transform = SharedObj::OBJ_NextOp,
 		VTX_AddVertices,
 		VTX_SetStyle,
 		VTX_Append,
		VTX_SetLocs,		// DEPRECATED
		VTX_SetNormals,		// DEPRECATED
		VTX_SetColors4,		// DEPRECATED
		VTX_SetTexCoords,	// DEPRECATED
		VTX_Merge,
		VTX_DupVtx,			// DEPRECATED
		VTX_Append4,		// DEPRECATED
		VTX_SetSpecColors,	// DEPRECATED
		VTX_SetMaxVtx,
		VTX_SelectTexCoords,// DEPRECATED
		VTX_SetNumTexCoords,// DEPRECATED
		VTX_SetColors,		// DEPRECATED
		VTX_SetLayout,
		VTX_NextOp = SharedObj::OBJ_NextOp + 20
	};

protected:
	void		Output(Messenger* s, VXOpcode op, const float* vtx, long ofs, long n, int vsize) const;

//	Data members
	int32				m_Style;			//!< vertex style
	int32				m_VtxSize;			//!< vertex size in floats
	int32				m_NumTexCoords;		//!< total texture coordinate size in floats
	intptr				m_MaxVtx;			//!< maximum number of vertices
	intptr volatile		m_NumVtx;			//!< current number of vertices
	const DataLayout*	m_Layout;			//!< vertex layout description
	static Core::Dict<Core::String, DataLayout, BaseDict>*	s_Layouts;	//!< table of vertex layouts
};

} // end Vixen
