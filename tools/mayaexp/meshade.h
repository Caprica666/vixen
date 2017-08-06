#pragma once

#include "metypes.h"

class meConvState;
class meConvTex;
class meConvMtl;

/*!
 * @class meConvShader
 * @brief Base converter for all Maya shader nodes.
 *
 * Converts a shader from Maya into Vixen.
 * Shader nodes include materials, textures and UV mapping (anything
 * that affects rendering state).
 *
 * @see meConvObj meConvState meConvMtl
 */
class meConvShader : public meConvObj
{
public:
    meConvShader(MObject& mayaobj) : meConvObj(mayaobj) { }

    virtual int	Link(meConvObj* convparent);
    void		PrintShader();
    static int	GetMapUsage(const char* attrname, int& blendmode);
    static meConvShader* MakeConverter(MObject obj);
};


/*!
 * @class meConvState
 * @brief Converter for rendering state
 *
 * Converts a shader from Maya into a Vixen appearance.
 *
 * @see meConvObj meConvMtl meConvTex
 */
class meConvState : public meConvShader
{
   friend class meConvTex;
public:
    meConvState(MObject& mayaobj);

    int         Convert(meConvObj* = NULL);
    SharedObj*	Make();
    meConvTex*	FindTexture(const char* texname, int& index);
    int			FindTexIndex(meConvTex* conv);
    int			AddTexture(meConvTex* conv);
	void		RemoveTexture(meConvTex* texconv);
	bool		FindUVMapper(const char* meshname, int uvidx, MObject& uvobj);
    bool		MakeShaders();
	void		ConvertShaders();
    int			GetNumTex() const	{ return GetSize() - 1; }
    Appearance* GetAppearance()	{ return (Appearance*) (SharedObj*) m_VXObj; }

    bool		HasDiffuseTexture;
    bool		HasAmbientTexture;
    bool		HasSpecularTexture;
    bool		HasEmissionTexture;
	bool		HasBumpTexture;
    ObjArray	Samplers;

protected:
    bool		FindSurfaceShader();

};


/*!
 * @class meConvMtl
 * @brief Converter for materials
 *
 * Converts a shader from Maya into a Vixen material.
 * Handles MFnLamberShader, MFnReflectShader and MFnPhongShader
 *
 * @see meConvObj meConvState
 */
class meConvMtl : public meConvShader
{
public:
    meConvMtl(MObject& mayaobj) : meConvShader(mayaobj) { }

    int			Convert(meConvObj* = NULL);
    SharedObj*	Make();
    int			Link(meConvObj* convparent = NULL);

private:
    void		UpdateState(meConvState* state, const MColor& diffuse, float alpha);
	int			ConvertCustom(meConvState* state);
};

