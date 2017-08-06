#include "vixen.h"
#include "dx9/vbufdx.h"

namespace Vixen {


/*
 * Make a D3D index buffer of the given size.
 */
D3DINDEXBUFFER* DXIndexBuf::MakeBuffer(D3DDEVICE* d3d, int numbytes, bool dynamic) 
{
	D3DINDEXBUFFER* indexbuf;
	HRESULT hr;
	int		usage = dynamic ? (D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC) : 0;

	hr = d3d->CreateIndexBuffer(numbytes, usage, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &indexbuf, NULL);
	if (hr != D3D_OK)
		VX_ERROR(("DXIndexBuf::MakeBuffer ERROR cannot create index buffer %d bytes %x", numbytes, hr), NULL);
	DXRef<D3DINDEXBUFFER>::operator=(indexbuf);
	return indexbuf;
}

/*
 * Select our D3D index buffer as the current one.
 */
bool DXIndexBuf::Select(D3DDEVICE* d3d)
{
	D3DINDEXBUFFER* indexbuf = GetBuffer();
	HRESULT hr;

	if (indexbuf && SUCCEEDED(d3d->SetIndices(indexbuf)))
		return true;
	VX_ERROR(("DXIndexBuf::Select cannot access index buffer %x", hr), false);
}

/*
 * Copy the indices from the input array into the D3D index buffer.
 * The previous contents of the index buffer are discarded.
 * This routine will create a D3D index buffer large enough to hold the
 * input data if one does not already exist.
 */
bool DXIndexBuf::Update(D3DDEVICE* d3d, const IndexArray* inds) 
{
	ObjectLock		lock(inds);
	intptr			buffsize = inds->GetSize() * sizeof(VertexIndex);	
	VertexIndex*	ibdata = NULL;
	HRESULT			hr;
	D3DINDEXBUFFER*	ibuffer = (D3DINDEXBUFFER*) *this;

	VX_ASSERT(buffsize < INT_MAX);
	if (buffsize <= 0)
		return false;	
	if (ibuffer == NULL)
	{
		ibuffer = MakeBuffer(d3d, (int) buffsize);
		if (ibuffer == NULL)
			return false;
	}
	hr = ibuffer->Lock(0, (int) buffsize, (void**) &ibdata, D3DLOCK_DISCARD);
	if (hr != D3D_OK)
		VX_ERROR(("DXIndexBuf::Update cannot update index buffer %x", hr), false);
	memcpy(ibdata, inds->GetData(), buffsize);	
	ibuffer->Unlock();
	return true;
}

/*
 * Make a D3D vertex buffer of the given size and vertex format.
 */
D3DVERTEXBUFFER* DXVertexBuf::MakeBuffer(D3DDEVICE* d3d, int buffsize, int vtx_type) 
{
	D3DVERTEXBUFFER* vertexbuf;
	HRESULT			hr;
	
	hr = d3d->CreateVertexBuffer(buffsize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, vtx_type, D3DPOOL_DEFAULT, &vertexbuf, NULL);
	if (hr != D3D_OK)
		VX_ERROR(("DXVertexBuf::MakeBuffer cannot create vertex buffer %x", hr), NULL);
	DXRef<D3DVERTEXBUFFER>::operator=(vertexbuf);
	return vertexbuf;
}

/*
 * Copy all of the vertices in the input array into the vertex buffer.
 * The previous contents of the vertex buffer are overwritten.
 */
bool DXVertexBuf::Update(D3DDEVICE* d3d, const VertexArray* verts, int vtx_type, int stride)
{
	ObjectLock lock(verts);
	int		numvtx = (int) verts->GetNumVtx();
	int		buffsize = numvtx * stride;
	D3DVERTEXBUFFER* vbuffer = GetBuffer();
	
	if (numvtx == 0)
		return false;
	if (vbuffer == NULL)
	{
		VX_ASSERT(buffsize < INT_MAX);
		vbuffer = MakeBuffer(d3d, (int) buffsize, vtx_type);
		if (vbuffer == NULL)
			return false;
	}
	BYTE*			vbdata;
	HRESULT			hr;
	const float*	srcvtx = verts->GetData();

	hr = vbuffer->Lock(0, buffsize, (void**) &vbdata, D3DLOCK_DISCARD);
	if (hr != D3D_OK)
		VX_ERROR(("DXVertexBuf::Update cannot lock vertex buffer %x", hr), false);
	VX_ASSERT(srcvtx); VX_ASSERT(vbdata);
	memcpy(vbdata, srcvtx, buffsize);
	vbuffer->Unlock();
	return true;
}


/*
 * Return the D3D flexible vertex format for the input vertices.
 */
int DXVertexBuf::GetFVF(const VertexArray* verts, int& vtx_size) 
{
	const DataLayout*	layout = verts->GetLayout(); VX_ASSERT(layout);
	int	format = (layout->Slot[0].Size == 4) ? D3DFVF_XYZW : D3DFVF_XYZ;
	vtx_size = layout->Size * sizeof(float);
	switch (layout->NumTex)
	{
		case 0: break;
		case 1:	format |= D3DFVF_TEX1; break;
		case 2: format |= D3DFVF_TEX2; break;
		case 3: format |= D3DFVF_TEX3; break;
		default: format |= D3DFVF_TEX4; break;
	}
	for (int i = 0; i < layout->NumSlots; ++i)
	{
		const LayoutSlot& slot = layout->Slot[i];
		if (slot.Style & VertexPool::NORMALS)
			format |= D3DFVF_NORMAL;
		else if (slot.Style & VertexPool::COLORS)
			format |= D3DFVF_DIFFUSE;
		else if (slot.Style & VertexPool::TEXCOORDS)
		{
			switch (slot.Size)
			{
				case 1:		format |= D3DFVF_TEXCOORDSIZE1(slot.TexIndex); break;
				case 2:		format |= D3DFVF_TEXCOORDSIZE2(slot.TexIndex); break;
				case 3:		format |= D3DFVF_TEXCOORDSIZE3(slot.TexIndex); break;
				default:	VX_WARNING(("DXVertexBuf: %d textures too many, defaulting to 4", slot.Size)); 
				case 4:		format |= D3DFVF_TEXCOORDSIZE4(slot.TexIndex); break;
			}
		}
	}
	return format;
}

/*
 * Select our D3D vertex buffer as the current one.
 * Returns zero for success and -1 for failure.
 */
bool DXVertexBuf::Select(D3DDEVICE* d3d, const VertexArray* verts, int fvf, int stride)
{
	D3DVERTEXBUFFER*	vbuf = GetBuffer();

	HRESULT	hr = d3d->SetFVF(fvf);
	if (vbuf && SUCCEEDED(hr))
	{
		hr = d3d->SetStreamSource(0, vbuf, 0, stride);
		if (hr == D3D_OK)
			return true;
	}
	VX_ERROR(("DXVertexBuf::Select cannot access vertex buffer %x", hr), false);
}

}	// end Vixen