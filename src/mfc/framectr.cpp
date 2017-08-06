#include "vxmfc.h"
#include "mfc/framectr.h"

namespace Vixen {
namespace MFC {

FrameCounter::FrameCounter(): Engine()
{
	m_bShowCameraInfo = false;
	m_Frame = 0;
	m_FramesPerSec = 0;
	m_VertsTotal = 0;
	m_VertsCulled = 0;
	m_CameraPos.Set(0,0,0);
	m_CameraDir.Set(0,0,0);
	m_i = 0;
}

bool FrameCounter::Eval(float t)
{
	int		next;
	TCHAR	buffer1[256];
	Scene*	pScene = GetMainScene();

	if (pScene == NULL)
		return false;
	// Compute framerate as an average over the last 30 frames
	m_time[m_i] = (float) Core::GetTime();
	next = m_i + 1;
	if (next == 120) next = 0;
	float diff = m_time[m_i] - m_time[next];
	m_i = next;
	float framerate = (diff > 0) ? (120.0f / diff) : 0;

	Demo*	pApp = Demo::Get();
	++m_Frame;
	m_FramesPerSec = framerate;

	// Get statistics from the Scene
	const SceneStats *pStats = pScene->GetStats();
	m_VertsTotal = pStats->TotalVerts;			// total vertices seen this frame
	m_VertsCulled = pStats->CulledVerts;		// total vertices culled this frame
	m_PrimsRendered = pStats->PrimsRendered;	// primitives rendered this frame

#if defined(VIXEN_OGL) || defined(VIXEN_GLES2)
	// If the target is a TextImage or TextShape, update it with brief info
	SharedObj* target = GetTarget();
	if (target)
	{
		sprintf(buffer1, " %2.2f FPS\r\n %d verts", framerate, m_VertsTotal - m_VertsCulled);
		if (target->IsClass(VX_TextGeometry))
			((TextGeometry*) target)->SetText(buffer1);
		else if (target->IsClass(VX_Sprite))
			((Sprite*) target)->SetText(buffer1);
		return true;
	}
#endif

  // RAM_XX
  if (next != 0)
    return true;

	// Otherwise, put the information in the status text field
	int32 drawn = m_VertsTotal - m_VertsCulled;

	SPRINTF(buffer1, TEXT("%d: %d: %f fps, verts: total %d, culled %d, drawn %d, prims %d"),
		m_i, m_Frame, framerate, m_VertsTotal, m_VertsCulled, drawn, m_PrimsRendered);

	// If user requests it, append camera position and direction
	if (m_bShowCameraInfo)
	{
		Camera *pCam = pScene->GetCamera();
		m_CameraPos = pCam->GetTranslation();
		pCam->GetTransform()->TransformVector(Vec3(0.0f, 0.0f, -1.0f), m_CameraDir);

		TCHAR	buffer2[256];
		SPRINTF (buffer2, TEXT("Camera %.1f %.1f %.1f  dir %.4f %.4f %.4f"),
			m_CameraPos.x, m_CameraPos.y, m_CameraPos.z,
			m_CameraDir.x, m_CameraDir.y, m_CameraDir.z);
		STRCAT (buffer1, buffer2);
	}
	pApp->SetStatusText (buffer1);

	return true;
}

void FrameCounter::SetCameraInfo(bool bOnOff)
{
	m_bShowCameraInfo = bOnOff;
}

}	// end MFC
}	// end Vixen

