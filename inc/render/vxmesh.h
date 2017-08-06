/*!
 * @file vxmesh.h
 * @brief Geometry and mesh representations.
 *
 * Geometry defines the shape of 3D objects to display.
 * A Mesh represents shape as an indexed list of points in space
 * which may have additional attributes (like color, normals).
 * A TriMesh is a Mesh in which each 3 successive points in the
 * indexed list composes a triangle.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxshape.h
 */
#pragma once

namespace Vixen {

class TriHitEvent;

/*
 * Internal flags
 */
#define	GEO_Culled		2			/* set if mesh is culled */

/*!
 * @class Geometry
 * @brief Geometric mesh base class from different mesh types may be derived.
 *
 * All geometric primitives within the geometry are rendered with the
 * same appearance. This is the smallest geometry unit that can be sorted based on render state.
 * All functions should be overridden for a meaningful implementation.
 *
 * Mesh is a subclass of Geometry that describes an indexed mesh that will allow
 * you to draw line segments or points. TriMesh is a subclass that describes an
 * indexed triangle mesh. In addition to indices and vertices, it maintains an array of
 * primitives that describe how the index data is interpreted as triangular faces.
 *
 * @see TriMesh Mesh
 */
class Geometry : public SharedObj
{
public:
	VX_DECLARE_CLASS(Geometry);

	//! Construct empty geometry with no appearance.
	Geometry() : SharedObj()		{ DevHandle = 0; }

	//! Construct geometry which is the same as given input geometry.
	Geometry(const Geometry&);

	//! Indicate geometry was changed.
	virtual void	Touch();

	//! Make the geometry empty (no faces, no vertices).
	virtual void	Empty()				{ }

	//! Return number of faces for the geometry.
	virtual intptr	GetNumFaces() const	{ return 0; }

	//! Return number of vertices in the geometry.
	virtual	intptr	GetNumVtx() const	{ return 0; }

	//! Constructs the smallest axially-aligned bounding box enclosing the geometry.
	virtual bool	GetBound(Box3*) const			{ return false; }

	//! Transform geometry by matrix.
	virtual Geometry& operator*=(const Matrix&)	{ return *this; }

	//! Callback to determine if geometry should be culled.
	virtual intptr	Cull(const Matrix*, Scene*);

	//! Determine where ray intersects geometry.
	virtual bool	Hit(const Ray& ray, TriHitEvent* hitinfo) const;

	//! Callback to determine sort location for Z sorting.
	virtual Vec3	GetSortLoc() const;

	virtual bool	Do(Messenger&, int);

	enum Opcode
	{
 		GEO_Transform = SharedObj::OBJ_NextOp,
 		GEO_SetBound,
		GEO_NextOp = SharedObj::OBJ_NextOp + 10,
	};

	mutable voidptr	DevHandle;
};



/*!
 * @class Mesh
 * @brief Geometric mesh description which maintains a list of vertices,
 * and indices into this vertex list.
 *
 * This class is a base container for different types of indexed meshes.
 * The base class renders its vertex list as lines or points.
 *
 * @see TriMesh VertexArray Shape
 */
class Mesh : public Geometry
{
public:
	VX_DECLARE_CLASS(Mesh);

	Mesh();											//!< Construct empty mesh.
	Mesh(int style, intptr maxvtx = 0);				//!< Construct empty mesh.
	Mesh(const TCHAR* layout_desc, intptr maxvtx = 0);
	Mesh(const Mesh&);								//!< Share vertices and indices with input mesh.

//	Accessors
	VertexArray*	GetVertices();					//!< Get vertex array.
	const VertexArray*	GetVertices() const;
	IndexArray*		GetIndices();					//!< Get index array.
	const IndexArray* GetIndices() const;
	void			SetIndices(IndexArray*);		//!< Use input array for index storage.
	void			SetVertices(VertexArray*);		//!< Use input array for vertex storage.
	int				GetStyle() const;				//!< Get vertex style.
	int				GetVtxSize() const;
	virtual intptr	GetNumVtx() const;				//!< Get current number of vertices.
	bool			SetNumVtx(intptr);				//!< Set current number of vertices in array.
	bool			SetMaxVtx(intptr);				//!< Set maximum vertex array size.
	intptr			GetNumIdx() const;				//!< Return number of indices.
	intptr			GetIndex(intptr index) const;	//!< Get Nth vertex index from index array.
	bool			SetIndex(intptr index, intptr v);	//!< Replace index in index array.
	bool			AddIndex(intptr v);				//!< Append an index to index array.
	intptr			GetStartVtx() const;			//!< Get minimum vertex offset.
	bool			SetStartVtx(intptr index);		//!< Set minimum vertex offset (internal).
	intptr			GetEndVtx() const;				//!< Get maximum vertex offset.
	bool			SetEndVtx(intptr index);		//!< Set maximum vertex offset (internal).
	intptr			AddIndices(const int32* intArray, intptr size);	//!< Add indices to mesh.
	intptr			AddVertices(const float* floatArray, intptr size);	//!< Add vertices to mesh.


//	Internal overrides
	Geometry&		operator*=(const Matrix&);
	virtual bool	GetBound(Box3* OUTPUT) const;
	virtual void	Empty();
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger&, int);
	virtual int		Save(Messenger&, int) const;
	virtual Vec3	GetSortLoc() const;
	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;

	/*
	 * Mesh::Do opcodes (also for binary file format)
	 */
	enum Opcode
	{
		MESH_SetVertices = GEO_NextOp,
		MESH_SetIndices,	
		MESH_AddVertices,	// deprecated
		MESH_AddIndices,
		MESH_SetStartVtx,
		MESH_SetEndVtx,
		MESH_SetIndex,
		MESH_NextOp = GEO_NextOp + 20
	};


protected:
//	Data members
	mutable Box3		m_Bound;		// axial bounding box
	Ref<VertexArray>	m_Verts;		// vertex array
	Ref<IndexArray>		m_VtxIndex;		// indices into vertex array
	intptr				m_StartVtx;		// starting vertex
	intptr				m_EndVtx;		// ending vertex
};



/*!
 * @class TriMesh
 * @brief Indexed triangle mesh representing a set of connected faces.
 *
 * Contains an index and vertex array and a collection of triangles
 * that reference them. All triangles use the same appearance.
 *
 * This is the smallest unit of geometry that is Z sortable and view-volume cullable.
 * It is output by art converters and used internally during rendering.
 * This is a device-dependent class in that some of the functionality is
 * implemented differently depending on which graphics platform is used.
 *
 * @see VertexArray Geometry Mesh
 */
class TriMesh : public Mesh
{
public:
	VX_DECLARE_CLASS(TriMesh);

	/*!
	 * @brief Thread-safe iterator which iterates over the triangles in the mesh.
	 *
	 * Each time the iterator is called, it returns the indices in
	 * the vertex array of the vertices for the next triangle. It
	 * correctly handles strips, fans and triangle lists.
	 *
	 * @see TriMesh VertexArray
	 */
	class TriIter : protected ObjectLock
	{
	public:
		TriIter(const TriMesh* mesh)
		: ObjectLock(mesh)
		{
			m_TriIndex = m_PrevIndex = 0;
		}

		virtual bool	Next(intptr& vtx0, intptr& vtx1, intptr& vtx2);
		intptr			GetIndex() const		{ return m_PrevIndex; }
		void* operator	new(size_t, void* addr) { return addr; }

	protected:
		intptr			m_TriIndex;
		intptr			m_PrevIndex;
	};

	//! Construct empty triangle mesh with given vertex style and size.
	TriMesh(int style = 0, intptr nvtx = 0);
	TriMesh(const TCHAR* layout_desc, intptr nvtx = 0);

	//! Construct triangle mesh like input mesh.
	TriMesh(const TriMesh&);

	//! Returns number of triangles in the mesh.
	virtual intptr	GetNumFaces() const;

	//! Generate normals for the triangles in the mesh.
	virtual bool	MakeNormals(bool noclear = false);

	//! Perform ray / triangle intersection
	static bool		TriHit(const Ray& ray, const Vec3& V0, const Vec3& V1, const Vec3& V2, Vec3* intersect);

	// Internal overrides
	virtual bool	Do(Messenger&, int);
	virtual	bool	Hit(const Ray& ray, TriHitEvent* hitinfo) const;

	/*
	 * TriMesh::Do opcodes (also for binary file format)
	 */
	enum Opcode
	{
		TRIMESH_MakeNormals = MESH_NextOp,
		TRIMESH_NextOp = MESH_NextOp + 20
	};
};

/*!
 * @class GeoUtil
 * @brief Generates triangle meshes for common geometric forms.
 */
class GeoUtil
{
public:
	enum CylinderPart { CYL_SIDE = 1, CYL_TOP = 2, CYL_BASE = 4 };

	//! Create planar rectangular mesh
	static	bool	Rect(TriMesh* mesh, float w = 1.0f, float h = 1.0f, float xtex = 1.0f, float ytex = 1.0f);

	//! Create 3D block
	static	bool	Block(TriMesh* mesh, const Vec3& size);

	//! Create grid of quadrilaterals
	static	bool	QuadMesh(TriMesh* mesh, int rows, int cols, const float* INPUT, intptr firstvtx);

	//! Create a torus
	static	bool	Torus(TriMesh* mesh, float irad, float orad, int res);

	//! Create a cylinder with optional top and base
	static	bool	Cylinder(TriMesh* mesh, int parts, float tradius, float bradius, float height, int res);

	//! Create 3D ellipsoid or sphere
	static	bool	Ellipsoid(TriMesh* mesh, const Vec3& size, int res);

	//! Create sphere from subdivided icosahedron
	static	bool	IcosaSphere(TriMesh* mesh, float fRadius, int depth, bool hemi);

private:
	static void		subdivide(int32 idx[], Vec3 v1, Vec3 v2, Vec3 v3, 
					  int32 depth, int32 *numverts, bool hemi,
					  TriMesh* curtm, float fRadius);
};

} // end Vixen