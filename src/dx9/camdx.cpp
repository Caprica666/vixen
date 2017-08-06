#include "vixen.h"
#include "dx9/renderdx.h"

namespace Vixen {

void Camera::SetProjection(Scene* scene) const
{
	Matrix		mtx;
	DXRenderer*	render = (DXRenderer*) scene->GetRenderer();
	LPD3DDEVICE	d3d = render->GetDevice();

	if (!d3d)
		return;
	GetProjection(&mtx);

	const Matrix::Data& m = *((const Matrix::Data*) mtx.GetMatrix());
	D3DMATRIX	proj;

	proj._11 = m[0][0]; proj._12 = m[1][0]; proj._13 = m[2][0]; proj._14 = m[3][0];
	proj._21 = m[0][1]; proj._22 = m[1][1]; proj._23 = m[2][1]; proj._24 = m[3][1];
	proj._31 = m[0][2]; proj._32 = m[1][2]; proj._33 = m[2][2]; proj._34 = m[3][2];
	proj._41 = m[0][3]; proj._42 = m[1][3]; proj._43 = m[2][3]; proj._44 = m[3][3];
	d3d->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*) m);
}

/*!
 * @fn void Camera::SetViewTrans(Scene* scene)
 * @param mtx matrix to use for camera view (usually NULL)
 *
 * Computes the camera view matrix for the scene.
 * This routine is used internally when the scene manager concurrently runs
 * separate traversal and rendering threads. The camera view matrix may be
 * different across these threads.
 *
 * @internal
 * @see Camera::GetViewTrans
 */
void Camera::SetViewTrans(Scene* scene)
{
	Matrix worldmtx;
	Scene::TLS*	tls = Scene::GetTLS();
   
   // TODO: shouldn't we reverse the Z direction so the camera looks out the front?
   	TotalTransform(&worldmtx, scene->GetModels());
   	m_ViewTrans.Invert(worldmtx);
	Matrix tmp(m_ViewTrans);
	tmp.Transpose();
   	for (int i = 0; i < 6; ++i)
		m_WorldPlanes[i].Transform(tmp, m_CullPlanes[i]);
   	ClearFlags(MOD_BVinvalid);
	memcpy(tls->ViewMatrix, (const float*) m_ViewTrans.GetMatrix(), sizeof(Matrix::Data));
}

}	// end Vixen