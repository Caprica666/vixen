#include "vix_all.h"

#include <maya/MFnSet.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnReflectShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>

/*!
 * @fn SharedObj* meConvMtl::Make()
 *
 * Creates a Vixen material and associates it with this converter.
 * The Vixen material is empty upon return from this routine.
 *
 * @return root Vixen node created for the Maya node, NULL if Maya node cannot be converted
 *
 * @see meConvMtl::Convert
 */
SharedObj* meConvMtl::Make()
{
/*
 * If we have a "Code" attribute, this is a special Embree shader and we ignore the Lambert
 * shader attribute and only pass the extra attributes. In this case, the material
 * is not Phong.
 */
	MFnDependencyNode node(m_MayaObj);
	MPlug			codeplug(node.findPlug("Code", &m_Status));

	if (!m_Status)
		return m_VXObj = new PhongMaterial();

	Core::String desc;
	int			size = 0;
	MString		str;
				
	for (int i = 0; i < node.attributeCount(); ++i)
	{
		MObject			attrobj = node.attribute(i, &m_Status);
		if (!node.isNewAttribute(attrobj))
			continue;
		MFnAttribute	attr(attrobj);
		MPlug			plug(node.findPlug(attr.name(), &m_Status));
		Core::String	type;
		MObject			texobj;
		meConnIter		iter(plug, true);

		if (!m_Status)
			continue;
		attr.parent(&m_Status);		// ignore child attributes
		if (m_Status)
			continue;
		while (iter.Next(texobj, MFn::kTexture2d))
		{
			meConvObj* texconv = Exporter->makeOne(texobj);
			if (texconv)
				Append(texconv);
		}
		if (attrobj.hasFn(MFn::kNumericAttribute))
		{
			MFnNumericAttribute nattr(attrobj);
			switch (nattr.unitType())
			{
				case MFnNumericData::kBoolean:
				case MFnNumericData::kByte:
				case MFnNumericData::kChar:
				case MFnNumericData::kShort:
				case MFnNumericData::kInt:
				type = " int 1";
				size++;
				break;

				case MFnNumericData::kFloat: 
				case MFnNumericData::kDouble:
				type = " float 1";
				size++;
				break;

				case MFnNumericData::k2Short:
				case MFnNumericData::k2Int:
				type = " int 2";
				size += 2;
				break;

				case MFnNumericData::k3Int:
				type = " int 3";
				break;

				case MFnNumericData::k2Float: 
				case MFnNumericData::k2Double:
				type = " float 2";
				size += 2;
				break;

				case MFnNumericData::k3Float: 
				case MFnNumericData::k3Double:
				type = " float 3";
				size += 3;
				break;

				case MFnNumericData::k4Double:
				type = " float 4";
				size += 4;
				break;

				default:
				continue;
			}
			if (!desc.IsEmpty())
				desc += ", ";
			desc += attr.name().asChar() + type;
		}
		else
			continue;
	}
	Material*	mtl = new Material(desc, size * 4);
	m_VXObj = mtl;
	if (codeplug.getValue(str))
		mtl->SetName(str.asChar());
	meLog(2, "%s: Creating custom material %s", GetMayaName(), desc);
	return mtl;
}

/*!
 * @fn int meConvMtl::Convert(meConvObj*)
 *
 * Converts the Maya shader into a Vixen material and attaches the material
 * to this converter. This routine does not associate the new
 * Vixen materials with geometry in the hierarchy.
 *
 * @return -1 on error, 0 if Maya shader is skipped, 1 if successful conversion
 *
 * @see meConvMtl::Make
 */
int meConvMtl::Convert(meConvObj* shader)
{
    PhongMaterial*	mtl = (PhongMaterial*) (SharedObj*) m_VXObj;
    meConvState*	state = (meConvState*) shader;
    MColor			a, d, e, s;
    MColor			white(1,1,1);
    float			f, alpha = 1;
    MObject			mobj(m_MayaObj);
    const char*		namebuf = GetMayaName();
 	MObject			attr;
	MFnLambertShader mayamtl(mobj, &m_Status);

    if (!m_Status)
		ME_ERROR(("ERROR: MFnLambertShader constructor failed for %s", namebuf), -1);
    if (state == NULL)
		return 0;
	meLog(1, "%s: Converting material", namebuf);
    if (mtl == NULL)
        return -1;
	if (!m_VXObj->IsKindOf(CLASS_(PhongMaterial)))
		return ConvertCustom(state);
/*
 * Lambert shader gives us ambient, diffuse, emissive colors
 */
	a = mayamtl.ambientColor();				// extract ambient color
	d = mayamtl.transparency();				// extract transparency color
	if (m_Status)
		alpha = 1.0f - (d.r + d.g + d.b) / 3;// average the transparency to get alpha
	if (state->HasDiffuseTexture)
		d = white;
	else
	{
		f = mayamtl.diffuseCoeff();			// extract diffuse coefficient
		d = mayamtl.color();				// extract diffuse color
		d.r *= f; d.g *= f; d.b *= f;
	}
	UpdateState(state, d, alpha);			// update appearance info based on color and alpha
	if (state->HasEmissionTexture)
		e = white;
	else
		e = mayamtl.incandescence();		// extract emissive color
	mtl->SetEmission(Col4(e.r, e.g, e.b));
/*
 * Reflective shader gives us specular color
 */
	MFnReflectShader refmtl(mobj, &m_Status);
	if (!m_Status)
		return 1;
	if (state->HasSpecularTexture)
		s = white;
	else
		s = refmtl.specularColor();
	mtl->SetSpecular(Col4(s.r, s.g, s.b));
/*
 * Phong and Blinn shaders give us shininess
 */
	MFnBlinnShader blinnmtl(mobj, &m_Status);
	if (m_Status)
		f = 100.0f * (1 - blinnmtl.eccentricity());
	else
	{
		MFnPhongShader phongmtl(mobj, &m_Status);
		if (!m_Status)
			return 1;
		f = phongmtl.cosPower();
	}
	mtl->SetShine(f);
	return 1;
}

int meConvMtl::ConvertCustom(meConvState* state)
{
	Material*			mtl = (Material*) (SharedObj*) m_VXObj;
	Appearance*			appear = (Appearance*) state->GetVXObj();
	MFnDependencyNode	node(m_MayaObj);
	Shader*				shader = new Shader(Shader::PIXEL);

	shader->SetName(mtl->GetName());
	shader->SetSource("bucky");
	VX_ASSERT(appear && appear->IsClass(VX_Appearance));
	appear->SetPixelShader(shader);
	for (int i = 0; i < node.attributeCount(); ++i)
	{
		MObject			attrobj = node.attribute(i, &m_Status);
		MFnAttribute	attr(attrobj);
		MPlug			plug(node.findPlug(attr.name(), &m_Status));
		Core::String	type;
		MFn::Type		attrtype = attr.type();
		double			dval;
		float			fval;
		int32			ival;
		bool			b;
		int16			sval;
		char			cval;
		int32			ivec[4];
		float			fvec[4];
		const char*		name = attr.name().asChar();
		MFnNumericAttribute nattr(attrobj);

		if (!m_Status)
			continue;
		if (!node.isNewAttribute(attrobj))
			continue;
		attr.parent(&m_Status);		// ignore child attributes
		if (m_Status)
			continue;
		if (!attrobj.hasFn(MFn::kNumericAttribute))
			continue;
		switch (nattr.unitType())
		{
			case MFnNumericData::kBoolean:	plug.getValue(b); b = mtl->Set(name, b); break;
			case MFnNumericData::kByte:
			case MFnNumericData::kChar:		plug.getValue(cval); b = mtl->Set(name, cval); break;
			case MFnNumericData::kShort:	plug.getValue(sval); b = mtl->Set(name, sval); break;
			case MFnNumericData::kInt:		plug.getValue(ival); b = mtl->Set(name, ival); break;
			case MFnNumericData::kFloat:	plug.getValue(fval); b = mtl->Set(name, fval); break; 
			case MFnNumericData::kDouble:	plug.getValue(dval); b = mtl->Set(name, fval); break;
			case MFnNumericData::k2Int:
			case MFnNumericData::k3Int:
			for (int j = 0; j < plug.numChildren(); ++j)
			{
				MPlug child = plug.child(j);
				child.getValue(ivec[j]);
			}
			b = mtl->Set(name, (const float*) ivec) != NULL;				
			break;

			case MFnNumericData::k2Float: 
			case MFnNumericData::k3Float: 
			for (int j = 0; j < plug.numChildren(); ++j)
			{
				MPlug child = plug.child(j);
				child.getValue(fvec[j]);
			}
			b = mtl->Set(name, fvec) != NULL;				
			break;

			case MFnNumericData::k2Double:
			case MFnNumericData::k3Double:
			case MFnNumericData::k4Double:
			for (int j = 0; j < plug.numChildren(); ++j)
			{
				MPlug child = plug.child(j);
				child.getValue(dval);
				fvec[j] = dval;
			}
			b = mtl->Set(name, fvec) != NULL;				
			break;

			default:
			continue;
		}
		if (!b)
			ME_ERROR(("ERROR: Material attribute %s cannot be set", name), -1);
	}
	return 1;
}

/*!
 * @fn void meConvMtl::Link(meConvObj* convparent)
 * @param convparent	parent state converter to attach to
 *
 * Attaches the parent state converter to this material converter.
 * The material is the first child of the state converter.
 *
 * @code
 * meConvState (convparent)
 *	-> meConvMtl (this)
 *	...
 * @endcode
 *
 * @return \b true if attach successful, \b false on error
 */
int meConvMtl::Link(meConvObj* convparent)
{
    if (convparent == NULL)
		return 0;
    if (convparent->IsParent())
    {
		meConvMtl* child = (meConvMtl*) convparent->GetAt(0);
		if (child && child->GetMayaObj().hasFn(MFn::kLambert))
		{
			meLog(1, "ERROR: %s: Multiple materials for shader, replacing %s",
				   GetMayaName(), child->GetMayaName());
			child->Remove(Group::UNLINK_FREE);
		}
    }
	convparent->Append(this);
    DoConvert = true;				// convert this material
	meLog(1, "%s: Linking material to state %s", GetMayaName(), convparent->GetMayaName());
	if (!m_VXObj->IsKindOf(CLASS_(PhongMaterial)))
	{
		meConvObj::Iter iter(this);
		meConvObj* conv;

		while (conv = (meConvObj*) iter.Next())
		{
			meConvTex* texconv = dynamic_cast< meConvTex* > (conv);
			if (texconv)
				texconv->Link(convparent);
		}
	}
    return 1;
}

/*!
 * @fn void meConvMtl::UpdateState(const MColor& d, float alpha)
 * @param d	Maya diffuse material color
 * @param alpha Maya material alpha value
 *
 * Updates the appearance which uses this material. This routine
 * affects the Appearance::TRANSPARENCY mode in the appearance and
 * the diffuse color in the Material. If the Maya material
 * gets its diffuse color from a texture, the Maya color will be black.
 * We change it to white in Vixen so the textured object is visible.
 *
 * The parent of the meConvMtl material converter is the meConvState
 * converter for the GeoState which owns the material.
 *
 * @see meConvMtl::Convert meConvMtl::FindState meConvMtl::Link
 */
void meConvMtl::UpdateState(meConvState* state, const MColor& d, float alpha)
{
    PhongMaterial* mtl = (PhongMaterial*) (SharedObj*) m_VXObj;
    Appearance* app = state->GetAppearance();

	app->SetMaterial(mtl);
    if (alpha < 1)
    {
		mtl->SetDiffuse(Col4(d.r, d.g, d.b, alpha));
		app->Set(Appearance::TRANSPARENCY, true);
    }
	else
		mtl->SetDiffuse(Col4(d.r, d.g, d.b));
}

meConvState::meConvState(MObject& mayaobj)
 : meConvShader(mayaobj)
{
    HasAmbientTexture = false;
    HasDiffuseTexture = false;
    HasSpecularTexture = false;
    HasEmissionTexture = false;
	HasBumpTexture = false;
}

/*!
 * @fn SharedObj* meConvState::Make()
 *
 * Creates a Vixen appearance and associates it with this converter.
 * The appearance is empty upon return from this routine.
 * If there is only one texture associated with this converter,
 * a Appearance object is created. If more than one texture is
 * used by the Maya shader, a VXMultiTex object is created.
 * 
 * @return Vixen appearance created for the Maya object, NULL if Maya object cannot be converted
 *
 * @see meConvState::Convert
 */
SharedObj* meConvState::Make()
{
    bool		cull = false;	// backface culling on/off
    Appearance* app;			// appearance associated with this converter

    if (!FindSurfaceShader())	// find Maya surface shader
	{
		DoLink = false;
		return NULL;
	}
	DoLink = true;
    app = new Appearance();		// make new appearance
    m_VXObj = app;
	if (app == NULL)
		ME_ERROR(("ERROR: cannot make appearance for %s", GetMayaName()), NULL);
	meLog(1, "%s: Creating Appearance", GetMayaName());
    MakeShaders();				// make converters for nodes in shading group
    app->Set(Appearance::LIGHTING, true);
	app->Set(Appearance::CULLING, cull);
    SetName();
    return app;
}

/*!
 * @fn int meConvState::Convert(meConvObj*)
 *
 * Converts the Maya object into a Vixen appearance and attaches the appearance
 * to this converter. It is the responsibility of the caller to associate the new
 * Vixen state with geometry in the hierarchy.
 *
 * @return -1 on error, 0 if Maya object is skipped, 1 if successful conversion
 *
 * @see meConvState::Make
 */
int meConvState::Convert(meConvObj*)
{
    MFnDependencyNode mayaobj(m_MayaObj);

	meLog(1, "%s: Converting Appearance", GetMayaName());
    SetName();
	ConvertShaders();
    return 1;
}

/*!
 * @fn int meConvState::FindTexIndex(meConvTex* txconv)
 * Find the zero based texture index for the input texture converter.
 * The texture converters for a shader are children of the state converter
 *
 * @see meConvState::AddTexture meConvState::FindTexture
 */
int meConvState::FindTexIndex(meConvTex* txconv)
{
 	return Samplers.Find(txconv);
 }

/*!
 * @fn int meConvState::AddTexture(meConvTex* conv)
 *
 * Adds a new texture converter as a child to this converter.
 * For each texture associated with the shader, a texture converter
 * is made and attached to the state converter that owns the mesh
 * to which the textures belong.
 *
 * @see meConvState::FindTexIndex meConvTex::Link
 */
int meConvState::AddTexture(meConvTex* conv)
{
    int			texindex = FindTexIndex(conv);			// find texture index
	Appearance*	app = (Appearance*) (SharedObj*) m_VXObj;
	Sampler*	sampler = (Sampler*) (SharedObj*) conv->GetVXObj();

    if (texindex >= 0)							// already added?
		return texindex;
    texindex = Samplers.GetSize();			// how many textures so far
    if (texindex >= ME_MAX_TEXTURES)			// too many textures for Vixen
    {
		meLog(1, "ERROR: %s: Cannot add texture in slot %d, only %d textures allowed",
			  GetMayaName(), texindex, ME_MAX_TEXTURES);
		return -1;
    }
	if (app && app->IsClass(VX_Appearance) &&
		sampler && sampler->IsClass(VX_Sampler))
	{
		meLog(1, "%s: Attaching %s as texture %d", GetMayaName(), conv->GetMayaName(), texindex);
		Samplers.SetAt(texindex, conv);		// attach to the state converter
		app->SetSampler(texindex, sampler);		// attach in Vixen too
		return texindex;
	}
	ME_ERROR(("ERROR: %s missing appearance or sampler", GetMayaName()), -1);
}

void meConvState::RemoveTexture(meConvTex* texconv)
{
	int  texindex = FindTexIndex(texconv);		// find texture index
	Appearance* app = (Appearance*) (SharedObj*) m_VXObj;
	VX_ASSERT(app != NULL);
	VX_ASSERT(texindex >= 0);
	app->SetSampler(texindex, NULL);
	Samplers.SetAt(texindex, NULL);
	texconv->m_VXObj = NULL;
}


/*!
 * @fn meConvTex* meConvState::FindTexture(const char* texname, int& index)
 * @param texname name of Maya texture object to find
 *
 * Find the texture converter that is a child of this state converter and
 * is associated with the Maya texture object with the given name.
 *
 * @return texture converter for the named Maya texture
 *
 * @see meConvState::AddTexture meConvState::FindTexIndex
 */
meConvTex* meConvState::FindTexture(const char* texname, int& index)
{
    MStatus			status;
	ObjArray::Iter	iter(Samplers);
    meConvTex*		child;
    while (child = (meConvTex*) iter.Next())	// for each texture in this state
    {
		meConvTex* txconv = (meConvTex*) child;	// get converter for texture
		MObject    texobj = txconv->GetMayaObj();// get Maya object
		MPlug      plug;
		MPlugArray plugs;
		MFnDependencyNode texnode(texobj, &status);

		if (!status)
			continue;				// ignore if not a texture
		if (!texobj.hasFn(MFn::kTexture2d) && !texobj.hasFn(MFn::kTexture3d))
			continue;
		++index;
		if (STRCASECMP(txconv->GetMayaName(), texname) == 0)
			return txconv;			// found the texture converter
    }
    return NULL;
}

/*!
 * @fn bool meConvState::FindSurfaceShader()
 * @param shadergroup	Maya shader group to find surface shader for
 *
 * Finds the Maya surface shader object in the given shading group.
 * The surface shader is used to find the material, texture and other
 * rendering state properties for shading. The surface shader is store
 * in the m_Shader field of this converter.
 *
 * @return true if surface shader found, else false4
 *
 * @see meConvState::MakeMaterial meConvState::MakeTexture
 */
bool meConvState::FindSurfaceShader()
{
	const char*		namebuf = GetMayaName();
	meConnIter		iter(m_MayaObj, "surfaceShader", true);
    MObject			shader;

	while (iter.Next(shader))
		return true;
	ME_ERROR(("ERROR: cannot get shader for %s", namebuf), false);
	return false;
}

/*!
 * @fn bool meConvState::MakeShaders()
 * Scan backwards thru the shader dependency graph for all the nodes
 * used by this GeoState/shading group and make a converter for each one.
 * Textures which are not attached to the material for this specific state
 * are ignored. Currently, only file textures are supported, procedural
 * textures are skipped.
 *
 * In Maya, multiple textures may be used on the same material.
 * This routine checks the "outColor" attribute of each Maya texture and finds
 * the Maya material it is connected to. If the texture affects this
 * state, the meConvState for the material converter (meConvMtl) for
 * the Maya material will be a child of this meConvState.
 *
 * Internally, the meConvTex texture converter for each texture which affects
 * this state is saved in the  meConvState Samplers array. These are kept in
 * in the order in which they are used in the appearance.
  *
 * This routine performs two passes over the Maya shader graph. The first pass
 * makes a Maya to Vixen converter for each supported shader node. The second
 * pass links these nodes together as well as their Vixen data structures.
 * The third pass which converts each Maya shader node to Vixen format is
 * performed in the last state of export (vixExport::convertAll)
 *
 * @code
 * meConvState (conv)
 *	  Samplers
 *	  [0] meConvTex (texture 0)
 *	  [1] meConvTex (texture 1)
 *	--> meConvMtl (this)
 *	...
 * @endcode
 *
 * @return  number of textures found, 0 if none found
 *
 * @see meConvState::MakeConverter vixExporter::convertAll meConvShade::Link
 */
bool meConvState::MakeShaders()
{
    MItDependencyGraph iter(m_MayaObj, MFn::kDependencyNode, MItDependencyGraph::kUpstream,
			    MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kNodeLevel,
			    &m_Status);

/*
 * Make converters for all shader nodes associated with this state.
 * Graph is scanned breadth first so that nodes at a higher level
 * in the tree have converters created before their children.
 */
    for ( ; !iter.isDone(); iter.next() )
    {
		MObject obj(iter.thisNode());

		if (obj != m_MayaObj)		// prevent recursion
			Exporter->makeOne(obj);
    }
/*
 * Link together hierarchy for shader nodes and attach to this state.
 * Graph is scanned breadth first so that nodes at a higher level
 * in the tree are visited before their children.
 */
     for (iter.reset() ; !iter.isDone(); iter.next() )
	 {
		ConvRef		cvref = Exporter->MayaObjects[iter.thisNode()];
		meConvObj*	conv = cvref;
		meConvShader*	sconv = dynamic_cast<meConvShader*>( conv );

		if (conv == this)
			continue;
		if (sconv)
			if (sconv->DoLink)			// link to appearance converter?
				sconv->Link(this);		// try to link it
			else
				sconv->Link(NULL);		// may have Maya linkage duties
	 }
     return true;
}

void meConvState::ConvertShaders()
{
	MItDependencyGraph iter(m_MayaObj, MFn::kDependencyNode, MItDependencyGraph::kUpstream,
			    MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kNodeLevel,
			    &m_Status);

	for ( ; !iter.isDone(); iter.next() )
	 {
		ConvRef		cvref = Exporter->MayaObjects[iter.thisNode()];
		meConvObj*	conv = cvref;
		meConvShader*	sconv = dynamic_cast<meConvShader*>( conv );

		if (conv == this)
			continue;
		if (sconv && sconv->DoConvert)
			sconv->Convert(this);		// try to convert it
	 }
}

/*!
 * @fn meConvShader* meConvShader::MakeConverter(MObject obj)
 *
 * Makes a converter to convert a Maya shader node into a Vixen object.
 * The state converter (meConvState) is the root of the shader group.
 * The material and texture converters become children of the meConvState.
 * The converter for the UV mapper for a texture becomes a child of the
 * texture converter.
 *
 * @code
 * meConvState		(holds Appearance)
 *		Samplers
 *		[0]	meConvTex	(holds Sampler)
 *			-> meConvUV
 *		[1]	meConvTex	(holds Sampler)
 *			-> meConvUV
 *	--> meConvMtl	(holds Material)
 *
 *	Maya			Vixen	Converter
 *	------------------------------------------------
 *	Lambert			Material	meConvMtl
 *	Texture2d		Texture		meConvTex
 *	Texture3d		Texture		meConvTex
 *	LayeredTexture	none		meConvTexLayer
 *	LayeredShader	none		meConvTexLayer
 *	Place2DTexture	none		meConvUV
 *	Place3DTexture	none		meConvUV3
 *	Projection		none		meConvProj
 *	EnvSphere		none		meConvProj
 * @endcode
 */
meConvShader* meConvShader::MakeConverter(MObject obj)
{
    MFnDependencyNode node(obj);
    MString nodename = node.name();
    const char* name = nodename.asChar();
    meConvShader* conv;

	if (obj.hasFn(MFn::kShadingEngine))
		return new meConvState(obj);
    if (obj.hasFn(MFn::kPlace2dTexture))
		return new meConvUV(obj);
    if (obj.hasFn(MFn::kPlace3dTexture))
		return new meConvUV3(obj);
    if (obj.hasFn(MFn::kProjection) ||
		obj.hasFn(MFn::kEnvSphere))
        return conv = new meConvProj(obj);
    if (obj.hasFn(MFn::kLayeredTexture))
        conv = new meConvTexLayer(obj);
    else if (obj.hasFn(MFn::kLambert))
		conv = new meConvMtl(obj);
    else if (obj.hasFn(MFn::kTexture2d))
		conv = new meConvTex(obj);
    else if (obj.hasFn(MFn::kTexture3d))
		conv = new meConvTex(obj);
    else
    {
		meLog(4, "WARNING: %s ignoring shader node of type %s",name, obj.apiTypeStr());
		return NULL;
    }
    conv->DoLink = true;
    return conv;
}

/*!
 * @fn void meConvShader::Link(meConvObj* convparent)
 * @param convparent	parent state converter to attach to
 *
 * Attaches the parent state converter to this converter.
 * This provides a way to track which pfGeoState references which materials and textures.
 *
 * @code
 * meConvState (convparent)
 *		Samplers
 *		[0] meConvTex (texture 0)
 *		[1] meConvTex (texture 1)
 *	-> meConvMtl (this)
 *	...
 * @endcode
 *
 * @return \b true if attach successful, \b false on error
 */
int meConvShader::Link(meConvObj* convparent)
{
    if (convparent == NULL)
		return 0;
    if (IsChild())
    {
		meConvState* parent = dynamic_cast<meConvState*>( Parent() );
		if (parent == convparent)	// already attached?
			return 0;
		if (parent != NULL)			// in another tree?
			ME_ERROR(("ERROR: material %s with multiple parents", GetMayaName()), -1);
    }
    convparent->Append(this);		// attach to the state converter
    DoConvert = true;				// convert this shader
	meLog(1, "%s: Linking to state %s", GetMayaName(), convparent->GetMayaName());
    return 1;
}


void meConvShader::PrintShader()
{
    MItDependencyGraph iter(m_MayaObj, MFn::kInvalid, MItDependencyGraph::kUpstream,
			    MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kNodeLevel,
			    &m_Status);
    iter.disablePruningOnFilter();
    for ( ; !iter.isDone(); iter.next() )
		PrintPlugs(iter.thisNode());
}


