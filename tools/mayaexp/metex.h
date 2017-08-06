#pragma once

#include "metypes.h"
#include "meshade.h"

/*!
 * The first 13 entries correspond to Maya constants - don't change them
 */
enum meBlendModes
{
    ME_REPLACE = 1,	// color = fc, alpha = fa
    ME_DECAL,		// color = fc * fa + (bc * (1 - fa))
					// alpha = 1 - ((1 - ba) * (1 - fa))
    ME_MUL_ALPHA,	// color = bc * fa, alpha = ba * fa
    ME_MUL_INVALPHA,// color = bc * (1 - fa), alpha = ba * (1 - fa)
    ME_ADD,			// color = bc + (fc * fa), alpha = ba
    ME_SUB,			// color = bc - (fc * fa), alpha = ba
    ME_MUL,			// color = bc * (fc * fa + 1 - fa), alpha = ba
    ME_DIFF,		// color = (abs((fc * fa) - bc)) * fa + bc * (1 - fa), alpha = ba
    ME_LIGHTEN,		// color = (max((fc * fa), bc)) * fa + bc * (1 - fa), alpha = ba
    ME_DARKEN,		// color = (min((fc * fa), bc)) * fa + bc * (1 - fa), alpha = ba
    ME_MUL_ADD,		// color = bc * (1 + (fc * fa)), alpha = ba
    ME_MUL_SUB,		// color = bc * (1 - (fc * fa)), alpha = ba
    ME_ILLUM,		// color = bc * (2 * fc * fa + 1 - fa), alpha = ba
};

enum meUsage
{
    ME_MAP_MODULATE = 1,	// texture multiplied with background
    ME_MAP_REPLACE,			// texture replaces background
    ME_MAP_ADD,				// texture added to background
    ME_MAP_DIFFUSE,			// texture used as diffuse map
    ME_MAP_REFLECTION,		// texture used as reflection map
    ME_MAP_AMBIENT,			// texture used as ambient map
    ME_MAP_SPECULAR,		// texture used as specular map
    ME_MAP_BUMP,			// texture used as bump map
    ME_MAP_OPACITY,			// texture used as opacity map
    ME_MAP_EMISSION,		// texture used as emission (incandescence) map
};


/*!
 * @class meConvTex
 * @brief Converter for textures
 *
 * Converts a texture object from Maya into a Vixen texture.
 *
 * @see meConvObj meConvState
 */
class meConvTex : public meConvShader
{
	friend class meConvState;
public:
    meConvTex(MObject& mayaobj) : meConvShader(mayaobj) { }

    virtual int         Convert(meConvObj* = NULL);
	virtual  SharedObj*	Make();
	virtual int			Link(meConvObj* conv = NULL);
    static int			CheckUsage(const char* attrname, meConvState* state, int& blendmode, Sampler* sampler);
    static bool			MapUV(MObject uvobj, float* uvs, int n);
    Texture*			FindTexture(const char* filename);

    int					Usage;		// type of map (diffuse, specular, ...)
    int					BlendMode;	// how to blend (modulate, add, ...)
	Ref<meConvUV>		UVMapper;

protected:
	int					FindMaterial(meConvState* state);
    bool				MakeBitmap(const char* texfile, const char* ext, char* outbuf);
    bool				MakeTexEnv(Appearance* app, int texindex);
    void				LinkUV(MObject projection);
    bool				UpdateState(meConvState* state, bool hasalpha);
};

/*!
 * @class meConvTexLayer
 * @brief Converter for layered textures
 *
 * Converts a layered texture object from Maya into one
 * or more Vixen images.
 *
 * @see meConvTex meConvState
 */
class meConvTexLayer : public meConvShader
{
public:
    meConvTexLayer(MObject& mayaobj) : meConvShader(mayaobj) { }

    int         Convert(meConvObj* = NULL);
    SharedObj*	Make();
    int			Link(meConvObj* conv = NULL);

protected:
    virtual bool DoLayer(MPlug plug, int layerno);
    int			 FindMaterial(meConvState* state);

    MObjectArray m_TexInputs;
    MIntArray	 m_BlendModes;
};

class meConvShaderLayer : public meConvTexLayer
{
public:
    meConvShaderLayer(MObject& mayaobj) : meConvTexLayer(mayaobj) { }

protected:
    bool	DoLayer(MPlug plug, int layerno);
};
