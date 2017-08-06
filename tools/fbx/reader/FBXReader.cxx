#include "vixen.h"
#include "ConvNode.h"
#include "ConvAnim.h"
#include "ConvMesh.h"
#include "FBXReader.h"

namespace Vixen { namespace FBX {

const float ANGLE_TO_RADIAN = 3.1415926f / 180.f;

int		FBXReader::FrameRate;
bool	FBXReader::DoGeometry = true;
bool	FBXReader::DoDeformers = true;
bool	FBXReader::DoSkeleton = true;
bool	FBXReader::DoAnimation = true;
int		FBXReader::Debug = 0;

class VixenMemoryAllocator
{
public:
	static void* Alloc(size_t pSize)
    {
        char *p = (char*) malloc(pSize+1);
        *p = '#';
        return p+1;
    }

	static void* Calloc(size_t pCount, size_t pSize)
    {
        char *p = (char*)calloc(pCount, pSize+1);
        *p = '#';
        return p+1;
    }

	static void* Realloc(void* pData, size_t pSize)
    {
        if (pData)
        {
            FBX_ASSERT(*((char*)pData-1)=='#');
            if (*((char*)pData-1)=='#')
            {
                char *p = (char*)realloc((char*)pData-1, pSize+1);
                *p = '#';
                return p+1;
            }
            else
            {   // Mismatch
                char *p = (char*)realloc((char*)pData, pSize+1);
                *p = '#';
                return p+1;
            }
        }
        else
        {
            char *p = (char*)realloc(NULL, pSize+1);
            *p = '#';
            return p+1;
        }
    }

	static void Free(void* pData)
    {
        if (pData==NULL)
            return;
        FBX_ASSERT(*((char*)pData-1)=='#');
        if (*((char*)pData-1)=='#')
        {
            free((char*)pData-1);
        }
        else
        {   // Mismatch
            free(pData);
        }
    }
};

/*
 * Constructs a reader to translate an FBX file to a Vixen Scene.
 */
FBXReader::FBXReader(const char* pFileName)
:	mFileName(pFileName), mStatus(UNLOADED),
	mSdkManager(NULL), mScene(NULL), mImporter(NULL), mCurrentAnimLayer(NULL)
{
	// initialize cache start and stop time
	mCacheStart = FBXSDK_TIME_INFINITE;
	mCacheStop  = FBXSDK_TIME_MINUS_INFINITE;

   // Create the FBX SDK manager which is the object allocator for almost 
   // all the classes in the SDK and create the scene.
   InitSDK(mSdkManager, mScene);

   if (mSdkManager)
   {
       // Create the importer.
       int lFileFormat = -1;
       mImporter = FbxImporter::Create(mSdkManager,"");
       if (!mSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(mFileName, lFileFormat) )
       {
           // Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
           lFileFormat = mSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription( "FBX binary (*.fbx)" );;
       }

       // Initialize the importer by providing a filename.
       if(mImporter->Initialize(mFileName, lFileFormat) == true)
       {
           mStatus = MUST_BE_LOADED;
	   }
   }
}

FBXReader::~FBXReader()
{
    // Delete the FBX SDK manager. All the objects that have been allocated 
    // using the FBX SDK manager and that haven't been explicitly destroyed 
    // are automatically destroyed at the same time.
	DestroySDK(mSdkManager, true);
}

/*
 * Loads an FBX file and convert it into a Vixen Scene.
 * Vixen only supports indexed triangle meshes. All FBX geometry is triangulated
 * before converting to Vixen format. Skin, Blend Shape and Vertex Cache
 * deformers are supported. Animation of node transforms and skeleton
 * bones are supported. Other forms of animation (light colors, visibility, etc.)
 * are ignored.
 *
 * @return -> Vixen Scene constructed from file or NULL on error
 */
Vixen::Scene* FBXReader::LoadFile()
{
    bool lResult = false;
    // Make sure that the scene is ready to load.
    if (mStatus == MUST_BE_LOADED)
    {
        if (mImporter->Import(mScene) == true)
        {
            // Set the scene status flag to refresh 
            // the scene in the first timer callback.
            mStatus = MUST_BE_REFRESHED;
			FrameRate = FbxTime::GetFrameRate(mScene->GetGlobalSettings().GetTimeMode());
            // Convert Axis System to what is used in this example, if needed
            FbxAxisSystem SceneAxisSystem = mScene->GetGlobalSettings().GetAxisSystem();
            FbxAxisSystem OurAxisSystem(FbxAxisSystem::eOpenGL);
            if( SceneAxisSystem != OurAxisSystem )
                OurAxisSystem.ConvertScene(mScene);

            // Convert Unit System to what is used in this example, if needed
            FbxSystemUnit SceneSystemUnit = mScene->GetGlobalSettings().GetSystemUnit();
            if (SceneSystemUnit.GetScaleFactor() != 1.0)
            {
                //The unit in this example is centimeter.
 //               FbxSystemUnit::cm.ConvertScene( mScene);
            }

            // Convert mesh, NURBS and patch into triangle mesh
			FbxGeometryConverter lGeomConverter(mSdkManager);
			lGeomConverter.Triangulate(mScene, /*replace*/true);

			// Split meshes per material, so that we only have one material per mesh (for VBO support)
//			lGeomConverter.SplitMeshesPerMaterial(mScene, /*replace*/true);

            // Bake the scene for one frame
            ConvertScene(mCurrentAnimLayer);

            lResult = true;
			ConnectCamera();
        }
        else
        {
            // Import failed, set the scene status flag accordingly.
            mStatus = UNLOADED;
        }

        // Destroy the importer to release the file.
        mImporter->Destroy();
        mImporter = NULL;
    }
	if (lResult)
		return mVixScene;
	return NULL;
}

/*
 * Converts an FBX Scene into a Vixen Scene.
 * @param	pAnimLayer	FBX animation layer (currently unused).
 *
 * The Vixen scene graph mirrors the FBX scene graph.
 * FBX rigid body and soft body animations are attached to the
 * nodes they animation. In Vixen, they are kept in a separate
 * Simulation tree.
 *
 * Vixen only supports indexed triangle meshes. All FBX geometry is triangulated
 * before converting to Vixen format. Skin, Blend Shape and Vertex Cache
 * deformers are supported. Animation of node transforms and skeleton
 * bones are supported. Other forms of animation (light colors, visibility, etc.)
 * are ignored.
 *
 */
void FBXReader::ConvertScene(FbxAnimLayer* pAnimLayer)
{
	TCHAR	filebase[VX_MaxPath];
	Model*	sceneroot = NULL;
	Engine*	simroot = new Vixen::Engine();

	Core::Stream::ParseDirectory(mFileName, filebase, mDirectory);
	if (World::Get()->GetMediaDir() == NULL)
		World::Get()->SetMediaDir(mDirectory);
	mVixScene = new Vixen::Scene();
	mFileBase = filebase;
	mFileBase += TEXT('.');
	mVixScene->SetName(mFileBase + TEXT("scene"));
	simroot->SetName(mFileBase + TEXT("simroot"));
	mVixScene->SetEngines(simroot);

	/*
	 * Find all the FBX File textures in the scene and make the corresponding
	 * Vixen textures for them. Each Vixen texture is attached to the FBX
	 * texture via the user data pointer.
	 */
	const int n = mScene->GetTextureCount();
    for (int i = 0; i < n; ++i)
    {
        FbxTexture* lTexture = mScene->GetTexture(i);
        FbxFileTexture* lFileTexture = FbxCast<FbxFileTexture>(lTexture);
        if (lFileTexture && !lFileTexture->GetUserDataPtr())
        {
			ConvTexture fbxtex(lFileTexture);
			fbxtex.MakeTexture();
			fbxtex.Convert();
        }
    }
    ConvertNode(mScene->GetRootNode(), NULL, simroot);
	if (DoAnimation || DoDeformers)
		ConvertAnim(mScene->GetRootNode(), simroot);
	sceneroot =  mVixScene->GetModels();
	if (sceneroot && !sceneroot->IsParent())
		mVixScene->SetModels(NULL);
	if (simroot->IsParent())
		simroot->Stop(Engine::CONTROL_CHILDREN);
	else
		mVixScene->SetEngines(NULL);
}

/*
 * Convert an FBX node to the corresponding Vixen object.
 * @param pNode			-> FBX node
 * @param pAnimLayer	-> FBX animation layer (currently unused)
 * @param root			-> Vixen scene graph parent node (used as the parent
 *						for any newly created Models).
 * @param simroot		-> Vixen simulation tree parent node (used as the parent
 *						for any newly created Engines)
 *
 * The Vixen scene graph mirrors the FBX scene graph.
 * FBX rigid body and soft body animations are attached to the
 * nodes they animation. In Vixen, they are kept in a separate
 * Simulation tree.
 *
 * @return -> Vixen Model created from FBX node, NULL if no Model created
 *
 * Here are the supported FBX nodes and their corresponding Vixen objects.
 * @code
 *	FBX						Vixen
 *	FbxNull					Model
 *	FbxLight				Light, DirectLight, AmbientLight, SpotLight
 *	FbxCamera				Camera
 *	FbxMesh					Shape, TriMesh	(all meshes are triangulated)
 *	FbxFileTexture			Sampler, Texture
 *	FbxSurfaceMaterial		Appearance
 * @endcode
 *
 */
Model* FBXReader::ConvertNode(FbxNode* pNode, Model* root, Engine* simroot)
{
	Core::String		name(mFileBase + pNode->GetName());
    const int			lMaterialCount = pNode->GetMaterialCount();
	Model*				vModel = (Model*) pNode->GetUserDataPtr();
	Engine*				vEngine = NULL;
    const int			lChildCount = pNode->GetChildCount();
	FbxNodeAttribute*	lNodeAttribute = pNode->GetNodeAttribute();
	bool				isbone = false;

	/*
	 * If the user data pointer of the FBX node is not NULL,
	 * this FBX node already has a Vixen Model. This means it is instanced in FBX.
	 * We clone the hierarchy and instance all the meshes and appearances.
	 */
	if (vModel != NULL)
	{
		if (vModel->IsClass(Vixen::VX_Model))
		{
			vModel = (Model*) vModel->Clone();
			VX_TRACE(Debug, ("Cloning instanced mesh %s\n", name));
			root->Append(vModel);
		}
		/*
		 * If the FBX node has a Vixen Engine, just return it.
		 * Do not instance the engines.
		 */
		else
		{
			vEngine = (Engine*) vModel;
			VX_TRACE(Debug, ("Using instanced engine %s\n", name));
			vModel = NULL;
			if (vEngine->Parent() == NULL)
				simroot->Append(vEngine);
		}
		return vModel;
	}
	/*
	 * Determine what kind of FBX node it is and create the corresponding Vixen node.
	 */
	if (lNodeAttribute)
	{
		switch (lNodeAttribute->GetAttributeType())
		{
		/*
		 * FBX Camera nodes are converted to Vixen Camera objects.
		 * They are put in the scene graph but not attached to the Vixen Scene.
		 */
			case FbxNodeAttribute::eCamera:
			{
				ConvCamera fbxcam(pNode, name);
				vModel = fbxcam.MakeCamera(mScene);
				fbxcam.Convert();
				break;
			}
		/*
		 * FBX Light nodes are converted to Vixen Light objects.
		 * Vixen supports spot, direct, point and ambient lights by default.
		 * FBX area and volume lights could be implemented with custom shaders in
		 * Vixen but are not supported currently.
		 */
			case FbxNodeAttribute::eLight:
			{
				ConvLight fbxlight(pNode, name);
				vModel = fbxlight.MakeLight();
				fbxlight.Convert();
				break;
			}
		/*
		 * The FBX Null node is represented as a Vixen Model.
		 * It has a transformation matrix but no geometry.
		 */
			case FbxNodeAttribute::eNull:
			{
				ConvModel fbxnull(pNode, name);
				if (lChildCount > 0)
				{
					vModel = fbxnull.MakeModel();
					fbxnull.Convert();
				}
				break;
			}
		/*
		 * FBX Mesh nodes are converted to Vixen Shape objects.
		 * The Vixen scene graph has the same structure as the FBX scene graph.
		 * Mesh deformers (blend shape, skin, vertex cache) are put in the
		 * Vixen simulation tree.
		 */
			case FbxNodeAttribute::eMesh:
			{
				FbxMesh* pMesh = pNode->GetMesh();
				if (pMesh && DoGeometry)
				{
					ConvMesh fbxmesh(pNode);
					vModel = fbxmesh.MakeShapes(mFileBase);
					fbxmesh.Convert();
				}
				break;
			}
		/*
		 * An FBX Skeleton node is represented be either a Vixen Transformer (if
		 * it is a limb) or a Vixen Skeleton (if it is a Skeleton).
		 */
			case FbxNodeAttribute::eSkeleton:
			{
				FbxSkeleton* fbxskel = pNode->GetSkeleton();

				if (fbxskel && DoSkeleton)
				{
					ConvertSkeleton(pNode, root, simroot);
					return NULL;
				}
				break;
			}
		}	// end switch
	}
	/*
	 * If we don't know what it is, we represent it as a Vixen Model.
	 */
	else if (lChildCount > 0)
	{
		ConvModel fbxnull(pNode, name);
		vModel = fbxnull.MakeModel();
		fbxnull.Convert();
	}
/*
 * The Vixen scene graph mirrors the FBX scene graph.
 * FBX skeletons are kept in the scene graph and intermixed with geometry.
 * In Vixen, all skeletons and animations are kept in a separate
 * Simulation tree. Vixen Models are linked into the scene graph
 * while Engines are linked into the simulation tree.
 */
	if (vModel)							// goes in the scene graph?
	{
		if (root == NULL)				// make it the root
		{
			mVixScene->SetModels(vModel);
			vModel->SetName(mFileBase + TEXT("root"));
		}
		else if (vModel->Parent() == NULL)
		{
			root->Append(vModel);		// link it to the current root
			VX_TRACE(Debug, ("Linking %s -> %s\n", root->GetName(), vModel->GetName()));
		}
		root = vModel;					// make it the new root
	}
	if (vEngine)						// goes in the simulation tree?
	{
		if (vEngine->Parent() == NULL)	// not already linked in?
		{
			simroot->Append(vEngine);	// link into simulation tree
			VX_TRACE(Debug, ("Linking %s -> %s\n", simroot->GetName(), vEngine->GetName()));
		}
		if (isbone)						// if it is a bone, it becomes the new parent
			simroot = vEngine;			// bones will be in a hierarchy
	}
	/*
	 * Traverse the FBX child nodes and make their Vixen correspondents.
	 * If an FBX mesh is parented to a bone, in Vixen the Transformer
	 * representing the bone goes into the simulation tree and
	 * the mesh goes at the root. The bone will control it's world transform.
	 */
    for (int child = 0; child < lChildCount; ++child)
	{
        Model* mod = ConvertNode(pNode->GetChild(child), root, simroot);
		if (mod && isbone)
		{
			Transformer* bone = (Transformer*) vEngine;
			mod->Remove(Group::UNLINK_NOFREE);
			mVixScene->GetModels()->Append(mod);	// put under the root
			bone->SetTarget(mod);					// make it target of the bone
			bone->SetOptions(Transformer::WORLD);
			VX_TRACE(Debug, ("Target %s -> %s\n", bone->GetName(), mod->GetName()));
		}
	}
	return vModel;
}

/*
 * Constructs Vixen simulation tree from FBX animations.
 * @param pNode		FBX node whose animations should be converted
 * @param simroot	Vixen simulation tree parent
 *
 * The Vixen scene graph mirrors the FBX scene graph.
 * FBX rigid body and soft body animations are attached to the
 * nodes they animate. In Vixen, they are kept in a separate
 * Simulation tree. Vixen Models are linked into the scene graph
 * while Engines are linked into the simulation tree.
 * If there are animation curves on the position or rotation
 * of the FBX node, they are converted to Vixen Interpolators.
 * Skinning or other mesh deformations become Vixen Deformers.
 *
 * This function constructs the simulation tree after the scene graph
 * has been made. Any FBX node with a Vixen counterpart has a non-null user
 * data pointer. These nodes are examined for animation controllers and
 * deformations.
 * Here are the supported FBX nodes and their corresponding Vixen objects.
 * @code
 *	FBX						Vixen
 *	FbxSkeleton				Skeleton, Transformer
 *	FbxBlendShapeChannel	Morph
 *	FbxSkin					Skin
 *	FbxVertexCacheDeformer	MeshAnimator
 */
Transformer* FBXReader::ConvertAnim(FbxNode* pNode, Engine* simroot)
{
	Core::String		name(mFileBase + pNode->GetName());
	SharedObj*			vixobj = (SharedObj*) pNode->GetUserDataPtr();
	FbxNodeAttribute*	lNodeAttribute = pNode->GetNodeAttribute();
	Transformer*		xform = NULL;
	Engine*				newroot = simroot;

	if (vixobj)
	{
		ConvMatrixAnim		fbxanim(pNode, name + TEXT(".xform"));
		FbxNodeAttribute*	attr = pNode->GetNodeAttribute();

		/*
		 * Is a skeleton? check for animations on root bone
		 * (which is not attached to an FBX node thru user pointer)
		 */
		if (vixobj->IsClass(VX_Skeleton))
		{
			xform = ((Skeleton*) vixobj)->GetBone(0);
			fbxanim.MakeMatrixAnim(NULL, xform, true);
			if (fbxanim.Convert())
			{
				newroot = xform;
				simroot = (Engine*) vixobj;
			}
		}
		/*
		 * Is a bone? check for transform controllers.
		 */
		if (vixobj && vixobj->IsClass(VX_Transformer))
		{
			xform = (Transformer*) vixobj;
			fbxanim.MakeMatrixAnim(NULL, xform, false);
			if (fbxanim.Convert())
				newroot = xform;
		}
		/*
		 * Check for animated transform controllers and convert them
		 * to Vixen Transformer with an Interpolator for each curve
		 */
		else if (vixobj->IsClass(VX_Model))
		{
			FbxNode*	parent = pNode->GetParent();
			Model*		mod = (Model*) vixobj;

			xform = fbxanim.MakeMatrixAnim(simroot, NULL, true);
			if (xform)
			{
				newroot = xform;
				if (fbxanim.Convert())
				{
					VX_TRACE(Debug, ("Target %s -> %s\n", xform->GetName(), vixobj->GetName()));
					xform->SetTarget(vixobj);
				}
			}
			/*
			 * Check for mesh deformers - add these after animations.
			 */
			if (vixobj->IsClass(VX_Shape) && attr && (attr->GetAttributeType() == FbxNodeAttribute::eMesh))
			{
				FbxMesh* pMesh = pNode->GetMesh();
				Mesh*	vMesh = (Mesh*) ((Shape*) vixobj)->GetGeometry();

				if (vMesh && pMesh && DoDeformers)
				{
					Engine*		deformers = ConvDeformer::MakeDeformers(name, pMesh, vMesh->GetVertices(), NULL);
					if (deformers)
						simroot->Append(deformers);
				}
			}
		}
	}
	/*
	 * Traverse the FBX child nodes and convert their animations.
	 */
    for (int i = 0; i < pNode->GetChildCount(); ++i)
		ConvertAnim(pNode->GetChild(i), newroot);
	return xform;
}

/*
 * Convert an FBX Skeleton hierarchy to the corresponding Vixen Skeleton and Transformer hierarchy.
 * @param pNode		-> FBX node
 * @param root		-> Vixen scene graph parent for new nodes
 *
 * The Vixen Skeleton mirrors the FBX Skeleton. The bones have the same ordering and names.
 * The FBX Skeleton root becomes a Vixen Skeleton engine and each FBX Skeleton limb
 * becomes a Vixen Transformer (bone).
 *
 * If the incoming FBX node is the root bone, this function will recursively
 * process the entire skeleton and returns NULL. If the node is a limb,
 * a pointer to a Vixen Transformer that represents the bone is returned
 * (no recursion is done).
 *
 * @return Vixen Transformer corresponding to FBX bone or NULL if entire skeleton was processed
 */
Engine* FBXReader::ConvertSkeleton(FbxNode* pNode, Model* root, Engine* simroot)
{
	Engine*			bone;
	Engine*			skel;
	FbxNode*		fbxroot;
	FbxSkeleton*	skelnode = pNode->GetSkeleton();
	Core::String	name(mFileBase + pNode->GetName());
	ConvBone		fbxbone(pNode, name + TEXT(".xform"));

	/*
	 * Make a Vixen bone for this FBX node if one has not been made.
	 * Find the root bone if this is not it and start the
	 * Vixen skeleton there
	 */
	bone = fbxbone.MakeBone();			// make the bone for this node
	fbxbone.Convert();					// attach to FBX node thru data ptr
	VX_ASSERT(bone);
	fbxroot = FindRootBone(pNode);		// check if parent is the root
	if (fbxroot != pNode)				// this node not really the root?
		return ConvertSkeleton(fbxroot, root, simroot);
	/*
	 * This is really the root. Make a Vixen skeleton and associate it with the
	 * FBX node. The root bone will be named after the skeleton and it's
	 * Vixen pointer will NOT be referenced by an FBX node.
	 */
	ConvSkeleton	fbxskel(fbxroot, name + TEXT(".skeleton"));
	pNode->SetUserDataPtr(NULL);		// put Vixen Skeleton in user data ptr
	skel = fbxskel.MakeSkeleton(mVixScene->GetEngines());
	if (skel)							// skeleton has bones
	{
		ConvertBone(pNode, root, skel);	// convert the bones
		fbxskel.Convert();				// calculate the bind pose
	}
	return skel;
}

Transformer* FBXReader::ConvertBone(FbxNode* pNode, Model* root, Engine* simroot)
{
	Core::String	name(mFileBase + pNode->GetName());
	ConvBone		fbxbone(pNode, name + TEXT(".xform"));
	Transformer*	bone = fbxbone.MakeBone(simroot);

	fbxbone.Convert();
	for (int i = 0; i < pNode->GetChildCount(); ++i)
	{
		FbxNode*			child = pNode->GetChild(i);
		FbxNodeAttribute*	attr = child->GetNodeAttribute();
		Model*				mod;
		SharedObj*			vixobj = (SharedObj*) child->GetUserDataPtr();

		name = mFileBase + child->GetName();
		name += TEXT(".xform");
		if (attr) switch (attr->GetAttributeType())
		{
			case FbxNodeAttribute::eNull:		// possible bone
			if (!vixobj || !vixobj->IsClass(VX_Transformer))
				break;

			case FbxNodeAttribute::eSkeleton:	// real FBX bone?
			ConvertBone(child, root, bone);
			break;

			case FbxNodeAttribute::eMesh:		// reparent to world
			mod = ConvertNode(child, mVixScene->GetModels(), bone);
			if (mod)
			{
				bone->SetTarget(mod);			// make it target of the bone
				((Transformer*) bone)->SetOptions(Transformer::WORLD);
				VX_TRACE(Debug, ("Target %s -> %s\n", bone->GetName(), mod->GetName()));
			}
			break;
		}
	}
	return bone;
}

FbxNode* FBXReader::FindRootBone(FbxNode* pNode)
{
	FbxNode*	root = pNode;
	FbxNode*	newskel = pNode;

	if (root = pNode->GetParent())
	{
		FbxNodeAttribute*	nodeattr = root->GetNodeAttribute();
		int					numbones = 0;
		int					numshapes = 0;

		if (nodeattr && nodeattr->GetAttributeType() == FbxNodeAttribute::eNull)
		{
			for (int i = 0; i < root->GetChildCount(); ++i)
			{
				FbxNode* child = root->GetChild(i);
				SharedObj* vixobj = (SharedObj*) child->GetUserDataPtr();
				
				if (child->GetSkeleton() != NULL)
					++numbones;
				else if (vixobj && vixobj->IsClass(VX_Transformer))
					++numbones;
			}
		}
		if (numbones < 2)
			return pNode;
		newskel = root;
		VX_TRACE(Debug, ("Using %s as root bone instead of %s\n", newskel->GetName(), pNode->GetName()));
	}
	SharedObj* vixobj = (SharedObj*) newskel->GetUserDataPtr();
	if (vixobj)
		if (vixobj->IsClass(VX_Model))
		{
			VX_TRACE(Debug, ("Removing %s from Vixen scene graph\n", vixobj->GetName()));
			((Model*) vixobj)->Remove(Group::UNLINK_FREE);
			vixobj->Delete();
			newskel->SetUserDataPtr(NULL);
		}
		else
		{ VX_ASSERT(vixobj->IsClass(VX_Skeleton) || vixobj->IsClass(VX_Transformer)); }
	return newskel;
}

/*
 * Connects the current camera to the scene.
 * If the FBX current camera was in the hierarchy, it already has a Vixen Camera
 * in the scene graph. If not, we make a Vixen camera from the FBX camera
 * and attach it to the Vixen scene.
 */
void FBXReader::ConnectCamera()
{
	FbxNode* fbxcam = ConvCamera::GetCurrentCamera(mScene);
	Vixen::Camera* vixcam = (Vixen::Camera*) fbxcam->GetUserDataPtr();
	if (vixcam == NULL)
	{
		ConvCamera ccam(fbxcam);
		ccam.MakeCamera(mScene);
		vixcam = (Vixen::Camera*) ccam.Convert();
	}
	if (vixcam)
	{
		VX_TRACE(Debug, ("Linking Camera %s to scene\n", vixcam->GetName()));
		mVixScene->SetCamera(vixcam);
	}
}

/*
 * Loads an FBX file and convert it into a Vixen Scene.
 *
 * @param filename	name of FBX file to load.
 * @param instream	Vixen Stream to use for loading (ignored).
 * @param ev		Vixen Event describing file load
 *
 * The input filename may be absolute or relative. If relative,
 * the media directory (World::GetMediaDir) is used as the base directory.
 * The Vixen input stream is ignored - the FBX SDK reads directly from
 * disk files only.
 *
 * Upon return, the Vixen event will reference the Vixen Scene loaded.
 *
 * @return true if successful load, else false
 */
bool ReadFBXFile(const TCHAR* filename, Core::Stream* instream, LoadEvent* ev)
{
	FBXReader	reader(filename);
	Scene*		scene;

	reader.Debug = FileLoader::Debug;
	scene = reader.LoadFile();
	if (scene)
	{
		ev->Code = Event::LOAD_SCENE;
		if (ev->Sender.IsNull())
			ev->Sender = GetMainScene();
		((Vixen::LoadSceneEvent*) ev)->Object = scene;
		VX_TRACE(FileLoader::Debug, ("FBXReader::ReadScene %s\n", filename));
		return true;
	}
	VX_TRACE(FileLoader::Debug, ("FBXReader::ReadScene ERROR cannot load %s\n", filename));
	return false;
}

void FBXReader::InitSDK(FbxManager*& pManager, FbxScene*& pScene)
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if (!pManager)
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	SetupIO(pManager);
	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "Vixen Scene");
	if( !pScene )
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}

void FBXReader::SetupIO(FbxManager* pManager)
{
	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	ios->SetBoolProp(IMP_REMOVEBADPOLYSFROMMESH, false);
	ios->SetBoolProp(IMP_FORCEWEIGHTNORMALIZE, true);
	pManager->SetIOSettings(ios);
}

void FBXReader::DestroySDK(FbxManager* pManager, bool pExitStatus)
{
    //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
    if( pManager ) pManager->Destroy();
	if( pExitStatus ) FBXSDK_printf("Program Success!\n");
}

} }	// end FBX