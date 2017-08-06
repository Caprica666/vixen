/****
 *
 *
 * These are the DX5 implementations of rendering for
 * the scene manager
 *
 ****/
#include "psm.h"
#include "dx10/vbufdx.h"

#define SAFE_RELEASE(x) { if(x) { x->Release(); x = NULL; } }

void PSSurface::FreeCache()
{
	PSMesh* geo;
	PSSurface::Iter iter(this);

	while (geo = (PSMesh*) iter.Next())
		if (geo->IsClass(PSM_Mesh))
			DelProp(PROP_DXMesh);
}

void* PSMesh::SetMeshState(PSScene* display_scene) const
{
	PSDXScene*	dxscene = (PSDXScene*) display_scene;
	if (t_CurVertices == GetVertices())
		return NULL;
	t_CurVertices = GetVertices();
	bool mesh_changed = HasChanged() | GetVertices()->HasChanged();
	SetChanged(false);
	DXMesh* cache = (DXMesh*) GetProp(PROP_DXMesh);
	if (cache == NULL)
	{
		cache = new DXMesh();				// make D3D vertex and index buffers
		((PSMesh*) this)->AddProp(cache);	// attach to mesh
		mesh_changed = true;
	}
	if (mesh_changed && !IsSet(GEO_Static))	// cannot change a static mesh
		cache->Update(dxscene, this);		// copy vertex and index data
	cache->Select(dxscene, this);
	return NULL;
}

void PSTriangles::RenderPrim(const PSTriPrim* prim, PSScene* scene, const PSAppearances* apps) const
{
	static D3D10_PRIMITIVE_TOPOLOGY	dxprim[3] = { D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	PSDXScene*				dxscene = (PSDXScene*) scene;
	LPD3DDEVICE				d3d = dxscene->GetDevice();
	UINT					Size = prim->Size;
	D3D10_PRIMITIVE_TOPOLOGY prim_type  = dxprim[prim->Prim];

//	if (((PSDXScene*) scene)->MaxVerts < prim->Size)
//		PSM_ERROR_RETURN(("Triangles::RenderPrim too many vertices"));
	d3d->IASetPrimitiveTopology(prim_type);
	switch (prim_type)
	{
		case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST: Size = Size / 3; break;
		case D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED:
		case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: Size = Size - 2; break;
	}
	VInterlockAdd(&(scene->GetStats()->PrimsRendered), Size);
	if (GetIndices())
		d3d->DrawIndexed(Size, prim->VtxIndex, GetStartVtx());
	else
		d3d->Draw(Size, prim->VtxIndex);
}

void PSMesh::Render(PSScene* scene, const PSAppearances* apps)
{
	int		style = GetStyle();
	int		appidx = m_AppIndex;
	int		numIndices = GetNumIdx();
	int		numVerts = GetNumVtx();
	const PSAppearance*	curapp = NULL;
	PSVtxArray*	verts = (PSVtxArray*) GetVertices();
	ID3D10Device* d3d = ((PSDXScene*) scene)->GetDevice();
	D3D10_PRIMITIVE_TOPOLOGY primtype = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;

//	if (((PSDXScene*) scene)->MaxVerts < numVerts)
//		PSM_ERROR_RETURN(("Mesh::Render too many vertices"));
	PSGeometry::Render(scene, apps);
//
// Set appearances once for the entire vertex array
// Don't do multi-texturing passes
//
	t_CurVertices = verts;
	if (apps && (curapp = apps->GetAt(m_AppIndex)))
	{	
		PSAppearance* cheat = (PSAppearance*) curapp;
		cheat->LoadShaders(scene, verts->GetDefaultShader());
		curapp->LoadDev(scene);				// set state from Appearance
		if (curapp->Get(APPEAR_Shading) == APPEAR_Points)
			primtype = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;
	}
	else
		PSAppearance::DefaultDev(scene);
	if (numIndices == 0)					// no indices? draw it all at once
	{
		d3d->Draw(numVerts - 1, 0);
		return;
	}
//
// Each index gives the ending vertex for a line segment
//	
	int curvtx = 0;
	for (int i = 0; i < numIndices; i++)
	{		
		int nIndex = GetIndex(i);		// Need to check for linear placement of indices!

		if (nIndex < curvtx)			// nIndex is start of next line
			return;						// error
		if (nIndex > numVerts)
			return;						// error
		d3d->Draw(nIndex - curvtx, curvtx);
		curvtx = nIndex;
	}
}

/*!
 * @fn int PSMesh::LoadTexCoords(PSScene* scene, const PSAppearance* app, int texunit) const
 * @param scene		scene this mesh is being used in
 * @param app		appearance used to render this mesh
 * @param texunit	texture unit index to load texture coordinates for
 *
 * Loads the texture coordinates for the appearance and texture unit given.
 * The texture coordinates can be supplied in the vertex array or generated
 * automatically by the graphics device. Bump-mapping is not supported by
 * the base implementation.
 *
 * @return code indicating to the caller what the next step should be:
 * @code
 *  -1	this appearance finished, continue to next primitive
 *	0	load appearance state and continue with next appearance
 * @endcode
 *
 * @see PSAppearance::LoadDev
 */
int PSMesh::LoadTexCoords(PSScene* scene, const PSAppearance* app, int texunit)
{
	return 0;
}

/*!
 * @fn int PSTriangles::LoadTexCoords(PSScene* scene, const PSAppearance* app, int texunit) const
 * @param scene		scene this mesh is being used in
 * @param app		appearance used to render this mesh
 * @param texunit	texture unit index to load texture coordinates for
 *
 * Loads the texture coordinates for the appearance and texture unit given.
 *
 * @see PSAppearance::LoadDev
 */
int PSTriangles::LoadTexCoords(PSScene* scene, const PSAppearance* app, int texunit)
{
	return 0;
}

/*!
 * @fn void PSTriangles::Render(PSScene* scene, const PSAppearances* apps)
 *
 * Renders the geometry in the mesh using the given PSAppearances table.
 * If there is no index data, the \e VtxIndex field
 * of each primitive is assumed to be an offset into the vertex
 * table rather than the index table, giving the index of the
 * first vertex for that primitive. If indices are supplied,
 * \e VtxIndex is an offset into the index table telling which
 * index to start with for this primitive.
 *
 * Efficient use of multi-texturing hardware is handled here.
 *
 * Rendering a triangle mesh does not change its vertices, indices
 * or primitives. It may update device-specific internal information
 * associated with the mesh.
 */
void PSTriangles::Render(PSScene* scene, const PSAppearances* apps)
{
	PSAppearance::TLS*	state = PSAppearance::GetTLS();

	PSGeometry::Render(scene, apps);
	SetMeshState(scene);
/*
 * For each primitive in the mesh, load appearance (if changed)
 * and render primitive
 */
	const PSAppearance* curapp = NULL;		// current appearance
	const PSTriPrim* prim;

	if (apps &&								// get appearance used by primitive
		(m_AppIndex >= 0) &&
		(m_AppIndex < apps->GetSize()))
		curapp = apps->GetAt(m_AppIndex);
	if (curapp == NULL)
		curapp = PSAppearance::GetDefault();// use default appearance
	curapp->LoadShaders(scene, GetVertices()->GetDefaultShader());
	PSTriangles::PrimIter iter(this);
	while (prim = iter.Next())
	{
/*
 * If the appearance hasn't changed since last time, just load the texture
 * coordinates for the new primitive and render it without changing render state
 */
		curapp->Lock();
		if (curapp == t_CurAppear)			// appearance did not change?
		{
			RenderPrim(prim, scene, apps);	// render primitive
			curapp->Unlock();
			continue;
		}
/*
 * New appearance. We load the number of sub-appearances that we texture units
 * in the graphics hardware for. If more are required, multiple rendering passes
 * will be needed. Multiple passes are also required for bump mapping, one pass
 * for each separate light.
 */
		t_CurAppear = curapp;				// save new appearance
		VInterlockInc(&(scene->GetStats()->RenderStateChanges));
		if (curapp)
			curapp->LoadDev(scene);
		else
			PSAppearance::DefaultDev(scene);
		RenderPrim(prim, scene, apps);	// render the primitive
		curapp->Unlock();
	}
}

