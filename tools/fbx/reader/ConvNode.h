/****************************************************************************************

Copyright (C) 2013 Autodesk, Inc.
All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise accompanies
this software in either electronic or hard copy form.

****************************************************************************************/

#pragma once
#include <fbxsdk.h>

namespace Vixen { namespace FBX {

/*
 * Base class for conversion from FbxNode to Vixen Model or Engine objects.
 * It handles linking the Vixen object to the user pointer in the FbxNode
 * and naming the Vixen object based on the FBX node name.
 */
class ConvNode
{
public:
	ConvNode(FbxNode* pNode, const TCHAR* name = NULL);
	ConvNode(FbxObject* pObj, const TCHAR* name = NULL);
	ConvNode(const TCHAR* name = NULL);

	virtual SharedObj*	Convert();

protected:
	FbxObject*		m_FBXObj;
	FbxNode*		m_FBXNode;
	SharedObj*		m_VixNode;
	Core::String	m_Name;
};

/*
 * Base class for conversion from FbxNode to Vixen Model object.
 * It handles adding the FBX node matrix as the current transform
 * of the Vixen model.
 */
class ConvModel : public ConvNode
{
public:
	ConvModel(FbxNode* pNode, const TCHAR* name = NULL) : ConvNode(pNode, name) { }
	Model*				MakeModel();
	virtual	SharedObj*	Convert();
	static	void		ConvertMatrix(const FbxMatrix& srcmtx, Matrix& dstmtx);
};

/*
 * Class for conversion from FbxCamera to Vixen Camera object.
 * It handles setting the Vixen view volume from the FBX camera aspect, fov, near and far
 * and computing the view matrix of the camera.
 * This class can also determine the current FBX camera for a scene.
 */
class ConvCamera: public ConvModel
{
public:
	ConvCamera(FbxNode* pNode, const TCHAR* name = NULL) : ConvModel(pNode, name) { m_Camera = pNode->GetCamera(); }
	ConvCamera(FbxCamera* pCamera, const TCHAR* name = NULL) : ConvModel(pCamera->GetNode(), name), m_Camera(pCamera) { }
	ConvCamera(FbxScene* pScene);
	Camera*				MakeCamera(FbxScene* pScene);
	virtual	SharedObj*	Convert();
	void				ComputeView(Matrix& mtx);
	void				ComputeProjection();

	static void			GetAnimatedParameters(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer);
	static FbxNode*		GetCurrentCamera(FbxScene* pScene);
	static FbxNode*		GetCurrentCamera(FbxScene* pScene, FbxTime& pTime, FbxAnimLayer* pAnimLayer);
	static bool			IsProducerCamera(FbxCamera*, FbxScene*);
	static int			AddCamera(FbxNode*);
	static FbxNode*		GetCamera(int i)	{ return m_Cameras[i]; }
	static FbxNode*		FindCamera(const TCHAR* name);
	static int			FindCamera(FbxNode*);

protected:
	FbxScene*		m_Scene;
	FbxCamera*		m_Camera;
	static FbxArray<FbxNode*> m_Cameras;
};

/*
 * Class for conversion from FbxLight to Vixen Light object.
 * It handles adding the FBX light parameters to the Vixen light
 * (color, spotlight angles).
 */
class ConvLight : public ConvModel
{
public:
	ConvLight(FbxNode* pNode, const TCHAR* name = NULL) : ConvModel(pNode, name) { m_Light = pNode->GetLight(); }
	Light*				MakeLight();
	virtual SharedObj*	Convert();

private:
	const FbxLight*	m_Light;
};

/*
 * Class for conversion from FbxSurfaceMaterial to Vixen Appearance object.
 * It handles conversion of the FBX material properties to the Vixen Material
 * and creating Vixen Sampler objects for all of the FBX file textures.
 */
class ConvAppear : public ConvNode
{
public:
	ConvAppear(FbxSurfaceMaterial* pMtl, const TCHAR* name = NULL) : ConvNode(pMtl, name) { m_Material = pMtl; }
	Appearance*			MakeAppearance();
	virtual SharedObj*	Convert();
	Texture*			GetTextureProperty(const char * pPropertyName);
	FbxDouble3			GetMaterialProperty(const char * pPropertyName, const char * pFactorPropertyName);
private:
	const FbxSurfaceMaterial*	m_Material;
};

/*
 * Class for conversion from FbxFileTexture to Vixen Texture object.
 * It creates a Vixen Texture which references the file from the FbxFileTexture.
 */
class ConvTexture : public ConvNode
{
public:
	ConvTexture(FbxFileTexture* pTex, const TCHAR* name = NULL) : ConvNode(pTex, name) { m_Texture = pTex; }
	Texture*			MakeTexture();
	virtual SharedObj*	Convert();

private:
	const FbxFileTexture*	m_Texture;
};

} }	// end FBX


