#pragma once


namespace Vixen
{
class DXRenderer;

class DXBuffer : public DXRef<D3DBUFFER>
{
public:
	DXBuffer();

	bool		Update(DXRenderer* render, const void* data, int nbytes);
	bool		HasBuffer() const	{ return *this != (D3DBUFFER*) NULL; }
	D3DBUFFER*	MakeBuffer(DXRenderer* render, const void* data, int nbytes, D3D_BUFFERDESC& bufdesc); 
	D3DBUFFER*	MakeBuffer(DXRenderer* render, const void* data, int nbytes, int type = D3D_CONSTANTBUFFER, int usage = D3D_DEFAULT);
	D3DBUFFER*	GetBuffer()	{ return (D3DBUFFER*) *this; }
};

/*!
 * @class DXConstantBuf
 *
 * Encapsulates a Direct3D constant buffer.
 * This property is attached to a DeviceBuffer when a D3D representation
 * of the data is created.
 *
 *
 * @see VertexArray DXBuffer
 */
class DXConstantBuf : public DXBuffer
{
public:
	DXConstantBuf();

	static inline D3DBUFFER*	GetBuffer(const DeviceBuffer* devbuf)
	{
		if (devbuf)
		{
			DXConstantBuf& dxbuf = (DXConstantBuf&) devbuf->DevHandle;
			return dxbuf.DXBuffer::GetBuffer();
		}
		else
			return NULL;
	}

	static inline void Release(const DeviceBuffer* devbuf)
	{
		if (devbuf)
		{
			DXConstantBuf& dxbuf = (DXConstantBuf&) devbuf->DevHandle;
			dxbuf.DXBuffer::Release();
		}
	}

	bool			Update(DXRenderer*, const void* data, int nbytes);
	static bool		Update(DXRenderer*, const DeviceBuffer*);
};

/*!
 * @class DXIndexBuf
 *
 * Encapsulates a Direct3D index buffer for a mesh.
 * This property is attached to the index buffer when a D3D representation
 * of the index data is created.
 *
 *
 * @see IndexArray DXBuffer
 */
class DXIndexBuf : public DXBuffer
{
public:
	DXIndexBuf();

	bool	Update(DXRenderer*, const IndexArray*, bool dynamic = false);
};

/*!
 * @class DXVertexBuf
 *
 * Encapsulates a Direct3D vertex buffer for a mesh.
 * This property is attached to a vertex array when a D3D representation
 * of the vertex data is created.
 *
 *
 * @see VertexArray DXBuffer
 */
class DXVertexBuf : public DXBuffer
{
public:
	DXVertexBuf();

	bool			Update(DXRenderer*, const VertexArray*, bool dynamic = false);
	D3DSHADERVIEW*	GetView(D3DDEVICE* device, intptr nbytes);
};

class DXLayout : public DXRef<D3DINPUTLAYOUT>
{
public:
	DXLayout();
	DXLayout& operator=(D3DINPUTLAYOUT* p)	{ DXRef<D3DINPUTLAYOUT>::operator=(p); return *this; }

	D3DINPUTLAYOUT*	Update(DXRenderer*, const Shader* shader, const void* code, size_t codebytes);

protected:
	static void	MakeLayout(const DataLayout* layout, D3D11_INPUT_ELEMENT_DESC* desc, bool vertex);
};

#define DXVTX_MaxIndex		8000
#define DXVTX_MaxVerts		8000

}	// end Vixen