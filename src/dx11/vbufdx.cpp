#include "dx11/vixendx11.h"

namespace Vixen
{

/*
 * Initialize a static index buffer in managed memory. D3D keeps an extra copy
 * of the indices so we can remove those associated with the mesh.
 */
DXBuffer::DXBuffer()
{
}


/*
 * Makes a DX11 buffer of the given usage containing the input data.
 * @param render	DX11 renderer
 * @param data		-> data to store in the buffer
 * @param nbytes	number of bytes in the buffer
 * @param type		type of DX11 buffer (D3D11_BIND_CONSTANT_BUFFER, D3D11_BIND_VERTEX_BUFFER, D3D11_BIND_INDEX_BUFFER)
 * @param usage		DX11 usage (D3D_DYNAMIC or D3D_STATIC)
 *
 * Creates a DX11 buffer large enough to hold the input data with the specified
 * characteristics. The data is copied into the buffer before return.
 *
 * @return DX11 buffer if successful, NULL on error
 */
D3DBUFFER* DXBuffer::MakeBuffer(DXRenderer* render, const void* data, int nbytes, int type, int usage) 
{
	D3DBUFFER*			buf;
	D3D11_BUFFER_DESC	bufdesc;
	HRESULT				hr;
	D3D11_SUBRESOURCE_DATA srd;

	ZeroMemory(&bufdesc, sizeof(bufdesc));
	ZeroMemory(&srd, sizeof(srd));
	bufdesc.ByteWidth = nbytes;
	bufdesc.BindFlags = type;
	if (usage == D3D11_USAGE_DYNAMIC)
		bufdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	else
		bufdesc.CPUAccessFlags = 0;
	bufdesc.MiscFlags = 0;
	bufdesc.Usage = (D3D11_USAGE) usage;
	srd.pSysMem = data;
	srd.SysMemPitch = 0;
	srd.SysMemSlicePitch = 0;
	hr = render->GetDevice()->CreateBuffer(&bufdesc, &srd, &buf);
	if (FAILED(hr))
		VX_ERROR(("DXBuffer::MakeBuffer ERROR cannot create DX11 buffer\n"), NULL);
	DXRef<D3DBUFFER>::operator=(buf);
	return buf;
}

/*
 * Makes a DX11 buffer of the given usage containing the input data.
 * @param render	DX11 renderer
 * @param data		-> data to store in the buffer
 * @param nbytes	number of bytes in the buffer
 * @param bufdesc	DX11 buffer descriptor
 *
 * Creates a DX11 buffer large enough to hold the input data with the specified
 * characteristics. The data is copied into the buffer before return.
 *
 * @return DX11 buffer if successful, NULL on error
 */
D3DBUFFER* DXBuffer::MakeBuffer(DXRenderer* render, const void* data, int nbytes, D3D_BUFFERDESC& bufdesc) 
{
	D3DBUFFER*		buf;
	HRESULT				hr;
	D3D11_SUBRESOURCE_DATA srd;

	ZeroMemory(&srd, sizeof(srd));
	bufdesc.ByteWidth = (nbytes + 0X0F) & ~0X0F;
	srd.pSysMem = data;
	srd.SysMemPitch = 0;
	srd.SysMemSlicePitch = 0;
	hr = render->GetDevice()->CreateBuffer(&bufdesc, &srd, &buf);
	if (FAILED(hr))
		VX_ERROR(("DXBuffer::MakeBuffer ERROR cannot create DX buffer\n"), NULL);
	DXRef<D3DBUFFER>::operator=(buf);
	VX_TRACE(DXRenderer::Debug, ("DXBuffer::MakeBuffer %d bytes\n", nbytes));
	return buf;
}

/*
 * Copy the data from the input array into the DX11 buffer.
 * The previous contents of the buffer are discarded.
 * @param render	DX11 renderer
 * @param data		-> data to store in the buffer
 * @param size		number of bytes in the buffer
 *
 * The DX11 buffer is assumed to have been created with D3D_DYNAMIC usage.
 */
bool DXBuffer::Update(DXRenderer* render, const void* data, size_t nbytes)
{
	HRESULT			hr;
	D3DCONTEXT* ctx = render->GetContext();
	D3D11_MAPPED_SUBRESOURCE lock;
	D3DBUFFER*	buf;

	if (nbytes <= 0)
		return false;	
	if (!HasBuffer())
		return false;
	buf = (D3DBUFFER*) *this;
	hr = ctx->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &lock);
	if (FAILED(hr))
		VX_ERROR(("DXBuffer::Update ERROR cannot update buffer\n"), false);
	memcpy(lock.pData, data, nbytes);	
	ctx->Unmap(buf, 0);
	VX_TRACE(DXRenderer::Debug > 1, ("DXBuffer::Update %d bytes\n", nbytes));
	return true;
}

/*
 * Copy the data from the input device buffer into a DX11 constant buffer.
 * The previous contents of the constant buffer are discarded.
 * @param render	DX11 renderer
 * @param devbuf	Vixen DeviceBuffer with the data to copy
 *
 * The DX11 buffer will be created if it does not exist.
 * The DX11 usage will be D3D_DYNAMIC, allowing the CPU to update the buffer.
 */
bool DXConstantBuf::Update(DXRenderer* render, const DeviceBuffer* devbuf) 
{
	ObjectLock		lock(devbuf);
	const void*		data;
	int				size = (devbuf->GetByteSize() + 0X0F) & ~0X0F;
	DXConstantBuf&	dxbuf = (DXConstantBuf&) devbuf->DevHandle;

	if (size == 0)
		return false;
	data = devbuf->GetData();
	VX_ASSERT(data);
	VX_TRACE(DXRenderer::Debug > 1, ("DXConstantBuf::Update %s %d bytes\n", devbuf->GetName(), size));
	devbuf->SetChanged(false);
	return dxbuf.Update(render, data, size);
}

/*
 * Copy the data from the input array into the DX11 constant buffer.
 * The previous contents of the constant buffer are discarded.
 * @param render	DX11 renderer
 * @param data		-> data to store in the buffer
 * @param size		number of bytes in the buffer
 *
 * The DX11 buffer will be created if it does not exist.
 * The DX11 usage will be D3D_DYNAMIC, allowing the CPU to update the buffer.
 */
bool DXConstantBuf::Update(DXRenderer* render, const void* data, int size) 
{
	if (size == 0)
		return false;
	if (data == NULL)
		return false;
	VX_TRACE(DXRenderer::Debug > 1, ("DXConstantBuf::Update %d bytes\n", size));
	if (HasBuffer())
		return DXBuffer::Update(render, data, size);
	return MakeBuffer(render, data, size, D3D11_BIND_CONSTANT_BUFFER, D3D_DYNAMIC) != NULL;
}

/*
 * Copy the indices from the input array into the D3D index buffer.
 * The previous contents of the index buffer are discarded.
 * @param render	DX11 renderer
 * @param inds		Vixen indices to store in index buffer
 * @param dynamic	If true, the indices will be updated by the CPU (D3D_DYNAMIC).
 *					Otherwise, they are written once only (D3D_DEFAULT).
 *
 * The caller specifies the usage of the index buffer each time it is updated.
 * If the buffer does not exist, it is created with the input usage model.
 * If that changes from static to dynamic, the buffer will be destroyed and recreated.
 */
bool DXIndexBuf::Update(DXRenderer* render, const IndexArray* inds, bool dynamic) 
{
	ObjectLock	lock(inds);
	const void*	data;
	intptr		size = inds->GetSize() * sizeof(uint32);

	if (size == 0)
		return false;
	data = inds->GetData();
	VX_ASSERT(data);
	VX_TRACE(DXRenderer::Debug, ("DXIndexBuf::Update %d bytes\n", size));
	if (HasBuffer())
	{
		if (DXBuffer::Update(render, data, size))
			return true;
		if (dynamic)
		{
			DXRef<D3DBUFFER>::operator=(NULL);
			VX_WARNING(("DXIndexBuf::Update recreating static buffer as dynamic %d bytes\n", size));
		}
		VX_ERROR(("DXIndexBuf::Update ERROR cannot update static buffer %d bytes\n", size), false);
	}
	return MakeBuffer(render, data, size, D3D11_BIND_INDEX_BUFFER, dynamic ? D3D_DYNAMIC : D3D_DEFAULT) != NULL;
}


/*
 * Copy all of the vertices in the input array into the vertex buffer.
 * The previous contents of the vertex buffer are overwritten.
 * @param render	DX11 renderer
 * @param verts		Vixen vertices to store in vertex buffer
 * @param dynamic	If true, the vertices will be updated by the CPU (D3D_DYNAMIC).
 *					Otherwise, they are written once only (D3D_DEFAULT).
 *
 * The caller specifies the usage of the vertex buffer each time it is updated.
 * If the buffer does not exist, it is created with the input usage model.
 * If that changes from static to dynamic, the buffer will be destroyed and recreated.
 */
bool DXVertexBuf::Update(DXRenderer* render, const VertexArray* verts, bool dynamic)
{
	ObjectLock	lock(verts);
	size_t		buffsize = verts->GetVtxSize() * verts->GetNumVtx() * sizeof(float);
	const void*	data = verts->GetData();

	if (buffsize == 0)
		return false;
	VX_ASSERT(data);
	VX_TRACE(DXRenderer::Debug, ("DXVertexBuf::Update %d bytes\n", buffsize));
	if (HasBuffer())
	{
		if (DXBuffer::Update(render, data, buffsize))
			return true;
		if (dynamic)
		{
			DXRef<D3DBUFFER>::operator=(NULL);
			VX_WARNING(("DXVertexBuf::Update recreating static buffer as dynamic %d bytes\n", buffsize));
		}
		VX_ERROR(("DXVertexBuf::Update ERROR cannot update static buffer %d bytes\n", buffsize), false);
	}
	return MakeBuffer(render, data, buffsize, D3D11_BIND_VERTEX_BUFFER, dynamic ? D3D_DYNAMIC : D3D_DEFAULT) != NULL;
}


D3DSHADERVIEW* DXVertexBuf::GetView(D3DDEVICE* device, UINT nbytes)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	D3DSHADERVIEW* view;
	HRESULT			hr;
	D3DBUFFER*	dxbuf = (D3DBUFFER*) *this;

	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.Buffer.ElementOffset = 0;
	desc.Buffer.ElementWidth = nbytes;
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	hr = device->CreateShaderResourceView(dxbuf, &desc, &view);
	if (FAILED(hr))
		VX_ERROR(("DXVertexBuf::GetView ERROR cannot create DX shader resource view for vertex buffer\n"), NULL);
	return view;
}

DXLayout::DXLayout() : DXRef<D3DINPUTLAYOUT>()
{

}

/*
 * Populates a DX11 element descriptor based on a Vixen data layout.
 * @param layout	-> Vixen DataLayout describing format of vertex or constant buffer
 * @param desc		DX11 element descriptor to initialize
 * @param vertex	true if vertex buffer, false if index buffer
 *
 * The D3D11_INPUT_ELEMENT_DESC pointer passed on input is assumed to be large
 * enough to contain the number of elements in the Vixen layout.
 */
void DXLayout::MakeLayout(const DataLayout* layout, D3D11_INPUT_ELEMENT_DESC* desc, bool vertex)
{
	D3D11_INPUT_ELEMENT_DESC* d = desc;

	VX_TRACE(DXRenderer::Debug, ("DXLayout::MakeLayout %s", layout->Descriptor));
	for (int i = 0; i < layout->NumSlots; ++i, ++d)
	{
		const LayoutSlot& slot = layout->Slot[i];
		d->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
//		d->AlignedByteOffset = slot.Offset* sizeof(float);
		if (slot.Style & VertexPool::INTEGER)
			switch (slot.Size)
			{
				case 4: d->Format = DXGI_FORMAT_R32G32B32A32_UINT; break;
				case 3: d->Format = DXGI_FORMAT_R32G32B32_UINT; break;
				case 2: d->Format = DXGI_FORMAT_R32G32_UINT; break;
				default: d->Format = DXGI_FORMAT_R32_UINT; break;
			}
		else
			switch (slot.Size)
			{
				case 4: d->Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
				case 3: d->Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
				case 2: d->Format = DXGI_FORMAT_R32G32_FLOAT; break;
				default: d->Format = DXGI_FORMAT_R32_FLOAT; break;
			}
		d->InputSlotClass = vertex ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
		if (slot.Style & VertexPool::TEXCOORDS)
			d->SemanticIndex = slot.TexIndex;
		else
			d->SemanticIndex = 0;
		d->SemanticName = slot.Name;
		d->InstanceDataStepRate = 0;
		d->InputSlot = 0;
	}
}


D3DINPUTLAYOUT* DXLayout::Update(DXRenderer* render, const Shader* shader, const void* code, size_t codebytes)
{
	const DataLayout*	layout = shader->GetInputLayout();
	DXLayout&			l = (DXLayout&) layout->DevHandle;
	D3DINPUTLAYOUT*	dxlayout = *l;
	D3D11_INPUT_ELEMENT_DESC* desc;
	HRESULT				hr;

	if (dxlayout)
		return dxlayout;
	desc = (D3D11_INPUT_ELEMENT_DESC*) alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * layout->NumSlots);
	MakeLayout(layout, desc, shader->GetShaderType() == Shader::VERTEX);
	hr = render->GetDevice()->CreateInputLayout(desc, layout->NumSlots, code, codebytes, &dxlayout);
	if (FAILED(hr))
		VX_ERROR(("DXLayout::Update ERROR cannot create input layout %s\n", layout->Descriptor), NULL);
	l = dxlayout;
	return dxlayout;
}


} // end Vixen