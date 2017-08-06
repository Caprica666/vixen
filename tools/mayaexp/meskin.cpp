#include "vix_all.h"
#include <maya/MFnAttribute.h>
#include <maya/MFnDagNode.h>
#include <maya/MAnimUtil.h>
#include <maya/MFnWeightGeometryFilter.h>
#include <maya/MFnGeometryFilter.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MItGeometry.h>

/*!
 * @fn SharedObj* meConvSkin::Make()
 *
 * Creates a Vixen Skin engine from the Maya node.
 * The Vixen engine is empty upon return from this routine.
 *
 * @return Skin engine created for the Maya node, NULL if Maya node cannot be converted
 *
 * @see meConvEng::Convert
 */
SharedObj* meConvSkin::Make()
{
	Skin*			vxskin = new Skin();
	Core::String	name(GetMayaName());

	m_VXObj = vxskin;
	m_OutMesh = NULL;
	name += ".skin";
	SetName(vxskin, name);
	Exporter->SimRoot->Append(vxskin);
	if (m_MayaObj.hasFn(MFn::kGeometryFilt))
	{
		MFnGeometryFilter geo(m_MayaObj);
		MObjectArray inputs;
		geo.getInputGeometry(inputs);
		for (uint32 i = 0; i < inputs.length(); ++i)
		{
			MFnDependencyNode node(inputs[i]);
			meLog(3, "\tinput %d <- %s", i, node.name().asChar());
		}
	}
   return vxskin;
}


int meConvSkin::Link(meConvObj* convparent)
{
	MFnSkinCluster	skincluster(m_MayaObj);
	MObjectArray	output;
	Skin*			skin = (Skin*) GetVXObj();

	VX_ASSERT(skin && skin->IsClass(VX_Skin));
	if (skincluster.getOutputGeometry(output))
	{
		for (uint32 i = 0; i < output.length(); ++i)
		{
			MObject		obj(output[i]);
			MFnDagNode	dagnode(obj, &m_Status);
			ConvRef		cvref = Exporter->MayaObjects[obj];
			meConvObj*	conv = cvref;
			meConvPoly*	pconv;
			MDagPath	dagpath;

			if (!m_Status)
				continue;
			dagnode.getPath(dagpath);
			cvref = Exporter->MayaNodes[dagpath];
			conv = cvref;
			if (conv == NULL)
				continue;
			pconv = dynamic_cast<meConvPoly*>( conv );
			if (pconv)
			{
				meConvDeform*	dconv = pconv->FindDeformer();
				LinkTarget(pconv);
				m_OutMesh = pconv;
				if (dconv)
				{
					if (dconv->DoLink)
						dconv->Link(this);
					dconv->LinkTarget(this);
				}
			}
		}
	}
	return meConvEng::Link(convparent);
}

SharedObj* meConvSkin::LinkTarget(meConvObj* conv)
{
	meConvPoly* mconv = dynamic_cast<meConvPoly*>( conv );
	Skin*		skin = (Skin*) GetVXObj();

	if ((mconv == NULL) || !meConvEng::LinkTarget(mconv))
		return NULL;
	VX_ASSERT(skin && skin->IsClass(VX_Skin));
	meLog(2, "%s: found geometry %s, making skin source vertices", GetMayaName(), mconv->GetMayaName());
	VertexArray*	skinverts = mconv->GetMayaVerts();
	Shape*			shape = (Shape*) mconv->GetVXObj();

	VX_ASSERT(shape && shape->IsClass(VX_Shape));
	shape->SetHints(Model::MORPH);
	skin->SetRestPose(skinverts);
	if (mconv->HasDuplicates())
	{
		skin->SetVertexMap(mconv->GetVertexMap());
		skin->SetTarget(mconv->GetVertices());
	}
	return skinverts;
}

void meConvSkin::LinkSkeleton(meConvJoint* rootjoint)
{
	meConnIter	siter(rootjoint->GetMayaObj(), "lockInfluenceWeights", false);
	MPlug		plug;

	if (!Exporter->DoSkinning)
		return;
  	while (siter.Next(plug))
	{
		MObject		obj(plug.node());

		if (obj.hasFn(MFn::kSkinClusterFilter))
		{
			ConvRef		cvref = Exporter->MayaObjects[obj];
			meConvObj*	conv = cvref;
			meConvSkin*	skconv = dynamic_cast<meConvSkin*>(conv);
			if (skconv)
				skconv->Link(rootjoint);
		}
	}
}

int meConvSkin::Convert(meConvObj*)
{
	Skin*			skin = (Skin*) GetVXObj();
	MFnSkinCluster	skincluster(m_MayaObj);
	MDagPathArray	influences;
	VertexArray*	sourceverts;
	int				numbones = 0;
	int				numinfluences = 0;
	int				stride;
	meConvJoint*	rootjoint = NULL;
	int*			jointmap = NULL;


    if (skin == NULL)
        return -1;
	meLog(1, "%s: Converting skin", GetMayaName());
	numinfluences = skincluster.influenceObjects(influences);
	if (numinfluences <= 0)
	{	ME_ERROR(("ERROR: %s no influences for skin", GetMayaName()), -1); }
	/*
	 * Attach the skin to its skeleton
	 */
	rootjoint = FindRootJoint(influences);
	if (rootjoint)
	{
		Skeleton* skel = rootjoint->MakeSkeleton();
		if (skel && skel->IsClass(VX_Skeleton))
		{
			skin->Remove(Group::UNLINK_NOFREE);
			skin->SetSkeleton(skel);
			skel->Append(skin);
			meLog(1, "\t%s -> %s", skel->GetName(), skin->GetName());
			numbones = skel->GetNumBones();
		}
	}
	if (numbones <= 0)
	{	ME_ERROR(("ERROR: %s has no bones", GetMayaName()), -1); }
	jointmap = (int*) alloca(sizeof(int) * numbones);
	sourceverts = skin->GetRestPose();
	if (sourceverts == NULL)
		ME_ERROR(("ERROR: %s no source vertices for skin", GetMayaName()), -1);
	skin->Init(NULL);
	stride = sourceverts->GetVtxSize();
	VX_ASSERT(numbones >= numinfluences);
	VX_ASSERT(skin->GetBonesPerVertex() > 0);
	rootjoint->MakeJointMap(influences, jointmap);

	/*
	 * Compute weights and matrix indices for each vertes
	 */
	for (int j = 0; j < numinfluences; ++j)
	{
		MDagPath		path = influences[j];
		int				jointindex = jointmap[j];
		MDoubleArray	mayaweights;
		MSelectionList	result;

		skincluster.getPointsAffectedByInfluence(path, result, mayaweights);

		MItSelectionList	siter(result);
		int					nweights = mayaweights.length();
		MObject				obj;

		if (jointindex == -1)
		{
			ME_WARNING(("ERROR: %s no joint found for influence %d - ignoring", GetMayaName(), j));
			continue;
		}
		for ( ; !siter.isDone(); siter.next())
		{
			siter.getDagPath(path, obj);
			if (!obj.hasFn(MFn::kSingleIndexedComponent))
				continue;
			MFnSingleIndexedComponent vertcomp(obj);
			MIntArray		mayaindices;

			vertcomp.getElements(mayaindices);
			for (int i = 0; i < nweights; ++i)
			{
				int		vtxindex = mayaindices[i];
				float	w = (float) mayaweights[i];
				float*	weights = skin->GetBoneWeights(vtxindex);
				int32*	mtxindices = skin->GetBoneIndices(vtxindex);
				int		b;
				float	wmin = w;
				int		minidx = -1;

				for (b = 0; b < skin->GetBonesPerVertex(); ++b)
				{
					if (mtxindices[b] <= 0)	// -1 signals available slot
					{
						weights[b] = w;
						mtxindices[b] = jointindex;
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
						mtxindices[minidx] = jointindex;
					}
					else
						wmin = w;
					ME_WARNING(("WARNING: %s has more than %d influences on vertex %d, discarding weight %f", GetMayaName(), skin->GetBonesPerVertex(), vtxindex, wmin));
				}
			}
		}
		meLog(3, "\t%s: %d weights added", path.partialPathName().asChar(), nweights);
	}
	return 1;
}

meConvJoint* meConvSkin::FindRootJoint(const MDagPathArray& influences)
{
	meConvJoint*	rootjoint = dynamic_cast< meConvJoint* >( Parent() );
	int				n = influences.length();

	if (rootjoint != NULL)
		return rootjoint;
	for (int i = 0; i < n; ++i)
	{
		ConvRef		cref = Exporter->MayaNodes[influences[i]];
		meConvJoint*	tmp;

		if (cref == NULL)
			continue;
		rootjoint = dynamic_cast< meConvJoint* >( *cref );
		if (rootjoint == NULL)
			continue;
		while (tmp = dynamic_cast< meConvJoint*> ( rootjoint->Parent() ))
			rootjoint = tmp;
	}
	return rootjoint;
}

/*!
 * @fn SharedObj* meConvDeform::Make()
 *
 * Creates a Vixen Morph engine from the Maya blend shape node.
 * The Vixen engine is empty upon return from this routine.
 *
 * @return Deformer engine created for the Maya node, NULL if Maya node cannot be converted
 *
 * @see meConvEng::Convert
 */
SharedObj* meConvDeform::Make()
{
	if (!m_MayaObj.hasFn(MFn::kBlendShape))
		return NULL;

	MFnBlendShapeDeformer blend(m_MayaObj, &m_Status);
	MObjectArray	outputs;
	MObjectArray	inputs;
	MIntArray		weightindices;
	Morph*			vxmorph;
	Core::String	name(GetMayaName());

	if (!m_Status)
		ME_ERROR(("%s - not a Maya blend deformer", name), NULL);
	if (blend.numWeights() == 0)
		ME_ERROR(("%s - no input blend shapes for blender", name), NULL);
	blend.getBaseObjects(outputs);
	m_VXObj = vxmorph = new Morph();
	name += ".morph";
	SetName(vxmorph, name);
	Exporter->SimRoot->Append(vxmorph);
	if (outputs.length() == 0)
		ME_WARNING(("%s - cannot find output mesh to deform", m_MayaName));
	if (outputs.length() > 1)
		ME_WARNING(("%s has more than one output mesh", m_MayaName, outputs.length()));
	return vxmorph;
}

int meConvDeform::Link(meConvObj* convparent)
{
	MFnBlendShapeDeformer blend(m_MayaObj);
	MObjectArray	outputs;
	MDagPath		dagPath;
	int				n = 0;
	Morph*			deform = (Morph*) (SharedObj*) m_VXObj;
	Skin*			skin;

	if (convparent)
	{
		deform->SetTarget(NULL);
		meConvEng::Link(convparent);
	}
	skin = (Skin*) deform->Parent();
	if (skin && skin->IsClass(VX_Skin))
		return 0;
	/*
	 * Find the first base object (output geometry) and make it the target
	 * of the blend deformer. Only one base object is allowed.
	 */
	blend.getBaseObjects(outputs);
	n = outputs.length();
	for (int o = 0; o < n; o++)
	{
		MObject		obj = outputs[o];
		ConvRef		cvref;
		meConvObj*	conv;
		meConvMesh*	meshconv = NULL;
		Shape*		shape;

		if (!obj.hasFn(MFn::kDagNode))	// do we have an output mesh?
			continue;
		MFnDagNode(obj).getPath(dagPath);
		cvref = Exporter->MayaNodes[dagPath];
		conv = cvref;
		meLog(2, "%s: found output geometry %s", GetMayaName(), dagPath.partialPathName().asChar());
		if (conv == NULL)
			continue;
		shape = (Shape*) (SharedObj*) conv->GetVXObj();
		if (!shape || !shape->IsClass(VX_Shape))
			continue;
		meshconv = dynamic_cast<meConvMesh*>( conv );
		if (meshconv)
		{
			m_OutMesh = obj;
			LinkTarget(meshconv);
			break;
		}
	}
	return 1;
 }


SharedObj* meConvDeform::LinkTarget(meConvObj* conv)
{
	meConvMesh* mconv = dynamic_cast<meConvMesh*>( conv );
	meConvSkin* sconv = dynamic_cast<meConvSkin*>( conv );
	Morph*		deform = (Morph*) GetVXObj();

	VX_ASSERT(deform && deform->IsClass(VX_Deformer));
	if (mconv)
	{
		Shape*	shape = (Shape*) mconv->GetVXObj();
		Mesh*	mesh;

		VX_ASSERT(shape && shape->IsClass(VX_Shape));
		shape->SetHints(Model::MORPH);
		mesh = (Mesh*) shape->GetGeometry();
		VX_ASSERT(mesh && mesh->IsClass(VX_Mesh));
		m_TargetVerts = mesh->GetVertices();
		meLog(2, "%s: found geometry %s", GetMayaName(), mconv->GetMayaName());
	}
	else if (sconv)
	{
		Skin*	skin = (Skin*) sconv->GetVXObj();

		VX_ASSERT(skin && skin->IsClass(VX_Skin));
		meLog(2, "%s: found Skin %s", GetMayaName(), sconv->GetMayaName());
		m_TargetVerts = skin->GetRestPose();
	}
	else
		return NULL;
	if (m_TargetVerts == NULL)
		ME_ERROR(("ERROR: %s missing target vertices", GetMayaName()), NULL);
	m_TargetVerts->SetFlags(VertexPool::MORPH);
	return meConvEng::LinkTarget(conv);
}

int meConvDeform::Convert(meConvObj*)
{
	MFnBlendShapeDeformer blend(m_MayaObj);
	MObjectArray	inputs;
	MIntArray		weightindices;
	MDagPath		dagPath;
	MFnDagNode		dagNode;
	MObject			obj;
	ConvRef			cvref;
	meConvObj*		conv;
	meConvMesh*		meshconv = NULL;
	Shape*			shape;
	int				nweights;
	int				n = 0;
	const char*		mayaname = GetMayaName();
	Morph*			deform = (Morph*) (SharedObj*) m_VXObj;
	VertexArray*	verts;

	/*
	 * Find the Vixen vertices associated with each input to the blend shape deformer
	 */
	blend.weightIndexList(weightindices);
	nweights = blend.numWeights();
	n = weightindices.length();
	verts = MakeBlendSource(NULL);
	if (verts)
		deform->SetRestPose(verts);
	else
	{	ME_ERROR(("ERROR: Deformer %s has no target vertices", GetMayaName()), NULL); }
	deform->Init(NULL);
	for (int j = 0; j < n; ++j)
	{
		int		windex = weightindices[j];
		float	weight = blend.weight(windex);

		if (!blend.getTargets(m_OutMesh, windex, inputs))
		{	ME_ERROR(("ERROR: Cannot get Deformer %s blend target #%d", GetMayaName()), windex); }
		obj = inputs[0];				// just take the first influence
		if (!obj.hasFn(MFn::kDagNode))	// do we have an input mesh?
			continue;
		MFnDagNode(obj).getPath(dagPath);
		meLog(2, "%s: found source blend shape %s", mayaname, dagPath.partialPathName().asChar());
		cvref = Exporter->MayaNodes[dagPath];
		conv = cvref;
		if (conv == NULL)
			continue;
		shape = (Shape*) (SharedObj*) conv->GetVXObj();
		if (!shape || !shape->IsClass(VX_Shape))
			continue;
		verts = MakeBlendSource(shape);
		if (verts)
		{
			deform->SetSource(windex, verts);
			deform->SetWeight(windex, weight);
			meLog(2, "\tblend source %d %s", windex, shape->GetName());
		}
		else
		{	ME_ERROR(("ERROR: Deformer %s cannot make blend shape for ", shape->GetName()), 0); }
	}
	return 1;
 }

VertexArray* meConvDeform::MakeBlendSource(Shape* source)
{
	VertexArray*	srcverts = NULL;
	VertexArray*	diffverts;
	Core::String	name("base");

	if (m_TargetVerts == NULL)
		return NULL;
	if (source)
	{
		Shape*	shape = (Shape*) source;
		Mesh*	mesh = (Mesh*) shape->GetGeometry();

		if (!mesh || !mesh->IsClass(VX_Mesh))
			return NULL;
		srcverts = mesh->GetVertices();
		if (srcverts == NULL)
			return NULL;
		VX_ASSERT(m_TargetVerts->GetNumVtx() == srcverts->GetNumVtx());
		name = source->GetName();
		shape->SetActive(false);
	}
	diffverts = new VertexArray("float3 position, float3 normal", m_TargetVerts->GetNumVtx());
	diffverts->SetName(name);
	diffverts->SetNumVtx(m_TargetVerts->GetNumVtx());

	VertexArray::ConstIter	s2iter(m_TargetVerts);
	VertexArray::Iter		diter(diffverts);
	Vec3*					s2;
	Vec3*					dst;

	if (srcverts)
	{
		VertexArray::ConstIter	s1iter(srcverts);
		Vec3*					s1;

		VX_ASSERT(s1iter.HasNormals() && s2iter.HasNormals());
		while ((s1 = (Vec3*) s1iter.Next()) &&
			   (s2 = (Vec3*) s2iter.Next()) &&
			   (dst = (Vec3*) diter.Next()))
		{
			dst[0] = s2[0] - s1[0];		// compute position difference
			dst[1] = s2[1] - s1[1];		// compute position difference
		}
	}
	else
	{
		VX_ASSERT(s2iter.HasNormals());
		while ((s2 = (Vec3*) s2iter.Next()) &&
			   (dst = (Vec3*) diter.Next()))
		{
			dst[0] = s2[0];
			dst[1] = s2[1];
		}
	}
	return diffverts;
}

