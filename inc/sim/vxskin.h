/*!
 * @file vxskin.h
 * @brief Skinning engine that controls a set of vertex deformations.
 *
 * Coordinates deformations done by child engines across a single set of vertices.
 *
 * @author Nola Donato
 * @ingroup vixen
 *
 * @see vxdeformer.h vxbone.h vxmorph.h
 */
#pragma once

namespace Vixen {

/*!
 * @class Skin
 * @brief Controls the deformation of a single vertex array based on a set of transforms.
 *
 * The target of a skin mesh must be a vertex array. The starting locations and normals in the vertex array
 * represent the neutral position. These source vertices, together with the bone transformations,
 * are used to calculate new vertices for the target mesh.
 * Typically, all vertices in the target mesh will be updated each frame.
 *
 * @image html character_animation.png
 *
 * Although a skin is typically associated with a Skeleton and gets its bone transformations from that,
 * you can also supply the matrix palette directly.
 *
 * To implement a skeletal animation system where the bones of a rigid skeleton
 * are used to control different mesh sections, each neutral vertex has a set
 * of influence weights for each bone that affects the mesh. The skin also has
 * a mapping table that contains the transformation matrix for the bone in the
 * coordinate space of the neutral positions.
 *
 * Skinned animations are usually made with art tools which export the appropriate engine hierarchy.
 *
 * @see Transformer Deformer Skeleton ClothSkin SCAPESkin
 * @ingroup vixen
 */
class Skin : public Deformer
{
public:
	VX_DECLARE_CLASS(Skin);
	Skin();
	~Skin();

	void			SetSkeleton(Skeleton*);				//!< Set skeleton which controls this skin.
	Skeleton*		GetSkeleton() const;				//!< Get skeleton which controls this skin.
	int				GetNumBones() const;				//!< Get number of bones used by this skin (size of matrix array).
	int				GetBonesPerVertex() const;			//!< Get number of influences per vertex.
	float*			GetBoneWeights(intptr i = 0) const;
	int32*			GetBoneIndices(intptr i = 0) const;
	void			Validate(intptr start, intptr nvtx) const;

	virtual int		Save(Messenger&, int) const;
	virtual bool	Do(Messenger& s, int op);
	virtual bool	Copy(const SharedObj*);
	virtual bool	Eval(float t);
	virtual void	Compute(float t);
	virtual bool	Init(SharedObj* target, VertexCache* vdict = NULL);

	virtual DebugOut&	Print(DebugOut& = vixen_debug, int opts = SharedObj::PRINT_Default) const;
	enum Opcode
	{
		SKIN_SetSkeletonOLD = Deformer::DEFORM_SetSkeletonOLD,
		SKIN_SetSourceVertsOLD = Deformer::DEFORM_SetRestLocs,
		SKIN_SetMatricesOLD = Deformer::DEFORM_SetMatricesOLD,
		SKIN_SetVertexMapOLD = Deformer::DEFORM_SetVertexMap,

		SKIN_SetSkeleton = Deformer::DEFORM_Next,
		SKIN_Next = Deformer::DEFORM_Next + 10
	};

protected:
	virtual	bool	Reset();					//!< reset skin to pre-transformed state.
	void			DeformLocs(intptr start, intptr n);
	void			DeformLocsAndNormals(intptr start, intptr n);
	void			DeformNormals(intptr start, intptr n);

	WeakRef<Skeleton>	m_Skeleton;
	int					m_BonesPerVtx;
	int					m_BlendWeightOfs;
	int					m_BlendIndexOfs;
	int					m_NormalOfs;
	bool				m_Initialized;
	Array<Matrix>		m_Matrices;
};


/*!
 * @fn nt Skin::GetNumBones() const
 * @return number of bones used by this skin, same as size of matrix palette.
 *
 * @see Skin::SetMatrices Skeleton::GetNumBones
 */
inline int Skin::GetNumBones() const
{
	Skeleton* skel = GetSkeleton();
	if (skel == NULL)
		return 0;
	return skel->GetNumBones();
}

/*!
 * @fn Skeleton* Skin::GetSkeleton() const
 * Each vertex in the skin can be influenced by one or more bones. The skeleton
 * controls the location and orientation of each bone which influences the skin.
 *
 * @return skeleton which is controlling this skin
 *
 * @see Skeleton::GetSkinPose Skin::SetSkeleton Skeleton
 */
inline Skeleton* Skin::GetSkeleton() const
{
	Skeleton* skel = m_Skeleton;

	if (skel)
		return skel;
	skel = (Skeleton*) Parent();
	if (skel && skel->IsClass(VX_Skeleton))
		return skel;
	return NULL;
}


/*!
 * @fn float* Skin::GetBoneWeights(intptr i) const
 * @return pointer to bone weights, NULL if none found in source verts.
 *
 * @see Skin::GetBoneIndices Skin::SetSourceVerts
 */
FORCE_INLINE float* Skin::GetBoneWeights(intptr i) const
{
	if (m_RestLocs.IsNull())
		return NULL;
	if (m_BlendWeightOfs < 0)
		return NULL;
	float* data = (float*) m_RestLocs->GetData();
	data += i * m_RestLocs->GetVtxSize();
	return data + m_BlendWeightOfs;
}

/*!
 * @fn int32* Skin::GetBoneIndices(int i) const
 * @return pointer to bone indices, NULL if none found in source verts.
 *
 * @see Skin::GetBoneWeights Skin::SetSourceVerts
 */
FORCE_INLINE int32* Skin::GetBoneIndices(intptr i) const
{
	if (m_RestLocs.IsNull())
		return NULL;
	if (m_BlendIndexOfs < 0)
		return NULL;
	int32*	data = (int32*) m_RestLocs->GetData();
	data += i * m_RestLocs->GetVtxSize();
	return data + m_BlendIndexOfs;
}

/*!
 * @fn int Skin::GetBonesPerVertex() const
 * @return the maximum number of bones which can influence a single vertes.
 */
FORCE_INLINE int Skin::GetBonesPerVertex() const
{ return m_BonesPerVtx; }



} // end Vixen



