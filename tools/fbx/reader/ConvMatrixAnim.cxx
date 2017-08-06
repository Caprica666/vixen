#include "vixen.h"
#include "ConvNode.h"
#include "ConvAnim.h"
#include "FBXReader.h"

namespace Vixen { namespace FBX {


ConvMatrixAnim::ConvMatrixAnim(FbxNode* node, const char* name) : ConvNode(node, name)
{
	m_Rotation = NULL;
	m_Translation = NULL;
}

/*
 * If there are animation curves on the local translation and/or rotation
 * of the FBX node, this function generates a Vixen Transformer with child
 * KeyFramers to describe the animation.
 *
 * @return -> Vixen Transformer or NULL if no animation
 */
Transformer* ConvMatrixAnim::MakeMatrixAnim(Engine* parent, Transformer* root, bool dotranslation)
{
	bool				animated = false;
	FbxAnimCurveNode*	fbxpos = m_FBXNode->LclTranslation.GetCurveNode(false);
	FbxAnimCurveNode*	fbxrot = m_FBXNode->LclRotation.GetCurveNode(false);

	if (fbxpos && fbxpos->IsAnimated() && dotranslation)
		animated = true;
	if (fbxrot && fbxrot->IsAnimated())
		animated = true;
	if (!animated)
		return NULL;
	if (root == NULL)
		root = new Transformer();
	m_VixNode = root;
	if (fbxrot)
	{
		Interpolator*	rot = new Interpolator();
		FbxTimeSpan		s;

		rot->SetName(m_Name + TEXT(".rot"));
		rot->SetInterpType(Interpolator::QSTEP);
		rot->SetDestType(Interpolator::ROTATION);
		rot->SetValSize(sizeof(Quat) / sizeof(float));
		m_Rotation = rot;
		fbxrot->GetAnimationInterval(s);
		m_TimeSpan.UnionAssignment(s);
		root->Append(rot);
		VX_TRACE(FBXReader::Debug, ("Linking %s -> %s\n", m_Name, rot->GetName()));
	}
	if (fbxpos && dotranslation)
	{
		Interpolator*	trans = new Interpolator();
		FbxTimeSpan		s;

		trans->SetName(m_Name + TEXT(".pos"));
		trans->SetInterpType(Interpolator::STEP);
		trans->SetValSize(sizeof(Vec3) / sizeof(float));
		trans->SetDestType(Interpolator::POSITION);
		m_Translation = trans;
		fbxrot->GetAnimationInterval(s);
		m_TimeSpan.UnionAssignment(s);
		root->Append(trans);
		VX_TRACE(FBXReader::Debug, ("Linking %s -> %s\n", m_Name, trans->GetName()));
	}
	if (parent)
	{
		parent->Append(root);
		VX_TRACE(FBXReader::Debug, ("Linking %s -> %s\n", parent->GetName(), m_Name));
	}
	return root;
}

SharedObj*	ConvMatrixAnim::Convert()
{
	Transformer*	root = (Transformer*) m_VixNode;
	Interpolator*	trans = NULL;
	FbxTime			timeinc;

	if (!ConvNode::Convert())
		return NULL;
	timeinc.SetSecondDouble(1 / 30.0);	// 30 fps
	ComputeAnimation(timeinc);
	return root;
}

int ConvMatrixAnim::ComputeAnimation(FbxTime timeinc)
{
	int			nkeys = 0;
	FbxTime		t;
	double		start = m_TimeSpan.GetStart().GetSecondDouble();
	double		dur = m_TimeSpan.GetDuration().GetSecondDouble();

	for (t = m_TimeSpan.GetStart(); t < m_TimeSpan.GetStop(); t += timeinc)
	{
		FbxAMatrix	fbxlocal(m_FBXNode->EvaluateLocalTransform(t));
		float		time = t.GetSecondDouble() - start;

		if (m_Rotation)
		{
			FbxQuaternion	fbxrot = fbxlocal.GetQ();
			Interpolator::WQuat rot(fbxrot[0], fbxrot[1], fbxrot[2], fbxrot[3]);

			rot.weight = 1.0f;
			m_Rotation->AddKey(time, rot);
		}
		if (m_Translation)
		{
			FbxVector4	fbxpos = fbxlocal.GetT();
			Vec3		pos(fbxpos[0], fbxpos[1], fbxpos[2]);

			m_Translation->AddKey(time, pos);
		}
		++nkeys;
	}
	VX_TRACE(FBXReader::Debug, ("%s Computed %d animation keys from %f for %f secs\n", m_Name, nkeys, start, dur));
	return nkeys;
}


} }	// end FBX