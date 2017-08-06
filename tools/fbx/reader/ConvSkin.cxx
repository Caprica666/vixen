#include "vixen.h"
#include "ConvNode.h"
#include "ConvMesh.h"
#include "ConvAnim.h"
#include "FBXReader.h"

namespace Vixen { namespace FBX {

/*
 * @fn Engine* ConvDeformer::MakeDeformers(const TCHAR* namebase, FbxMesh* pFbxMesh, VertexArray* pVerts, IntArray* vertexmap)
 * @param namebase	prefix for Vixen node names
 * @param pFbxMesh	FBX Mesh being deformed
 * @param pVerts	Vixen VertexArray for Vixen mesh being deformed
 * @param vertexmap	Maps vertices in the Vixen VertexArray to vertices in the FBX mesh
 *
 * This function examines all of the FBX deformers on a given FBX mesh and generates one or more Vixen
 * deformers which perform the same function on the Vixen mesh.
 *
 * FBX deformers operate on a set of control points with the unique location of each vertex in the
 * mesh being deformed. Each control point may have one or more normals and texture coordinates
 * depending on how many triangles share the vertex. Vixen meshes have a single normals and texcoord
 * per vertex but many vertices may have the same location. The input vertexmap array maps each
 * vertex in the Vixen mesh to the corresponding control point in the FBX mesh.
 *
 * Vixen deformers operate on a Vixen VertexArray of rest locations and use the same vertex map to map between
 * the output mesh and the rest locations. The Vixen rest locations are the same as the FBX control points.
 *
 * @return -> single Vixen Engine or group of Vixen engines, NULL if no FBX mesh deformations
 *
 * @code
 *	FBX						Vixen
 *	FbxBlendShapeChannel	Morph
 *	FbxSkin					Skin
 *	FbxVertexCacheDeformer	MeshAnimator
 * @endcode
 *
 * @see ConvDeformer::MakeMorph ConvSkin::MakeSkin ConvCache::MakeMeshAnim
 */
Engine* ConvDeformer::MakeDeformers(const TCHAR* namebase, FbxMesh* pFbxMesh, VertexArray* pVerts, IntArray* vertexmap)
{
	int		n = pFbxMesh->GetDeformerCount();
	
	if (n == 0)
		return NULL;

	ConvMesh		fbxmesh(pFbxMesh->GetNode());
	Deformer*		deform;
	Engine*			root = new Engine();
	int				childcount = 0;
	bool			hasnormals = (pVerts->GetStyle() & VertexPool::NORMALS) != 0;
	Core::String	name(namebase);

	pVerts->SetFlags(VertexPool::MORPH);
	root->SetName(name + TEXT(".deformers"));
	for (int i = 0; i < n; ++i)
	{
		FbxDeformer*	fbxdeform = pFbxMesh->GetDeformer(i);
		switch (fbxdeform->GetDeformerType())
		{
			/*
			 * FbxSkin becomes Vixen::Skin.
			 * Rest locations match FBX mesh control points.
			 * Each vertex has location, normal, 4 weights, 4 bone indices
			 */
			case FbxDeformer::EDeformerType::eSkin:
			{
				FbxSkin*		fbxskin = (FbxSkin*) fbxdeform;
				const TCHAR*	vtxdesc = TEXT("float4 position, float4 weight, int4 blendindex");
				VertexArray*	restlocs = fbxmesh.MakeVerts(vtxdesc);
				FloatArray*		restnmls = NULL;
				ConvSkin		convskin(fbxskin, pFbxMesh, name + TEXT(".skin"));

				restlocs->SetName(name + TEXT(".skin.rest"));
				deform = convskin.MakeSkin(0);
				deform->SetVertexMap(vertexmap);
				deform->SetRestLocs(restlocs);
				deform->SetTarget(pVerts);
				restnmls = deform->GetRestNormals();
				convskin.InitWeights();
				convskin.Convert();
				VX_TRACE(FBXReader::Debug, ("Linking skin %s -> mesh %s\n", fbxskin->GetName(), pFbxMesh->GetName()));
				if (deform->Parent() == NULL)
					++childcount;
				break;
			}
			break;

			/*
			 * FbxBlendShapeChannel becomes Vixen::Morph.
			 * Rest locations match FBX mesh control points.
			 * Each vertex has location, normal.
			 */
			case FbxDeformer::EDeformerType::eBlendShape:
			{
				Core::String	tmpname(name);
				const TCHAR*	vtxdesc = TEXT("float4 position");
				VertexArray*	restlocs = fbxmesh.MakeVerts(vtxdesc);
				FbxBlendShape*	blend = ((FbxBlendShape*) fbxdeform);
				int				nclusters = blend->GetBlendShapeChannelCount();
				int				index = (nclusters > 0) ? 1 : 0;

				tmpname += TEXT(".morph");
				restlocs->SetName(name + TEXT(".morph.rest"));
				for (int j = 0; j < nclusters; ++j)
				{
					FbxBlendShapeChannel* chan = blend->GetBlendShapeChannel(i);
					ConvDeformer	fbxmorph(chan, pFbxMesh, name);

					deform = fbxmorph.MakeMorph(vtxdesc, index);
					deform->SetVertexMap(vertexmap);
					deform->SetRestLocs(restlocs);
					deform->SetTarget(pVerts);
					fbxmorph.Convert();
					VX_TRACE(FBXReader::Debug, ("Linking morph (FBX blend shape deformer) %s -> mesh %s\n", tmpname, pFbxMesh->GetName()));
					++childcount;
					break;
				}
			}

			/*
			 * FbxVertexCacheDeformer becomes Vixen::MeshAnim.
			 * Rest locations match FBX mesh control points.
			 * Each vertex has location, normal.
			 */
			case FbxDeformer::EDeformerType::eVertexCache:
			{
				Core::String	tmpname(name);
				ConvMeshAnim	fbxcache((FbxVertexCacheDeformer*) fbxdeform, pFbxMesh, tmpname += TEXT(".meshanim"));
				const TCHAR*	vtxdesc = TEXT("float4 position");
				VertexArray*	restlocs = fbxmesh.MakeVerts(vtxdesc);

				restlocs->SetName(name + TEXT(".meshanim.rest"));
				deform = fbxcache.MakeMeshAnim();
				deform->SetVertexMap(vertexmap);
				deform->SetRestLocs(restlocs);
				deform->SetTarget(pVerts);
				fbxcache.Convert();
				VX_TRACE(FBXReader::Debug, ("Linking mesh animator (FBX vertex cache) %s -> mesh %s\n", tmpname, pFbxMesh->GetName()));
				++childcount;
				break;
			}

			default:	continue;	// unsupported type
		}
		if (deform->Parent() == NULL)
			root->Append(deform);
	}
	if (childcount == 0)			// no deformers created?
	{
		root->Delete();				// delete empty root
		return NULL;				// return failure
	}
	if (childcount == 1)			// a single deformer only?
	{
		Engine* tmp = root->First();// return the first child
		tmp->Remove(Group::UNLINK_NOFREE);
		root->Delete();				// delete the empty root
		return tmp;
	}
	return root;
}

/*
 * @fn Morph* ConvDeformer::MakeMorph(const TCHAR* vtxdesc, int index)
 * @param vtxdesc	string with descriptor for extra vertex components. If NULL,
 *					only position, normal and texcoord are included.
 * @param index		index appended to object name, if 0 nothing appended.
 *
 * Makes a Vixen Morph engine which will deform a Vixen VertexArray
 * with positions, normals and the designated extra components.
 * If the input index is > 0, it is appended to the name of the
 * deformer created (separated by a dash) e.g. fbxfile.fbxmesh.morph-2
 *
 * @return -> Vixen Morph engine created
 *
 * @see ConvSkin::MakeSkin ConvCache::MakeMeshAnim
 */
Morph*	ConvDeformer::MakeMorph(const TCHAR* vtxdesc, int index)
{
	Morph*	morph = new Morph();
	Core::String	name(m_Name);

	m_VixNode = morph;
	mVertDesc = vtxdesc;
	if (index > 0)
	{
		name += TEXT('-');
		name += Core::String(index);
	}
	morph->SetName(name);
	VX_TRACE(FBXReader::Debug, ("Creating morph %s\n", m_Name));
	return morph;
}

/* !
 * @fn SharedObj*	ConvDeformer::Convert(FbxAnimLayer* pAnimLayer)
 * Generates Vixen VertexArray objects attached to the Vixen Morph
 * for each FBX target shape associated with the FBX blend deformer.
 *
 * @return -> Vixen Morph with blend shapes and weights, NULL on error
 *
 * @see ConvDeformer::MakeMorph ConvDeformer::MakeDeformers
 */
SharedObj*	ConvDeformer::Convert()
{
	Morph*	morph = (Morph*) m_VixNode;
	double*	weights = mBlend->GetTargetShapeFullWeights();

	if (m_VixNode == NULL)
		return NULL;
	for (int i = 0; i < mBlend->GetTargetShapeCount(); ++i)
	{
		FbxShape*		shape = mBlend->GetTargetShape(i);
		FbxGeometry*	geo = shape->GetBaseGeometry();
		VertexArray*	verts;
		ConvMesh		fbxmesh(geo->GetNode());

		if (!geo->IsRuntime(FbxMesh::ClassId))
			VX_ERROR(("ERROR: %s Blend Channel target shape not a mesh\n", m_Name), NULL);
		verts = fbxmesh.MakeVerts(mVertDesc);
		morph->SetSource(i, verts);
		morph->SetWeight(i, (float) weights[i]);
	}
	return m_VixNode;
}

/*
 * @fn Skin* ConvSkin::MakeSkin(int index)
 * @param index	index appended to object name, if 0 nothing appended.
 *
 * Makes a Vixen Skin engine which will deform a Vixen VertexArray
 * with positions and normals. If the input index is > 0,
 * it is appended to the name of the skil created (separated by a dash) e.g. fbxfile.fbxmesh.skin-2
 *
 * @return -> Vixen Skin engine created
 *
 * @see ConvDeformer::MakeMorph ConvCache::MakeMeshAnim
 */
Skin*	ConvSkin::MakeSkin(int index)
{
	Skin*			skin = new Skin();
	Core::String	name(m_Name);

	if (index > 0)
	{
		name += TEXT('-');
		name += Core::String(index);
	}
	skin->SetName(name);
	VX_TRACE(FBXReader::Debug, ("Creating skin %s\n", m_Name));
	m_VixNode = skin;
	return skin;
}

/* !
 * @fn SharedObj*	ConvSkin::Convert()
 * Generates blend weights and bone indices for the rest locations
 * in the Vixen Skin based on the FBX skin clusters associated
 * with the FBX skin. Each FbxCluster designates the weights
 * which control the influence of a specific bone.
 *
 * @return -> Vixen Skin with bone indices and weights, NULL on error
 *
 * @see ConvSkin::MakeSkin ConvDeformer::MakeDeformers
 */
SharedObj*	ConvSkin::Convert()
{
	Skin*	skin = (Skin*) m_VixNode;

	if (skin)
	{
		Skeleton*	skel = skin->GetSkeleton();
		int			nclusters = mFbxSkin->GetClusterCount();

		for (int i = 0; i < nclusters; ++i)
		{
			FbxCluster*	cluster = mFbxSkin->GetCluster(i);
			ConvertCluster(cluster);
		}
		NormalizeWeights();
		skin->Validate(0, 0);
		skel = skin->GetSkeleton();
		if (skel)
		{
			if (skin->Parent() == NULL)
			{
				skel->Append(skin);
				VX_TRACE(FBXReader::Debug, ("Linking  skin %s -> %s\n", m_Name, skel->GetName()));
			}
//			CalcBindPose(mSkeleton);
		}
	}
	return m_VixNode;
}

void ConvSkin::CalcBindPose(Skeleton* skel)
{
	Pose*	bindpose = skel->MakePose(Pose::SKELETON);
	int		numbones = skel->GetNumBones();

	if (numbones != mBindPose.GetSize())
		VX_ERROR_RETURN(("Skin %s bind pose with %d bones does not match skeleton %s with %d bones\n",
					m_Name, mBindPose.GetSize(), skel->GetName(), numbones));
	for (int i = 0; i < numbones; ++i)
	{
		Matrix& mtx = mBindPose.GetAt(i);
		bindpose->SetWorldMatrix(i, mtx);
	}
	skel->SetBindPose(bindpose);
	skel->GetPose()->Copy(bindpose);
}

/*!
 * @fn int	ConvSkin::ConvertCluster(FbxCluster* cluster)
 * @pararm	FBX cluster for the bone whose weights to convert
 *
 * Copy the bone weights and indices from the FBX skin cluster for
 * a bone to the Vixen VertexArray attached as the rest locations for the Vixen Skin.
 */
int	ConvSkin::ConvertCluster(FbxCluster* cluster)
{
	int			nweights = cluster->GetControlPointIndicesCount();
	int*		fbxindices = cluster->GetControlPointIndices();
	double*		fbxweights = cluster->GetControlPointWeights();
	Skin*		skin = (Skin*) m_VixNode;
	FbxNode*	fbxbone = cluster->GetLink();
	int			boneindex = -1;

	if (nweights == 0)
		return 0;
	/*
	 * Find the Vixen bone index for the FBX node linked to this skin cluster.
	 * This is the bone index saved in the Vixen vertex array for this cluster.
	 */
	if (fbxbone)
	{
		Transformer* bone = (Transformer*) fbxbone->GetUserDataPtr();
		if (bone)
		{
			FbxAMatrix	fbxmtx1;
			FbxAMatrix	fbxmtx2;
			Matrix		vixmtx;
			Quat		r;
			Vec3		p;
			Skeleton*	skinskel = skin->GetSkeleton();
			Skeleton*	boneskel = ConvBone::FindSkeleton(bone);

			if (boneskel)
			{
				if (skinskel == NULL)				// remember Skeleton associated with this skin
					skin->SetSkeleton(boneskel);
				else if (skinskel != boneskel)
					VX_WARNING(("ERROR: Skin %s bound to multiple skeletons - this is not supported\n", m_Name));
			}
			else
				boneskel = skinskel;
			if (boneskel == NULL)
			{ VX_WARNING(("ERROR: Skin %s no skeleton found\n", m_Name)); }
			boneindex = bone->GetBoneIndex();
			cluster->GetTransformMatrix(fbxmtx2);		// global matrix on mesh being skinned
			fbxmtx2.Inverse();
			cluster->GetTransformLinkMatrix(fbxmtx1);	// global matrix for bone in bind pose
			fbxmtx2 *= fbxmtx1;							// bind pose for bone w/r to mesh
			ConvModel::ConvertMatrix(fbxmtx2, vixmtx);
			if (boneindex >= 0)
				mBindPose.SetAt(boneindex, vixmtx);		// remember the bind pose
			vixmtx.GetTranslation(p);
			r = vixmtx;
			r.Normalize();
			VX_TRACE(FBXReader::Debug, ("Bone %s -> %s bindpose wrot = (%0.3f, %0.3f, %0.3f, %0.3f) wpos = (%0.3f, %0.3f, %0.3f)\n",
				bone->GetName(), cluster->GetName(), r.x, r.y, r.z, r.w, p.x, p.y, p.z));
		}
	}
	for (int i = 0; i < nweights; ++i)
	{
		int		vtxindex = fbxindices[i];
		double	w = fbxweights[i];
		float*	weights = skin->GetBoneWeights(vtxindex);
		int32*	mtxindices = skin->GetBoneIndices(vtxindex);
		int		b;
		double	wmin = w;
		int		minidx = -1;

		if (w < FLT_EPSILON)
			continue;
		VX_ASSERT(mtxindices != NULL);
		VX_ASSERT(weights != NULL);
		for (b = 0; b < skin->GetBonesPerVertex(); ++b)
		{
			if (mtxindices[b] < 0)	// -1 signals available slot
			{
				weights[b] = w;
				mtxindices[b] = boneindex;
				break;
			}
			if (wmin > weights[b])	// remember where minimum weight was
			{
				wmin = weights[b];
				minidx = b;
			}
		}
		if (b == skin->GetBonesPerVertex())
		{
			if ((wmin < w) && (minidx >= 0))
			{
				weights[minidx] = w;
				mtxindices[minidx] = boneindex;
			}
			else
				wmin = w;
			VX_TRACE2(Debug, ("WARNING: %s has more than %d influences on vertex %d, discarding weight %f\n", m_Name, skin->GetBonesPerVertex(), vtxindex, wmin));
		}
	}
	VX_TRACE(FBXReader::Debug, ("Skin Cluster %s: %d weights added\n", m_Name, nweights));
	return nweights;
}

/*!
 * @fn ConvSkin::NormalizeWeights()
 * Initializes the bone weights for all the vertices in the skin rest pose.
 * All of the weights are set to 0 and all the bone indices are set to -1
 * (which signifies and empty array).
 *
 * @see ConvSkin::NormalizeWeights
 */
void ConvSkin::InitWeights()
{
	Skin*			skin = (Skin*) m_VixNode;
	VertexArray*	restlocs = skin->GetRestLocs();
	int				nverts = restlocs->GetNumVtx();

	for (int i = 0; i < nverts; ++i)
	{
		float*	weights = skin->GetBoneWeights(i);
		int32*	mtxindices = skin->GetBoneIndices(i);

		VX_ASSERT(mtxindices != NULL);
		VX_ASSERT(weights != NULL);
		for (int b = 0; b < skin->GetBonesPerVertex(); ++b)
		{
			mtxindices[b] = -1;			// -1 signals available slot
			weights[b] = 0.0f;
		}
	}
}

/*
 * @fn ConvSkin::NormalizeWeights()
 * Normalizes the bone weights for the rest locations associated
 * with the Vixen Skin. Upon return, all of the weights for a 
 * vertex will sum to 1.0.
 *
 * @see ConvSkin::InitWeights
 */
void ConvSkin::NormalizeWeights()
{
	Skin*			skin = (Skin*) m_VixNode;
	VertexArray*	restlocs = skin->GetRestLocs();
	int				nverts = restlocs->GetNumVtx();
	int				nbones = skin->GetBonesPerVertex();
	int				b;

	for (int i = 0; i < nverts; ++i)
	{
		float*	weights = skin->GetBoneWeights(i);
		int32*	mtxindices = skin->GetBoneIndices(i);
		double	total = 0.0f;

		VX_ASSERT(mtxindices != NULL);
		VX_ASSERT(weights != NULL);
		for (b = 0; b < nbones; ++b)	// total the weights for this vertex
		{
			if (mtxindices[b] >= 0)
				total += weights[b];
		}
		if (total < VX_EPSILON)
		{
			VX_WARNING(("Skin %s vertex %d has very small weights\n", m_Name, i));
			continue;
		}
		total = 1 / total;
		for (b = 0; b < nbones; ++b)	// normalize so they add to 1
		{
			if (mtxindices[b] >= 0)
				weights[b] *= total;
		}
	}
}


} }	// end FBX