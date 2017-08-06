#include "vix_all.h"

#include <maya/MFnCamera.h>
#include <maya/MFnLight.h>
#include <maya/MFnNonAmbientLight.h>
#include <maya/MFnSpotLight.h>
#include <maya/MAnimUtil.h>

meConvNode::meConvNode(const MDagPath& mpath) : meConvObj(mpath)
{
	ParentIsJoint = false;
	Animation = NULL;
}

/*!
 * @fn meConvObj* meConvNode::MakeConverter(const MDagPath& mayaPath)
 * @param mayaPath	Maya DAG path of node to make converter for
 *
 * Makes a converter for the given Maya node. This is a class factory for converters.
 * Each Maya node type uses a different meConvNode subclass for converting it to Vixen.
 *
 * @see meConvNode::Make
 */
meConvObj* meConvNode::MakeConverter(const MDagPath& mayaPath)
{
    MStatus		status = MS::kSuccess;
    MFnDagNode	dagNode(mayaPath, &status);
    MString		pathstr = mayaPath.partialPathName();
	MDagPath	dagPath(mayaPath);

	if (dagNode.isIntermediateObject())		// skip intermediate objects
    {
		meLog(3, "Skipping intermediate object %s of type %s",
			  pathstr.asChar(), dagNode.typeName().asChar());
		return NULL;
    }
	meLog(4, "%s: Examining node of type %s", pathstr.asChar(), dagNode.typeName().asChar());
	if (mayaPath.hasFn(MFn::kTransform))		// node is a transform?
	{
		if (mayaPath.hasFn(MFn::kManipulator2D) || mayaPath.hasFn(MFn::kManipulator3D))
			return NULL;
		if (mayaPath.hasFn(MFn::kIkEffector) || mayaPath.hasFn(MFn::kHikEffector))
			return NULL;
		if (mayaPath.hasFn(MFn::kNurbsCurve))
			return NULL;
		if (mayaPath.hasFn(MFn::kJoint))		// joint of a skeleton?
			if (Exporter->DoAnimation || Exporter->DoSkinning)
				return new meConvJoint(mayaPath);
			else return NULL;
		if (!Exporter->DoGeometry)				// no geometry output?
			return NULL;						// we are done
		if (mayaPath.hasFn(MFn::kCamera))		// node is a camera?
			return new meConvCam(dagPath);
		if (mayaPath.hasFn(MFn::kLight))		// node is a light?
			return new meConvLight(dagPath);
		dagPath.extendToShape();
		if (dagPath.hasFn(MFn::kNCloth))		// cloth simulation mesh?
			if (Exporter->DoAnimation || Exporter->DoSkinning)
				return new meConvCloth(dagPath);
		if (dagPath.hasFn(MFn::kMesh) || dagPath.hasFn(MFn::kNurbsSurface))	// node is a mesh?
			return meConvPoly::MakeConverter(mayaPath);
		if (dagPath.apiType() == MFn::kTransform)
			return new meConvNode(mayaPath);
		return NULL;
	}
	if (dagNode.parentCount() == 0)				// node is the root?
		return new meConvNode(mayaPath);
    return NULL;
}


/*!
 * @fn SharedObj* meConvNode::Make()
 *
 * Creates one or more Vixen nodes from the Maya node associated with this converter.
 * The Vixen node is empty upon return from this routine.
 *
 * @return root Vixen node created for the Maya node, NULL if Maya node cannot be converted
 *
 * @see meConvNode::Convert
 */
SharedObj* meConvNode::Make()
{
  	Model*		mod = new Model();
	Matrix		localmtx;

	Exporter->Root->Append(mod);
	m_VXObj = mod;
	SetName();
	if (GetMatrix(localmtx))
		mod->SetTransform(&localmtx);
    return mod;
}


/*!
 * @fn int meConvNode::Convert(meConvObj*)
 *
 * Converts the Maya node into Vixen data structures and attaches the Vixen root
 * node to this converter. It is the responsibility of the caller to attach the new
 * Vixen node(s) into the overall hierarchy.
 *
 * This base implementation links the node to its parent(s).
 *
 * @return -1 on error, 0 if Maya node is skipped, 1 if successful conversion
 *
 * @see meConvNode::Make meConvNode::MakeConverter
 */
int meConvNode::Convert(meConvObj*)
{
    if (m_VXObj == NULL)
        return -1;
    return 1;
}


/*!
 * @fn int meConvNode::Link(meConvObj* convparent)
 *
 * Links the Vixen node to the correct parent within the Vixen hierarchy.
 * We iterate over all the Maya parents and link this node to the corresponding
 * Vixen parent. For Maya nodes without a Vixen counterpart, we link the
 * Vixen node to the global Vixen root.
 */
int meConvNode::Link(meConvObj* convparent)
{
    MFnDagNode	dagnode(m_DagPath);
    int			numparents = dagnode.parentCount();
    bool		didlink = false;
    Model*		psparent = NULL;
    Model*		psnode = (Model*) (SharedObj*) m_VXObj;

    if (convparent)
    {
		convparent->Append(this);
		psparent = (Model*) convparent->GetVXObj();
		if (psnode && psparent && psparent->IsClass(VX_Model))
		{
			VX_ASSERT(psnode->IsClass(VX_Model));
			psnode->Remove(Group::UNLINK_NOFREE);
			psparent->Append(psnode);
			LinkAnimation();
			return 1;
		}
    }
	if (numparents == 0)
	{
		meLog(1, "%s: Linking to ROOT", dagnode.name().asChar());
		return 0;
	}
	// TODO: handle instancing
    for (int p = 0; p < numparents; ++p)
    {
		MObject		parentObj(dagnode.parent(p, &m_Status));
 
		if (m_Status != MS::kSuccess)
			ME_ERROR(("ERROR: %s cannot get parent %d", dagnode.name().asChar(), p), false);

		MFnDagNode	dagParent(parentObj, &m_Status);
		MDagPath	dagPath;

		if ((m_Status != MS::kSuccess) || (dagParent.getPath(dagPath) != MS::kSuccess))
			ME_ERROR(("ERROR: %s cannot get DAG path for parent %d", dagnode.name().asChar(), p), false);
		ConvRef		cvref = Exporter->MayaNodes[dagPath];
		meConvObj*	conv = cvref;

		meLog(1, "%s: Linked to parent %s", dagnode.name().asChar(), dagPath.partialPathName().asChar());
		if (conv)								// find Vixen parent
		{
			if (conv != Parent())
				conv->Append(this);
			psparent = (Model*) conv->GetVXObj();
		}
		if (psparent && psparent->IsClass(VX_Model))
		{
			didlink = true;
			if (psnode && psparent != psnode->Parent())
			{
				meLog(1, "\t%s -> %s", psparent->GetName(), psnode->GetName());
				VX_ASSERT(psnode->IsClass(VX_Model));
				psnode->Remove(Group::UNLINK_NOFREE);
				psparent->Append(psnode);
			}
		}
		/*
		 * In the case of a model parented to a joint in Maya,
		 * we make a Transformer engine and a Model in Vixen (two nodes),
		 * one in the simulation tree and one in the scene graph.
		 * The Model becomes a target of the Transformer.
		 */
		else if (parentObj.hasFn(MFn::kJoint))	// parented to a joint?
		{
			meConvJoint* jconv = dynamic_cast<meConvJoint*>( conv );
			ParentIsJoint = true;
			if (jconv)
				jconv->LinkTarget(this);
		}
    }
	LinkAnimation();
    return didlink;
}


/*!
 * @fn bool meConvNode::GetMatrix(Matrix& psmatrix) const
 * @param psmatrix	Vixen matrix to get the Maya matrix data
 *
 * Copies the local matrix from the Maya node into a Vixen matrix
 *
 * @returns true if Maya node has a non-identity matrix, else false
 *	if successful, the Vixen matrix passed will get the
 *	local matrix from the Maya node associated with this converter
 */
bool meConvNode::GetMatrix(Matrix& vxmatrix) const
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
    MMatrix		mmtx = tmatrix.asMatrix();
    MVector		trans = tmatrix.translation(MSpace::kTransform);
	MPoint		rpivot = tmatrix.rotatePivot(MSpace::kTransform);
    double		d[3];
    float		fltdata[4][4];
	double		x, y, z, w;

    mmtx.get(fltdata);
	if (tmatrix.isEquivalent(MTransformationMatrix::identity))
        return false;
    vxmatrix.SetMatrix(&fltdata[0][0]);
	vxmatrix.Transpose();
    tmatrix.getRotationQuaternion(x, y, z, w);
	tmatrix.getScale(d, MSpace::kTransform);
    meLog(2, "%s: trans (%.3f %.3f %.3f) rot (%.3f %.3f %.3f %.3f) scale (%.3f %.3f %.3f)", transform.name().asChar(),
			trans[0], trans[1], trans[2], x, y, z, w, d[0], d[1], d[2]);
    return true;
}

bool meConvNode::LinkAnimation()
{
	Transformer*		xform = NULL;
	Interpolator*		translate = NULL;
	Interpolator*		rotate = NULL;
	MPlugArray			animplugs;
	Core::String		name(m_VXObj->GetName());

	if (!MAnimUtil::isAnimated(m_DagPath, true))
		return false;
	MAnimUtil::findAnimatedPlugs(GetMayaObj(), animplugs, false, &m_Status);
	xform = new Transformer();
	name += ".xform";
	xform->SetName(name);
	xform->SetTarget(GetVXObj());
	xform->SetOptions(Transformer::WORLD);
	Exporter->SimRoot->Append(xform);
	Animation = xform;
	for (int i = 0; i < animplugs.length(); ++i)
	{
		MPlug		plug = animplugs[i];
		MPlug		parentplug;
		Core::String s = plug.name().asChar();

		if ((translate == NULL) && (s.FindNoCase(TEXT("translate")) >= 0))
		{
			translate = new Interpolator();
			translate->SetName(name + TEXT(".pos"));
			translate->SetInterpType(Interpolator::STEP);
			translate->SetValSize(sizeof(Vec3) / sizeof(float));
			translate->SetDestType(Interpolator::POSITION);
			xform->Append(translate);
			meLog(1, "%s: Sampling translation as %s", GetMayaName(), translate->GetName());
			if (rotate)
				return true;
		}
		else if ((rotate == NULL) && (s.FindNoCase(TEXT("rotate")) >= 0))
		{
			rotate = new Interpolator();
			rotate->SetName(name + TEXT(".rot"));
			rotate->SetInterpType(Interpolator::QSTEP);
			rotate->SetDestType(Interpolator::ROTATION);
			rotate->SetValSize(sizeof(Interpolator::WQuat) / sizeof(float));
			xform->Append(rotate);
			meLog(1, "%s: Sampling rotation as %s", GetMayaName(), rotate->GetName());
			if (translate)
				return true;
		}
	}
	return false;
}

/*!
 * @fn SharedObj* meConvCam::Make()
 *
 * Creates a Vixen camera node for this converter.
 * The default Maya camera looks down the negative Z axis.
 * The default Vixen camera looks down the positive Z axis.
 * We rotate the Maya camera to correct for this.
 *
 * @return camera node created
 *
 * @see meConvCam::Convert
 */
SharedObj* meConvCam::Make()
{
  	Camera*		cam = new Camera();
	Matrix		localmtx;

	m_VXObj = cam;
	SetName();
	Exporter->Root->Append(cam);
	if (GetMatrix(localmtx))
		cam->SetTransform(&localmtx);
	return m_VXObj;
}

int meConvCam::Convert(meConvObj*)
{
    MFnDagNode  dagnode(m_DagPath);
	MFnCamera	mayacam(m_DagPath);
    MString		pathstr = m_DagPath.fullPathName();
    const char* pathname = pathstr.asChar();
	double		left, right, top, bottom;
	Camera*		vixcam = (Camera*) (SharedObj*) m_VXObj;
	
	meLog(1, "%s: Converting camera", pathstr.asChar());
    if (vixcam == NULL)
        return -1;
	VX_ASSERT(vixcam->IsClass(VX_Camera));
	if (mayacam.isOrtho())
		vixcam->SetType(Camera::ORTHOGRAPHIC);
	if (mayacam.getViewingFrustum(1.0f, left, right, bottom, top, false, false, false))
	{
		float	hither = (float) mayacam.nearClippingPlane();
		float	yon = (float) mayacam.farClippingPlane();
		float	fov = (float) mayacam.horizontalFieldOfView();
		double	hfov, vfov;
		Box3	vvol((float) left, (float) right, hither, (float) top, (float) bottom, yon);

		mayacam.getPortFieldOfView((int) vvol.Width(), (int) vvol.Height(), hfov, vfov);
		if (vvol.min.x > vvol.max.x)
		{
			vvol.min.x = (float) right;
			vvol.max.x = (float) left;
		}
		if (vvol.min.y > vvol.max.y)
		{
			vvol.min.y = (float) bottom;
			vvol.max.y = (float) top;
		}
		meLog(2, "%s: View Volume (%f, %f, %f, %f, %f, %f)", pathstr.asChar(), vvol.min.x, vvol.max.x, hither, vvol.min.y, vvol.max.y, yon);
		vixcam->SetViewVol(vvol);
		vixcam->SetFOV(fov);
	}
    return 1;
}

/*!
 * @fn bool meConvNode::GetTotalTransform(Matrix& vxmatrix) const
 * @param vxmatrix	Vixen matrix to get the Maya matrix data
 *
 * Get the Maya inclusive matrix (total matrix from root node to this node)
 *
 * @returns true if Maya node has a non-identity matrix, else false
 *	if successful, the Vixen matrix passed will get the
 *	total matrix from the Maya node associated with this converter
 */
bool meConvNode::GetTotalTransform(Matrix& vxmatrix) const
{
    MStatus	status;
    MMatrix	mtx = m_DagPath.inclusiveMatrix(&status);
    float	fltdata[4][4];

    if (!status)
        return false;
    if (mtx.isEquivalent(MMatrix::identity))
		return false;
    if (!mtx.get(fltdata))
		return false;
    vxmatrix.SetMatrix(&fltdata[0][0]);
	vxmatrix.Transpose();
    return true;
}

/*!
 * @fn bool meConvNode::GetTotalInverse(Matrix& vxmatrix) const
 * @param vxmatrix	Vixen matrix to get the Maya matrix data
 *
 * Get the Maya inclusive matrix inverse (inverse of total matrix from
 * root node to this node). If this matrix is put at the root of the
 * scene graph, it applies the proper camera transform to view the
 * scene from this camera.
 *
 * @returns true if Maya node has a non-identity matrix, else false
 *	if successful, the Vixen matrix passed will get the
 *	inverse matrix from the Maya node associated with this converter
 */
bool meConvNode::GetTotalInverse(Matrix& vxmatrix) const
{
    MStatus	status;
    MMatrix	mtx = m_DagPath.inclusiveMatrixInverse(&status);
    float	fltdata[4][4];

    if (!status)
        return false;
    if (!mtx.get(fltdata))
		return false;
    vxmatrix.SetMatrix(&fltdata[0][0]);
	vxmatrix.Transpose();
    return true;
}

/*!
 * @fn SharedObj* meConvLight::Make()
 *
 * Creates a Vixen light source for this converter.
 *
 * @return Vixen light created
 *
 * @see meConvLight::Convert
 */
SharedObj* meConvLight::Make()
{
	MStatus		is_spot;
	MStatus		is_point;
	MStatus		is_light;
    MFnLight	mayalight(m_DagPath, &is_light);
	MFnSpotLight spotlight(m_DagPath, &is_spot);
	Light*		light;
	Matrix		localmtx;

	if (!is_light)
    {
		vixen_debug << "ERROR: meConvLight MFnLight failed for " << GetMayaName() << std::endl;
		return NULL;
	}
	if (is_spot == MS::kSuccess)
		light = new SpotLight();
	else if (m_DagPath.hasFn(MFn::kPointLight))
		light = new Light();
	else
		light = new DirectLight();
	m_VXObj = light;
	SetName();
	Exporter->Root->Append(light);
	if (GetMatrix(localmtx))
		light->SetTransform(&localmtx);
	return m_VXObj;
}


int meConvLight::Convert(meConvObj*)
{
	MStatus		has_decay;
    MFnLight	mayalight(m_DagPath, &m_Status);
	MFnNonAmbientLight nalight(m_DagPath, &has_decay);
    Light*		pslight = (Light*) (SharedObj*) m_VXObj;
    MColor		c;
    float		f;
 
    if (pslight == NULL)
        return -1;
	meLog(1, "%s: Converting light source", GetMayaName());
/*
 * Process ambient and diffuse light source color
 */
    c = mayalight.color();
    f = mayalight.intensity();
    c *= f;							// modulate color with intensity
    if (mayalight.lightDiffuse())	// has diffuse component?
		pslight->SetColor(Col4(c.r, c.g, c.b));
/*
 * Check for non-ambient light types and set decay (0 = none, 1 = linear, 2 = quadratic)
 */
	if (has_decay)
		switch (nalight.decayRate())
		{
			case 1: pslight->SetDecay(SpotLight::INVERSE); break;
			case 2: pslight->SetDecay(SpotLight::INVERSE_SQUARE); break;
		}
/*
 * Check for spot lights and set cone angle.
 */
	if (pslight->IsClass(VX_SpotLight))
	{
		MFnSpotLight spotlight(m_DagPath);
		SpotLight* spot = (SpotLight*) pslight;

		spot->SetOuterAngle((float) (spotlight.coneAngle() + 2.0f * spotlight.penumbraAngle()));
		spot->SetInnerAngle((float) spotlight.coneAngle());
	}
    return 1;
}
