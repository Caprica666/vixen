#include "vix_all.h"
#include <maya/MFnAttribute.h>
#include <maya/MFnDagNode.h>
#include <maya/MAnimUtil.h>
#include <maya/MFnAnimCurve.h>

meConvEng::meConvEng(MObject& mayaObj) : meConvObj(mayaObj) { }
meConvEng::meConvEng(const MDagPath& dagNode) : meConvObj(dagNode) { }


/*!
 * @fn meConvEng* meConvEng::MakeConverter(MObject& mayaObj)
 * @param mayaObj	Maya object to make converter for
 *
 * Makes a converter for the given Maya node. This is a class factory for converters.
 * Each Maya node type uses a different meConvNode subclass for converting it to Vixen.
 *
 * @see meConvNode::Make
 */
meConvEng* meConvEng::MakeConverter(MObject& mayaObj)
{
    MStatus				status = MS::kSuccess;
    MFnDependencyNode	node(mayaObj, &status);
    MString				pathstr = node.name();

	if (!Exporter->DoAnimation && !Exporter->DoSkinning) // not exporting animation?
		return NULL;							// no engines then
	if (mayaObj.hasFn(MFn::kJoint))				// joint of a skeleton?
		return new meConvJoint(mayaObj);
	if (!Exporter->DoSkinning || !Exporter->DoGeometry)
		return NULL;
	if (mayaObj.hasFn(MFn::kBlendShape))		// blend shape deformer?
		return new meConvDeform(mayaObj);
	if (mayaObj.hasFn(MFn::kSkinClusterFilter))	// skin cluster for a skeleton?
		return new meConvSkin(mayaObj);
	if (mayaObj.hasFn(MFn::kNCloth))			// cloth simulation?
		return new meConvCloth(mayaObj);
     return NULL;
}

SharedObj* meConvEng::Make()
{
 	Engine*	eng = new Engine();

	m_VXObj = eng;
	SetName();
	Exporter->SimRoot->Append(eng);
    return eng;
}

/*!
 * @fn int meConvEng::Link(meConvObj* convparent)
 *
 * Links the Vixen engine to the correct parent within the Vixen hierarchy.
 * If the original Maya object is a DAG node, we iterate over
 * all the Maya parents and link this node to the corresponding Vixen parent.
 * Otherwise, the engine remains linked to the simulation root.
 *
 * @return 1 if engine was linked to a parent, else 0
 */
int meConvEng::Link(meConvObj* convparent)
{
    MFnDagNode	dagnode(m_DagPath, &m_Status);
    int			numparents;
    bool		didlink = false;
    Engine*		parent = NULL;
    Engine*		eng = (Engine*) (SharedObj*) m_VXObj;

    if (convparent)
    {
		convparent->Append(this);
		parent = (Engine*) convparent->GetVXObj();
		if (parent)
		{
			VX_ASSERT(parent->IsClass(VX_Engine));
			meLog(1, "%s: Linking to parent %s", GetMayaName(), parent->GetName());
			if (eng)
			{
				VX_ASSERT(eng->IsClass(VX_Engine));
				eng->Remove(Group::UNLINK_NOFREE);
				parent->Append(eng);
			}
		}
		DoConvert = true;
		DoLink = false;
		return 1;
    }
	if (!m_DagPath.hasFn(MFn::kDagNode))		// not a DAG node?
	{
		DoConvert = true;
		DoLink = false;
	}
	numparents = dagnode.parentCount();
	if (numparents == 0)						// has no parents?
		return 0;
    for (int p = 0; p < numparents; ++p)
    {
		MStatus		status1, status2;
        MObject&	parobj(dagnode.parent(p));	// get next Maya parent
		ConvRef		cvref;
        meConvObj*	conv;
		meConvEng*	engconv;
        MFnDagNode	dagParent(parobj, &status1);
		MDagPath	dagPath;

		status2 = dagParent.getPath(dagPath);
		if (!status1 || !status2)
			ME_ERROR(("ERROR: %s parent not a dag node", GetMayaName()), 0);
		cvref = Exporter->MayaNodes[dagPath];
		conv = cvref;
        engconv = dynamic_cast<meConvEng*>( conv );
		if (engconv == NULL)
		{
			cvref = Exporter->MayaObjects[parobj];
			conv = cvref;
			engconv = dynamic_cast<meConvEng*>( conv );
		}
        if (engconv)							// find Vixen parent
		{
			meLog(1, "%s: Linking to parent %s", GetMayaName(), dagPath.partialPathName().asChar());
			if (engconv != Parent())
				engconv->Append(this);
			parent = (Engine*) engconv->GetVXObj();
		}
		if (parent && parent->IsClass(VX_Engine))
		{
			didlink = true;
			if (eng && (parent != eng->Parent()))
			{
				meLog(1, "\t%s -> %s", parent->GetName(), eng->GetName());
				eng->Remove(Group::UNLINK_NOFREE);
				parent->Append(eng);
			}
			DoConvert = true;
			DoLink = false;
		}
    }
    return didlink;
}

SharedObj* meConvEng::LinkTarget(meConvObj* conv)
{
	SharedObj* vxobj = NULL;

	VX_ASSERT(conv);
	vxobj = conv->GetVXObj();
	if (vxobj)
	{
		MFnDependencyNode targetNode(conv->GetMayaObj());
		Engine* eng = (Engine*) GetVXObj();
		SharedObj* prevtarget;

		meLog(1, "%s: Target set to %s", GetMayaName(), targetNode.name().asChar());
		VX_ASSERT(eng && eng->IsClass(VX_Engine));
		prevtarget = eng->GetTarget();
		if (prevtarget && (prevtarget != vxobj))
			meLog(1, "WARNING: Changing target of %s from %s to %s", GetMayaName(), prevtarget->GetName(), vxobj->GetName());
		eng->SetTarget(vxobj);
		return vxobj;
	}
	return NULL;
}



bool meConvEng::GatherCurves(MPlug& plug)
{
	MObjectArray animation;
	if (!MAnimUtil::findAnimation(plug, animation))
		return false;

	MPlug			attrPlug(plug);
	MObject			attrObj = attrPlug.attribute();
	MFnAttribute	fnAttr(attrObj);
	MString			fullAttrName(fnAttr.name());

	attrPlug = attrPlug.parent();
	while (!attrPlug.isNull())
	{
		attrObj = attrPlug.attribute();
		MFnAttribute fnAttr2 (attrObj);
		fullAttrName = fnAttr2.name() + "." + fullAttrName;
		attrPlug = attrPlug.parent();
	}
	attrObj = plug.attribute();
	MFnAttribute fnLeafAttr(attrObj);
	unsigned int numCurves = animation.length();
	for (unsigned int j = 0; j < numCurves; j++)
	{
		MStatus	status;
		MFnAnimCurve animCurve(animation[j], &status);
		if (status != MS::kSuccess)
			continue;
		switch (animCurve.animCurveType())
		{
			case MFnAnimCurve::kAnimCurveTA:
			case MFnAnimCurve::kAnimCurveUA:
			break;

			case MFnAnimCurve::kAnimCurveTL:
			case MFnAnimCurve::kAnimCurveUL:
			break;

			case MFnAnimCurve::kAnimCurveTT:
			case MFnAnimCurve::kAnimCurveUT:
			break;

			default:
			break;
		}
	}
	return true;
}