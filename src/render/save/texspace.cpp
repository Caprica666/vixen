#include "psm.h"
#include "render/texspace.h"

PS_IMPLEMENT_CLASS(PSTexSpace, PSProp);

inline void PSTexSpace::GetMatrix(int n, PSMatrix& trans) const
{
	PSVec3*	vecs = GetTexVecs(n);
	
	for (int i = 0; i < 3; ++i)
	 {
		trans.Set(0, i, vecs[i].x);
		trans.Set(1, i, vecs[i].y);
		trans.Set(2, i, vecs[i].z);
	}
}

inline void PSTexSpace::ZeroVecs()
{
	PSVec3*	vecs = &m_Vectors[LIGHT_MaxLights];
	int32	n = m_Vectors.GetSize() - LIGHT_MaxLights;

	n *= sizeof(PSVec3);
	if (n > 0)
		memset(vecs, 0, n);
}

inline void PSTexSpace::NormalizeVecs()
{
	PSVec3*	vecs = &m_Vectors[LIGHT_MaxLights];
	int32	n = m_Vectors.GetSize() - LIGHT_MaxLights;

	for (int i = 0; i < n; ++i)
		(*vecs++).Normalize();
}

inline void PSTexSpace::UpdateVecs(int i, const PSVec3& v, const PSVec3& u, const PSVec3& vu)
{
	i *= 3;						// 3 vectors per vertex
	i += LIGHT_MaxLights;		// skip light positions
	PSM_ASSERT(i < m_Vectors.GetMaxSize());
	PSVec3 t = m_Vectors.GetAt(i);
	t += v;
	m_Vectors.SetAt(i++, t);
	t = m_Vectors.GetAt(i);
	t += u;
	m_Vectors.SetAt(i++, t);
	t = m_Vectors.GetAt(i);
	t += vu;
	m_Vectors.SetAt(i, t);
}

/****
 *
 * ComputeTexVecs
 * Computes the texture space matrix vectors for each vertex
 * in the mesh. These are computed like normals, by averaging the
 * contribution of multiple triangles at each vertex.
 *
 ****/
bool PSTexSpace::ComputeTexVecs()
{
	PSTriMesh::TriIter	triter(m_Mesh);
	PSVtxArray*		verts = m_Mesh->GetVertices();
	PSVec3			vtx0, vtx1, vtx2;
	PSTexCoord		tc0, tc1, tc2;
	int32			i0, i1, i2;

	ZeroVecs();									// zero all tex vecs
	while (triter.Next(i0, i1, i2))				// for each triangle
	{
		tc0 = verts->GetTexCoord(i0);
		vtx0 = verts->GetLoc(i0);
		tc1 = verts->GetTexCoord(i1);
		vtx1 = verts->GetLoc(i1);
		tc2 = verts->GetTexCoord(i2);
		vtx2 = verts->GetLoc(i2);
		PSTexCoord	d1(tc1.u - tc0.u, tc1.v - tc0.v);
		PSTexCoord d2(tc2.u - tc0.u, tc2.v - tc0.v);
		PSVec3	v1(vtx1 - vtx0);
		PSVec3	v2(vtx2 - vtx0);
		float a = d1.u * d2.v - d1.v * d2.u;
		if (a == 0)								// if this happens,
		{
			UpdateVecs(i0, MOD_XAxis, MOD_YAxis, MOD_ZAxis);
			UpdateVecs(i1, MOD_XAxis, MOD_YAxis, MOD_ZAxis);
			UpdateVecs(i2, MOD_XAxis, MOD_YAxis, MOD_ZAxis);
			continue;			
		}
		PSM_ASSERT(a != 0);
		PSM_ASSERT(i0 != i1);
		PSM_ASSERT(i1 != i2);
		PSM_ASSERT(i0 != i2);
		a = 1.0f / a;
		PSVec3 Uaxis(	a * (d2.u * v1.x - d1.u * v2.x),
						a * (d2.u * v1.y - d1.u * v2.y),
						a * (d2.u * v1.z - d1.u * v2.z));
		PSVec3 Vaxis(	a * (d1.v * v2.x - d2.v * v1.x),
						a * (d1.v * v2.y - d2.v * v1.y),
						a * (d1.v * v2.z - d2.v * v1.z));
		Uaxis.Normalize();						// normalize them??
		Vaxis.Normalize();
		a = tc0.u * tc1.v - tc1.u * tc0.v +
			tc1.u * tc2.v - tc2.u * tc1.v +
			tc2.u * tc0.v - tc0.u * tc2.v;
		PSVec3 DVxDU = Vaxis.Cross(Uaxis);
		if (a < 0)
			DVxDU.Negate();
		UpdateVecs(i0, Vaxis, Uaxis, DVxDU);	// update vertices
		UpdateVecs(i1, Vaxis, Uaxis, DVxDU);
		UpdateVecs(i2, Vaxis, Uaxis, DVxDU);
	}
	NormalizeVecs();							// normalize results
	return true;
}

/****
 *
 *
 * UpdateLight
 * Computes the new positions of a light in object space.
 * If any have changed since last time, these lights will need
 * updated vectors
 *
 ****/
void PSTexSpace::UpdateLight(const PSLightProp* lp, int kind)
{
	PSVec3	objvec;
	int32	changed = 0;

	switch (kind)
	{
		case LIGHT_Directional:
		m_World2Obj.TransformVector((PSVec3&) lp->WorldDir, objvec);
		objvec.Normalize();
		objvec.Negate();
		break;
				
		default:
		m_World2Obj.Transform((PSVec3&) lp->WorldPos, objvec);
		break;
	}
	SetLightPos(lp->DevIndex, objvec);	// and their new vectors
}

/****
 *
 * ComputeLightVecs
 * Computes the light to vertex vectors for the next light and
 * stores them in the diffuse color channel of the mesh.
 * Each time this routine is called, it will compute
 * the vectors for a different light.
 *
 * Returns:
 *	-1 no lights, 0 = this is last light, > 0 more lights to go
 *
 ****/
int PSTexSpace::ComputeDiffuseColors(PSScene* scene)
{
	PSVec3		v;
	PSMatrix	texmtx;
	int			n = m_Mesh->GetNumVtx();
	PSColor		diff;
	int			t;
	int			id;
	bool		enabled;
	const PSLight*	l = m_NextLight;
	const PSLightProp* prop;
	PSVtxArray* verts = m_Mesh->GetVertices();

	if (m_NextLight == NULL)				// first time thru?
	{
		m_LightIter.Reset(scene);			// world to object mapping
		m_World2Obj.Invert(*(PSScene::GetTLS()->RenderMatrix));
		State = TEXS_End;
		NumLights = 0;
	}
	while (prop = m_LightIter.NextProp(enabled)) // get first light
		if (enabled)						// light enabled for this object?
			break;
	if (prop == NULL)						// no active lights
	{
		if (m_NextLight != NULL)
		{
			m_NextLight = NULL;
			return -1;						// indicate no more lights
		}
		diff.Set(128, 128, 128, 255);
		for (int i = 0; i < n; ++i)			// zero all light vectors
			verts->SetColor(i, diff);
		return 0;							// indicate no lights at all
	}
	l = (PSLight*) prop->Owner;
	id = prop->DevIndex;
	t = l->GetKind();
	UpdateLight(prop, t);
	for (int i = 0; i < n; ++i)
	{
		if (t == LIGHT_Directional)			// point light?
			v = GetLightPos(id);
		else								// directional light
		{
			v = GetLightPos(id);
			v -= verts->GetLoc(i);
			v.Normalize();
		}
		GetMatrix(i, texmtx);				// get matrix for this vertex
		v *= texmtx;						// put light vector in texture space
		v.Normalize();
		diff.r = (char) ((v.x + 1) * 128);	// convert to a color
		diff.g = (char) ((v.y + 1) * 128); 
		diff.b = (char) ((v.z + 1) * 128); 
		diff.a = -1;
		verts->SetColor(i, diff);
	}
	State = TEXS_DiffuseIntensity;
	m_NextLight = l;
	++NumLights;
	verts->SetChanged(true);
	return 1;								// say there are more lights
}

int PSTexSpace::ComputeDiffuseTexCoords(PSScene* scene, int texcoords)
{
	PSVec3		v;
	PSMatrix	texmtx;
	int			n = m_Mesh->GetNumVtx();
	int			t;
	intptr		id;
	bool		enabled;
	const PSLight*	l = m_NextLight;
	const PSLightProp*	prop;
	PSVtxArray* verts = m_Mesh->GetVertices();

	if (m_NextLight == NULL)				// first time thru?
	{										// world to object mapping
		m_World2Obj.Invert(*(PSScene::GetTLS()->RenderMatrix));
		m_LightIter.Reset(scene);			// start iteration
		State = TEXS_End;
		NumLights = 0;
	}
	while (prop = m_LightIter.NextProp(enabled)) // get first light
		if (enabled)						// light enabled for this object?
			break;
	verts->SelectTexCoords(texcoords);		// select texture coordinate set
	if (prop == NULL)						// no active lights
	{
		if (m_NextLight != NULL)
		{
			m_NextLight = NULL;
			return -1;						// indicate no more lights
		}
		v.Set(0,0,0);
		for (int i = 0; i < n; ++i)			// zero all light vectors
			verts->SetTexCoord(i, (const float*) &v);
		return 0;							// indicate no lights at all
	}
	id = prop->DevIndex;					// get 0 based device light index
	l = (PSLight*) prop->Owner;
	id = prop->DevIndex;
	t = l->GetKind();
	UpdateLight(prop, t);
	for (int i = 0; i < n; ++i)
	{
		if (t == LIGHT_Directional)			// point light?
			v = GetLightPos(id);
		else								// directional light
		{
			v = GetLightPos(id);
			v -= verts->GetLoc(i);
			v.Normalize();
		}
		GetMatrix(i, texmtx);				// get matrix for this vertex
		v *= texmtx;						// put light vector in texture space
		verts->SetTexCoord(i, (const float*) &v);
	}
	State = TEXS_DiffuseNormalize;
	m_NextLight = l;
	++NumLights;
	return 1;								// say there are more lights
}

void PSTexSpace::ComputeSpecularColors(int where)
{
	PSVec3	v;
	PSMatrix texmtx;
	int32	n = m_Mesh->GetNumVtx();
	PSCol4	c;
	PSColor	spec;
	PSVtxArray* verts = m_Mesh->GetVertices();
	PSVec3 eye;

	m_World2Obj.TransformVector(PSVec3(0,0,1), eye);
	for (int i = 0; i < n; ++i)
	{
		c = verts->GetColor(i);			// get color as 4 floats
		v = eye;
		GetMatrix(i, texmtx);			// get matrix for this vertex
		v *= texmtx;					// put light vector in texture space
		v.x += c.r * 2 - 1;
		v.y += c.g * 2 - 1;
		v.z += c.b * 2 - 1;
		v.Normalize();
		spec.r = (char) ((v.x + 1) * 128);	// convert to 4 byte color
		spec.g = (char) ((v.y + 1) * 128); 
		spec.b = (char) ((v.z + 1) * 128); 
		spec.a = -1;
		verts->SetColor(i, spec);			// store in diffuse color
	}
	State = TEXS_SpecularColor;
}

void PSTexSpace::ComputeSpecularTexCoords(int texcoord)
{
	PSVec3	v1, v2;
	PSMatrix texmtx;
	int32	n = m_Mesh->GetNumVtx();
	PSVtxArray* verts = m_Mesh->GetVertices();
	PSVec3 eye;

	m_World2Obj.TransformVector(PSVec3(0,0,1), eye);
	PSM_ASSERT(texcoord > 3);
	for (int i = 0; i < n; ++i)
	{
		verts->SelectTexCoords(texcoord - 3);
		verts->GetTexCoord(i, (float*) &v2);
		v1 = eye;
		GetMatrix(i, texmtx);			// get matrix for this vertex
		v1 *= texmtx;					// put eye vector in texture space
		v2 += v1;						// add to diffuse light vector
		verts->SelectTexCoords(texcoord);
		verts->SetTexCoord(i, (const float*) &v2);
	}
	State = TEXS_SpecularColor;
}
