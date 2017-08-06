#include "vixen.h"

namespace Vixen {

static const TCHAR* opnames[] = {
		TEXT("SetBoneMap"),
		TEXT("MakeBoneMap"),
};

const TCHAR** PoseMapper::DoNames = opnames;

const TCHAR* PoseMapper::DAZBoneNames[Skeleton::NUM_BONES] = {
	TEXT("hip"),
	TEXT("abdomen"),
	TEXT("neck"),
	TEXT("head"),
	TEXT("rCollar"),
	TEXT("rShldr"),
	TEXT("rForeArm"),
	TEXT("rHand"),
	NULL,		// TEXT("rCarpal1"),
	NULL,		// TEXT("rIndex"),
	TEXT("lCollar"),
	TEXT("lShldr"),
	TEXT("lForeArm"),
	TEXT("lHand"),
	NULL,		// TEXT("lCarpal1"),
	NULL,		// TEXT("lIndex"),
	TEXT("rThigh"),
	TEXT("rShin"),
	TEXT("rFoot"),
	NULL,		// TEXT("rToe"),
	TEXT("lThigh"),
	TEXT("lShin"),
	TEXT("lFoot"),
	NULL,		// TEXT("lToe"),
};

const TCHAR* PoseMapper::AMYBoneNames[Skeleton::NUM_BONES] = {
	TEXT("amyroot"),
	TEXT("spine1"),
	TEXT("neck"),
	TEXT("head1"),
	TEXT("Rclavicle"),
	TEXT("Rshoulder_O"),
	TEXT("Relbow_O"),
	TEXT("Rwrist_O"),
	NULL,		// TEXT("rCarpal1"),
	NULL,		// TEXT("rIndex"),
	TEXT("Lclavicle"),
	TEXT("Lshoulder_O"),
	TEXT("Lelbow_O"),
	TEXT("Lwrist_O"),
	NULL,		// TEXT("lCarpal1"),
	NULL,		// TEXT("lIndex"),
	TEXT("Rleg"),
	TEXT("Rknee"),
	TEXT("Rankle"),
	TEXT("Rball"),
	TEXT("Lleg"),
	TEXT("Lknee"),
	TEXT("Lankle"),
	TEXT("Lball"),
};

VX_IMPLEMENT_CLASS(PoseMapper, Engine);


/*!
 * @fn PoseMapper::PoseMapper()
 *
 * Constructs an animation retargeting engine.
 */

PoseMapper::PoseMapper()
:	Engine()
{

}

/*
 * @fn bool PoseMapper::SetSource(Skeleton* source)
 * @param source	source skeleton
 *
 * The source skeleton provides the input animation
 * which will be remapped to the target skeleton.
 *
 * @see Engine::SetTarget Skeleton MapPose
 */
void	PoseMapper::SetSource(Skeleton* source)
{
	VX_STREAM_BEGIN(s)
		*s << OP(VX_PoseMapper, POSEMAPPER_SetSource) << this << source;
	VX_STREAM_END(  )

	m_Source = source;
	if (source)
	{
		source->MakeLock();
		source->GetPose()->MakeLock();
	}
}

/*
 * @fn bool PoseMapper::SetBoneMap(int32* bonemap)
 * @param bonemap	bone mapping data
 *
 * The bone map specifies the source bone index for each
 * target bone. The input should be an array of integers
 * with an entry for each target skeleton bone.
 * In the case where the target skeleton has a bone that is not in
 * the source skeleton, the index should be -1. 
 *
 * @see Engine::SetTarget Skeleton MapPose
 */
void	PoseMapper::SetBoneMap(int32* bonemap)
{
	Skeleton*	dstskel = GetDest();
	int			numbones;

	if (bonemap == NULL)
		return;
	if (dstskel == NULL)
		return;
	numbones = dstskel->GetNumBones();
	if (numbones == 0)
		return;
	VX_STREAM_BEGIN(s)
		*s << OP(VX_PoseMapper, POSEMAPPER_SetBoneMap) << this << numbones;
		s->Output(bonemap, numbones);
	VX_STREAM_END(  )

	m_BoneMap.SetSize(numbones);
	memcpy(bonemap, m_BoneMap.GetData(), numbones);
}

void	PoseMapper::SetTarget(SharedObj* target)
{
	Skeleton*	dstskel = (Skeleton*) target;

	if (dstskel && dstskel->IsClass(VX_Skeleton))
	{
		m_BoneMap.SetSize(dstskel->GetNumBones());
		dstskel->MakeLock();
		dstskel->GetPose()->MakeLock();
		dstskel->GetBindPose()->MakeLock();
	}
	Engine::SetTarget(target);
}

/*
 * Allow the data area for the bitmaps for color and depth to be freed
 */
PoseMapper::~PoseMapper()
{
}

/*!
 * @fn void PoseMapper::MakeBoneMap(IntArray& bonemap, const Skeleton* srcskel, const Skeleton* dstskel)
 * @param	bonemap	array to get bone mapping indices
 * @param	srcskel	source Skeleton
 * @pararm	dstskel	destination Skeleton
 *
 * Constructs a bone mapping table which gives the index of the destination skeleton bone
 * corresponding to each bone in the source skeleton.
 *
 * @see PoseMapper::MapPose
 */
void PoseMapper::MakeBoneMap(IntArray& bonemap, const Skeleton* srcskel, const Skeleton* dstskel)
{
	int			numsrcbones = srcskel->GetNumBones();
	ObjectLock	lock1(srcskel);
	ObjectLock	lock2(dstskel);

	bonemap.SetSize(numsrcbones);
	for (int i = 0; i < numsrcbones; ++i)
	{
		int				boneindex = -1;
		const TCHAR*	bonename = srcskel->GetBoneName(i);
		
		if (bonename)
			boneindex = dstskel->GetBoneIndex(bonename);
		if (boneindex < 0)
			{ VX_WARNING(("MakeBoneMap: cannot find bone %s\n", bonename)); }
		bonemap[boneindex] =  i;
	}
}

/*
 * Updates the color and depth map textures from the Kinect cameras.
 * If a Skeleton is our target or a child, we update the joint angles
 * for the user associated with it.
 */
bool PoseMapper::Eval(float t)
{
	if (m_Source == NULL)
		return true;
	if (m_Source->IsActive())
		MapLocalToTarget();
	return true;
}

/*!
 * @fn void PoseMapper::SelectBoneMap(const TCHAR* mapname)
 * @param mapname	name of bone map to use:
 *					"default" Skeleton bone names
 *					"genesis" DAZ Studio Genesis figure bone mapping
 *					"amy" Amy's rig bone mapping
 *
 * Determines how to map the bones in the destination skeleton to the source skeleton bones
 * based on their names. There are two built-in bone mapping tables for the
 * default Vixen Skeleton (the default) and the DAZ Studio Genesis figure (DAZBoneNames).
 * This function will only work if the source skeleton is compatible with the
 * default skeleton. You can use MakeBoneMap to construct a custom bone map for other skeletons.
 *
 * @return true if bone map selected, else false (skeleton mismatch)
 *
 * @see Skeleton PoseMapper::MakeBoneMap Skeleton::MakeBoneMap
 */
bool PoseMapper::SelectBoneMap(const TCHAR* mapname)
{
	const Skeleton*	srcskel = m_Source;
	const Skeleton*	dstskel = GetDest();
	const TCHAR** bonenames;

	if ((dstskel == NULL) || (srcskel == NULL))
		return false;
	if (mapname == NULL)
		mapname = TEXT("default");
	if (srcskel->GetNumBones() != Skeleton::NUM_BONES)
		VX_ERROR(("PoseMapper::SelectBoneMap %s incompatible with source skeleton\n", mapname), false);

	m_BoneMap.SetSize(Skeleton::NUM_BONES);
	if (STRCASECMP(mapname, TEXT("genesis")) == 0)
		bonenames = DAZBoneNames;
	else if (STRCASECMP(mapname, TEXT("amy")) == 0)
		bonenames = AMYBoneNames;
	else
		bonenames = Skeleton::DefaultBoneNames;
	dstskel->MakeBoneMap(bonenames, m_BoneMap.GetData(), Skeleton::NUM_BONES);
	return true;
}

/*!
 * @fn bool PoseMapper::MapLocalToTarget()
 *
 * Maps the pose of the source skeleton onto the destination skeleton in local space.
 * The local bone rotations of matching bones are copied.
 * If the PoseMapper has a bone map, it is used to determine which bones
 * of the source skeleton correspond to which bones in the destination skeleton.
 * This function requires both the source and target skeletons to be set.
 *
 * @returns true if successful, false on error
 */
bool PoseMapper::MapLocalToTarget()
{
	const Skeleton*	srcskel = m_Source;
	Skeleton*		dstskel = GetDest();

	if ((dstskel == NULL) || (srcskel == NULL))
		return false;
	if (m_BoneMap[0] < 0)
		MakeBoneMap(m_BoneMap, srcskel, dstskel);
	VX_TRACE2(PoseMapper::Debug, ("PoseMapper::MapLocalToTarget %s -> %s\n", srcskel->GetName(), dstskel->GetName()));
	const Pose*	srcpose = srcskel->GetPose();
	const Pose*	bindpose = srcskel->GetBindPose();
	ObjectLock	lock1(srcpose);
	ObjectLock	lock2(bindpose);
	Pose*		pose = m_DestPose;
	int			numsrcbones = srcpose->GetNumBones();
	int			numdstbones;

	srcpose->Sync();
	if (pose == NULL)
		m_DestPose = pose = dstskel->MakePose(Pose::BIND_POSE_RELATIVE);
	else
		pose->ClearRotations();
	numdstbones = pose->GetNumBones();
	dstskel->SetPosition(srcskel->GetPosition());
	for (int i = 0; i < numsrcbones; ++i)
	{
		int	boneindex = m_BoneMap[i];
		
		if (boneindex >= 0)
		{
			Quat	q(srcpose->GetWorldRotation(i));
			Quat	b(bindpose->GetWorldRotation(i));

			b.Conjugate();					// make it bind pose relative
			q *= b;
			pose->SetWorldRotation(boneindex, q);
			VX_TRACE2(PoseMapper::Debug, ("\t%s[%d] -> %s[%d] (%0.3f, %0.3f, %0.3f, %0.3f)\n",
					srcskel->GetBoneName(i), i,
					dstskel->GetBoneName(boneindex), boneindex,
					q.x, q.y, q.z, q.w));
		}
	}
	dstskel->ApplyPose(pose);
	return true;
}


/*!
 * @fn bool PoseMapper::MapWorldToTarget()
 *
 * Maps the pose of the source skeleton onto the destination skeleton in world space.
 * The world bone rotations of matching bones are copied.
 * If the PoseMapper has a bone map, it is used to determine which bones
 * of the source skeleton correspond to which bones in the destination skeleton.
 */
bool PoseMapper::MapWorldToTarget()
{
	const Skeleton*	srcskel = m_Source;
	Skeleton*		dstskel = GetDest();

	if ((dstskel == NULL) || (srcskel == NULL))
		return false;
	if (m_BoneMap[0] < 0)
		MakeBoneMap(m_BoneMap, srcskel, dstskel);
	const Pose*	srcpose = srcskel->GetPose();
	Pose*		dstpose = dstskel->GetPose();
	ObjectLock	lock1(srcpose);
	ObjectLock	lock2(dstpose);
	int			numsrcbones = srcpose->GetNumBones();

	srcpose->Sync();
	dstpose->SetPosition(srcpose->GetWorldPosition(0));
	for (int i = 0; i < numsrcbones; ++i)
	{
		int	boneindex = m_BoneMap[i];
		Matrix	mtx;
		
		if (boneindex >= 0)
		{
			srcpose->GetWorldMatrix(i, mtx);
			dstpose->SetWorldMatrix(boneindex, mtx);
		}
	}
	dstpose->Sync();
	return true;
}

/*!
 * @fn bool PoseMapper::MapWorldToSource()
 *
 * Maps the pose of the destination skeleton onto the source skeleton in world space.
 * The world bone rotations of matching bones are copied.
 * If the PoseMapper has a bone map, it is used to determine which bones
 * of the source skeleton correspond to which bones in the destination skeleton.
 */
bool PoseMapper::MapWorldToSource()
{
	const Skeleton*	srcskel = m_Source;
	Skeleton*		dstskel = GetDest();

	if ((dstskel == NULL) || (srcskel == NULL))
		return false;
	if (m_BoneMap[0] < 0)
		MakeBoneMap(m_BoneMap, srcskel, dstskel);

	Pose*		srcpose = srcskel->GetPose();
	const Pose*	dstpose = dstskel->GetPose();
	ObjectLock	lock1(srcpose);
	ObjectLock	lock2(dstpose);
	int			numsrcbones = srcpose->GetNumBones();

	dstpose->Sync();
	srcpose->SetPosition(dstpose->GetWorldPosition(0));
	for (int i = 0; i < numsrcbones; ++i)
	{
		int	boneindex = m_BoneMap[i];
		Matrix	mtx;
		
		if (boneindex >= 0)
		{
			dstpose->GetWorldMatrix(boneindex, mtx);
			srcpose->SetWorldMatrix(i, mtx);
		}
	}
	srcpose->Sync();
	return true;
}


bool PoseMapper::Copy(const SharedObj* src_obj)
{
	const PoseMapper* src = (const PoseMapper*) src_obj;
	
	ObjectLock dlock(this);
	ObjectLock slock(src);
	if (!Engine::Copy(src_obj))
		return false;
	if (src_obj->IsClass(VX_PoseMapper))
	{
		m_Source = src->m_Source;
		m_BoneMap.Copy(&(src->m_BoneMap));
	}
	return true;
}


int PoseMapper::Save(Messenger& s, int opts) const
{
	int32	h = Engine::Save(s, opts);
	int		nbones;

	if (h <= 0)
		return h;
	if (m_Source != NULL)
 		s << OP(VX_PoseMapper, POSEMAPPER_SetSource) << h << m_Source;
	nbones = (int) m_BoneMap.GetSize();
	if (nbones > 0)
	{
		s << OP(VX_PoseMapper, POSEMAPPER_SetBoneMap) << h << int32(nbones);
		s.Output(m_BoneMap.GetData(), nbones);
	}
	return h;
}

bool PoseMapper::Do(Messenger& s, int op)
{
	int32		n;
	SharedObj*	obj;
	int32*		data;

	switch (op)
	{
		case POSEMAPPER_SetSource:
		s >> obj;
		SetSource((Skeleton*) obj);
		break;

		case POSEMAPPER_SetBoneMap:
		s >> n;
		VX_ASSERT(n > 0);
		data = (int32*) alloca(n * sizeof(int32));
		s.Input(data, n);
		SetBoneMap(data);
		break;

		default:
		return Engine::Do(s, op);
	}
#ifdef _DEBUG
	if (s.Debug)
		endl(vixen_debug << ClassName() << "::"
					   << PoseMapper::DoNames[op - POSEMAPPER_SetSource]
					   << " " << this);
#endif
	return true;
}

}	// end Vixen