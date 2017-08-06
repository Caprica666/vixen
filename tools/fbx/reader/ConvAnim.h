#pragma once

namespace Vixen { namespace FBX {

/*
 * Class for conversion from an FbxSkeleton limb to a Vixen Transformer (bone) object.
 * It handles calculating the bone transformation matrix and linking the bone
 * into the Vixen Skeleton hierarchy.
 */
class ConvBone : public ConvNode
{
public:
	ConvBone(FbxNode* pNode, const TCHAR* name = NULL) : ConvNode(pNode, name) { }
	Transformer*		MakeBone(Engine* parent = NULL);
	virtual SharedObj*	Convert();
	static Skeleton*	FindSkeleton(Transformer* bone);
};

/*
 * Class for conversion from an FbxSkeleton root to a Vixen Skeleton object.
 * It handles calculating the bone linkage and bind pose for the skeleton.
 */
class ConvSkeleton : public ConvNode
{
public:
	ConvSkeleton(FbxNode* pNode, const TCHAR* name = NULL) : ConvNode(pNode, name) { }
	Skeleton*			MakeSkeleton(Engine* parent = NULL);
	virtual SharedObj*	Convert();
	int					CountBones(FbxNode* pSkelNode);
	static void			CalcBindPose(Skeleton* skel);
	static int			NumBones;
};

/*
 * Base class for conversion from an Fbxeformer to a Vixen Deformer object.
 * It can also perform the more specific conversion from FbxBlendShapeChannel
 * to Vixen Morph (blend shape deformation).
 *
 * This class is a factory for making more specific converters:
 *	FbxBlendShapeChannel	Vixen::Morph
 *	FbxSkin					Vixen::Skin
 *	FbxVertexCacheDeformer	Vixen::MeshAnim
 */
class ConvDeformer : public ConvNode
{
public:
	ConvDeformer(FbxBlendShapeChannel* pBlend, FbxMesh* mesh, const TCHAR* name = NULL) : ConvNode(pBlend, name)
	{ mBlend = pBlend; mFbxMesh = mesh; }

	ConvDeformer(FbxDeformer* src, FbxMesh* mesh, const TCHAR* name = NULL) : ConvNode(src, name)
	{ mBlend = NULL; mFbxMesh = mesh; }

	Morph*				MakeMorph(const TCHAR* vtxdesc, int index);
	virtual SharedObj*	Convert();
	static Engine*		MakeDeformers(const TCHAR* name, FbxMesh* pFbxMesh, VertexArray* pVerts, IntArray* vertexmap);

protected:
	FbxMesh*				mFbxMesh;
	FbxBlendShapeChannel*	mBlend;
	const TCHAR*			mVertDesc;
};

/*
 * Class for conversion from an FbxSkin to a Vixen Skin object.
 * It computes the rest locations of the skin, including vertex weights for
 * each bone index. It also computes the bind pose of the skeleton.
 */
class ConvSkin : public ConvDeformer
{
public:
	ConvSkin(FbxSkin* pSkin, FbxMesh* mesh, const TCHAR* name = NULL) : ConvDeformer(pSkin, mesh, name)
	{ mFbxSkin = pSkin;  }

	Skin*				MakeSkin(int index);
	void				InitWeights();
	virtual SharedObj*	Convert();

protected:
	int				ConvertCluster(FbxCluster* cluster);
	void			NormalizeWeights();
	void			CalcBindPose(Skeleton* skel);

	FbxSkin*		mFbxSkin;
	int				mBoneIndex;
	Array<Matrix>	mBindPose;
};

/*
 * Class for conversion from an FbxVertexCacheDeformer to a Vixen MeshAnim object.
 * It reads the FBX vertex cache files and computes the vertex array sources
 * for the Vixen MeshAnimator.
 */
class ConvMeshAnim : public ConvDeformer
{
public:
	ConvMeshAnim(FbxVertexCacheDeformer* pCache, FbxMesh* mesh, const TCHAR* name = NULL) : ConvDeformer(pCache, mesh, name) { mCacheDeformer = pCache; }
	MeshAnimator*		MakeMeshAnim();
	virtual SharedObj*	Convert();
	int					ReadCache();
	bool				OpenCache();
	void				CloseCache();
	int					ReadCacheChannel(int i);
	VertexArray*		ReadCacheSample(int channel, FbxTime time);
	const FbxTime&		GetStartTime() const	{ return mCacheStart; }
	const FbxTime&		GetStopTime() const		{ return mCacheStop; }

protected:
	FbxVertexCacheDeformer*	mCacheDeformer;
	FbxTime					mCacheStart;
	FbxTime					mCacheStop;
};

/*
 * Class to convert position and rotation animation from an FBX node
 * to a Vixen Transformer with Interpolator children.
 * Samples the local matrix on the node and extracts the position and rotation
 * track to populate the engines.
 */
class ConvMatrixAnim : public ConvNode
{
public:
	ConvMatrixAnim(FbxNode* node, const char* name = NULL);
	Transformer*		MakeMatrixAnim(Engine* parent = NULL, Transformer* root = NULL, bool dotranslation = false);
	virtual SharedObj*	Convert();
	int					ComputeAnimation(FbxTime timeinc);

protected:
	Interpolator*		m_Translation;
	Interpolator*		m_Rotation;
	FbxTimeSpan			m_TimeSpan;
};

} }	// end FBX


