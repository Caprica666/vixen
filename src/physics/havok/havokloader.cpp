#include "vixen.h"
#ifdef _DEBUG
#include <fstream>
#endif


#include "physics/havok/vxphysics.h"
#include "Physics/havok/vxcloth.h"

#include <Common/Base/KeyCode.h>
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Math/QsTransform/hkQsTransform.h>
#include <Common/Base/System/Io/IStream/hkIstream.h>
#include <Common/Base/Memory/System/hkMemorySystem.h>
#include <Common/Serialize/Util/hkLoader.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#include <Common/Serialize/Util/hkSerializeDeprecated.h>
#include <Common/SceneData/Scene/hkxScene.h>
#include <Common/SceneData/Skin/hkxSkinBinding.h>
#include <Animation/Animation/hkaAnimationContainer.h>
#include <Animation/Animation/Deform/Skinning/hkaMeshBinding.h>
#include <Animation/Animation/Rig/hkaSkeleton.h>
#include <Animation/Animation/hkaAnimation.h>
#include <Animation/Animation/Rig/hkaPose.h>
#include <Animation/Animation/Mapper/hkaSkeletonMapper.h>
#include <Animation/Animation/Mapper/hkaSkeletonMapperData.h>
#include <Animation/Animation/Motion/Default/hkaDefaultAnimatedReferenceFrame.h>
#include <Cloth/Setup/Container/hclClothSetupContainer.h>

#if 1
#include <Physics2012/Utilities/Serialize/hkpPhysicsData.h>
#include <Physics2012/Dynamics/hkpDynamics.h>
#include <Physics2012/Dynamics/World/hkpWorld.h>
#include <Physics2012/Dynamics/Entity/hkpRigidBody.h>
#include <Physics2012/Collide/Dispatch/hkpAgentRegisterUtil.h>
#include <Animation/Physics2012Bridge/Instance/hkaRagdollInstance.h>
#else
#include <Physics/Utilities/Serialize/hkpPhysicsData.h>
#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>
#include <Animation/Ragdoll/Instance/hkaRagdollInstance.h>
#endif

#include "Physics/havok/math_interop.h"

namespace Vixen {

template <> inline uint32 Core::Dict<intptr, ObjRef, Vixen::BaseDict>::HashKey(const intptr& k) const
{
	uint32 tmp = (uint32) k;
	return HashInt(tmp);
}

template <> inline bool Core::Dict<intptr, ObjRef, Vixen::BaseDict>::CompareKeys(const intptr& i, const intptr& j)
{
	return (i == j);
}

class HavokLoader
{
public:
	HavokLoader(const TCHAR* filebase, const TCHAR* directory);
	void		Quit();
	Scene*		Load(const TCHAR* filename);
	NameTable*	GetSceneDict()	{ return m_Messenger.GetNameDict(); }

	FileMessenger			m_Messenger;
	Core::Stream*			m_InputStream;
	bool					m_HasCloth;
	bool					m_HasAnimation;
	int						m_VecSize;
	hkMemoryRouter			m_MemoryRouter;
	hkRootLevelContainer*	m_Root;
	hkRefPtr<hkLoader>		m_Loader;
	hkRefPtr<hkpWorld>		m_World;
	Scene*					m_Scene;
	Core::String			m_FileBase;
	Core::String			m_Directory;
	Core::String			m_NamePrefix;
	Core::Dict<intptr, ObjRef, Vixen::BaseDict> m_ParsedObjs;

protected:
	Physics*	MakeWorld(hkpPhysicsData* physdata);
	Scene*		ParseScene(hkxScene* hkscene, Engine* simroot = NULL);
	Model*		ParseMesh(const hkxMesh* hkmesh, const char* name);
	Mesh*		ParseVertexBuffer(const hkxVertexBuffer* hkvbuf);
	Skin*		ParseSkin(const hkxVertexBuffer* hkvbuf, int numbones);
	bool		ParseIndexBuffer(const hkxIndexBuffer* hkibuf, IndexArray* indices);
	Appearance*	ParseMaterial(const hkxMaterial* hkmtl);
	Model*		ParseNode(const hkxNode* node, Model* root, Engine* simroot);
	Light*		ParseLight(const hkxLight* node);
	Transformer* ParseRigidBody(hkpRigidBody* node);
	Camera*		ParseCamera(const hkxCamera* node);
	Shape*		ParseMeshSection(const hkxMeshSection* hkmesh);
	RagDoll*	ParseSkeleton(const hkaSkeleton* hkskel, RagDoll* skeleton = NULL);
	RagDoll*	ParseRagdoll(hkaRagdollInstance* hkdoll, Model* root);
	void		AttachBones(Skeleton* skelptr, Engine* simroot);
	Engine*		ParseMeshBind(const hkaMeshBinding* node, Model* root, RagDoll* skel);
	ClothSim*	ParseCloth(hclClothSetupContainer* setup, Scene* vixenscene, hkxScene* havokscene);
	void		MakeBones(const hkaSkeleton* hkskel, Skeleton* skeleton);
	Model*		MakeSkeletonShapes(RagDoll* skeleton);
	void		SampleRotation(Transformer* bone, const hkaAnimation* anim, int trackid);
	void		SampleTranslation(Transformer* bone, const hkaAnimation* anim, int trackid);
	void		SetName(const TCHAR* name, SharedObj* obj);
};

/*!
 * @fn bool Physics::ReadScene(const TCHAR* filename, Core::Stream* instream, LoadEvent* event)
 * @param filename	name of scene file to load.
 * @param instream	stream to use for reading.
 * @param event		event to initialize if load is successful.
 *
 * Called by the loader to load a scene file.
 * ReadScene will issue a blocking read that waits until all
 * of the geometry and associated textures have been loaded.
 * If you want to load an entire scene from a file and
 * replace the current scene, the SceneLoader::Load function will do
 * this without blocking.
 *
 * @return \b true if load was successful, else \b false
 *
 * @see World::LoadAsync FileLoader::Load
 */
bool Physics::ReadScene(const TCHAR* filename, Core::Stream* instream, LoadEvent* e)
{
	LoadSceneEvent*	ev = (LoadSceneEvent*) e;
	Scene*			inscene = (Scene*) (SharedObj*) ev->Object;
	const Model*	sceneroot;
	const Engine*	simroot;
	bool			hasanimation = false;
	const TCHAR*	havokfile = NULL;
	TCHAR			filebase[VX_MaxPath];
	TCHAR			dirbuf[VX_MaxPath];

	ev->Code = Event::LOAD_SCENE;
	if (inscene)							// scene already loaded?
		return true;
	Core::Stream::ParseDirectory(filename, filebase, dirbuf);
	instream->Close();						// allow Havok access to the file
	if (instream->IsKindOf(CLASS_(Core::NetStream)))
	{
		havokfile = ((Core::NetStream*) instream)->GetCacheFile();
		if (havokfile == NULL)
			havokfile = filename;
	}
	{
		HavokLoader		loader(filebase, dirbuf);
		NameTable*		scenedict;

		inscene = loader.Load(havokfile);
		hasanimation = loader.m_HasAnimation;
		loader.Quit();
		scenedict = loader.GetSceneDict();
		World3D::Get()->GetLoader()->SetSceneDict(filebase, scenedict);
	}
	if (inscene == NULL)
		VX_ERROR(("Physics::Load ERROR cannot parse physics file %s", (const char*) havokfile), false);
	VX_TRACE(FileLoader::Debug, ("Physics::ReadScene %s", (const char *) filename));
/*
 * Define simulation tree and scene tree root names in the global dictionary
 */
	Messenger* disp = GetMessenger();
	const TCHAR* name = inscene->GetName();
	disp->Define(name, inscene);
	sceneroot = inscene->GetModels();
	if (sceneroot)
	{
		if (sceneroot->IsParent() && !hasanimation)
		{
			name = sceneroot->GetName();
			if (name)
				disp->Define(name, sceneroot);
		}
		else
		{
			sceneroot = NULL;
			inscene->SetModels(NULL);
		}
	}
	simroot = inscene->GetEngines();
	if (simroot)
	{
		name = simroot->GetName();
		if (name)
			disp->Define(name, simroot);
	}
	ev->Object = inscene;
	return true;
}

HavokLoader::HavokLoader(const TCHAR* filebase, const TCHAR* basedir)
{
	hkMemorySystem::getInstance().threadInit( m_MemoryRouter, "hkLoadThread" );
	hkBaseSystem::initThread(&m_MemoryRouter );
	m_Loader = new hkLoader();
	hkSerializeDeprecated::initDeprecated();
	m_Directory = basedir;
	m_FileBase = filebase;
	m_HasCloth = false;
	m_HasAnimation = false;
	m_VecSize = GetMessenger()->SysVecSize;
}

void HavokLoader::Quit()
{
	m_Loader = NULL;
	hkBaseSystem::quitThread();
	hkMemorySystem::getInstance().threadQuit(m_MemoryRouter);
}

void HavokLoader::SetName(const TCHAR* name, SharedObj* obj)
{
	m_Messenger.Define(name, obj);
}

Scene* HavokLoader::Load(const TCHAR* filename)
{
	Ref<Physics>	physics;
	Scene*			scene;
	hkpPhysicsData*	physicsData;
	hkxScene*		hkscene;
	hclClothSetupContainer* clothSetup;

	m_NamePrefix = m_FileBase + TEXT('.');
	m_Root = m_Loader->load(filename);
	if (m_Root == HK_NULL)
		VX_ERROR(("Physics::Load cannot load physics file %s", filename), NULL);
	physicsData = reinterpret_cast<hkpPhysicsData*>(m_Root->findObjectByType(hkpPhysicsDataClass.getName()));
	hkscene = reinterpret_cast<hkxScene*>(m_Root->findObjectByType(hkxSceneClass.getName()));
	clothSetup = reinterpret_cast<hclClothSetupContainer*> (m_Root->findObjectByType( hclClothSetupContainerClass.getName()));

	m_HasCloth = (clothSetup != HK_NULL);
	if (hkscene == HK_NULL)
		VX_ERROR(("Physics::Load cannot find scene in physics file"), NULL);
	physics = new Physics(physicsData);
	physics->SetActive(false);
	SetName(m_NamePrefix + TEXT("simroot"), physics);
	scene = ParseScene(hkscene, physics);
	if (physicsData)
		for (int i = 0; i < physicsData->getPhysicsSystems().getSize(); ++i)
		{
			hkpPhysicsSystem* physys = physicsData->getPhysicsSystems()[i];

			for (int b = 0; b < physys->getRigidBodies().getSize(); ++b)
			{
				Transformer* body = ParseRigidBody(physys->getRigidBodies()[b]);
				if (body && !body->IsChild())
				{
					Core::String name(body->GetName());

					if (name.Right(6) == TEXT(".xform"))
						name = name.Left(-6);
					physics->Append(body);
					Model*	shape = (Model*) scene->GetModels()->Find(name, Group::FIND_DESCEND | Group::FIND_EXACT);
					if (shape)
					{
						VX_TRACE(Physics::Debug, ("ParseRigidBody:  %s -> %s", body->GetName(), shape->GetName()));
						body->SetTarget(shape);
					}
				}
			}
		}
	if (m_HasCloth)
	{
		ClothSim* cloth = ParseCloth(clothSetup, scene, hkscene);
		if (cloth)
			physics->Append(cloth);
	}
	// Output scene as XML
	//scene->Print(vixen_debug, SharedObj::PRINT_Default);
	return scene;
}

Physics* HavokLoader::MakeWorld(hkpPhysicsData* physdata)
{
	hkpWorldCinfo	worldInfo;
	hkpWorldCinfo*	temp =  physdata->getWorldCinfo();
	Physics*		physics;

	worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM);
	worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_CONTINUOUS;
	worldInfo.m_gravity.set(0.0f, -9.8f, 0.0f);
	worldInfo.setBroadPhaseWorldSize(1000.0f);
	worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM);
	physdata->setWorldCinfo(&worldInfo);
	m_World = physdata->createWorld(true);
	physics = new Physics(m_World);
	physics->SetActive(false);
	SetName(m_NamePrefix + TEXT("simroot"), physics);
	return physics;
}

Scene* HavokLoader::ParseScene(hkxScene* hkscene, Engine* simroot)
{
	Scene*		scene = new Scene();
	hkxNode*	hkroot = hkscene->m_rootNode;
	Model*		model = new Model();

	/*
	 * Parse all the scene nodes.
	 */
	m_Scene = scene;
	SetName(m_NamePrefix + TEXT("scene"), scene);
	SetName(m_NamePrefix + TEXT("camera"), scene->GetCamera());
	if (simroot == NULL)
	{
		simroot = new Engine();
		SetName(m_NamePrefix + TEXT("simroot"), simroot);
	}
	scene->SetEngines(simroot);
	if (hkroot)
	{
		model = ParseNode(hkroot, NULL, simroot);
		SetName(m_NamePrefix + TEXT("root"), model);
		scene->SetModels(model);
	}
	/*
	 * Then parse all the Ragdoll skeletons
	 */
	hkaAnimationContainer*	anims = reinterpret_cast<hkaAnimationContainer*>( m_Root->findObjectByType( hkaAnimationContainerClass.getName() ));
	hkaRagdollInstance*		ragdoll = NULL;
	hkaSkeletonMapper*		mapper = NULL;
	RagDoll*				skeleton = NULL;


	/*
	 * Parse the ragdolls and rigid bodies
	 */
	while (ragdoll = reinterpret_cast<hkaRagdollInstance*>( m_Root->findObjectByType( hkaRagdollInstanceClass.getName(), ragdoll )))
	{
		skeleton = ParseRagdoll(ragdoll, model);
		simroot->Append(skeleton);
	}
	if (anims != HK_NULL)
	{
		/*
		 * Parse all the skeletons
		 */
		for (int s = 0; s < anims->m_skeletons.getSize(); ++s)
		{
			skeleton = ParseSkeleton(anims->m_skeletons[s]);
			AttachBones(skeleton, simroot);
		}

		/*
		 * Parse all the skins and bind them to skeletons
		 */
		for (int a = 0; a < anims->m_skins.getSize(); ++a)
		{
			const hkaMeshBinding*	mb = anims->m_skins[a];
			skeleton = ParseSkeleton(mb->m_skeleton);

			Engine* skinroot = ParseMeshBind(mb, model, skeleton);
			if (skinroot)
				simroot->Append(skinroot);
		}
		/*
		 * Parse the animations and sample the tracks for each joint
		 */
		for (int b = 0; b < anims->m_bindings.getSize(); ++b)
		{
			const hkaAnimationBinding* ab = anims->m_bindings[b];
			const hkaAnimation* an = ab->m_animation;
			const hkaSkeleton* hkskel;
			ObjRef* ref;
			Skeleton* skel;

			hkskel = anims->findSkeletonByName(ab->m_originalSkeletonName);
			if (hkskel == NULL)
				continue;
			ref = m_ParsedObjs.Find((intptr) hkskel);
			if (ref == NULL)
				continue;
			skel = (Skeleton*) (SharedObj*) *ref;
			if (skel == NULL)
				continue;

			Engine::Iter iter(skel, Group::DEPTH_FIRST);
			Transformer* bone;

			while (bone = (Transformer*) iter.Next())
				if (bone->IsClass(VX_Transformer))
				{
					int boneid = bone->GetBoneIndex();
					int trackid;
					
					if (boneid < 0)
						continue;
					trackid = ab->findTrackIndexFromBoneIndex(boneid);
					if (boneid == 0)
						SampleTranslation(bone, an, trackid);
					SampleRotation(bone, an, trackid);
				}
		}
		/*
		 * Find the skeleton mappers and bind them to the skeletons
		 */
		while (mapper = reinterpret_cast<hkaSkeletonMapper*>( m_Root->findObjectByType( hkaSkeletonMapperClass.getName(), mapper)))
		{
			const hkaSkeletonMapperData& mapdata = mapper->m_mapping;
			const hkaSkeleton* hkskelA = mapdata.m_skeletonA;
			const hkaSkeleton* hkskelB = mapdata.m_skeletonB;
			SkeletonMapper* skelmap = new SkeletonMapper(mapper);
			ObjRef*			ref = m_ParsedObjs.Find((intptr) hkskelA);
			RagDoll*		skelA;
			RagDoll*		skelB;
			Core::String	name(m_NamePrefix);

			if (ref && (skelA = (RagDoll*) (SharedObj*) *ref))
			{
				name += hkskelA->m_name;
				skelA->Append(skelmap);
				skelmap->SetSource(skelA);
			}
			else
			{
				VX_WARNING(("Physics::Load source skeleton %s referenced by mapper not found", hkskelA->m_name));
				name += TEXT("skelA");
				simroot->Append(skelmap);
			}
			ref = m_ParsedObjs.Find((intptr) hkskelB);
			if (ref && (skelB = (RagDoll*) (SharedObj*) *ref))
			{
				skelmap->SetTarget(skelB);
				name += TEXT('-');
				name += hkskelB->m_name;
			}
			else
			{
				VX_WARNING(("Physics::Load target skeleton %s referenced by mapper not found", hkskelB->m_name));
				name += TEXT("skelB");
			}
			name += TEXT(".mapper");
			SetName(name, skelmap);
			skelmap->SetActive(false);
		}
	}
#if 0
	if (Physics::Debug > 1)
		scene->Print(vixen_debug, SharedObj::PRINT_Default);
#endif
	return scene;
}

Model* HavokLoader::ParseMesh(const hkxMesh* hkmesh, const char* name)
{
	int		nsections = hkmesh->m_sections.getSize();
	ObjRef*	ref = m_ParsedObjs.Find((intptr) hkmesh);

	if (ref)
	{
		Model* tmp = (Model*) (SharedObj*) *ref;
		VX_ASSERT(tmp && tmp->IsClass(VX_Model));
		return tmp;
	}
	VX_ASSERT(nsections > 0);
	if (nsections == 1)
	{
		VX_TRACE(Physics::Debug, ("ParseMesh: %s", name));
		Shape* shape = ParseMeshSection(hkmesh->m_sections[0]);
		SetName(m_NamePrefix + name, shape);
		SetName(name, shape->GetGeometry());
		m_ParsedObjs.Set((intptr) hkmesh, shape);
		return shape;
	}

	Model* temp = new Model();
	SetName(m_NamePrefix + name, temp);
	VX_TRACE(Physics::Debug, ("ParseMesh: %s %d sections", name, nsections));
	for (int i = 0; i < nsections; ++i)
	{
		Shape*	shape = ParseMeshSection(hkmesh->m_sections[i]);

		if (shape)
		{
			Core::String s(name);

			s += TEXT('-') + Core::String(i);
			SetName(m_NamePrefix + s, shape);
			SetName(s, shape->GetGeometry());
			temp->Append(shape);
		}
	}
	m_ParsedObjs.Set((intptr) hkmesh, temp);
	return temp;
}

Shape* HavokLoader::ParseMeshSection(const hkxMeshSection* hkmesh)
{
	ObjRef*	ref = m_ParsedObjs.Find((intptr) hkmesh);
	Shape*	shape;
	Mesh*	geomesh;
	IndexArray* inds;

	if (ref)							// already parsed this mesh section?
	{
		shape = (Shape*) (SharedObj*) *ref;
		VX_ASSERT(shape && shape->IsClass(VX_Shape));
		return shape;
	}
	geomesh = ParseVertexBuffer(hkmesh->m_vertexBuffer);
	if (geomesh == NULL)
		return NULL;
	inds = new IndexArray();
	geomesh->SetIndices(inds);
	for (int j = 0; j < hkmesh->m_indexBuffers.getSize(); ++j)
	{
		hkxIndexBuffer* hkibuf = hkmesh->m_indexBuffers[j];
		ParseIndexBuffer(hkibuf, inds);
	}
	shape = new Shape();
	shape->SetGeometry(geomesh);
	if (hkmesh->m_material != HK_NULL)
		shape->SetAppearance(ParseMaterial(hkmesh->m_material));
	m_ParsedObjs.Set((intptr) hkmesh, shape);
	return shape;
}

bool HavokLoader::ParseIndexBuffer(const hkxIndexBuffer* hkibuf, IndexArray* indices)
{
	int			n = hkibuf->getNumTriangles();
	indices->SetMaxSize(indices->GetMaxSize() + n * 3);
	if (hkibuf->m_indexType != hkxIndexBuffer::INDEX_TYPE_TRI_LIST)
		VX_ERROR(("Physics::Load ERROR bad index buffer type"), false);
	if (hkibuf->m_indices32.getSize() > 0)
		for (int i = 0; i < n; ++i)
		{
			indices->Append(hkibuf->m_indices32[i * 3]);
			indices->Append(hkibuf->m_indices32[i * 3 + 1]);
			indices->Append(hkibuf->m_indices32[i * 3 + 2]);
		}
	else if (hkibuf->m_indices16.getSize() > 0)
		for (int i = 0; i < n; ++i)
		{
			indices->Append(hkibuf->m_indices16[i * 3]);
			indices->Append(hkibuf->m_indices16[i * 3 + 1]);
			indices->Append(hkibuf->m_indices16[i * 3 + 2]);
		}	return true;
}

Mesh* HavokLoader::ParseVertexBuffer(const hkxVertexBuffer* hkvbuf)
{
	const hkxVertexDescription& hkvdesc = hkvbuf->getVertexDesc();
	const hkxVertexDescription::ElementDecl*	edecl;
	int				n = hkvbuf->getNumVertices();
	int				i = 0;
	TriMesh*		trimesh;
	Core::String	vertexdesc;

	/*
	 * Figure out the vertex buffer layout
	 */
	while (edecl = hkvdesc.getElementDeclByIndex(i))
	{
		const char*		type;
		const char*		name;
		int				tmp = 0;
		int				numelems = edecl->m_numElements;

		if (!vertexdesc.IsEmpty())
			vertexdesc += TEXT(", ");
		switch (edecl->m_type)
		{
			case hkxVertexDescription::HKX_DT_FLOAT: type = TEXT("float"); break;
			case hkxVertexDescription::HKX_DT_UINT32:
			case hkxVertexDescription::HKX_DT_UINT8: type = TEXT("int"); break;
			default: VX_ERROR(("Physics::Load ERROR bad vertex descriptor type"), NULL);
		}
		++i;
		switch (edecl->m_usage)
		{
			case hkxVertexDescription::HKX_DU_POSITION:
			name = TEXT("position");
			numelems = m_VecSize;
			break;

			case hkxVertexDescription::HKX_DU_COLOR:
			name = TEXT("diffuse");
			break;

			case hkxVertexDescription::HKX_DU_NORMAL:
			name = TEXT("normal");
			numelems = m_VecSize;
			break;

			case hkxVertexDescription::HKX_DU_TEXCOORD:
			name = TEXT("texcoord");
			break;

			case hkxVertexDescription::HKX_DU_TANGENT:
			name = TEXT("tangent");
			numelems = m_VecSize;
			break;

			case hkxVertexDescription::HKX_DU_BINORMAL:
			name = TEXT("bitangent");
			numelems = m_VecSize;
			break;

			default:
			continue;
		}
		vertexdesc += type;
		if (numelems > 1)
			vertexdesc += Core::String(numelems);
		vertexdesc += TEXT(" ");
		vertexdesc += name;
	}
	trimesh = new TriMesh(vertexdesc, n);
	/*
	 * Copy the vertices from Havok into Vixen
	 */
	VertexArray*		verts = trimesh->GetVertices();
	VertexArray::Iter	viter(verts);
	const DataLayout*	layout = verts->GetLayout();
	i = 0;
	while (edecl = hkvdesc.getElementDeclByIndex(i))
	{
		int ofs = layout->Slot[i].Offset;
		int nfloats = layout->Slot[i].Size;
		int stride = edecl->m_byteStride;
		const char* pbase = static_cast<const char*>( hkvbuf->getVertexDataPtr(*edecl) );
		++i;
		switch (edecl->m_usage)
		{
			case hkxVertexDescription::HKX_DU_COLOR:
			case hkxVertexDescription::HKX_DU_TEXCOORD:
			for (int v = 0; v < n; ++v)
			{
				float* vptr = viter.GetVtx(v) + ofs;
				const float* pos = reinterpret_cast<const float*>(pbase + stride * v);
				for (int j = 0; j < nfloats; ++j)
					*vptr++ = *pos++;
			}
			break;

			case hkxVertexDescription::HKX_DU_POSITION:
			for (int v = 0; v < n; ++v)
			{
				float* vptr = viter.GetVtx(v) + ofs;
				const float* pos = reinterpret_cast<const float*>(pbase + stride * v);
				*vptr++ = *pos++;
				*vptr++ = *pos++;
				*vptr++ = *pos++;
				if (m_VecSize == 4)
					*vptr++ = 1.0f;

			}
			break;

			case hkxVertexDescription::HKX_DU_NORMAL:
			case hkxVertexDescription::HKX_DU_TANGENT:
			case hkxVertexDescription::HKX_DU_BINORMAL:
			for (int v = 0; v < n; ++v)
			{
				float* vptr = viter.GetVtx(v) + ofs;
				const float* pos = reinterpret_cast<const float*>(pbase + stride * v);
				*vptr++ = *pos++;
				*vptr++ = *pos++;
				*vptr++ = *pos++;
				if (m_VecSize == 4)
					*vptr++ = 0.0f;
			}
			break;
		}
	}
	verts->SetNumVtx(n);
	return trimesh;
}

Appearance* HavokLoader::ParseMaterial(const hkxMaterial* hkmtl)
{
	ObjRef*		ref = m_ParsedObjs.Find((intptr) hkmtl);
	Appearance* appear;

	if (ref)
	{
		appear = (Appearance*) (SharedObj*) *ref;
		VX_ASSERT(appear && appear->IsClass(VX_Appearance));
		return appear;
	}

	PhongMaterial* mtl = new PhongMaterial();
	appear = new Appearance();
	SetName(hkmtl->m_name, appear);
	m_ParsedObjs.Set((intptr) hkmtl, appear);
	VX_TRACE(Physics::Debug, ("ParseMaterial: %s", hkmtl->m_name));

	Col4 diffuse(hkmtl->m_diffuseColor(0), hkmtl->m_diffuseColor(1), hkmtl->m_diffuseColor(2), hkmtl->m_diffuseColor(3));
	Col4 specular(hkmtl->m_specularColor(0), hkmtl->m_specularColor(1), hkmtl->m_specularColor(2), hkmtl->m_specularColor(3));
	Col4 ambient(hkmtl->m_ambientColor(0), hkmtl->m_ambientColor(1), hkmtl->m_ambientColor(2), hkmtl->m_ambientColor(3));
	Col4 emission(hkmtl->m_emissiveColor(0), hkmtl->m_emissiveColor(1), hkmtl->m_emissiveColor(2), hkmtl->m_emissiveColor(3));
	mtl->SetDiffuse(diffuse);
	mtl->SetSpecular(specular);
	mtl->SetAmbient(ambient);
	mtl->SetEmission(emission);
	appear->SetMaterial(mtl);
	if (diffuse.a != 1.0f)
		appear->Set(Appearance::TRANSPARENCY, 1);
	for (int i = 0; i < hkmtl->m_stages.getSize(); ++i)
	{
		const hkxMaterial::TextureStage& stage = hkmtl->m_stages[i];
		const hkxTextureFile*	hktex = reinterpret_cast<const hkxTextureFile*>(stage.m_texture.val());
		int			usage = Sampler::DIFFUSE;
		Sampler*	sampler = new Sampler();

		switch (stage.m_usageHint)
		{
			case hkxMaterial::TEX_DIFFUSE:
			sampler->SetName("DiffuseMap");
			break;

			case hkxMaterial::TEX_SPECULARANDGLOSS:
			case hkxMaterial::TEX_SPECULAR:
			usage = Sampler::SPECULAR;
			sampler->SetName("SpecularMap");
			break;

			case hkxMaterial::TEX_BUMP:
			case hkxMaterial::TEX_NORMAL:
			case hkxMaterial::TEX_DISPLACEMENT:
			usage = Sampler::BUMP;
			sampler->SetName("NormalMap");
			break;

			case hkxMaterial::TEX_EMISSIVE:
			usage = Sampler::EMISSION;
			sampler->SetName("EmissionMap");
			break;

			default: VX_WARNING(("Physics::Load WARNING: unuspported texture usage, texture ignored"));
			continue;
		}
		appear->SetSampler(i, sampler);
		sampler->Set(Sampler::TEXTUREOP, usage);
		if (stage.m_tcoordChannel >= 0)
			sampler->Set(Sampler::TEXCOORD, stage.m_tcoordChannel);
		if (hktex)
		{
			const TCHAR* fname = hktex->m_filename;
			const TCHAR* p = fname + strlen(fname);
			Core::String filepath(fname);
			Texture* tex;
			
			while (--p > fname)
			{
				if ((*p == TEXT('\\')) || (*p == TEXT('/')))
				{
					if (m_Directory.IsEmpty())
						filepath = p + 1;
					else
						filepath = m_Directory + p;
					break;
				}
			}
			tex = new Texture(filepath);
			sampler->SetTexture(tex);
		}
	}
	return appear;
}

Model* HavokLoader::ParseNode(const hkxNode* node, Model* root, Engine* simroot)
{
	Model*			model = NULL;
	Engine*			eng = NULL;
	Core::String	name(node->m_name);
	MatrixH			mtx;
	bool			isbone = node->m_bone;
	const hkClass*	nodeclass = node->m_object.getClass();
	const hkReferencedObject*	hkobj = reinterpret_cast<const hkReferencedObject*>(node->m_object.val());

	if (node->m_keyFrames.getSize() >= 1)
		mtx = node->m_keyFrames[0];
	if (nodeclass == &hkxCameraClass)
	{
		model = ParseCamera((hkxCamera*) hkobj);
		if (Core::String(node->m_name).Left(5) == TEXT("persp"))
			m_Scene->SetCamera((Camera*) model);
		VX_TRACE(Physics::Debug, ("ParseCamera: %s", name));
	}
	else if (nodeclass == &hkxLightClass)
	{
		model = ParseLight((hkxLight*) hkobj);
		model->SetActive(false);
		VX_TRACE(Physics::Debug, ("ParseLight: %s", name));
	}
	else if (nodeclass == &hkxMeshClass)
	{
		model = ParseMesh((hkxMesh*) hkobj, name);
		model->SetTransform(&mtx);
	}
	else if (nodeclass == &hkxSkinBindingClass)
	{
		const hkxSkinBinding*	hkskin = reinterpret_cast<const hkxSkinBinding*>(node->m_object.val());
		eng = (Engine*) simroot->Find(m_NamePrefix + name + TEXT(".xform"), Group::FIND_DESCEND | Group::FIND_EXACT);
		VX_TRACE(Physics::Debug, ("ParseNode: SkinBind %s", node->m_name));
		model = ParseMesh(hkskin->m_mesh, name);
		for (int i = 0; i < hkskin->m_nodeNames.getSize(); ++i)
		{
			Core::String bonename(m_NamePrefix + hkskin->m_nodeNames[i] + TEXT(".xform"));
			Engine* tmp = (Engine*) m_Scene->GetEngines()->Find(bonename, Group::FIND_DESCEND | Group::FIND_EXACT);
			if (tmp != NULL)
				continue;
			tmp = new Transformer();
			SetName(bonename, tmp);
			simroot->Append(tmp);
		}
	}
	else if (nodeclass == &hkpRigidBodyClass)
	{
		VX_TRACE(Physics::Debug, ("ParseNode: skipping rigid body %s", name));
		return NULL;
	}
	if (hkobj == HK_NULL)
	{
		eng = (Engine*) m_Scene->GetEngines()->Find(m_NamePrefix + name + TEXT(".xform"), Group::FIND_DESCEND | Group::FIND_EXACT);
		if (eng)
		{
			isbone = true;
			VX_TRACE(Physics::Debug, ("ParseNode: Found Bone %s", name));
			eng->Remove(Group::UNLINK_NOFREE);
		}
		if (isbone)							// more than 2 keyframes is a bone (probably)
		{
			if (eng == NULL)
			{
				eng = new Transformer();
				VX_TRACE(Physics::Debug, ("ParseNode: Create Bone %s", name));
			}
			((Transformer*) eng)->SetTransform(&mtx);
			SetName(m_NamePrefix + name + TEXT(".xform"), eng);
		}
		else									// 2 keyframes is a model
		{
			model = new Model();
			VX_TRACE(Physics::Debug, ("ParseNode: Model %s", name));
		}
	}
	if (model)
	{
		SetName(m_NamePrefix + name, model);
		if (simroot->IsClass(VX_Transformer) && (simroot->GetTarget() == NULL))
		{
			((Transformer*) simroot)->SetOptions(Transformer::WORLD);
			simroot->SetTarget(model);
		}
		if (root)
			root->Append(model);
		root = model;
	}
	else if (eng)
	{
		m_ParsedObjs.Set((intptr) hkobj, eng);
		simroot->Append(eng);
		simroot = eng;
	}
	for (int i = 0; i < node->m_children.getSize(); ++i)
	{
		hkxNode* hkchild = node->m_children[i];
		ParseNode(hkchild, root, simroot);
	}
	return model;
}

Transformer* HavokLoader::ParseRigidBody(hkpRigidBody* hkbody)
{
	intptr		tmp = (intptr) hkbody;
	ObjRef*		ref = m_ParsedObjs.Find(tmp);
	RigidBody*	body;

	if (ref)
	{
		body = (RigidBody*) (SharedObj*) *ref;
		VX_ASSERT(body && body->IsClass(VX_Transformer));
		return body;
	}
	Core::String	name(m_NamePrefix + hkbody->getName());
	MatrixH			mtx(hkbody->getTransform());

	body = new RigidBody(hkbody);
	name += TEXT(".xform");
	SetName(name, body);
	m_ParsedObjs.Set(tmp, body);
	body->SetTransform(&mtx);
	VX_TRACE(Physics::Debug, ("ParseRigidBody: %s", hkbody->getName()));
	return body;
}

Camera* HavokLoader::ParseCamera(const hkxCamera* node)
{
	Camera*		cam = new Camera();
	hkVector4	from = node->m_from;
	hkVector4	to = node->m_focus;
	hkVector4	up = node->m_up;
	hkVector4	at;
	Vec3H		pos(from);
	Vec3H		vecs[3];
	Matrix		mtx;	

	at.setSub(to, from);
	vecs[1] = up; vecs[1].Normalize();
	vecs[2] = at; vecs[2].Normalize();
	vecs[0] = vecs[2].Cross(vecs[1]);
	mtx.Rotate(vecs);
	mtx.Translate(pos);
	cam->SetTransform(&mtx);
	cam->SetFOV(node->m_fov);
	cam->SetHither(node->m_near);
	cam->SetYon(node->m_far);
	return cam;
}

Light* HavokLoader::ParseLight(const hkxLight* node)
{
	Light*	l;
	Vec3H	dir(node->m_direction);
	Color	c;

	switch (node->m_type)
	{
		case hkxLight::DIRECTIONAL_LIGHT:
		l = new DirectLight();
		break;

		case hkxLight::SPOT_LIGHT:
		l = new SpotLight();
		((SpotLight*) l)->SetOuterAngle(node->m_angle);
		((SpotLight*) l)->SetInnerAngle(node->m_angle);
		break;

		default:
		l = new Light();
		break;
	}
	c = node->m_color;
	l->SetColor(Col4(c));
	l->Translate(Vec3H(node->m_position));
	l->Rotate(Quat(Model::ZAXIS, dir));
	return l;
}

Skin* HavokLoader::ParseSkin(const hkxVertexBuffer* hkvbuf, int numbones)
{
	const hkxVertexDescription& hkvdesc = hkvbuf->getVertexDesc();
	const hkxVertexDescription::ElementDecl*	edecl;
	int				n = hkvbuf->getNumVertices();
	Core::String	skindesc;
	bool			has_skin = false;
	int				i = 0;

	if (m_HasCloth)
	{
		VX_TRACE(Physics::Debug, ("ParseSkin: USING CLOTH - no Skin output"));
		return NULL;
	}
	/*
	 * Figure out the vertex buffer layout
	 */
	while (edecl = hkvdesc.getElementDeclByIndex(i))
	{
		const TCHAR*	type;
		const TCHAR*	name;
		int				tmp = 0;
		int				numelems= edecl->m_numElements;

		switch (edecl->m_type)
		{
			case hkxVertexDescription::HKX_DT_FLOAT: type = TEXT("float"); break;
			case hkxVertexDescription::HKX_DT_UINT32:
			case hkxVertexDescription::HKX_DT_UINT8: type = TEXT("int"); break;
			default: VX_ERROR(("Physics::Load ERROR bad vertex descriptor type"), NULL);
		}
		++i;
		switch (edecl->m_usage)
		{
			case hkxVertexDescription::HKX_DU_POSITION:
			name = TEXT("position");
			numelems = m_VecSize;
			break;

			case hkxVertexDescription::HKX_DU_NORMAL:
			name = TEXT("normal");
			numelems = m_VecSize;
			break;

			case hkxVertexDescription::HKX_DU_TANGENT:
			name = TEXT("tangent");
			numelems = m_VecSize;
			break;

			case hkxVertexDescription::HKX_DU_BINORMAL:
			name = TEXT("bitangent");
			numelems = m_VecSize;
			break;

			case hkxVertexDescription::HKX_DU_BLENDWEIGHTS:
			name = TEXT("blendweight");
			type = TEXT("float");
			has_skin = true;
			break;

			case hkxVertexDescription::HKX_DU_BLENDINDICES:
			name = TEXT("blendindex");
			has_skin = true;
			break;

			default:
			continue;
		}
		if (!skindesc.IsEmpty())
			skindesc += ", ";
		skindesc += type;
		if (numelems > 1)
			skindesc += Core::String(numelems);
		skindesc += TEXT(" ");
		skindesc += name;
	}
	if (!has_skin)
		return NULL;
	/*
	 * Copy the vertices from Havok into Vixen
	 */
	Skin*				skin = new Skin();
	VertexArray*		verts = new VertexArray(skindesc, n);
	VertexArray::Iter	viter(verts);
	const DataLayout*	layout = verts->GetLayout();
	int					stride = 0;
	int					slot = 0;
	
	i = 0;
	verts->SetFlags(VertexPool::MORPH);
	while (edecl = hkvdesc.getElementDeclByIndex(i))
	{
		const char* pbase = static_cast<const char*>( hkvbuf->getVertexDataPtr(*edecl) );
		int ofs = layout->Slot[slot].Offset;
		int nfloats = layout->Slot[slot].Size;
		int stride = edecl->m_byteStride;
					
		VX_ASSERT((nfloats > 0) && (stride > 0));
		++i;
		switch (edecl->m_usage)
		{
			case hkxVertexDescription::HKX_DU_POSITION:
			VX_ASSERT(edecl->m_type == hkxVertexDescription::HKX_DT_FLOAT);
			for (int v = 0; v < n; ++v)
			{
				float* vptr = viter.GetVtx(v) + ofs;
				const float* pos = reinterpret_cast<const float*>(pbase + stride * v);
				*vptr++ = *pos++;
				*vptr++ = *pos++;
				*vptr++ = *pos++;
				if (m_VecSize == 4)
					*vptr++ = 1.0f;
			}
			break;

			case hkxVertexDescription::HKX_DU_NORMAL:
			case hkxVertexDescription::HKX_DU_TANGENT:
			case hkxVertexDescription::HKX_DU_BINORMAL:
			VX_ASSERT(edecl->m_type == hkxVertexDescription::HKX_DT_FLOAT);
			for (int v = 0; v < n; ++v)
			{
				float* vptr = viter.GetVtx(v) + ofs;
				const float* pos = reinterpret_cast<const float*>(pbase + stride * v);
				*vptr++ = *pos++;
				*vptr++ = *pos++;
				*vptr++ = *pos++;
				if (m_VecSize == 4)
					*vptr++ = 0.0f;
			}
			break;

			case hkxVertexDescription::HKX_DU_BLENDWEIGHTS:
			VX_ASSERT(edecl->m_type == hkxVertexDescription::HKX_DT_UINT8);
			for (int v = 0; v < n; ++v)
			{
				float*	vptr = viter.GetVtx(v) + ofs;
				float	wtotal = 0.0f;
				const uchar* pos = reinterpret_cast<const uchar*>(pbase + stride * v);
				for (int j = 0; j < nfloats; ++j)
				{
					float w = float(*pos++) / 255.0f;
					*vptr++ = w;
					wtotal += w;
				}
				if (wtotal == 0.0f)
					continue;
				if (fabs(wtotal - 1.0f) > FLT_EPSILON)
				{ VX_WARNING(("Physics::Load vertex %d weights %f not equal to 1", v, wtotal)); }
			}
			break;

			case hkxVertexDescription::HKX_DU_BLENDINDICES:
			VX_ASSERT(edecl->m_type == hkxVertexDescription::HKX_DT_UINT8);
			VX_ASSERT(layout->Slot[slot].Style & DataLayout::INTEGER);
			for (int v = 0; v < n; ++v)
			{
				int32* vptr = (int32*) (viter.GetVtx(v) + ofs);
				const uchar* pos = reinterpret_cast<const uchar*>(pbase + stride * v);
				for (int j = 0; j < nfloats; ++j)
				{
					int32 blend_index = int32(*pos++);

					VX_ASSERT(blend_index < numbones);
					*vptr++ = blend_index;
				}
			}
			break;

			default:
			continue;
		}
		if (++slot >= layout->NumSlots)
			break;
	}
	verts->SetNumVtx(n);
	skin->Enable(Engine::DATA_PARALLEL, Engine::ONLYME);
	skin->SetRestLocs(verts);
#ifdef _DEBUG
	// Output skin vertices as XML
	//std::ofstream textout(TEXT("skinverts.xml"), std::ios_base::out);
	//verts->Print(textout, SharedObj::PRINT_All);
#endif
	return skin;
}

RagDoll* HavokLoader::ParseRagdoll(hkaRagdollInstance* hkdoll, Model* root)
{
	RagDoll*	skel = new RagDoll(hkdoll);

	ParseSkeleton((hkaSkeleton*) (const hkaSkeleton*) hkdoll->m_skeleton, skel);
	for (int i = 0; i < hkdoll->m_rigidBodies.getSize(); ++i)
	{
		int			parentid = hkdoll->m_skeleton->m_parentIndices[i];
		Engine*		body = ParseRigidBody(hkdoll->m_rigidBodies[i]);
		Engine*		parent = skel;

		if (parentid >= 0)
		{
			intptr	tmp = (intptr) hkdoll->m_rigidBodies[parentid];
			ObjRef* ref = m_ParsedObjs.Find(tmp);
			if (ref)
			{
				parent = (Engine*) (SharedObj*) *ref;
				VX_ASSERT(parent && parent->IsClass(VX_Transformer));
			}
		}
		if (body->IsChild())
			body->Remove(Group::UNLINK_NOFREE);
		parent->Append(body);
	}
	Core::String name(m_NamePrefix + hkdoll->m_rigidBodies[0]->getName());
	Model*	rootshape = (Model*) root->Find(name, Group::FIND_DESCEND | Group::FIND_EXACT);

	if (rootshape)
		skel->SetTarget(rootshape);
	Ref<HavokPose> temp = skel->MakePose(Pose::BIND_POSE);
	skel->SetBindPose(temp);
	return skel;
}

Model* HavokLoader::MakeSkeletonShapes(RagDoll* skeleton)
{
	int			numbones = skeleton->GetNumBones();
	Appearance* RED = new Appearance(Col4(1.0f, 0.0f, 0.0f));
	Shape**		skelshapes = (Shape**) alloca(numbones * sizeof(Shape*));
	Vec3*		positions = (Vec3*) alloca(numbones * sizeof(Vec3));
	Quat*		rotations = (Quat*) alloca(numbones * sizeof(Quat));
	const Pose*	bindpose = skeleton->GetBindPose();

	bindpose->GetWorldRotations(rotations);
	bindpose->GetWorldPositions(positions);
	for (int b = 0; b < numbones; ++b)
	{
		int		parentid = ((hkaSkeleton&) skeleton).m_parentIndices[b];
		Model*	parent = NULL;
		Shape*	limb;
		Core::String bonename(skeleton->GetBoneName(b));

		if (parentid >= 0)
		{
			parent = skelshapes[parentid];
			VX_ASSERT(parent);
			if (rotations[b].IsEmpty())
				rotations[b] = rotations[parentid];
		}
		bonename = "bone-" + bonename;
		limb = skeleton->MakeLimb(bonename, parent, rotations[b], positions[b], RED, 20.0f, 2.0f);
		skelshapes[b] = limb;
	}
	skeleton->AttachBones(skelshapes[0], skeleton);
	return skelshapes[0];
}

void HavokLoader::MakeBones(const hkaSkeleton* hkskel, Skeleton* skeleton)
{	
	int			numbones =  hkskel->m_bones.getSize();
	Engine**	bones = (Engine**) alloca(numbones * sizeof(Engine*));

	memset(bones, 0, sizeof(Engine*) * numbones);
	VX_ASSERT(skeleton->GetNumBones() == numbones);
	for (int i = 0; i < numbones; ++i)
	{
		int				parentid = hkskel->m_parentIndices[i];
		Transformer*	bone = new Transformer();
		Engine*			parent = (parentid >= 0) ? bones[parentid] : skeleton;
		Core::String	bonename(m_NamePrefix);

		bones[i] = bone;
		bonename += hkskel->m_bones[i].m_name;
		bonename += TEXT(".xform");
		SetName(bonename, bone);
		if (parent)
			parent->Append(bone);
		else
			skeleton->Append(bone);
	}
}

RagDoll* HavokLoader::ParseSkeleton(const hkaSkeleton* hkskel, RagDoll* skeleton)
{	
	ObjRef*	ref = m_ParsedObjs.Find((intptr) hkskel);
	int		numbones =  hkskel->m_bones.getSize();

	if (ref)					// already parsed this mesh?
	{
		skeleton = (RagDoll*) (SharedObj*) *ref;
		VX_ASSERT(skeleton && skeleton->IsClass(VX_Skeleton));
		return skeleton;
	}
	if (skeleton == NULL)
	{
		skeleton = new RagDoll(hkskel);
		VX_TRACE(Physics::Debug, ("ParseSkeleton: %s", hkskel->m_name));
	}
	m_ParsedObjs.Set((intptr) hkskel, skeleton);
	SetName(m_NamePrefix + hkskel->m_name + TEXT(".skeleton"), skeleton);
	VX_ASSERT(skeleton->GetNumBones() == numbones);
	return skeleton;
}

void HavokLoader::AttachBones(Skeleton* skeleton, Engine* simroot)
{
	Transformer*		boneroot;
	RagDoll*			ragdoll = (RagDoll*) skeleton;
	const hkaSkeleton*	hkskel = ragdoll->GetHavokSkeleton();
	Core::String		name(m_NamePrefix + skeleton->GetBoneName(0) + TEXT(".xform"));
	Ref<HavokPose>		bindpose = skeleton->MakePose(Pose::SKELETON);

	boneroot = (Transformer*) simroot->Find(name, Group::FIND_DESCEND | Group::FIND_EXACT);
	VX_ASSERT(boneroot && boneroot->IsClass(VX_Transformer));
	if (!skeleton->IsChild())
		simroot->Append(skeleton);
	if (boneroot)
	{
		boneroot->Remove(Group::UNLINK_NOFREE);
		skeleton->Append(boneroot);
	}
	skeleton->FindBones(bindpose);
	skeleton->SetBindPose(bindpose);
}

Engine* HavokLoader::ParseMeshBind(const hkaMeshBinding* node, Model* root, RagDoll* skeleton)
{
	hkxMesh*		hkmesh = node->m_mesh;
	int				nsections = hkmesh->m_sections.getSize();
	Core::String	basename(m_NamePrefix + node->m_name);
	Model*			meshroot = ParseMesh(hkmesh, node->m_name);
	int				numbones = skeleton->GetNumBones();
	Engine*			skinroot = NULL;

	root->Append(meshroot);
	/*
	 * Parse all the mesh sections and make a skin for each
	 */
	if (!m_HasCloth)
	{
		if (nsections == 1)
		{
			Shape*	shape = ParseMeshSection(hkmesh->m_sections[0]);
			Skin*	skin = ParseSkin(hkmesh->m_sections[0]->m_vertexBuffer, skeleton->GetNumBones());
		
			if (skin)
			{
				SetName(basename + TEXT(".skin"), skin);
				skinroot = skin;
				skin->SetSkeleton(skeleton);
				skin->SetTarget(shape);
				shape->SetFlags(Model::MORPH);
				VX_TRACE(Physics::Debug, ("ParseMeshBind: %s", basename));
			}
		}
		else
		{
			skinroot = new Engine();
			skinroot->SetControl(Engine::CONTROL_CHILDREN);
			SetName(basename + TEXT(".skin"), skinroot);
			skinroot->SetTarget(meshroot);
			for (int i = 0; i < nsections; ++i)
			{
				Shape*	shape = ParseMeshSection(hkmesh->m_sections[i]);
				Skin*	skin = ParseSkin(hkmesh->m_sections[i]->m_vertexBuffer, skeleton->GetNumBones());
				if (skin)
				{
					Core::String	s(basename);

					s += TEXT('-');
					s += Core::String(i);
					s += TEXT(".skin");
					SetName(s, skin);
					skin->SetTarget(shape);
					shape->SetFlags(Model::MORPH);
					VX_TRACE(Physics::Debug, ("ParseMeshBind: %d %s", i, s));
					skinroot->Append(skin);
					skin->SetSkeleton(skeleton);
				}
			}
		}
	}
	/*
	 * Get the bind pose from the mesh binding
	 */
	Vec3H*			positions = (Vec3H*) alloca(numbones * sizeof(Vec3H));
	QuatH*			rotations = (QuatH*) alloca(numbones * sizeof(QuatH));
	VX_ASSERT(numbones == node->m_boneFromSkinMeshTransforms.getSize());

	for (int b = 0; b < numbones; ++b)
	{
		int			parentid = node->m_skeleton->m_parentIndices[b];
		hkTransform	hkt = node->m_boneFromSkinMeshTransforms[b];
		Vec3H		hkv(hkt.getTranslation());
		QuatH		hkq(hkt.getRotation());
		MatrixH		mtx;
			
		mtx.Rotate(hkq);
		mtx.Translate(hkv);
		hkt.setInverse(hkt);
		rotations[b] = hkt.getRotation();
		positions[b] = hkt.getTranslation();
	}
 	if (skeleton->GetTarget() == NULL)
		skeleton->SetTarget(root);
	skeleton->FindBones();
	skeleton->SetBindPose(rotations, positions);
	return skinroot;
}

ClothSim* HavokLoader::ParseCloth(hclClothSetupContainer* setup, Scene* vixenscene, hkxScene* havokscene)
{
	hclClothSetupContainer* inputClothSetupContainer = reinterpret_cast<hclClothSetupContainer*> (m_Root->findObjectByType( hclClothSetupContainerClass.getName()));
	ClothSim*				cloth;
	Core::String			name(m_NamePrefix);

	cloth = new ClothSim(NULL);
	SetName(name + TEXT("cloth"), cloth);
	for (int i = 0; i < setup->m_clothSetupDatas.getSize(); ++i)
	{
		hclClothSetupObject* setupObj = setup->m_clothSetupDatas[i];
		ClothSkin*			 data = SetupCloth(vixenscene, havokscene, setupObj);
		if (data)
		{
			VX_TRACE(Physics::Debug, ("ParseCloth:  %s", data->GetName()));

			name += data->GetName();
			SetName(name, data);
			cloth->Append(data);
		}
	}
	cloth->Disable(Engine::ACTIVE);
	return cloth;
}


void	HavokLoader::SampleRotation(Transformer* bone, const hkaAnimation* anim, int trackid)
{
	Interpolator*	rotate = NULL;
	Core::String	name;
	float			dur = anim->m_duration;
	int				nframes = anim->getNumOriginalFrames();
	float			step = dur / nframes;

	m_HasAnimation = true;
	name = bone->GetName();
	rotate = new Interpolator();
	SetName(name + TEXT(".rot"), rotate);
	rotate->SetInterpType(Interpolator::QSTEP);
	rotate->SetDestType(Interpolator::ROTATION);
	rotate->SetValSize(sizeof(Interpolator::WQuat) / sizeof(float));
	bone->Append(rotate);
	VX_TRACE(Physics::Debug, ("%s Sampling rotation %d tracks", rotate->GetName(), nframes));
	for (float t = 0; t < dur; t += step)
	{
		hkQsTransform	hkt;

		anim->sampleSingleTransformTrack(t, trackid, &hkt);
		QuatH	q = hkt.getRotation();
		Interpolator::WQuat	rw(q.x, q.y, q.z, q.w, 1.0);
	
		rotate->AddKey(t, rw);
		t += step;
	}
	rotate->SetDuration(dur);
}


void	HavokLoader::SampleTranslation(Transformer* bone, const hkaAnimation* anim, int trackid)
{
	Interpolator*	translate = NULL;
	Core::String	name;
	float			dur = anim->m_duration;
	int				nframes = anim->getNumOriginalFrames();
	float			step = dur / nframes;

	m_HasAnimation = true;
	name = bone->GetName();
	translate = new Interpolator();
	SetName(name + TEXT(".pos"), translate);
	translate->SetInterpType(Interpolator::STEP);
	translate->SetValSize(sizeof(Vec3) / sizeof(float));
	translate->SetDestType(Interpolator::POSITION);
	bone->Append(translate);
	VX_TRACE(Physics::Debug, ("%s Sampling translation track", translate->GetName()));
	for (float t = 0.0f; t < dur; t += step)
	{
		hkQsTransform hkt;
		anim->sampleSingleTransformTrack(t, trackid, &hkt);
		Vec3H	p(hkt.getTranslation());
		translate->AddKey(t, p);
	}
	translate->SetDuration(dur);
}

}	// end Vixen