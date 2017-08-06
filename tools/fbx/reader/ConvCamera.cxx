#include "vixen.h"
#include "ConvNode.h"
#include "GetPosition.h"

#define HFOV2VFOV(h, ar) (2.0 * atan((ar) * tan( (h * FBXSDK_PI_DIV_180) * 0.5)) * FBXSDK_180_DIV_PI) //ar : aspectY / aspectX
#define VFOV2HFOV(v, ar) (2.0 * atan((ar) * tan( (v * FBXSDK_PI_DIV_180) * 0.5)) * FBXSDK_180_DIV_PI) //ar : aspectX / aspectY

namespace Vixen { namespace FBX {

static double gsOrthoCameraScale = 178.0; 

FbxArray<FbxNode*> ConvCamera::m_Cameras;

int	ConvCamera::AddCamera(FbxNode* cam)
{
	int n = m_Cameras.GetCount();
	for (int i = 0; i < n; ++i)
	{
		if (m_Cameras[i] == cam)
			return i;
	}
	m_Cameras.Add(cam);
	return n;
}

FbxNode*	ConvCamera::FindCamera(const TCHAR* name)
{
	for (int i = 0; i < m_Cameras.GetCount(); ++i)
	{
		FbxNode* cam = m_Cameras[i];
		if (strcmp(cam->GetName(), name) == 0)
			return cam;
	}
	return NULL;
}

int	ConvCamera::FindCamera(FbxNode* cam)
{
	for (int i = 0; i < m_Cameras.GetCount(); ++i)
	{
		if (m_Cameras[i] == cam)
			return i;
	}
	return -1;
}


// Find the current camera at the given time.
FbxNode* ConvCamera::GetCurrentCamera(FbxScene* pScene, FbxTime& pTime, FbxAnimLayer* pAnimLayer)
{
    FbxGlobalSettings& lGlobalSettings = pScene->GetGlobalSettings();
    FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();
    FbxString lCurrentCameraName = lGlobalSettings.GetDefaultCamera();

    // check if we need to create the Producer cameras!
    if (lGlobalCameraSettings.GetCameraProducerPerspective() == NULL &&
        lGlobalCameraSettings.GetCameraProducerBottom() == NULL &&
        lGlobalCameraSettings.GetCameraProducerTop() == NULL &&
        lGlobalCameraSettings.GetCameraProducerFront() == NULL &&
        lGlobalCameraSettings.GetCameraProducerBack() == NULL &&
        lGlobalCameraSettings.GetCameraProducerRight() == NULL &&
        lGlobalCameraSettings.GetCameraProducerLeft() == NULL)
    {
        lGlobalCameraSettings.CreateProducerCameras();
    }

    if (lCurrentCameraName.Compare(FBXSDK_CAMERA_PERSPECTIVE) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerPerspective()->GetNode();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_BOTTOM) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerBottom()->GetNode();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_TOP) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerTop()->GetNode();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_FRONT) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerFront()->GetNode();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_BACK) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerBack()->GetNode();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_RIGHT) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerRight()->GetNode();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_LEFT) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerLeft()->GetNode();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_SWITCHER) == 0)
    {
        FbxCameraSwitcher* lCameraSwitcher = pScene->GlobalCameraSettings().GetCameraSwitcher();
		FbxAnimCurve* lCurve = NULL;
		if (lCameraSwitcher)
		{
			lCurve = lCameraSwitcher->CameraIndex.GetCurve(pAnimLayer);
			int			camindex = lCurve ? int(lCurve->Evaluate(pTime)) - 1 : 0;
			FbxNode*	cam = GetCamera(camindex);
			// Get the animated parameters of the camera.
			GetAnimatedParameters(cam, pTime, pAnimLayer);
			return cam;
		}
    }
    else
    {
        FbxNode* node = FindCamera(lCurrentCameraName);
        if (node)
        {
            // Get the animated parameters of the camera.
            GetAnimatedParameters(node, pTime, pAnimLayer);
            return node;
        }
    }
    return lGlobalCameraSettings.GetCameraProducerPerspective()->GetNode();
}

// Get the animated parameters of a camera contained in the scene
// and store them in the associated member variables contained in 
// the camera.
void ConvCamera::GetAnimatedParameters(FbxNode* node, FbxTime& pTime, FbxAnimLayer* pAnimLayer)
{
	FbxCamera*	cam = node->GetCamera();
    cam->Position.Set(GetGlobalPosition(node, pTime).GetT());

    FbxAnimCurve* fc = cam->Roll.GetCurve(pAnimLayer);
    if (fc)
        cam->Roll.Set(fc->Evaluate(pTime));

    FbxCamera::EApertureMode lCameraApertureMode = cam->GetApertureMode();
    if (lCameraApertureMode == FbxCamera::eHorizontal || 
        lCameraApertureMode == FbxCamera::eVertical) 
    {
        double lFieldOfView = cam->FieldOfView.Get();
        fc = cam->FieldOfView.GetCurve(pAnimLayer);
        if (fc)
            lFieldOfView = fc->Evaluate(pTime);

        //update FOV and focal length
        cam->FieldOfView.Set( lFieldOfView);
        cam->FocalLength.Set( cam->ComputeFocalLength( lFieldOfView));
        
    }
    else if ( lCameraApertureMode == FbxCamera::eHorizAndVert)
    {
        double lOldFieldOfViewX = cam->FieldOfViewX.Get();
        double lOldFieldOfViewY = cam->FieldOfViewY.Get();

        //update FOV
        double lNewFieldOfViewX = lOldFieldOfViewX;
        double lNewFieldOfViewY = lOldFieldOfViewY;
        fc = cam->FieldOfViewX.GetCurve(pAnimLayer);
        if (fc)
            lNewFieldOfViewX = fc->Evaluate(pTime);

        fc = cam->FieldOfViewY.GetCurve(pAnimLayer);
        if (fc)
            lNewFieldOfViewY = fc->Evaluate(pTime);

        cam->FieldOfViewX.Set(lNewFieldOfViewX);
        cam->FieldOfViewY.Set(lNewFieldOfViewY);

        //update aspect
        double lUpdatedApertureX = cam->GetApertureWidth();
        double lUpdatedApertureY = cam->GetApertureHeight();
        lUpdatedApertureX *= tan( lNewFieldOfViewX * 0.5 * FBXSDK_PI_DIV_180) / tan( lOldFieldOfViewX * 0.5 * FBXSDK_PI_DIV_180);
        lUpdatedApertureY *= tan( lNewFieldOfViewY * 0.5 * FBXSDK_PI_DIV_180) / tan( lOldFieldOfViewY * 0.5 * FBXSDK_PI_DIV_180);
        
        cam->FilmWidth.Set( lUpdatedApertureX);
        cam->FilmHeight.Set( lUpdatedApertureY);
        cam->FilmAspectRatio.Set( lUpdatedApertureX / lUpdatedApertureY);
    }
    else if ( lCameraApertureMode == FbxCamera::eFocalLength)
    {
        double lFocalLength = cam->FocalLength.Get();
        fc = cam->FocalLength.GetCurve(pAnimLayer);
        if (fc && fc ->Evaluate(pTime))
            lFocalLength = fc->Evaluate( pTime);
        //update FOV and focal length
        cam->FocalLength.Set( lFocalLength);
        cam->FieldOfView.Set( cam->ComputeFieldOfView( lFocalLength));
    }
}

ConvCamera::ConvCamera(FbxScene* pScene)
	: ConvModel(ConvCamera::GetCurrentCamera(pScene))
{
}

bool ConvCamera::IsProducerCamera(FbxCamera* pCamera, FbxScene* pScene)
{
    FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();
    if (pCamera == lGlobalCameraSettings.GetCameraProducerPerspective())
        return true;
    if (pCamera == lGlobalCameraSettings.GetCameraProducerTop())
        return true;
    if (pCamera == lGlobalCameraSettings.GetCameraProducerBottom())
        return true;
    if (pCamera == lGlobalCameraSettings.GetCameraProducerFront())
        return true;
    if (pCamera == lGlobalCameraSettings.GetCameraProducerBack())
        return true;
    if (pCamera == lGlobalCameraSettings.GetCameraProducerRight())
        return true;
    if (pCamera == lGlobalCameraSettings.GetCameraProducerLeft())
        return true;

    return false;
}

FbxNode* ConvCamera::GetCurrentCamera(FbxScene*  pScene)
{
    FbxCamera* lRet = NULL;
    FbxString	lCurrentCameraName;

    FbxGlobalCameraSettings& lGlobalCameraSettings = pScene->GlobalCameraSettings();
    FbxGlobalSettings& lGlobalSettings = pScene->GetGlobalSettings();

    lCurrentCameraName = lGlobalSettings.GetDefaultCamera();

    if (lCurrentCameraName == FBXSDK_CAMERA_PERSPECTIVE)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerPerspective();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_TOP)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerTop();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_BOTTOM)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerBottom();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_FRONT)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerFront();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_BACK)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerBack();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_RIGHT)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerRight();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_LEFT)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerLeft();
    }
    else
    {
        FbxNode* lCameraNode = pScene->FindNodeByName( lCurrentCameraName);
        if( lCameraNode)
        {
            lRet = lCameraNode->GetCamera();
        }
    }
    return lRet->GetNode();
}


double TransformAperture( double pAperture, double pTransform)
{
    double lTransformAperture = ( pAperture + pTransform);
    if( lTransformAperture < 0.25)
    {
        lTransformAperture = 0.25;
    }
    if( lTransformAperture  > 179.0)
    {
        lTransformAperture = 179.0;
    }
    return lTransformAperture;
}

Vixen::Camera* ConvCamera::MakeCamera(FbxScene* scene)
{
	Vixen::Camera* vcam = new Vixen::Camera();
	m_Scene = scene;
	m_VixNode = vcam;
	return vcam;
}


void ConvCamera::ComputeView(Vixen::Matrix& dstmtx)
{
    // Compute the camera position and direction.
    FbxVector4	eye = m_Camera->Position.Get();
    FbxVector4	up = m_Camera->UpVector.Get();
    FbxVector4	forward(0,0,0);
	FbxVector4	right(0,0,0);
    FbxVector4	lookat(0,0,0);
	FbxTime		time(0);
    double		roll = m_Camera->Roll.Get() * FBXSDK_PI_DIV_180;

    if (m_FBXNode->GetTarget())
        lookat = GetGlobalPosition(m_FBXNode->GetTarget(), time).GetT();
    else
    {
        if (!m_FBXNode || IsProducerCamera(m_Camera, m_Scene))
			lookat = m_Camera->InterestPosition.Get();
        else
        {
            // Get the direction
            FbxAMatrix globalrot;
            FbxVector4 rv(GetGlobalPosition(m_FBXNode, time).GetR());
            globalrot.SetR(rv);

            // Get the length
            lookat = m_Camera->InterestPosition.Get();
            FbxVector4 campos(GetGlobalPosition(m_FBXNode, time).GetT());
            double      dist = ((lookat - campos).Length());

            // Set the center.
            // A camera with rotation = {0,0,0} points to the X direction. So create a
            // vector in the X direction, rotate that vector by the global rotation amount
            // and then position the center by scaling and translating the resulting vector
            rv = FbxVector4(1.0,0,0);
            lookat = globalrot.MultT(rv);
            lookat *= dist;
            lookat += eye;

            // Update the default up vector with the camera rotation.
            rv = FbxVector4(0,1.0,0);
            up = globalrot.MultT(rv);
        }
    }

    // Align the up vector.
    forward = lookat - eye;
    forward.Normalize();
    right = forward.CrossProduct(up);
    right.Normalize();
    up = right.CrossProduct(forward);
    up.Normalize();

    // Rotate the up vector with the roll value.
    up = up * cos(roll) + right * sin(roll);

  	Vixen::Camera*	vxcam = new Vixen::Camera();
	Vixen::Matrix	mtx;
	Vixen::Vec3		vecs[3] = { Vixen::Vec3(right[0], right[1], right[2]),
								Vixen::Vec3(up[0], up[1], up[2]),
								Vixen::Vec3(forward[0], forward[1], forward[2]) };

	mtx.RotationMatrix(vecs);
	mtx.SetTranslation(Vixen::Vec3(eye[0], eye[1], eye[2]));
}

void ConvCamera::ComputeProjection()
{
	Vixen::Camera*	vxcam = (Vixen::Camera*) m_VixNode;
	double			lNearPlane = m_Camera->GetNearPlane();    
    double			lFarPlane = m_Camera->GetFarPlane();
	FbxTime			time(0);
    FbxVector4		scale = GetGlobalPosition(m_FBXNode, time).GetS();
    static const int  FORWARD_SCALE = 2;

    //scaling near plane and far plane
    lNearPlane *= scale[FORWARD_SCALE];
    lFarPlane *= scale[FORWARD_SCALE];

    // Get the relevant camera settings for a perspective view.
    if (m_Camera->ProjectionType.Get() == FbxCamera::ePerspective)
    {
        //get the aspect ratio
        FbxCamera::EAspectRatioMode lCamAspectRatioMode = m_Camera->GetAspectRatioMode();
        double lAspectX = m_Camera->AspectWidth.Get();
        double lAspectY = m_Camera->AspectHeight.Get();
        double lAspectRatio = 1.333333;
        switch( lCamAspectRatioMode)
        {
			case FbxCamera::eWindowSize:
            lAspectRatio = lAspectX / lAspectY;
            break;

			case FbxCamera::eFixedRatio:
            lAspectRatio = lAspectX;
            break;

			case FbxCamera::eFixedResolution:
            lAspectRatio = lAspectX / lAspectY * m_Camera->GetPixelRatio();
            break;

			case FbxCamera::eFixedWidth:
            lAspectRatio = m_Camera->GetPixelRatio() / lAspectY;
            break;

			case FbxCamera::eFixedHeight:
            lAspectRatio = m_Camera->GetPixelRatio() * lAspectX;
            break;
        }

        //get the aperture ratio
        double lFilmHeight = m_Camera->GetApertureHeight();
        double lFilmWidth = m_Camera->GetApertureWidth() * m_Camera->GetSqueezeRatio();
        //here we use Height : Width
        double lApertureRatio = lFilmHeight / lFilmWidth;

        //change the aspect ratio to Height : Width
        lAspectRatio = 1 / lAspectRatio;
        //revise the aspect ratio and aperture ratio
        FbxCamera::EGateFit lCameraGateFit = m_Camera->GateFit.Get();
        switch( lCameraGateFit )
        {
			case FbxCamera::eFitFill:
            if( lApertureRatio > lAspectRatio)  // the same as eHORIZONTAL_FIT
            {
                lFilmHeight = lFilmWidth * lAspectRatio;
                m_Camera->SetApertureHeight( lFilmHeight);
                lApertureRatio = lFilmHeight / lFilmWidth;
            }
            else if( lApertureRatio < lAspectRatio) //the same as eVERTICAL_FIT
            {
                lFilmWidth = lFilmHeight / lAspectRatio;
                m_Camera->SetApertureWidth( lFilmWidth);
                lApertureRatio = lFilmHeight / lFilmWidth;
            }
            break;

			case FbxCamera::eFitVertical:
            lFilmWidth = lFilmHeight / lAspectRatio;
            m_Camera->SetApertureWidth( lFilmWidth);
            lApertureRatio = lFilmHeight / lFilmWidth;
            break;

			case FbxCamera::eFitHorizontal:
            lFilmHeight = lFilmWidth * lAspectRatio;
            m_Camera->SetApertureHeight( lFilmHeight);
            lApertureRatio = lFilmHeight / lFilmWidth;
            break;

			case FbxCamera::eFitStretch:
            lAspectRatio = lApertureRatio;
            break;

			case FbxCamera::eFitOverscan:
            if( lFilmWidth > lFilmHeight)
                lFilmHeight = lFilmWidth * lAspectRatio;
            else
                lFilmWidth = lFilmHeight / lAspectRatio;
            lApertureRatio = lFilmHeight / lFilmWidth;
            break;
        }
        //change the aspect ratio to Width : Height
        lAspectRatio = 1 / lAspectRatio;

        double lFieldOfViewX = 0.0;
        double lFieldOfViewY = 0.0;
        if (m_Camera->GetApertureMode() == FbxCamera::eVertical)
        {
			lFieldOfViewY = m_Camera->FieldOfView.Get();
			lFieldOfViewX = VFOV2HFOV( lFieldOfViewY, 1 / lApertureRatio);
        }
        else if (m_Camera->GetApertureMode() == FbxCamera::eHorizontal)
        {
            lFieldOfViewX = m_Camera->FieldOfView.Get(); //get HFOV
            lFieldOfViewY = HFOV2VFOV( lFieldOfViewX, lApertureRatio);
        }
        else if (m_Camera->GetApertureMode() == FbxCamera::eFocalLength)
        {
            lFieldOfViewX = m_Camera->ComputeFieldOfView(m_Camera->FocalLength.Get());    //get HFOV
            lFieldOfViewY = HFOV2VFOV( lFieldOfViewX, lApertureRatio);
        }
        else if (m_Camera->GetApertureMode() == FbxCamera::eHorizAndVert) {
            lFieldOfViewX = m_Camera->FieldOfViewX.Get();
            lFieldOfViewY = m_Camera->FieldOfViewY.Get();
        }

        //revise the Perspective since we have film offset
        double lFilmOffsetX = m_Camera->FilmOffsetX.Get();
        double lFilmOffsetY = m_Camera->FilmOffsetY.Get();

        lFilmOffsetX = -lFilmOffsetX / lFilmWidth * 2.0;
        lFilmOffsetY = -lFilmOffsetY / lFilmHeight * 2.0;

		vxcam->SetType(Vixen::Camera::PERSPECTIVE);
		vxcam->SetAspect(lAspectRatio);
		vxcam->SetFOV(lFieldOfViewY * 3.1415926f / 180.f);
//		vxcam->SetHither(lNearPlane);
//		vxcam->SetYon(lFarPlane);
    }
    // Get the relevant camera settings for an orthogonal view.
    else
    {
		vxcam->SetType(Vixen::Camera::ORTHOGRAPHIC);
		vxcam->SetViewVol(Vixen::Box3(-gsOrthoCameraScale, -gsOrthoCameraScale, 0, gsOrthoCameraScale, gsOrthoCameraScale, 1));
    }
}

Vixen::SharedObj* ConvCamera::Convert()
{
	if (!ConvModel::Convert())
		return NULL;
	Vixen::Matrix	viewmtx;
	Vixen::Camera*	vxcam = (Vixen::Camera*) m_VixNode;

	ComputeProjection();
	ComputeView(viewmtx);
	vxcam->SetTransform(&viewmtx);
	return vxcam;
}

} }	// end FBX