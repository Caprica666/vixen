#include "vix_all.h"

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItDependencyGraph.h>

/*!
 * @fn meConvGeo::ShaderInfo::ShaderInfo()
 * Initializes the information for a single shader
 * used on a Maya mesh. Maintains UV mapping information
 * to generate each set of Vixen texture coordinates
 * from a Maya UV set and a Maya placeTexture2d.
 */
meConvGeo::ShaderInfo::ShaderInfo()
{
    Offset = Size = NumUVSets = 0;
    ShaderConv = NULL;
    Geom = (Mesh*) NULL;
    MayaShader = MObject::kNullObj;
    ShaderIndex = 0;
    NumVerts = 0;
    for (int i = 0; i < ME_MAX_TEXTURES; ++i)
    {
		UVMappers[i] = NULL;
		UVSets[i] = ME_NOUVS;
    }
}

/*!
 * @fn meConvGeo::meConvGeo
 * Constructor for geometry converter base.
 * This class gathers information that associates
 * Maya shaders with Vixen appearances.
 */
meConvGeo::meConvGeo(const MDagPath& dagpath)
: meConvNode(dagpath)
{
    MFnDagNode node(dagpath);
    MPoint ctr = node.boundingBox().center();
    m_Center.Set((float) ctr.x, (float) ctr.y, (float) ctr.z);
    m_Space = MSpace::kObject;
}

/*!
 * @fn Appearance* meConvGeo::MakeAppearance(MObject shadergroup)
 * @param shadergroup	Maya shader group to make appearance for
 *
 * Makes a Appearance for the given Maya shader group. 
 * If Maya uses several material objects which are the same, they will all
 * map to the same Vixen object).
 *
 * @see meConvState::Share meConvState meConvGeo::LinkUVMappers
 */
Appearance* meConvGeo::MakeAppearance(ShaderInfo* sinfo)
{
    MObject			shadergroup = sinfo->MayaShader;
    meConvState*	cstate = NULL;
    Appearance*		app;
	ConvRef			cvref;
	meConvObj*		conv;
    MFnDependencyNode shadernode(shadergroup, &m_Status);

    if (shadergroup == MObject::kNullObj)
		return NULL;
    if (!m_Status)						// error creating appearance
		ME_ERROR(("ERROR: could not create geometry for %s", shadernode.name()), NULL);
	cvref = Exporter->MayaObjects[shadergroup];
	conv = cvref;
	if (conv == NULL)					// make a converter if we don't have one
		conv = Exporter->makeOne(shadergroup);
 	if (conv == NULL)
		return NULL;
	cstate = dynamic_cast<meConvState*> (conv);
    if (cstate)							// already have converter?
    {
		app = (Appearance*) (SharedObj*) cstate->GetVXObj();
		if (app == NULL)					// error creating appearance
			ME_ERROR(("ERROR: could not create appearance for %s", shadernode.name()), NULL);
		assert(app->IsClass(VX_Appearance));
		sinfo->ShaderConv = cstate;
		return app;
    }
    return NULL;
}

/*!
 * @fn bool meConvGeo::LinkUVMappers(ShaderInfo* state, int nuvsets)
 * @param state	shader state information (designates a particular shader)
 * @param nuvsets number of Maya UV sets
 *
 * Finds the UV mapper (placeTexture2D object) to use for each set of texture
 * coordinates within the Maya mesh. This function queries Maya for the names
 * of the textures used on each UV set. We ask the input shader to find a
 * texture converter for each of the textures Maya returns. The UV converter
 * for each set of texture coords is saved in the shader state information.
 *
 * A single Maya surface can potentially use more than one shader and the UV mapping
 * might differ across the shaders.
 * Vixen requires a separate set of texture coordinates for each texture mapped.
 * Multiple Vixen UV sets can be derived from a single Maya UV set.
 * The Maya UV mapping is applied at conversion time to produce the Vixen UVs.
 *
 * Some UV mappings, such as 3D textures, do not use Maya UV sets. In Vixen,
 * we approximate these by snapshotting part of the 3D texture as a bitmap image
 * and then generating texture coordinates internally. The exporter arbitrary
 * chooses the first set of Maya UV coordinates if there are any, otherwise it
 * does a spherical projection to generate UVs.
 *
 * @see meConvSubd:LinkUvMappers
 */
void meConvGeo::LinkUVMappers(ShaderInfo* state, int nuvsets)
{
    MStatus      status;
    MString      command;
    MStringArray results;
    MString		texname;
    char		buffer[256];

    if (state->ShaderConv == NULL)
		return;
/*
 * The shader could have multiple textures associated with it.
 * We choose the first texture associated with the shader which has a
 * 2D UV mapping associated with it (not a projection or sphere map
 * which does a UV gen and doesn't use texcoords)
 */
    ObjArray::Iter iter(state->ShaderConv->Samplers);
    meConvTex*	txconv;
    meConvUV*	uvconv;
    int			texindex = -1;

    while (txconv = (meConvTex*) iter.Next())
    {
		MObject obj = txconv->GetMayaObj();
		Sampler* sampler = (Sampler*) txconv->GetVXObj();

		if (!obj.hasFn(MFn::kTexture2d) && !obj.hasFn(MFn::kTexture3d))
			continue;								// not a texture
		assert(sampler && sampler->IsClass(VX_Sampler));
		++texindex;									// on to next texture
		uvconv = (meConvUV*) txconv->UVMapper;		// get UV mapper child
		if (uvconv == NULL)
			continue;
		meLog(1, "%s: UV mapping found for texture %s on texindex = %d",
			txconv->GetMayaName(), GetMayaName(), texindex);
		state->UVMappers[texindex] = uvconv;		// save UV mapper
		++(state->NumUVSets);
/*
 * Try to find Maya UV set associated with this texture
 */
		sprintf(buffer, "uvLink -q -t %s -qo %s;", txconv->GetMayaName(), GetMayaName());
		command = buffer;
		state->UVSets[texindex] = 0;
		if (!MGlobal::executeCommand(command, results, false, false))
		{
			sampler->Set(Sampler::TEXCOORD, 0);
			meLog(1, "ERROR: uvLink failed for %s", GetMayaName());
			continue;
		}
		for (uint32 j = 0; j < results.length(); ++j)	// textures using the UV set
		{
			const char* uvsetname = results[j].asChar();
			const char* p = strstr(uvsetname, ".uvSet[");
			if (p)
			{
				int uvset = atol(p + 7);
				state->UVSets[texindex] = uvset; // save Maya UV set
				sampler->Set(Sampler::TEXCOORD, uvset);
				meLog(1, "%s: Texcoord set %d linked to %s", GetMayaName(), uvset, txconv->GetMayaName());
				break;
			}
		}
    }
}


