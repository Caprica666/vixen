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

bool ReadFBXFile(const TCHAR* filename, Core::Stream* instream, LoadEvent* ev);

/*
 * Imports an FBX file and converts it to a Vixen Scene.
 *
 * -	Vixen only supports indexed triangle meshes. All FBX geometry is triangulated
 *		before converting to Vixen format.
 * -	Skin, Blend Shape and Vertex Cache deformers are supported.
 * -	Animation of node transforms and skeleton bones are supported.
 *		Other forms of animation (light colors, visibility, etc.) are ignored.
 */
class FBXReader
{
public:
    enum Status
    {
        UNLOADED,               // Unload file or load failure;
        MUST_BE_LOADED,         // Ready for loading file;
        MUST_BE_REFRESHED,      // Something changed and redraw needed;
        REFRESHED               // No redraw needed.
    };

    FBXReader(const char * pFileName);
    ~FBXReader();

	Status GetStatus() const { return mStatus; }

    Scene*	LoadFile();

	static	int		FrameRate;
	static	bool	DoDeformers;
	static	bool	DoGeometry;
	static	bool	DoSkeleton;
	static	bool	DoAnimation;
	static	int		Debug;

protected:
	void			ConvertScene(FbxAnimLayer* pAnimLayer);
	Model*			ConvertNode(FbxNode* pNode, Model* root, Engine* simroot);
	Engine*			ConvertSkeleton(FbxNode* pNode, Model* root, Engine* simroot);
	Transformer*	ConvertBone(FbxNode* pNode, Model* root, Engine* simroot);
	Transformer*	ConvertAnim(FbxNode* pNode, Engine* simroot);
	void			ConnectCamera();
	FbxNode*		FindRootBone(FbxNode* pNode);
	void			SetupIO(FbxManager* pManager);
	void			InitSDK(FbxManager*& pManager, FbxScene*& pScene);
	void			DestroySDK(FbxManager* pManager, bool pExitStatus);

private:
	Scene*				mVixScene;
	Core::String		mFileBase;
	TCHAR				mDirectory[VX_MaxPath];
    const char*			mFileName;
    mutable Status		mStatus;
    FbxManager*			mSdkManager;
    FbxScene*			mScene;
    FbxImporter*		mImporter;
    FbxAnimLayer*		mCurrentAnimLayer;
	FbxTime				mCacheStart;
	FbxTime				mCacheStop;
};


} }	// end FBX


