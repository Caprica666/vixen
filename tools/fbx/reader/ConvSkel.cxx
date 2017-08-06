#include "vixen.h"
#include "ConvNode.h"
#include "ConvMesh.h"
#include "ConvAnim.h"
#include "FBXReader.h"

namespace Vixen { namespace FBX {

int ConvSkeleton::NumBones = 0;

/*
 * Makes a Vixen Transformer (bone) from an FBX Skeleton (limb) node.
 * @param parent	-> parent node in the Vixen simulation tree for
 *					the newly created bone. It may be a Skeleton
 *					(if it is the root bone) or another Transformer.
 *
 * The new bone created is empty and is linked under the Vixen parent provided.
 *
 * @return -> Vixen Transformer
 */
Transformer* ConvBone::MakeBone(Engine* parent)
{
	Transformer* bone = (Transformer*) m_FBXNode->GetUserDataPtr();
	
	if (bone && bone->IsClass(VX_Skeleton))
		bone = (Transformer*) ((Skeleton*) bone)->First();
	if (!bone || !bone->IsClass(VX_Transformer))
	{
		bone = new Vixen::Transformer();
		bone->SetName(m_Name);
		VX_TRACE(FBXReader::Debug, ("Creating Bone %s\n", m_Name));
	}
	if (parent && (bone->Parent() == NULL))
	{
		parent->Append(bone);
		VX_TRACE(FBXReader::Debug, ("Linking %s -> bone %s\n", parent->GetName(), m_Name));
	}
	m_VixNode = bone;
	return bone;
}

/*
 * Finds the Vixen Skeleton associated with the given FBX Skeleton node.
 *
 * @return Vixen Skeleton or NULL if not found.
 */
Skeleton* ConvBone::FindSkeleton(Transformer* bone)
{
	Engine*	parent = bone->Parent();

	while (parent)
	{
		if (parent->IsClass(VX_Skeleton))
			return (Skeleton*) parent;
		parent = parent->Parent();
	}
	return NULL;
}

/*
 * Initializes a Vixen Transformer (bone) from an FBX Skeleton (limb) node.
 *
 * The Vixen Transformer (bone) has already been created and linked into
 * the Vixen simulation tree. This function initializes the empty bone.
 * It searches up the Vixen hierarchy for the Skeleton.
 * The name of the new bone and it's parentage are set in the skeleton.
 * The name of the bone will be the name of the FBX Skeleton node.
 *
 * @return -> Vixen Transformer
 */
SharedObj*	ConvBone::Convert()
{
	if (!ConvNode::Convert())
		return NULL;

	Transformer*	bone = (Transformer*) m_VixNode;
	Skeleton*		skel = FindSkeleton(bone);
	FbxAMatrix		fbxlocal(m_FBXNode->EvaluateLocalTransform());
	FbxQuaternion	q = fbxlocal.GetQ();
	FbxVector4		t = fbxlocal.GetT();
	FbxVector4		s = fbxlocal.GetS();
	Quat			r1(q[0], q[1], q[2], q[3]);
	Vec3			t1(t[0], t[1], t[2]);
	Quat			r2;
	Vec3			t2;
	Vixen::Matrix	vixlocal;

	ConvModel::ConvertMatrix(fbxlocal, vixlocal);
	bone->SetTransform(&vixlocal);
	vixlocal.GetTranslation(t2);
	r2 = vixlocal;
	if (skel && (bone->GetBoneIndex() < 0))
	{
		int		boneindex = ConvSkeleton::NumBones++;
		Engine*	parent = bone->Parent();

		skel->SetBoneName(boneindex, m_FBXNode->GetName());
		bone->SetBoneIndex(boneindex);
		if (parent)
		{
			if (parent->IsClass(VX_Transformer))
			{
				int parentindex = ((Transformer*) parent)->GetBoneIndex();
				skel->SetParentBoneIndex(boneindex, parentindex);
			}
		}
		VX_TRACE(FBXReader::Debug, ("Bone #%d %s lrot = (%0.3f, %0.3f, %0.3f, %0.3f) lpos = (%0.3f, %0.3f, %0.3f)\n",
				boneindex, bone->GetName(), r2.x, r2.y, r2.z, r2.w, t2.x, t2.y, t2.z));
	}
	return m_VixNode;
}

/*
 * Makes a Vixen Skeleton (bone) from an FBX Skeleton (root node).
 *
 * The new skeleton created is empty (has no bones) and is not linked into the Vixen hierarchy.
 *
 * @return -> Vixen Skeleton
 */
Skeleton* ConvSkeleton::MakeSkeleton(Engine* parent)
{
	Skeleton*	skel = (Skeleton*) m_FBXNode->GetUserDataPtr();

	if (skel == NULL)
	{
		int numbones = CountBones(m_FBXNode) - 1;
		skel = new Vixen::Skeleton(numbones);
		if (numbones == 0)
		{
			skel->Delete();
			return NULL;
		}
		VX_TRACE(FBXReader::Debug, ("Creating Skeleton %s\n", m_Name));
		NumBones = 0;
		skel->SetName(m_Name);
		skel->AddRef();
		m_FBXNode->SetUserDataPtr(skel);
		if (parent)
		{
			VX_TRACE(FBXReader::Debug, ("Linking  %s -> skeleton %s\n", parent->GetName(), m_Name));
			parent->Append(skel);
		}
	}
	VX_ASSERT(skel->IsClass(VX_Skeleton));
	m_VixNode = skel;
	return skel;
}


/*
 * Counts the number of bones in the FBX skeleton.
 * This is the total number of Skeleton nodes below the root.
 * @return number of skeleton bones, 0 if none or error
 */
int ConvSkeleton::CountBones(FbxNode* node)
{
	int			total = 1;
	int			n;

	if (node == NULL)
		return 0;
	n = node->GetChildCount();
	VX_TRACE(FBXReader::Debug, ("Bone %s\n", node->GetName()));
	for (int i = 0; i < n; ++i)
	{
		FbxNode*			child = node->GetChild(i);
		FbxNodeAttribute*	attr = child->GetNodeAttribute();

		if (attr && (attr->GetAttributeType() == FbxNodeAttribute::eSkeleton))
			total += CountBones(child);
	}
	return total;
}

/*
 * After the bones of the skeleton have been processed,
 * set the bind pose from the individual bone matrices.
 */
SharedObj*	ConvSkeleton::Convert()
{
	Skeleton*	skel = (Skeleton*) m_VixNode;

	if (!ConvNode::Convert())
		return NULL;
	CalcBindPose(skel);
	return skel;
}

void ConvSkeleton::CalcBindPose(Skeleton* skel)
{
	Pose*	bindpose = skel->MakePose(Pose::SKELETON);
		
	skel->FindBones(bindpose);
	skel->SetBindPose(bindpose);
	skel->GetPose()->Copy(bindpose);
}

} }	// end FBX