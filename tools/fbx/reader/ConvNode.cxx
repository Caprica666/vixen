#include "vixen.h"
#include "ConvNode.h"
#include "FBXReader.h"

namespace Vixen { namespace FBX {

const float ANGLE_TO_RADIAN = 3.1415926f / 180.f;

ConvNode::ConvNode(FbxNode* pNode, const TCHAR* name)
  :	m_FBXNode(pNode),
	m_VixNode(NULL)
{
	m_FBXObj = pNode;
	if (name)
		m_Name = name;
	else if (pNode)
		m_Name = pNode->GetName();
}

ConvNode::ConvNode(FbxObject* pObj, const TCHAR* name)
  :	m_FBXObj(pObj),
	m_VixNode(NULL)
{
	if (name)
		m_Name = name;
	else if (pObj)
		m_Name = pObj->GetName();
}

Vixen::SharedObj*	ConvNode::Convert()
{
	Messenger*	mess = GetMessenger();

	if ((m_VixNode == NULL) || (m_FBXObj == NULL))
		return NULL;
	if (m_FBXObj->GetUserDataPtr() == NULL)
	{
		m_FBXObj->SetUserDataPtr(m_VixNode);
		m_VixNode->AddRef();
	}
	if (m_VixNode->GetName() == NULL)
		m_VixNode->SetName(m_Name);
	mess->Define(m_VixNode->GetName(), m_VixNode);
	VX_TRACE(FBXReader::Debug, ("Converting %s %s\n", m_VixNode->ClassName(), m_Name));
	return m_VixNode;
}

void ConvModel::ConvertMatrix(const FbxMatrix& srcmtx, Vixen::Matrix& dstmtx)
{
	for (int col = 0; col < 4; ++col)
		for (int row = 0; row < 4; ++row)
			dstmtx.Set(row, col, (float) srcmtx.Get(col, row));
}

Vixen::Model*	ConvModel::MakeModel()
{
	Vixen::Model* mod  = new Vixen::Model();
	m_VixNode = mod;
	return mod;
}

Vixen::SharedObj*	ConvModel::Convert()
{
    FbxAMatrix	fbxlocal(m_FBXNode->EvaluateLocalTransform());
	Matrix		vixlocal;

	ConvModel::ConvertMatrix(fbxlocal, vixlocal);
	if (ConvNode::Convert())
	{
		Model* mod = (Model*) m_VixNode;
		mod->SetActive(m_FBXNode->GetVisibility());
		mod->SetTransform(&vixlocal);
		return m_VixNode;
	}
	return NULL;
}

Vixen::Light*	ConvLight::MakeLight()
{
	Vixen::Light*	vLight = NULL;

	switch (m_Light->LightType.Get())
	{
		case FbxLight::eVolume:	vLight = new Vixen::AmbientLight(); break;
		case FbxLight::ePoint:	vLight = new Vixen::Light(); break;
		case FbxLight::eSpot:	vLight = new Vixen::SpotLight(); break;
		default:				vLight = new Vixen::DirectLight(); break;
	}
	m_VixNode = vLight;
    return vLight;
}

Vixen::SharedObj*	ConvLight::Convert()
{
	if (ConvModel::Convert())
	{
		FbxPropertyT<FbxDouble3> lColorProperty = m_Light->Color;
		FbxDouble3		c = lColorProperty.Get();
		Vixen::Col4		vc(static_cast<float>(c[0]), static_cast<float>(c[1]), static_cast<float>(c[2]));
		Vixen::Light*	vLight = (Vixen::Light*) m_VixNode;
		vLight->SetColor(vc);
		if (m_Light->LightType.Get() == FbxLight::eSpot)
		{
			FbxPropertyT<FbxDouble> lConeAngleProperty = m_Light->InnerAngle;
			float coneangle = static_cast<float>(lConeAngleProperty.Get());
			((Vixen::SpotLight*) vLight)->SetInnerAngle(ANGLE_TO_RADIAN * coneangle / 2.0f);
			((Vixen::SpotLight*) vLight)->SetOuterAngle(ANGLE_TO_RADIAN * coneangle);
		}
		return vLight;
	}
	return NULL;
}

} }	// end FBX