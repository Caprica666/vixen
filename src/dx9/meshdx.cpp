/****
 *
 *
 * These are the DX5 implementations of rendering for
 * the scene manager
 *
 ****/
#include "vixen.h"
#include "dx9/vbufdx.h"
#include "dx9/renderdx.h"

namespace Vixen {

class DXMesh
{
public:
	DXMesh(DXRenderer& render, const Mesh& mesh)
	  :	m_Mesh(mesh), m_Render(render) {  }

	bool	Update();
	void	RenderTris();
	void	RenderLines();

protected:
	const Mesh&		m_Mesh;
	DXRenderer&		m_Render;
};

/*
 * Render a mesh with a given appearance and transformation matrix.
 * @param geo	TriMesh or Mesh to render. Only triangle meshes and line strips are supported currently.
 * @param app	Appearance to use for rendering. If NULL, a default appearance is used.
 * @param mtx	Matrix to use to transform the mesh vertices into world space.
 *
 * If the Vixen vertex and/or index buffers associated with the mesh have changed,
 * they are copied to the DX11 equivalents. Under DX9, the Material associated with
 * a Vixen Appearance must be a PhongMaterial to supply the ambient, diffuse and specular
 * components for the fixed format pipeline. User-defined Material formats are not supported.
 */
void	DXRenderer::RenderMesh(const Geometry* geo, const Appearance* appear, const Matrix* mtx)
{
	const Mesh*	mesh = (const Mesh*) geo;

	if (geo == NULL)
		return;
	if (!geo->IsClass(VX_Mesh))
		return;
	if (d3dDevice == NULL)
		return;
	if (mtx == NULL)
		d3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*) Matrix::GetIdentity()->GetMatrix());
	else
	{
		const float (&m)[4][4] = *((const float (*)[4][4]) mtx->GetMatrix());
		D3DMATRIX world;
		world._11 = m[0][0]; world._12 = m[1][0]; world._13 = m[2][0]; world._14 = m[3][0];
		world._21 = m[0][1]; world._22 = m[1][1]; world._23 = m[2][1]; world._24 = m[3][1];
		world._31 = m[0][2]; world._32 = m[1][2]; world._33 = m[2][2]; world._34 = m[3][2];
		world._41 = m[0][3]; world._42 = m[1][3]; world._43 = m[2][3]; world._44 = m[3][3];
		d3dDevice->SetTransform(D3DTS_WORLD, &world);
	}
	/*
	 * Set up the appearance for this mesh
	 */
	if (appear != NULL)
	{
		if (appear != s_CurAppear)			// appearance changed?
		{
			ObjectLock lock1(appear);
			s_CurAppear = appear;			// save new appearance
			UpdateAppearance(appear);		// load new appearance, changes s_CurTexUnit
		}
	}
	else
	{
		s_CurAppear = NULL;
		UpdateAppearance(NULL);
	}
	DXMesh		dxmesh(*this, *mesh);
	ObjectLock	lock2(mesh);

	if (!dxmesh.Update())
		return;
	if (mesh->IsClass(VX_TriMesh))
		dxmesh.RenderTris();
	else
		dxmesh.RenderLines();
}

bool DXMesh::Update()
{
	const VertexArray*	verts = (const VertexArray*) m_Mesh.GetVertices();
	DXVertexBuf&		vbuf = (DXVertexBuf&) verts->DevHandle;
	bool				mesh_changed = m_Mesh.HasChanged();
	const IndexArray*	inds = m_Mesh.GetIndices();
	D3DDEVICE*			d3d = m_Render.GetDevice();
	int					stride;
	int					fvf = DXVertexBuf::GetFVF(verts, stride);

	m_Mesh.SetChanged(false);
	if (inds)
	{
		DXIndexBuf& ibuf = (DXIndexBuf&) m_Mesh.DevHandle;
		if (!ibuf.HasBuffer() || inds->HasChanged() || mesh_changed)
		{
			ibuf.Update(d3d, inds);
			inds->SetChanged(false);
		}
		ibuf.Select(d3d);
	}
	if (!vbuf.HasBuffer() || verts->HasChanged() || mesh_changed)
	{
		if (vbuf.Update(d3d, verts, fvf, stride))
		verts->SetChanged(false);
	}
	vbuf.Select(d3d, verts, fvf, stride);
	if (verts->GetStyle() & VertexPool::COLORS)
	{
		d3d->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		d3d->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	}
	return true;
}

/*!
 * @fn void DXMesh::RenderTris()
 *
 * Renders the geometry in the mesh as a triangle list.
 */
void DXMesh::RenderTris()
{
	D3DDEVICE*				d3d = m_Render.GetDevice();
	const VertexArray*		verts = m_Mesh.GetVertices();
	intptr					start = m_Mesh.GetStartVtx();
	intptr					end = m_Mesh.GetEndVtx();
	intptr					nvtx = m_Mesh.GetNumVtx();
	UINT					ntri;
	intptr					nidx;
	HRESULT					hr;

	if (nvtx == 0)
		return;
	VX_ASSERT(nvtx < INT_MAX);
	nidx = m_Mesh.GetNumIdx();
	if (start < 0)
		start = 0;
	if (end > 0)
		nvtx = end - start;
	VX_ASSERT(start < INT_MAX);
	if (nidx)
	{
		VX_ASSERT(nidx < INT_MAX);
		ntri = UINT(nidx / 3);
		hr = d3d->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (UINT) start, (UINT) nvtx, 0, (UINT) ntri);
	}
	else
	{
		ntri = UINT(nvtx / 3);
		hr = d3d->DrawPrimitive(D3DPT_TRIANGLELIST, 0, (UINT) ntri);
	}
	if (verts->GetStyle() & VertexPool::COLORS)
	{
		d3d->SetRenderState(D3DRS_COLORVERTEX, FALSE);
		d3d->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	}
}

/*!
 * @fn void DXMesh::RenderLines()
 *
 * Renders the geometry in the mesh as lines.
 */
void DXMesh::RenderLines()
{
	intptr				numIndices = m_Mesh.GetNumIdx();
	intptr				numVerts = m_Mesh.GetNumVtx();
	VertexArray*		verts = (VertexArray*) m_Mesh.GetVertices();
	int					vs = verts->GetVtxSize() * sizeof(float);
	int					vtxtype = D3DFVF_XYZ;
	D3DPRIMITIVETYPE	primtype = D3DPT_LINESTRIP;
	D3DDEVICE*			d3d = m_Render.GetDevice();

	if (verts->GetStyle() & VertexPool::COLORS)
	{
		vtxtype |= D3DFVF_DIFFUSE;
		d3d->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		d3d->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	}
	d3d->SetFVF(vtxtype);
	if (numIndices == 0)					// no indices? draw it all at once
	{
		int nvtx = (int) numVerts - 1;
		VX_ASSERT(nvtx < INT_MAX);
		d3d->DrawPrimitiveUP(primtype, nvtx - 1, verts->GetData(), vs);
		d3d->SetRenderState(D3DRS_COLORVERTEX, FALSE);
		d3d->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
		return;
	}
//
// Each index gives the ending vertex for a line segment
//	
	intptr curvtx = 0;
	for (int i = 0; i < numIndices; i++)
	{		
		intptr nIndex = m_Mesh.GetIndex(i);	// Need to check for linear placement of indices!

		if (nIndex < curvtx)			// nIndex is start of next line
			return;						// error
		if (nIndex > numVerts)
			return;						// error
		d3d->DrawPrimitiveUP(primtype, nIndex - curvtx, verts->GetData() + curvtx, vs);
		curvtx = nIndex * verts->GetVtxSize();
	}
	d3d->SetRenderState(D3DRS_COLORVERTEX, FALSE);
	d3d->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
}

}	// end Vixen