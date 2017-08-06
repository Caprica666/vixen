#include "vix_all.h"
#include <maya/MFnSet.h>
#include <maya/MFnTransform.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnCompoundAttribute.h>

/*!
 * @fn SharedObj* meConvProj::Make()
 *
 * A Maya projection node applies a 3D projection onto a texture. It has two inputs -
 * the texture image and a 3D texture placement (4x4 matrix).
 * This converter exists to provide a way to link the 3D texture matrix to the texture.
 * It does not have a corresponding Vixen object.
 *
 * @return NULL
 *
 * @see meConvTex::Make meConvProj::Convert
 */
SharedObj* meConvProj::Make()
{
    meConnIter	iter(m_MayaObj, "image", true);
	MPlug		plug;

    while (iter.Next(m_TexImage, MFn::kTexture2d))
		return NULL;
    ME_ERROR(("ERROR: Projection texture %s has no image file", GetMayaName()), NULL);
    return NULL;
}



/*!
 * @fn int meConvProj::Link(meConvObj* convparent)
 *
 * Links this UV mapper to the texture converter associated with
 * the image used as input to the projection. This effectively associates
 * the 3D texture matrix with the right texture.
 *
 * @return -1 on error, 0 if no link, 1 if successful link
 *
 * @see meConvTex::Make
 */
int meConvProj::Link(meConvObj* convparent)
{
	ConvRef		cvref = Exporter->MayaObjects[m_TexImage];
    meConvTex*	texconv = dynamic_cast<meConvTex*>( (meConvObj*) cvref );
    MObject		uvmapper;
    meConvUV3*	uvconv;

    if (!texconv)
		return -1;
    assert(convparent == NULL);
/*
 * Find the UV converter which uses the projection matrix
 */
    meConnIter	iter(m_MayaObj, "placementMatrix", true);

    if  (!iter.Next(uvmapper, MFn::kPlace3dTexture))
		return -1;
	cvref = Exporter->MayaObjects[uvmapper];
	uvconv = dynamic_cast<meConvUV3*> ((meConvObj*) cvref);
	if (!uvconv)
		return -1;
    uvconv->NoTexCoords = true;				// texcoords generated automatically
    uvconv->UVGen = Sampler::TEXGEN_SPHERE;	// assume spherical projection
/*
 * Link the UV converter to the texture converter
 */
    if (!texconv->UVMapper.IsNull())
    {
		ME_ERROR(("ERROR: texture %s with multiple UV mappers", texconv->GetMayaName()), 0);
    }
    meLog(1, "UV projection %s linked to texture %s", uvconv->GetMayaName(), texconv->GetMayaName());
    texconv->UVMapper = uvconv;		// link UV mapper to the texture
    texconv->DoConvert = true;		// link texture to state
    return 1;
}

meConvUV::meConvUV(MObject& uvobj) : meConvShader(uvobj)
{ 
    MStatus status;
    MFnDependencyNode uvmap(uvobj, &status);
    MPlug plug;

    RotateFrame = 0;
    RotateUV = 0;
    CoverageU = 1;
    CoverageV = 1;
    TranslateU = 0;
    TranslateV = 0;
    RepeatU = 1;
    RepeatV = 1;
    OffsetU = 1;
    OffsetV = 0;
    WrapU = 1;
    WrapV = 1;
    Stagger = 0;
    Mirror = 0;
    HasMapper = false;
    NoTexCoords = false;
    MakeTexCoords = false;
}

/*!
 * @fn SharedObj* meConvUV::Make()
 *
 * Extracts UV mapping parameters from the Maya texture placement object.
 * If the UV mapper is attached to a texture, this converter becomes a
 * child of the texture's converter.
 *
 * @see meConvGeo::LinkUVMappers
 */
SharedObj* meConvUV::Make()
{ 
    MStatus status;
    MFnDependencyNode uvmap(m_MayaObj, &status);
    MPlug plug;

    if (!status || !m_MayaObj.hasFn(MFn::kPlace2dTexture))
		return NULL;
    HasMapper = true;
    plug = uvmap.findPlug("coverageU");
    plug.getValue(CoverageU);
    plug = uvmap.findPlug("coverageV");
    plug.getValue(CoverageV);
    plug = uvmap.findPlug("translateU");
    plug.getValue(TranslateU);
    plug = uvmap.findPlug("translateV");
    plug.getValue(TranslateV);
    plug = uvmap.findPlug("repeatU");
    plug.getValue(RepeatU);
    plug = uvmap.findPlug("repeatV");
    plug.getValue(RepeatV);
    plug = uvmap.findPlug("offsetU");
    plug.getValue(OffsetU);
    plug = uvmap.findPlug("offsetV");
    plug.getValue(OffsetV);
    plug = uvmap.findPlug("wrapU");
    plug.getValue(WrapU);
    plug = uvmap.findPlug("wrapV");
    plug.getValue(WrapV);
    plug = uvmap.findPlug("rotateUV");
    plug.getValue(RotateUV);
    plug = uvmap.findPlug("rotateFrame");
    plug.getValue(RotateFrame);
    return this;
}

int meConvUV::Convert(meConvObj*)
{
	meLog(1, "%s: Converting UV mapper", GetMayaName());
    return 1;
}

/*!
 * @fn bool meConvUV::MapUV(Vec2* uv, Vec3* loc, Vec3* ctr)
 * @param uv	U and V coordinates before (input) and after (output) mapping
 * @param loc	location at this UV
 * @param ctr	center of object
 *
 * Maps a U,V coordinate from a Maya mesh using the given texture placement object.
 * The location and center are not used by the base implementation but are
 * included for subclasses which generate texcoords from object space coordinates.
 *
 * @returns true if mapping successful, false if no mapper
 *
 * @see meConvUV3::MapUV
 */
bool meConvUV::MapUV(Vec2* uv, Vec3* loc, Vec3* ctr)
{
    float outu, outv;

    if (!HasMapper)
		return false;
    map(uv->x, uv->y, outu, outv);
    uv->x = outu;
    uv->y = outv;
    return true;
}


/*!
 * @fn bool meConvUV::MapUV(Vec2* uvs, int n)
 * @param uvs	input and output array of UVs
 * Maps U,V coordinates from a Maya mesh using the given texture placement object.
 *
 * @returns true if mapping successful, false if no texture mapper
 */
bool meConvUV::MapUV(Vec2* uvs, int n)
{
    float	outU, outV;
    float	inU, inV;

    if (!HasMapper)
		return false;
    for (int i = 0; i < n; ++i)
    {
		Vec2* uvptr = uvs + i;
		Vec2& tc = *uvptr;
		inU = tc.x;
		inV = tc.y;
		map(inU, inV, outU, outV);
		tc.x = outU;
		tc.y = outV;
    }
    return true;
}

/*
 * @fn int meConvUV::Link(meConvObj* convparent)
 * Find the converter for texture which uses this UV mapper and attach
 * the UV mapper as a child of the texture converter.
 * This relationship is used later in determining which UV mapper
 * to use for which set of mesh texture coordinates.
 */
int meConvUV::Link(meConvObj* convparent)
{ 
    MObject		texobj;
    meConnIter	iter(m_MayaObj, "outUV", false);

    while (iter.Next(texobj))
    {
		ConvRef		cvref;
		meConvTex*	txconv;

		if (!texobj.hasFn(MFn::kTexture2d) &&		// not a texture?
			!texobj.hasFn(MFn::kTexture3d))
			continue;
		cvref = Exporter->MayaObjects[texobj];
		txconv = dynamic_cast<meConvTex*>( (meConvObj*) cvref );
		if (!txconv)
			continue;
		if (!txconv->UVMapper.IsNull() && ((meConvUV*) txconv->UVMapper != this))
		{
			meLog(1, "ERROR: %s texture with multiple UV mappers", txconv->GetMayaName());
			continue;
		}
		else
		{
			meLog(1, "%s: UV mapper linked to texture %s", GetMayaName(), txconv->GetMayaName());
			txconv->UVMapper = this;			// make UV mapper a child of the texture
		}
   }
    return 1;
}

#define ME_MAPMETHOD1

void meConvUV::map(float inU, float inV, float& outU, float& outV)
{
    bool inside = true;
    float tmp;

    outU = mapU(inU, inV, inside);
    outV = mapV(inU, inV, inside);

    if (inside)
    {
        if (Stagger)
            if (int(ffloor(outV)) % 2) outU += 0.5f;
        if (Mirror)
		{
            if (int(ffloor(outU)) % 2) outU = 2 * ffloor(outU) + 1.0f - outU;
            if (int(ffloor(outV)) % 2) outV = 2 * ffloor(outV) + 1.0f - outV;
        }

#ifdef ME_MAPMETHOD1
	float cosa = cos(-RotateUV);
	float sina = sin(-RotateUV);
	outU -= 0.5f;
	outV -= 0.5f;

	tmp = outU * cosa - outV * sina + 0.5f;
	outV = outU * sina + outV * cosa + 0.5f;
#else
	float cosa = cos(RotateUV);
	float sina = sin(RotateUV);
	outU -= 0.5f;
	outV -= 0.5f;

        tmp = outU * sina + outV * cosa + 0.5;
        outV = outU * cosa - outV * sina + 0.5;
#endif
        outU = tmp;
    }
}


float meConvUV::mapU(float inU, float inV, bool& inside)
{
#ifdef ME_MAPMETHOD1
    double outU = (inV - 0.5) * sin(-RotateFrame) + (inU - 0.5) * cos(-RotateFrame) + 0.5;
    if (CoverageU < 1.0)
    {
		if (outU >= 1.0)
			outU -= floor(outU);
		else if (outU < 0.0)
			outU = outU - floor(outU) + 1.0;
		outU -= TranslateU;
        if (WrapU)
		{
			if (outU >= CoverageU)
			outU -= 1.0;
			else if (outU < 0.0)
				outU += 1.0;
		}
#else
    float outU = (inU - 0.5) * sin(RotateFrame) + (inV - 0.5) * cos(RotateFrame) + 0.5;
    if (CoverageU < 1.0)
    {
        if (WrapU)
		{
			if (outU > 1.0)
			outU -= ffloor(outU);
			else if (outU < 0.0)
			outU = outU - ffloor(outU) + 1.0;
			outU = outU - (TranslateU - ffloor(TranslateU));
			if (outU > CoverageU)
			outU -= 1.0;
				else if (outU < 0.0)
			outU += 1.0;
				outU /= CoverageU;
		}
#endif
        if (outU < 0.0 || outU > 1.0)
			inside = false;
    }
    else
    {
        outU = (outU - TranslateU) / CoverageU;
        if (!WrapU)
		{
			if (outU < 0.0 || outU > 1.0)
				inside = false;
		}
    }
    return (float) (outU * RepeatU + OffsetU);
}

float meConvUV::mapV(float inU, float inV, bool& inside)
{
#ifdef ME_MAPMETHOD1
    float outV = (inV - 0.5f) * cos(-RotateFrame) - (inU - 0.5f) * sin(-RotateFrame) + 0.5f;
    if (CoverageV < 1.0f)
    {
        if (WrapV)
	{
	    if (outV > 1.0f)
		outV -= ffloor(outV);
	    else if (outV < 0.0f)
		outV = outV - ffloor(outV) + 1.0f;
	    outV = outV - (TranslateV - ffloor(TranslateV));
	    if (outV > CoverageV)
		outV -= 1.0f;
            else if (outV < 0.0f)
		outV += 1.0f;
            outV /= CoverageV;
	}

#else
    float outV = (inU - 0.5f) * cos(RotateFrame) - (inV - 0.5f) * sin(RotateFrame) + 0.5f;
    if (CoverageV < 1.0f)
    {
        if (WrapV)
	{
            if (outV > 1.0f)
                outV -= ffloor(outV);
            else if (outV < 0.0f)
                outV = outV - ffloor(outV) + 1.0f;
            outV = outV - (TranslateV - ffloor(TranslateV));
            if (outV > CoverageV)
                outV -= 1.0f;
            else if (outV < 0.0f)
               outV += 1.0f;
            outV /= CoverageV;
	}
#endif
        if (outV < 0.0f || outV > 1.0f)
            inside = false;
    }
    else
    {
        outV = (outV - TranslateV) / CoverageV;
        if (!WrapV)
	{
            if (outV < 0.0f || outV > 1.0f)
                inside = false;
	}
    }                                                           
    return outV * RepeatV + OffsetV;
}


meConvUV3::meConvUV3(MObject& uvobj) : meConvUV(uvobj)
{ 
    HasMapper = false;
    NoTexCoords = true;
    UVMatrix.Identity();
}

/*!
 * @fn SharedObj* meConvUV3::Make()
 *
 * Extracts UV mapping matrix from the Maya 3D texture placement object.
 * The Maya UV mapping may be directly attached to a texture or it may
 * be linked to an environment mapping node.
 *
 * 3D texture placement generates texture coordinates - it does not
 * start with a UV set from Maya. The 3D matrix becomes the Vixen
 * texture projection matrix in the GeoState. If it is used as a
 * reflection map, the texture coordinates that are generated dynamically
 * use a spherical projection and vary with the eyepoint.
 *
 * Note: 3D matrix currently disabled - not sure how to apply it in Vixen.
 *
 * @see meConvGeo::LinkUVMappers meConvUV3::Link meConvUV3::UpdateState
 */
SharedObj* meConvUV3::Make()
{ 
    MFnTransform uvmap(m_MayaObj, &m_Status);
    MDagPath dagPath;

    if (!m_Status || !m_MayaObj.hasFn(MFn::kPlace3dTexture))
		return NULL;
    MTransformationMatrix tmtx = uvmap.transformation();
    MMatrix mtx = tmtx.asMatrix();
    float	fltdata[4][4];

    if (!mtx.isEquivalent(MMatrix::identity))
    {
		mtx.get(fltdata);
		UVMatrix.SetMatrix(&fltdata[0][0]);
		UVMatrix.Transpose();
    }
//    HasMapper = true;
    return NULL;
}

int meConvUV3::Link(meConvObj* convparent)
{ 
    MFnTransform uvmap(m_MayaObj);
    MPlug	plug = uvmap.findPlug("worldInverseMatrix");
    MObject	texobj;

    if (IsChild())							// already attached?
		return 1;
    if (plug.numElements() < 1)
		return 0;
    meConnIter	iter(plug[0], false);
    while (iter.Next(texobj))
    {
		if (texobj.hasFn(MFn::kTexture3d))	// associated with 3D texture?
			MakeTexCoords = true;			// generate static texcoords
		else if (!texobj.hasFn(MFn::kTexture2d)) // not a texture?
			continue;
		ConvRef		cvref = Exporter->MayaObjects[texobj];
		meConvTex* txconv = dynamic_cast<meConvTex*>( (meConvObj*) cvref );
		if (!txconv)
			continue;
		meLog(1, "UV mapping %s linked to texture %s", GetMayaName(), txconv->GetMayaName());
		txconv->Append(this);				// make UV mapper a child of the texture
		assert(UVGen == Sampler::NONE);
    }
    return 1;
}

int meConvUV3::Convert(meConvObj*)
{
	meLog(1, "%s: Converting 3D UV mapper", GetMayaName());
    return 1;
}

/*!
 * @fn void meConvUV3::UpdateState(meConvState* state, int texindex)
 * @state state converter for GeoState to update
 * @texindex index of texture to update
 *
 * Updates the Vixen GeoState invormation for 3D UV mapping.
 * The UV matrix from this converter becomes the Vixen
 * texture matrix.
 */
void meConvUV3::UpdateState(meConvState* state, int texindex)
{
}

void meConvUV3::map(float inU, float inV, float& outU, float& outV)
{
    if (!HasMapper || NoTexCoords)
    {
		outU = inU;
		outV = inV;
		return;
    }
    Vec3 vec(inU, inV, 0);
	vec *= UVMatrix;
    outU = vec[0];
    outV = vec[1];
}

/*!
 * @fn bool meConvUV::MapUV(Vec2* uv, Vec3* loc, Vec3* ctr)
 * @param uv	U and V coordinates before (input) and after (output) mapping
 * @param loc	location at this UV
 * @param ctr	center of object
 *
 * Maps a U,V coordinate from a Maya mesh using the given texture placement object.
 * The location and center are only used if texture coordinates are being generated
 * from the object space coordinates (MakeTexCoords is set). In this case,
 * there are no input UVs.
 *
 * @returns true if mapping successful, false if no mapper
 */
bool meConvUV3::MapUV(Vec2* uv, Vec3* loc, Vec3* center)
{
    Vec3 v;

    if (!HasMapper)
		return false;
    if (!MakeTexCoords)
		return meConvUV::MapUV(uv, loc, center);
    if (NoTexCoords)
		return false;
    v = *loc - *center;		// generate UVs in spherical projection
    v.Normalize();
    uv->x = v[0] / 2 + 0.5f;
    uv->y = v[2] / 2 + 0.5f;
    uv->x *= 4;
    uv->y *= 4;
    return true;
}
