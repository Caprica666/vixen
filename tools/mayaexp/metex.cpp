#include "vix_all.h"
#include <maya/MFnSet.h>
#include <maya/MFnTransform.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnCompoundAttribute.h>

/*!
 * @fn Texture* meConvTex::FindTexture(const char* filename)
 * @param filename	file name of texture to find
 *
 * Looks for an existing texture with the given file name.
 * Although Maya may have several texture objects which reference
 * the same bitmap image file, we attempt to keep a single Vixen
 * Texture in this case. When a new Maya texture is created, we
 * scan the existing texture converter table to find if the file
 * has already been used or not.
 *
 * @returns -> Vixen texture with given filename, NULL if not found
 *
 * @see meConvTex::Make
 */
Texture* meConvTex::FindTexture(const char* filename)
{
    meObjDict::Iter iter(&(Exporter->MayaObjects));
    ConvRef* ref;

    assert(GetVXObj() != NULL);
    while (ref = iter.Next())           // for all objects
    {
        meConvObj* conv = *ref;			// get converter this entry references
		Sampler* sampler;
		Texture* image;

		if (conv == NULL)
			continue;
        sampler = (Sampler*) conv->GetVXObj();
        if (sampler == NULL)            // get Vixen sampler created for it
            continue;
        if (!sampler->IsClass(VX_Sampler))
            continue;                   // Vixen object is not a texture sampler?
		image = sampler->GetTexture();	// get texture referenced by sampler
		if (image == NULL)				// no image?
			continue;					// ignore it then

		MObject& texobj = conv->GetMayaObj();
		MStatus status;
		MFnDependencyNode texnode(texobj, &status);
		MString texfile;
		MPlug plug;

		if (!texobj.hasFn(MFn::kFileTexture))
			continue;						// Maya object is not a texture?
		plug = texnode.findPlug("fileTextureName");
		if (!plug.getValue(texfile))		// does texture have file name?
			continue;
		if (STRCASECMP(filename, texfile.asChar()) == 0)
			return image;					// found one that matches
    }
    return NULL;
}

/*!
 * @fn SharedObj* meConvTex::Make()
 *
 * Creates a Vixen texture sampler and associates it with this converter.
 * Only one Vixen texture object is kept for a given bitmap image file.
 * If multiple Maya textures reference the same file, there will be multiple
 * meConvTex objects that point to the same Vixen texture.
 *
 * @return root Vixen node created for the Maya node, NULL if Maya node cannot be converted
 *
 * @see meConvTex::Convert
 */
SharedObj* meConvTex::Make()
{
    MFnDependencyNode mayatex(m_MayaObj, &m_Status);
    MPlug			plug = mayatex.findPlug("fileTextureName");
    Sampler*		tex = new Sampler();
	Texture*		image = NULL;
    MString			texpath;
	MFileObject		filepath;
	const char*		filename;	// filename without directory
	Core::String	texfile;
	char			outbuf[VX_MaxPath];

	m_VXObj = tex;
    if (plug.getValue(texpath))				// get texture file name
	{
		filepath.setRawFullName(texpath);
		filename = filepath.rawName().asChar();
	}
	else
	{
		if (Exporter->DoTextures)			// generate output bitmaps?
		{
			filename = outbuf;
			if (!MakeBitmap(mayatex.name().asChar(), "png", outbuf))
				return NULL;
		}
		else
			ME_ERROR(("%s: cannot get texture file name", GetMayaName()), NULL);
	}
	image = FindTexture(filename);
	if (image)								// texture already there?
	{
		tex->SetTexture(image);
		meLog(1, "%s: Reusing texture file %s", GetMayaName(), filename);
	}
	else
	{
		tex->SetTexture(new Texture(filename));
		meLog(2, "%s: Creating texture file %s", GetMayaName(), filename);
	}
    return tex;
}

/*!
 * @fn int meConvTex::Convert(meConvObj*)
 *
 * Converts the Maya shader into a Vixen texture. This routine also
 * updates GeoState information corresponding to how this texture is used.
 *
 * @return -1 on error, 0 if Maya shader is skipped, 1 if successful conversion
 *
 * @see meConvTex::Make
 */
int meConvTex::Convert(meConvObj* shader)
{
	meConvState*	state = dynamic_cast< meConvState* >( shader );
	Sampler*		sampler = (Sampler*) (SharedObj*) m_VXObj;
    MFnDependencyNode mayatex(m_MayaObj, &m_Status);

    if (!m_Status)
		ME_ERROR(("ERROR: MFnDependencyNode constructor failed for %s", GetMayaName()), -1);
    if (!sampler)
	{
		ME_ERROR(("ERROR: %s missing Sampler", GetMayaName()), -1);
		return 0;
	}
    MString		fname;
    MObject		texobj;
    bool		hasalpha = false;
    MPlug		plug = mayatex.findPlug("fileTextureName");

	meLog(2, "%s: Converting texture file %s", GetMayaName(), fname);
    if (plug.getValue(fname))	               	// get texture file name
    {
		plug = mayatex.findPlug("fileHasAlpha", &m_Status);
		if (m_Status)							// check for alpha channel in file
			plug.getValue(hasalpha);
    }
	if (state)
	{
		if (UpdateState(state, hasalpha))
			return 1;
		state->RemoveTexture(this);
		return 0;
	}
	ME_ERROR(("ERROR: texture %s missing appearance", GetMayaName()), 0);
}

/*!
 * @fn int meConvTex::Link(meConvObj* conv)
 * @param conv	state converter to attach to
 *
 * Attaches the input state converter to this texture converter.
 * This provides a way to track the textures associated with an appearance.
 * The material should always be the first child of the state converter.
 * The remaining children are textures.
 *
 * If this texture is owned by a layered texture, it is not attached here.
 * The layered texture converter attaches all of the input textures in
 * the appropriate order when it is made.
 *
 * @code
 * meConvState (conv)
 *	Samplers
 *	[0] meConvTex (texture 0)
 *	[1] meConvTex (texture 1)
 *	--> meConvMtl (this)
 *	...
 * @endcode
 *
 * @return 1 if state converter attached, else 0
 *
 * @see meConvTex::UpdateState meConvMtl::Link
 */
int meConvTex::Link(meConvObj* conv)
{
	meConvState* state = (meConvState*) conv;
    int usage = FindMaterial(state);

	if (state->AddTexture(this) >= 0)
	{
	    DoConvert = true;				// convert this shader
		meLog(1, "%s: Adding sampler to state %s", GetMayaName(), conv->GetMayaName());
	}
    return 1;
}

/*!
 * @fn bool meConvTex::MakeBitmap(char* filebase, char* ext)
 * @param filebase name of original texture file without directory or extension
 * @param ext	  extension of bitmap
 *				  designates what type of bitmap is created
 *
 * Converts the input texture file to another bitmap file format.
 * This function does file format conversions as well as snapshotting
 * procedural textures into bitmap files.
 *
 * The exporter applies any texture placement to the Maya UV
 * coordinates before they are stored in the Vixen mesh.
 * Since the input texture may have an arbitrary UV mapping
 * associated with it, we duplicate the Maya texture node to ensure
 * our snapshot will be undistorted.
 *
 * @return true if output file was created, false on error
 */
bool meConvTex::MakeBitmap(const char* filebase, const char* out_ext, char* outbuf)
{
	Core::String	outfile(Exporter->FilePath);
    char			command[1024];
	int				n;

    if (!filebase || !*filebase || !out_ext || !*out_ext)
		ME_ERROR(("ERROR: %s image format conversion argument error %s", GetMayaName()), false);
	outfile += filebase;
	if ((n = outfile.ReverseFind('.')) >= 0)
		outfile = outfile.Left(n + 1);
	else
		outfile += ".";
	outfile += out_ext;
    sprintf(command, "duplicate -n %s_%s %s; convertSolidTx -samplePlane true -force true -fileFormat \"%s\" -fileImageName \"%s\" -n \"%s\" -rx %d -ry %d %s_%s; delete %s_%s;",
			GetMayaName(), out_ext, GetMayaName(), out_ext, (const char*) outfile, filebase, Exporter->TexWidth, Exporter->TexHeight, GetMayaName(), out_ext, GetMayaName(), out_ext);
    m_Status = MGlobal::executeCommand(command, false, false);
    if (!m_Status)
		ME_ERROR(("ERROR: %s image format conversion to %s failed", GetMayaName(), out_ext), false);
	meLog(1, "%s: Making texture file %s", GetMayaName(), outfile);
	if (outbuf)
		strcpy(outbuf, outfile);
    return true;
}

/*!
 * @fn void meConvTex::UpdateState(bool hasalpha)
 * @param hasalpha true if Maya texture has alpha channel
 *
 * Updates the appearance which uses this texture. This routine
 * affects the Appearance::TRANSPARENCY mode in the appearance and
 * updates the material color.
 *
 * Pointers are kept from the meConvState to the meConvTex for each texture that
 * affects its appearance. These are maintained in the order they are used in the appearance.
 *
 * @code
 * meConvState
 *	--> meConvMtl (material)
 *	Samplers
 *	[0] meConvTex (texture 0)
 *		UVMapper -> meConvUV
 *	[1] meConvTex (texture 1)
 *	    UVMapper -> meConvUV
 *	...
 * @endcode
 *
 * @see meConvMtl::Convert meConvMtl::FindState meConvMtl::Link
 */
bool meConvTex::UpdateState(meConvState* state, bool hasalpha)
{
    int				texindex;
    Appearance*		app;
    PhongMaterial*	mtl;
	Sampler*		sampler;
    const char*		appname;
    bool			hastexgen = false;

    texindex = state->FindTexIndex(this);
    app = state->GetAppearance();
    assert(app && app->IsClass(VX_Appearance));
	if ((texindex == 0) && hasalpha)
		app->Set(Appearance::TRANSPARENCY, true);
    appname = state->GetMayaName();
/*
 * Find the material associated with the mesh and set its component color to white.
 * Maya sometimes saves colors in the material from procedural textures which should
 * not be applied in Vixen.
 */
    mtl = (PhongMaterial*) app->GetMaterial();
	VX_ASSERT(mtl->IsKindOf(CLASS_(PhongMaterial)));
	Col4 white(1,1,1);
    if (mtl)
		switch (Usage)
		{
			case ME_MAP_AMBIENT: mtl->SetAmbient(white); break;
			case ME_MAP_MODULATE:
			case ME_MAP_DIFFUSE: mtl->SetDiffuse(white); break;
			case ME_MAP_SPECULAR: mtl->SetSpecular(white); break;
 			case ME_MAP_EMISSION: mtl->SetEmission(white); break;
	   }
	sampler = (Sampler*) (SharedObj*) m_VXObj;
	if ((sampler == NULL) || !sampler->IsClass(VX_Sampler))
		return false;
/*
 * Check for UV mappers that generate texcoords
 */
    meConvUV3* uvconv = dynamic_cast<meConvUV3*>( (meConvUV*) UVMapper );

    if (uvconv)
    {
		MObject obj = uvconv->GetMayaObj();

		if (obj.hasFn(MFn::kPlace3dTexture))
		{
			uvconv->UpdateState(state, texindex);
			hastexgen = true;
		}
	}
	if (MakeTexEnv(app, texindex))
	{
		if ((Usage == ME_MAP_REFLECTION) && !hastexgen) // reflection map?
			sampler->Set(Sampler::TEXCOORD, Sampler::TEXGEN_SPHERE);
		return true;
	}
	else
		return false;
}

/*!
 * @fn bool meConvTex::MakeTexEnv(Appearance* app, int texindex)
 * @param app		Vixen appearance to update
 * @param texindex	0 based texture index in appearance
 *
 * Updates the texture environment and texture generation information in the appearance
 * based on the blend mode and usage of the Maya texture. Maya has 13 different blend
 * modes, only 5 can be directly mapped to Vixen. Unsupported modes default to
 * modulate (multiply).
 */
bool meConvTex::MakeTexEnv(Appearance* app, int texindex)
{
	Sampler*sampler = (Sampler*) (SharedObj*) m_VXObj;
    int		env = Sampler::DIFFUSE;

   switch (BlendMode)
    {
		case ME_MUL_ALPHA:
		app->Set(Appearance::TRANSPARENCY, true);
		// fall thru to ME_MUL

		case ME_MUL:
		env = Sampler::DIFFUSE;
		break;

		case ME_DECAL:		// over color = fc * fa + (bc * (1 - fa))
		app->Set(Appearance::TRANSPARENCY, true);
		env =  Sampler::EMISSION;
		break;

		case ME_REPLACE:	// color = fc
		env =  Sampler::EMISSION;
		break;

		case ME_ADD:		// color = (max((fc * fa), bc)) * fa + bc * (1 - fa)
		env =  Sampler::SPECULAR;
		break;

	// TODO: how to implement these?
		case ME_MUL_INVALPHA:	// color = bc * (1 - fa)
								// alpha = ba * (1 - fa)
								// dst = GL_ONE_MINUS_SRC_ALPHA, src = GL_ZERO
		case ME_SUB:		// color = bc - (fc * fa)
							// alpha = ba
		case ME_MUL_ADD:	// color = bc * (1 + (fc * fa))
							// alpha = ba
		case ME_MUL_SUB:	// color = bc * (1 - (fc * fa))
							// alpha = ba
		default:
		meLog(1, "ERROR: Unsupported texture blend mode %d on texture %s", BlendMode, GetMayaName());
		return false;
   }
   switch (Usage)
   {
		case ME_MAP_DIFFUSE:
		case ME_MAP_MODULATE:
		env = Sampler::DIFFUSE;
		break;

		case ME_MAP_OPACITY:
	   	app->Set(Appearance::TRANSPARENCY, true);
		break;

		case ME_MAP_BUMP:
		env = Sampler::BUMP;
		break;

		case ME_MAP_SPECULAR:
		env = Sampler::SPECULAR;
		break;
   }
	meLog(2, "%s textureop = %d", GetMayaName(), env);
    if (sampler)
	{
		sampler->Set(Sampler::TEXTUREOP, env);
		sampler->Set(Sampler::MINFILTER, Sampler::LINEAR);
		sampler->Set(Sampler::MAGFILTER, Sampler::LINEAR);
		sampler->Set(Sampler::MIPMAP, 1);
	}
	return true;
}


/*!
 * @fn int meConvTex::CheckUsage(const char* attrname, meConvState* state, int& blendmode, Sampler* sampler)
 * @param attrname	name of Maya material attribute 
 * @param state		converter texture is attached to
 * @param blendmode	on output gets the texture blend mode
 * @param sampler	Vixen sampler texture belongs to
 *
 * Determines how the texture is used within Maya and sets the blend mode
 * and usage parameters in the texture converter.
 * Blend modes are ME_ADD, ME_MUL, ME_REPLACE, ME_DECAL
 * Usages are ME_MAP_DIFFUSE, ME_MAP_AMBIENT, ME_MAP_REFLECTION, ME_MAP_EMISSION
 *
 * @return usage (which material component texture is attached to)
 */
int meConvTex::CheckUsage(const char* attrname, meConvState* state, int& blendmode, Sampler* sampler)
{
    const char*		p = strrchr(attrname, '.');
	Core::String	name;

    if (p == NULL)
		ME_ERROR(("ERROR: malformed attribute name %s", attrname), -1);
    ++p;									// skip the dot
	if (strcmp(p, "transparency") == 0)		// separate transparency map not supported
		return -1;
	name = p;
	name += "Map";
	sampler->SetName(name);
    blendmode = ME_MUL;						// default to modulate
    if (STRCASECMP(p, "color") == 0)		// map used as diffuse color?
    {
		state->HasDiffuseTexture = true;
		sampler->SetName("DiffuseMap");
		return ME_MAP_DIFFUSE;
    }
    if (STRCMP(p, "ambientColor") == 0)		// map used as ambient color?
    {
		state->HasAmbientTexture = true;
		sampler->SetName("AmbientMap");
		return ME_MAP_AMBIENT;
    }
    if (STRCMP(p, "reflectedColor") == 0)	// map used as reflection?
		return ME_MAP_REFLECTION;
    if (STRCMP(p, "incandescence") == 0)	// map used as emission?
    {
		blendmode = ME_ADD;
		state->HasEmissionTexture = true;
		return ME_MAP_EMISSION;
    }
    if (STRCMP(p, "specularColor") == 0)	// map used as specular highlight?
    {
		blendmode = ME_ADD;
		state->HasSpecularTexture = true;
		sampler->SetName("SpecularMap");
		return ME_MAP_SPECULAR;
    }
    if (STRCMP(p, "bumpMapping") == 0)		// bump map?
    {
		state->HasBumpTexture = true;
		sampler->SetName("BumpMap");
		return ME_MAP_BUMP;
    }
    meLog(1,"ERROR: %s  texture usage as %s unsupported", state->GetMayaName(), p);
    return -1;
}

/*!
 * @fn int meConvTex::FindMaterial(meConvState* state)
 *
 * Finds the converter for the Maya material this texture supplies color for.
 * This routine is mostly error checking to make sure linkage is not corrupted.
 *
 * @returns usage code for which material component texture is attached to
 *	    -1 if material not found
 *	    0 if material component usage unsupported
 */
int meConvTex::FindMaterial(meConvState* state)
{
    int			usage = 0;
    MPlug		plug;
    meConnIter	iter(m_MayaObj);
	MObject		mtlobj;
	Sampler*	sampler = (Sampler*) (SharedObj*) m_VXObj;

    while (iter.NextSource(plug, MFn::kLambert))
    {
		MObject		mtlobj(plug.node());
		ConvRef		cvref = Exporter->MayaObjects[mtlobj];
		meConvObj*	conv = cvref;
		meConvMtl*	mtlconv = dynamic_cast<meConvMtl*>( conv );
		MString		pname = plug.name();
		const char*	name = pname.asChar();

		if (!mtlconv)
			continue;						// not a material for this state
		if (!mtlconv->IsChild() ||
			(mtlconv->Parent() != state))	// belongs to this state?
			continue;						// this texture linked to another state too
		meLog(2, "%s: %s linked to %s", GetMayaName(), name, mtlconv->GetMayaName());
		Usage = usage = CheckUsage(name, state, BlendMode, sampler);
	}
	return usage;
}

/*!
 * @fn SharedObj* meConvTexLayer::Make()
 *
 * Saves all of the texture objects used as input and their blend modes.
 * The texture converters for these textures will later become children of
 * this converter. When the textures are converted, they will be detached from
 * this converter and attached to the parent meConvState state converter and
 * to the appearance
 *
 * @see meConvTexLayer::Link meConvState::AddTexture meConvTex::Link
 */
SharedObj* meConvTexLayer::Make()
{
    MStatus		status;
    MFnDependencyNode	layernode(m_MayaObj, &status);
    MPlug		inplug = layernode.findPlug("inputs");
    MString		layername = layernode.name();
    int			nlayers = inplug.numElements();

    nlayers = inplug.numElements();
    if (!status || (nlayers == 0))
		ME_ERROR(("ERROR: Cannot find inputs for layered shader %s", layername), NULL);
    for (int i = 0; i < nlayers; ++i)
		DoLayer(inplug[i], i);
    return NULL;
}

/*!
 * @fn bool meConvTexLayer::Link(meConvObj* convparent)
 *
 * Attaches all the texture inputs used by the layered shader/texture to
 * the state converter (which is the parent of this converter).
 * They are also added to the Vixen Appearance.
 *
 * The textures must be blended in the same order as they are in Maya.
 * This is the exact REVERSE order that they are collected.
 * It may also not be the order in which they are used on mesh texture coordinates.
 * This is currently a problem because Vixen does not support arbitrary
 * association of textures and UVs.
 *
 * @see meConvShaderLayer::Make meConvState::AddTexture
 */
int meConvTexLayer::Link(meConvObj* convparent)
{
    meConvState* state = (meConvState*) convparent;
    int usage;
    meConvTex* txconv;

    assert(!IsChild());						// this node should not be attached to anything
    usage = FindMaterial(state);			// check how it is used
    for (int i = m_TexInputs.length() - 1; i >= 0; --i)
    {
		ConvRef		cvref = Exporter->MayaObjects[m_TexInputs[i]];
		meConvObj*	conv = cvref;
		txconv = dynamic_cast<meConvTex*> (conv);
		if (!txconv)
			continue;
		if (usage > 0)
		{
			assert(!txconv->IsChild());
			txconv->BlendMode = 1 + m_BlendModes[i];
			txconv->Usage = usage;
			txconv->Link(state);		// link in correct order
		}
		else
		{
			ME_ERROR(("ERROR: Unsupported texture usage for layered texture %s", GetMayaName()), -1);
			txconv->DoConvert = false;	// suppress conversion
			txconv->DoLink = false;		// suppress attachment
		}
    }
    return true;
}

int meConvTexLayer::Convert(meConvObj*)
{
	meLog(1, "%s: Converting layered shader/texture with %d layers", GetMayaName(), m_TexInputs.length());
    return 1;
}

int meConvTexLayer::FindMaterial(meConvState* state)
{
    int			usage = 0;
    MPlug		plug;
    meConnIter	iter(m_MayaObj);
	MObject		mtlobj;
	Sampler*	sampler = (Sampler*) (SharedObj*) m_VXObj;
	int			blendmode = 0;

    while (iter.Next(plug, MFn::kLambert))
    {
		MObject		mtlobj(plug.node());
		ConvRef		cvref = Exporter->MayaObjects[mtlobj];
		meConvObj*	conv = cvref;
		meConvMtl*	mtlconv = dynamic_cast<meConvMtl*>( conv );

		if (!mtlconv)
			continue;						// not a material for this state
		if (!mtlconv->IsChild() ||
			(mtlconv->Parent() != state))	// belongs to this state?
			continue;						// this texture linked to another state too
		meLog(2, "%s: %s linked to %s", GetMayaName(), iter.PlugName(), mtlconv->GetMayaName());
		usage = meConvTex::CheckUsage(iter.PlugName(), state, blendmode, sampler);
	}
	return usage;
}

bool meConvShaderLayer::DoLayer(MPlug plug, int i)
{
    MPlug	colorplug;
    MPlug	childplug;

    if (!meConnIter::FindChild(plug, "color", colorplug))
		ME_ERROR(("ERROR: color input not found for layer %d of %s", i, GetMayaName()), false);

    meConnIter	iter(colorplug, true);
    int			numtex = 0;

    while (iter.Next(childplug))
    {
		MObject obj(childplug.node());
		MFnDependencyNode node(obj);
		MString name = node.name();
		
		if (obj.hasFn(MFn::kTexture2d) || obj.hasFn(MFn::kTexture3d))
		{
			meLog(1, "%s: Texture %s connected to layer %d",
				  GetMayaName(), name.asChar(), i);
			m_TexInputs.append(obj);
			++numtex;
		}
		else if (obj.hasFn(MFn::kLambert))
		{
			meConnIter mtliter(obj, "color", false);

			while (mtliter.Next(obj))
				if (obj.hasFn(MFn::kTexture2d) || obj.hasFn(MFn::kTexture3d))
				{
					meLog(1, "%s: Texture %s connected to layer %d",
						  GetMayaName(), name.asChar(), i);
					m_TexInputs.append(obj);
					++numtex;
				}
		}
		else if (obj.hasFn(MFn::kLayeredTexture) ||
				 obj.hasFn(MFn::kLayeredShader))
		{
			int n = DoLayer(childplug, i);
			if (n > 0)
			{
				numtex += n;
				continue;
			}
		}
		else
			meLog(1, "ERROR: %s Unsupported usage of layered shader type %s",
				  name.asChar(),obj.apiTypeStr());
    }
    return numtex > 0;
}

bool meConvTexLayer::DoLayer(MPlug plug, int i)
{
    MPlug	colorplug;
    MPlug	blendplug;
    MPlug	visplug;
    bool	isvis;
    int		blendmode = ME_MUL;

    if (meConnIter::FindChild(plug, "isVisible", visplug) && !visplug.getValue(isvis))
    {
		meLog(1, "WARNING:%s  skipping invisible layer %d of %s", GetMayaName(), i);
		return false;
    }
    if (!meConnIter::FindChild(plug, "blendMode", blendplug) ||
		!blendplug.getValue(blendmode))
		meLog(1, "ERROR: %s missing blend mode for layer %d\n", GetMayaName(), i);
    if (!meConnIter::FindChild(plug, "color", colorplug))
		ME_ERROR(("ERROR: %s color input not found for layer %d", GetMayaName(), i), false);

    meConnIter	iter(colorplug, true);
    MObject	texobj;
    int		numtex = 0;

    while (iter.Next(texobj))
    {
		MFnDependencyNode texnode(texobj);
		MString texname = texnode.name();
		
		if (texobj.hasFn(MFn::kTexture2d) || texobj.hasFn(MFn::kTexture3d))
		{
			meLog(1, "%s: Texture %s connected to layer %d", GetMayaName(), texname.asChar());
			m_TexInputs.append(texobj);
			m_BlendModes.append(blendmode);
			++numtex;
		}
		else
			meLog(1, "ERROR: %s Unsupported usage of layered shader, ignoring %s of type %s",
				  GetMayaName(), texname.asChar(), texobj.apiTypeStr());
    }
    return numtex > 0;
}

