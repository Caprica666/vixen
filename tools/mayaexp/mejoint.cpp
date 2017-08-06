#include "vix_all.h"
#include <maya/MQuaternion.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MAnimUtil.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnGeometryFilter.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnIkJoint.h>

const int	meConvSkin::MAX_BONES_PER_VERTEX = 6;

/*!
 * @fn SharedObj* meConvJoint::Make()
 *
 * Creates one or more Vixen engines from the Maya node associated with this converter.
 * The Vixen engine is empty upon return from this routine.
 *
 * @return root Vixen engine created for the Maya node, NULL if Maya node cannot be converted
 *
 * @see meConvJoint::Convert
 */
SharedObj* meConvJoint::Make()
{
 	Transformer*	joint = new Transformer();
	Matrix			worldmtx;
	Core::String	name(GetMayaName());
	Matrix			localmtx;
	MPlug			plug;

	DoScan = true;
	DoLink = true;
	m_VXObj = joint;
	name += ".xform";
	SetName(joint, name);
	m_JointIndex = -1;
	Exporter->SimRoot->Append(joint);
	if (GetMatrix(localmtx))
	{
		Vec3	localpos;
		Quat	localrot;

		joint->SetTransform(&localmtx);
		localrot = joint->GetRotation();
		localpos = joint->GetPosition();
		meLog(3, "%s:\tlrot (%.3f, %.3f, %.3f, %.3f)\tlpos (%.3f, %.3f, %.3f)",
			GetMayaName(), localrot.x, localrot.y, localrot.z, localrot.w,
			localpos.x, localpos.y, localpos.z);
	}
	if (Exporter->DoAnimation)
	{
		if (MAnimUtil::isAnimated(GetMayaObj(), true))
			HasAnimation = MAYA;
	/*
	 * Determine if there are IK handles or constraints attached to this joint.
	 */
		meConnIter	plugiter(GetMayaObj());
		MObject		mobj;
		while (plugiter.Next(mobj, MFn::kTransform))
		{
			int			 hasanim = MAnimUtil::isAnimated(mobj, true) ? MAYA : 0;
			MObject		 constraint;
			MObjectArray targets;
			meConnIter	 ikiter(plugiter.CurPlug(), false);
			
			if (MAnimUtil::findConstraint(plugiter.CurPlug(), constraint, targets))
			{
				if (MAnimUtil::isAnimated(constraint, true))
					hasanim = MAYA;
			}
			while (ikiter.Next(mobj, MFn::kTransform))
			{
				if (mobj.hasFn(MFn::kIkEffector))
				{
					HasIK = MFn::kIkEffector;
					hasanim = MAYA;
					break;
				}
				else if (mobj.hasFn(MFn::kIkHandle))
				{
					HasIK = MFn::kIkHandle;
					hasanim = MAYA;
					break;
				}
				else if (mobj.hasFn(MFn::kConstraint))
				{
					HasIK = MFn::kIkHandle;
					hasanim = MAYA;
					break;
				}
			}
			if (hasanim)
			{
				HasAnimation = MAYA;
				meLog(3, "%s:\tAnimated %s", GetMayaName(), plugiter.PlugName());
			}
		}
	/*
	 * If we are exporting animation and this is the root joint,
	 * set the skeleton to its bind pose.
	 */
		MFnDagNode	dagnode(m_MayaObj);
		int			numparents = dagnode.parentCount();
		for (int p = 0; p < numparents; ++p)
		{
			MObject parent(dagnode.parent(p, &m_Status));
			if (m_Status && !parent.hasFn(MFn::kJoint))
			{
				MObject	none;
				MGlobal::clearSelectionList();
				m_Status = MGlobal::select(m_DagPath, none);
				m_Status = MGlobal::executeCommand("dagPose -restore -global -bindPose;", true);
			}
		}
	}
    return joint;
}

/*!
 * @fn bool meConvJoint::GetMatrix(Matrix& psmatrix) const
 * @param psmatrix	Vixen matrix to get the Maya matrix data
 *
 * Copies the local matrix from the Maya node into a Vixen matrix
 *
 * @returns true if Maya node has a non-identity matrix, else false
 *	if successful, the Vixen matrix passed will get the
 *	local matrix from the Maya node associated with this converter
 */
bool meConvJoint::GetMatrix(Matrix& vxmatrix) const
{
    MStatus status;
    MObject transformNode = m_DagPath.transform(&status);

    if (!status && status.statusCode() == MStatus::kInvalidParameter)
        return false;			// node does not have transform

    MFnDagNode	transform(transformNode, &status);
    if (!status)
        return false;			// node does not have transform
    /*
     * Extract the Maya transformation matrix, convert to Vixen matrix format
     */
    MTransformationMatrix	tmatrix(transform.transformationMatrix());
    if (tmatrix.isEquivalent(MTransformationMatrix::identity))
        return false;
 
	MMatrix		mmtx = tmatrix.asMatrix();
    MVector		trans = tmatrix.translation(MSpace::kTransform);
    float		fltdata[4][4];

    mmtx.get(fltdata);
    vxmatrix.SetMatrix(&fltdata[0][0]);
	vxmatrix.Transpose();
    return true;
}

void meConvJoint::CalcJointPosRot(Vec3& pos, Quat& rot)
{
	MFnIkJoint		ikjoint(m_MayaObj);
	MVector			localpos;
	MQuaternion		scaleorient;
	MQuaternion		jointorient;
	MQuaternion		jointrot;
	Quat			localrot(0,0,0,1);

	ikjoint.getOrientation(jointorient);
	ikjoint.getScaleOrientation(scaleorient);
	ikjoint.getRotation(jointrot);
	localpos = ikjoint.getTranslation(MSpace::kTransform);
	localrot *= Quat((float) scaleorient.x, (float) scaleorient.y, (float) scaleorient.z, (float) scaleorient.w);
	localrot *= Quat((float) jointrot.x, (float) jointrot.y, (float) jointrot.z, (float) jointrot.w);
	localrot *= Quat((float) jointorient.x, (float) jointorient.y, (float) jointorient.z, (float) jointorient.w);
	pos.Set(localpos.x, localpos.y, localpos.z);
	rot.Set(localrot.x, localrot.y, localrot.z, localrot.w);
}

void meConvJoint::CalcJointMatrix(Matrix& worldmtx)
{
	Transformer*	joint = (Transformer*) GetVXObj();
	Vec3			localpos;
	Quat			localrot;
	Matrix			localmtx;

	GetMatrix(localmtx);
	joint->SetTransform(&localmtx);
	localrot = joint->GetRotation();
	localpos = joint->GetPosition();
	meLog(3, "%s:\tlrot (%.3f, %.3f, %.3f, %.3f)\tlpos (%.3f, %.3f, %.3f)",
			GetMayaName(), localrot.x, localrot.y, localrot.z, localrot.w,
			localpos.x, localpos.y, localpos.z);
	joint->CalcMatrix();
	worldmtx.Copy(joint->GetTotalTransform());
}

/*
 * Link this joint to a parent joint or a parent skeleton (if it is a root joint).
 * Maya dagPose object (skeleton bind pose) connects to worldMatrix plug of all skeleton joints.
 * meConvSkeleton	-> Skeleton
 *	meConvJoint (root)	-> Transformer
 *		meConvJoint	(torso)	-> Transformer
 *		meConvJoint (waist)	-> Transformer
 *
 * meConvJoint::Make already made the meConvSkeleton for each dagPose attached to the
 * joint. There can only be one Vixen Skeleton so we will use the meConvSkeleton
 * attached to the root joint and warn if there are others.
 * The joints with IK controls are marked using HasIK (kIkHandle, kIkEffector).
 * This function links in middle joints kJoint) as well. 
 *	...
 */
int meConvJoint::Link(meConvObj* convparent)
{
	meConvJoint*	pconv = dynamic_cast<meConvJoint*>( convparent );

	if (meConvEng::Link(convparent))
		pconv = dynamic_cast<meConvJoint*>( Parent() );
	/*
	 * Propagate the IK chain: start is IkHandle, end is IkEffector
	 * Mark middle joints as IkJoint. If any joint in the IK chain
	 * is animated, all of them are assumed to be.
	 */
	if (pconv)
	{
		if (!HasIK)
		{
			if (pconv->HasIK == MFn::kIkHandle)	// parent is start of IK chain?
			{
				HasIK = MFn::kJoint;			// this joint is in the middle
				HasAnimation |= pconv->HasAnimation;
			}
		}
		else if ((HasIK != MFn::kIkHandle) && !pconv->HasIK)
		{
			pconv->HasIK = MFn::kJoint;			// our parent is in the middle
			pconv->HasAnimation |= HasAnimation;
		}
	}
	/*
	 * If our parent is not a joint converter, we are the root joint of a skeleton
	 * Find any skins attached to this joint and link them to us.
	 */
	else
		meConvSkin::LinkSkeleton(this);
	return 1;
}


SharedObj* meConvJoint::LinkTarget(meConvObj* conv)
{
	Transformer* joint = (Transformer*) GetVXObj();
	Model*		target = (Model*) meConvEng::LinkTarget(conv);
	meConvNode*	nconv = dynamic_cast<meConvNode*>( conv );

   /*
	* In the case of a model parented to a joint in Maya,
	* we make a Transformer engine and a Model in Vixen (two nodes),
	* one in the simulation tree and one in the scene graph.
	* The Model becomes a target of the Transformer.
	*/
	if (joint && nconv && nconv->ParentIsJoint)
		joint->SetOptions(Transformer::WORLD);
	return target;
}

int meConvJoint::Convert(meConvObj*)
{
	meConvJoint*	pconv = dynamic_cast<meConvJoint*>( Parent() );
	Skeleton*		skel = m_Skeleton;
	Transformer*	joint = (Transformer*) (SharedObj*) m_VXObj;

    if (joint == NULL)
        return -1;
	meLog(1, "%s: Converting joint", GetMayaName());
	if ((pconv == NULL)	&&		// if it is the root joint, make the skeleton
		!DoLink)				// is this one linked to anything?
		MakeSkeleton();
	/*
	 * If we are exporting animation, then link in animation controller engines.
	 */
	if (Exporter->DoAnimation)
		LinkAnimation(m_MayaObj, pconv);
   return 1;
}

Skeleton* meConvJoint::MakeSkeleton()
{
	Skeleton*		skel = m_Skeleton;
	Transformer*	rootjoint = (Transformer*) GetVXObj();
	meConvEng*		conv;
	meConvJoint*	jconv;
	int				numbones = 0;

	if (skel != NULL)
		return skel;
	/*
	 * Count the joints and give them IDs based on breadth first scan.
	 */
	MItDag dagiter(MItDag::kBreadthFirst, MFn::kJoint, &m_Status);
	dagiter.reset(GetMayaObj(),MItDag::kBreadthFirst,MFn::kJoint); 
	for ( ; !dagiter.isDone(); dagiter.next() )
    {
		MDagPath	dagPath;
		meConvObj*	conv;
		meConvJoint* jconv;
		ConvRef		cvref;

		m_Status = dagiter.getPath(dagPath);
		if (MS::kSuccess != m_Status)
			continue;
		cvref = Exporter->MayaNodes[dagPath];
		conv = cvref;
		jconv = dynamic_cast< meConvJoint* > ( conv );

		if (jconv)
		{
			Transformer*	bone = (Transformer*) jconv->GetVXObj();
			jconv->m_JointIndex = numbones++;
			bone->CalcMatrix();
		}
	}
	/*
	 * Make the skeleton and link it as our parent
	 */
 	m_Skeleton = skel = new Skeleton(numbones);
	Exporter->SimRoot->Append(skel);
	skel->SetBoneAxis(Vec3(1.0f, 0.0f, 0.0f));
	SetName(skel, Core::String(GetMayaName()) + ".skeleton");
	rootjoint->Remove(Group::UNLINK_NOFREE);
	skel->Append(rootjoint);
	m_Skeleton = skel;
	meLog(1, "%s: Converting skeleton", skel->GetName());
	meLog(1, "\t%s -> %s", rootjoint->GetName(), skel->GetName());
	/* 
	 * Name the bones of the skeleton and establish bone indices
	 */
	meConvEng::Iter	iter(this, Group::DEPTH_FIRST);
	while (conv = (meConvEng*) iter.Next())
	{
		jconv = dynamic_cast< meConvJoint* > ( conv );
		if (jconv)
		{
			int				boneindex = jconv->GetJointIndex();
			Transformer*	bone = (Transformer*) jconv->GetVXObj();
			meConvJoint*	pconv = dynamic_cast< meConvJoint* >( jconv->Parent() );

			VX_ASSERT(bone && bone->IsClass(VX_Transformer));
			bone->SetBoneIndex(boneindex);
			skel->SetBoneName(boneindex, jconv->GetMayaName());
			if (pconv)
				skel->SetParentBoneIndex(boneindex, pconv->GetJointIndex());
			meLog(1, "\tbone %d -> %s", boneindex, jconv->GetMayaName());
		}
	}
	Ref<Pose> pose = skel->MakePose(Pose::SKELETON);

	pose->SetName(Core::String(skel->GetName()) + ".bindpose");
	skel->FindBones(pose);
	skel->SetBindPose(pose);
	skel->GetPose()->Copy(pose);
	if (Exporter->LogLevel > 2)
		pose->Print(*(Exporter->LogFile), SharedObj::PRINT_All);
    return skel;
}

int meConvJoint::MakeJointMap(const MDagPathArray& influences, int* jointmap)
{
	meConvObj::Iter iter(this, Group::DEPTH_FIRST);
	meConvObj*		conv;
	int				njoints = 0;
	int				n = influences.length();
	uint32			i;

	for (i = 0; i < n; ++i)
		jointmap[i] = -1;
	while (conv = (meConvObj*) iter.Next())
	{
		meConvJoint* joint = dynamic_cast< meConvJoint* >( conv );
		int			 jointindex;
		
		if (joint == NULL)
			continue;
		jointindex = joint->GetJointIndex();
		for (i = 0; i < n; ++i)
		{
			MDagPath ipath(influences[i]);
			MDagPath jpath(joint->GetDagPath());

			if (ipath == jpath)
			{
				jointmap[i] = jointindex;
				++njoints;
				meLog(2, "\t%s[%d] joint mapped to %d", joint->GetMayaName(), i, jointindex);
				break;
			}
		}
	}
	if (njoints != n)
	{	ME_ERROR(("WARNING: %s has %d joints but %d influences", GetMayaName(), njoints, n), njoints); }
	return njoints;
}


int meConvJoint::LinkAnimation(MObject& mayaobj, meConvJoint* parentconv)
{
	MFnDependencyNode	node(mayaobj);
	Transformer*		xform = (Transformer*) GetVXObj();
	Core::String		basename;
	int					found_animation = 0;
	basename = xform->GetName();
#if 0
	MAnimUtil::findAnimatedPlugs(mayaobj, animplugs, false, &m_Status);
	for (int i = 0; i < animplugs.length(); ++i)
	{
		MPlug		plug = animplugs[i];
		MPlug		parentplug;
		Core::String s = plug.name().asChar();

		if (s.FindNoCase(TEXT("translate")) >= 0)
		{
			if (GetMayaObj() != mayaobj)
				found_animation |= 1;
			if (!parentconv && (Position == NULL))
				found_animation |= 2;
		}
		else if ((s.FindNoCase(TEXT("rotate")) >= 0))
		{
			if (GetMayaObj() != mayaobj)
				found_animation |= 1;
			if (Rotation == NULL)
				found_animation |= 1;
		}
		else if ((s.FindNoCase(TEXT("scale")) >= 0))
			found_animation |= 1;
	}
	if (found_animation & 1)
	{
		Interpolator* rotate = new Interpolator();
		rotate->SetName(basename + TEXT(".rot"));
		rotate->SetInterpType(Interpolator::QSTEP);
		rotate->SetDestType(Interpolator::ROTATION);
		rotate->SetValSize(sizeof(Interpolator::WQuat) / sizeof(float));
		Rotation = rotate;
		xform->Append(rotate);
		meLog(1, "%s: Sampling rotation as %s", GetMayaName(), rotate->GetName());
		HasAnimation = true;
	}
	if (found_animation & 2)
	{
		Interpolator* translate = new Interpolator();
		translate->SetName(basename + TEXT(".pos"));
		translate->SetInterpType(Interpolator::STEP);
		translate->SetValSize(sizeof(Vec3) / sizeof(float));
		translate->SetDestType(Interpolator::POSITION);
		Position = translate;
		xform->Append(translate);
		meLog(1, "%s: Sampling translation as %s", GetMayaName(), translate->GetName());
		HasAnimation = true;
	}
	if (found_animation)
		return true;
	for (int j = 0; j < 3; j++)
	{
		const char* pname = plugnames[j];
		meConnIter iter(mayaobj, pname, false);
		while (iter.Next(parentobj, MFn::kTransform))
		{
			if ((parentobj != GetMayaObj()) &&
				(parentobj != mayaobj))
				found_animation |= LinkAnimation(parentobj, parentconv));
		}
	}
	return found_animation;
#else
	if (!HasAnimation)
		return 0;
	HasAnimation = ROTATION;
	if (Rotation == NULL)
	{
		Interpolator* rotate = new Interpolator();
		rotate->SetName(basename + TEXT(".rot"));
		rotate->SetInterpType(Interpolator::QSTEP);
		rotate->SetDestType(Interpolator::ROTATION);
		rotate->SetValSize(sizeof(Quat) / sizeof(float));
		Rotation = rotate;
		xform->Append(rotate);
		meLog(1, "%s: Sampling rotation as %s", GetMayaName(), rotate->GetName());
		HasAnimation = true;
	}
	if (!parentconv && (Position == NULL))
	{
		Interpolator* translate = new Interpolator();
		translate->SetName(basename + TEXT(".pos"));
		translate->SetInterpType(Interpolator::STEP);
		translate->SetValSize(sizeof(Vec3) / sizeof(float));
		translate->SetDestType(Interpolator::POSITION);
		Position = translate;
		xform->Append(translate);
		meLog(1, "%s: Sampling translation as %s", GetMayaName(), translate->GetName());
		HasAnimation = true;
	}
	return HasAnimation;
#endif

}