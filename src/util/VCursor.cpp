#include "vixen.h"
#include "vxutil.h"
#include "util/vxcursor.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Cursor3D, Engine, VX_Cursor3D);

Cursor3D::Cursor3D() : Engine()
{
	m_CursorPos.x = -1;
	m_CursorPos.y = -1;
}

bool Cursor3D::OnEvent(Event* e)
{
	Scene*	scene = GetMainScene();
	switch (e->Code)
	{
		case Event::MOUSE:
	   {
		Box2 sViewBox = scene->GetViewport();	// find offset from center of window
		float fWidth2 = sViewBox.Width() / 2.0f;	// Find the size of the window
		float fHeight2 = sViewBox.Height() / 2.0f;

		m_CursorPos = ((MouseEvent*) e)->MousePos;
		m_CursorPos.x = (m_CursorPos.x - fWidth2) / fWidth2;
		m_CursorPos.y = (fHeight2 - m_CursorPos.y) / fHeight2;
		SetChanged(true);
		break;
	   }

		case Event::NAVIGATE:
		m_CursorPos.x = ((NavInputEvent*) e)->Pos.x;
		m_CursorPos.y = ((NavInputEvent*) e)->Pos.y;
		SetChanged(true);
		break;

		default:
		return Engine::OnEvent(e);
	}
	return true;
}


bool Cursor3D::UpdateTarget()
{
	Scene *pScene = GetMainScene();

	// get the target, which should be the cursor object
	Model* target = (Model*) GetTarget();
	if (!target)
		return false;

	Camera *pCam = pScene->GetCamera();
	Box3 sViewBox = pCam->GetViewVol();	// find offset from center of window
	float fWidth2 = sViewBox.Width() / 2.0f;	// Find the size of the window
	float fHeight2 = sViewBox.Height() / 2.0f;
	float fHither = pCam->GetHither();
	float fFov = pCam->GetFOV();
	Vec3 offset2;

	offset2.z = -fHither - 0.1f;
	offset2.x = offset2.z * (float) tan(fFov/2.0) * -m_CursorPos.x;
	offset2.y = -offset2.z * (float) tan(fFov/2.0) * (fHeight2 / fWidth2) * m_CursorPos.y;
	target->SetTranslation(offset2);
	return true;
}

bool Cursor3D::Eval(float t)
{
	if (HasChanged())
		UpdateTarget();
	SetChanged(false);
	return true;
}

Model* Cursor3D::CreateCursor(float fSize)
{
	float			fSmall = fSize / 8.0f;
	Model*		cursor = new Model();
	const Appearance*	apps[3];

	apps[0] = MakeRGBAppearance(1.0f, 0.0f, 0.0f);
	apps[1] = MakeRGBAppearance(0.0f, 1.0f, 0.0f);
	apps[2] = MakeRGBAppearance(0.0f, 0.0f, 1.0f);

	for (int i = 0; i < 3; i++)
	{
		Shape* shape = new Shape();
		TriMesh* geo = new TriMesh(VertexPool::NORMALS);
		shape->SetAppearance(apps[i]);
		GeoUtil::Block(geo, Vec3(fSmall, fSmall, fSize));
		shape->SetGeometry(geo);
		cursor->Append(shape);
	}
	cursor->SetName(TEXT("scene.cursor3d"));
	return cursor;
}

//
// returns the index of the appearance added
//
Appearance* Cursor3D::MakeRGBAppearance(float r, float g, float b)
{
	PhongMaterial *pMat = new PhongMaterial(Col4(r, g, b));
	Appearance *pApp = new Appearance();

	pMat->SetSpecular(Col4(0.0f, 0.0f, 0.0f));
	pMat->SetShine(0);
	pApp->SetMaterial(pMat);
	pApp->Set(Appearance::LIGHTING, true);
	return pApp;
}

}	// end Vixen