/*
 * SWIG interface file for Geometry, Mesh, TriMesh
 */
namespace Vixen
{
	%ignore VTX_MAX_SLOTS;
	%ignore GEO_Culled;

	%ignore Geometry::operator=;
	%ignore Geometry::Cull;
	%ignore Geometry::Render;
	%ignore Geometry::LoadDev;
	%ignore Geometry::Hit;
	%ignore Geometry::GetSortLoc;
	%ignore Geometry::DevHandle;

	%ignore Mesh::DevHandle;
	%ignore Mesh::GetVertices() const;
	%ignore Mesh::GetIndices() const;
	%ignore TriMesh::TriIter;
	%ignore TriMesh::TriHit;

	%ignore VertexPool::GetData;
	%ignore VertexPool::ConstIter;
	%ignore VertexPool::Iter;
	%ignore VertexPool::Output;
	%ignore VertexPool::GetLayout;
	%ignore VertexPool::SetLayout;
	%ignore	VertexPool::FindLayout;
	%ignore VertexArray::DevHandle;

	%rename(Mul)	VertexPool::operator*=;
	%rename(Mul)	Geometry::operator*=;
	%apply	SharedObj*	{ Geometry* };
	%apply	SharedObj*	{ Mesh* };
	%apply	SharedObj*	{ TriMesh* };
	%apply	SharedObj*	{ VertexArray* };

#ifdef SWIGCSHARP
	%include "VixenCS/VXVertexArray_csharp.swg"
#endif
}

ATTR_READONLY(Vixen::VertexArray, int, Style);
ATTR_READONLY(Vixen::VertexArray, int, VertexSize, GetVtxSize);
ATTR_DEFINE(Vixen::Mesh, Vixen::VertexArray, Vertices);
ATTR_DEFINE(Vixen::Mesh, Vixen::IndexArray, Indices);
ATTR_READONLY(Vixen::Mesh, int, VertexSize, GetVtxSize);
ATTR_READONLY(Vixen::Mesh, long long, IndexCount, GetNumIdx);

#ifdef SWIGCSHARP
%attribute(Vixen::VertexArray, long long, VertexCount, GetNumVtx, SetNumVtx);
%attribute(Vixen::VertexArray, long long, MaxVerts, GetMaxVtx, SetMaxVtx);
%attribute(Vixen::Mesh, long long, VertexCount, GetNumVtx, SetNumVtx);
%attribute(Vixen::Mesh, long long, StartVertex, GetStartVtx, SetStartVtx);
%attribute(Vixen::Mesh, long long, EndVertex, GetEndVtx, SetEndVtx);
#endif

%include "render/vxvtxpool.h"
%include "render/vxvtxaos.h"
%include "render/vxmesh.h"


