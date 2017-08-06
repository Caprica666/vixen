#pragma once

namespace Vixen {

/*!
 * @class DXIndexBuf
 *
 * Encapsulates a Direct3D index buffer for a mesh.
 * This property is attached to the index buffer when a D3D representation
 * of the index data is created. Only large static primitives have their
 * own index buffers.
 *
 * @ingroup vixenint
 * @internal
 *
 * @see VertexArray Mesh DXVertexBuf DXRef
 */
class DXIndexBuf : public DXRef<D3DINDEXBUFFER>
{
public:
	DXIndexBuf();
	~DXIndexBuf();

	bool			HasBuffer() const	{ return GetPtr() != NULL; }
	bool			Select(D3DDEVICE* d3d);
	bool			Update(D3DDEVICE* d3d, const IndexArray* inds);
	D3DINDEXBUFFER*	GetBuffer() const	{ return (D3DINDEXBUFFER*) GetPtr(); }
	D3DINDEXBUFFER*	MakeBuffer(D3DDEVICE* d3d, int numbytes, bool dynamic = true);
};

/*!
 * @class DXVertexBuf
 *
 * Encapsulates a Direct3D vertex buffer for a mesh.
 * This property is attached to a vertex array when a D3D representation
 * of the vertex data is created. It assumes the vertex data never changes.
 *
 * @ingroup vixenint
 * @internal
 *
 * @see VertexArray Mesh DXIndexBuffer DXRef
 */
class DXVertexBuf : public DXRef<D3DVERTEXBUFFER>
{
public:
	DXVertexBuf();
	~DXVertexBuf();

	bool				Select(D3DDEVICE* d3d, const VertexArray* verts, int fvf, int stride);
	bool				Update(D3DDEVICE* d3d, const VertexArray* verts, int fvf, int stride);
	D3DVERTEXBUFFER*	MakeBuffer(D3DDEVICE* d3d, int numbytes, int fvf);
	bool				CopyVerts(const VertexArray* verts);
	bool				HasBuffer() const	{ return GetPtr() != NULL; }
	D3DVERTEXBUFFER*	GetBuffer() const	{ return (D3DVERTEXBUFFER*) GetPtr(); }
	static	int			GetFVF(const VertexArray* verts, int& vtxsize);
};


} // end Vixen