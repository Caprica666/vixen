#pragma once

/*!
 * @file vxposemapper.h
 * @brief Animation retargeting base class.
 *
 * Retargets animation from one skeleton to another.
 *
 * @author Nola Donato
 * @ingroup vixen
 */
namespace Vixen {


/*!
 * @class PoseMapper
 *
 * @brief Engine that maps a pose from one skeleton onto another.
 * This class provides a simple animation retargeting capability.
 * It maps the poses of it's parent skeleton to a different target skeleton.
 *
 * @see Skeleton Pose HavokPoseMapper
 * @ingroup vixen
 */
class PoseMapper : public Engine
{
public:
	VX_DECLARE_CLASS(PoseMapper);

	PoseMapper();
	~PoseMapper();

	//! Get source skeleton.
	Skeleton*		GetSource() const			{ return m_Source; }

	//! Set source skeleton
	void			SetSource(Skeleton*);

	//! Return the bone map between target and source skeletons.
	const IntArray&	GetBoneMap() const			{ return m_BoneMap; }

	//! Establish mapping between target and source skeletons.
	void			SetBoneMap(int32* intArray);

	//! Select one of the built-in bone mapping tables.
	bool			SelectBoneMap(const TCHAR* mapname);

	//! Map source skeleton pose to target skeleton in local space.
	virtual bool	MapLocalToTarget();

	//! Map source skeleton pose to target skeleton in world space.
	virtual bool	MapWorldToTarget();

	//! Map destination skeleton pose to source skeleton in local space.
	virtual bool	MapWorldToSource();

	static	void	MakeBoneMap(IntArray& bonemap, const Skeleton* srcskel, const Skeleton* dstskel);

	enum Opcode
	{
		POSEMAPPER_SetSource =  Engine::ENG_NextOp,
		POSEMAPPER_SetBoneMap,
		POSEMAPPER_NextOp =  Engine::ENG_NextOp + 10,
	};

	virtual void	SetTarget(SharedObj* target);
	virtual	int		Save(Messenger&, int) const;
	virtual	bool	Copy(const SharedObj*);
	virtual	bool	Do(Messenger& s, int op);
	virtual bool	Eval(float t);

protected:
	Skeleton*		GetDest() const;

	WeakRef<Skeleton>	m_Source;		// source skeleton
	IntArray			m_BoneMap;		// map target bones to source bones
	Ref<Pose>			m_DestPose;		// last computed pose for target skeleton
	static const TCHAR*	DAZBoneNames[Skeleton::NUM_BONES]; 
	static const TCHAR*	AMYBoneNames[Skeleton::NUM_BONES]; 
};

inline Skeleton* PoseMapper::GetDest() const
{
	Skeleton*	skel = (Skeleton*) GetTarget();
	
	if (skel && skel->IsClass(VX_Skeleton))
		return skel;
	skel = (Skeleton*) Parent();
	if (skel && skel->IsClass(VX_Skeleton))
		return skel;
	return NULL;
}

}	// end Vixen