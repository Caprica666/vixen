#include "vixen.h"
#include "physics/havok/vxphysics.h"
#include "physics/havok/vxcloth.h"

#include <Common/Base/KeyCode.h>
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Math/QsTransform/hkQsTransform.h>
#include <Common/SceneData/Skin/hkxSkinBinding.h>
#include <Common/SceneData/Scene/hkxScene.h>
#include <Common/SceneData/Scene/hkxSceneUtils.h>
#include <Common/SceneData/Graph/hkxNode.h>
#include <Animation/Animation/Rig/hkaSkeleton.h>

#include <Cloth/Setup/SetupMesh/SceneData/hclSceneDataSetupMesh.h>
#include <Cloth/Setup/SetupMesh/Named/hclNamedSetupMesh.h>
#include <Cloth/Setup/SetupMesh/ExtendedUser/hclExtendedUserSetupMesh.h>
#include <Cloth/Setup/TransformSet/Named/hclNamedTransformSetSetupObject.h>
#include <Cloth/Setup/Execution/hclClothSetupExecution.h>
#include <Cloth/Setup/Execution/hclUserSetupRuntimeDisplayData.h>
#include <Cloth/Setup/Execution/hclUserSetupRuntimeSkeletonData.h>
#include <Cloth/Setup/Cloth/hclClothSetupObject.h>

#include <Cloth/Cloth/World/hclWorld.h>
#include <Cloth/Cloth/Cloth/hclClothInstance.h>
#include <Cloth/Cloth/Cloth/hclClothData.h>
#include <Cloth/Cloth/Container/hclClothContainer.h>
#include <Cloth/Cloth/SimCloth/hclSimClothInstance.h>
#include <Cloth/Cloth/Buffer/hclBufferDefinition.h>
#include <Cloth/Cloth/Buffer/hclBuffer.h>
#include <Cloth/Cloth/Buffer/SceneData/hclSceneDataBuffer.h>
#include <Cloth/Cloth/TransformSet/hclTransformSet.h>
#include <Cloth/Cloth/Collide/Shape/Sphere/hclSphereShape.h>
#include <Cloth/Cloth/Collide/Shape/Plane/hclPlaneShape.h>
#include <Cloth/Cloth/Collide/hclCollidable.h>
//#include <Cloth/Cloth/Operator/RecalculateNormals/hclRecalculateAllNormalsOperator.h>
#include <Cloth/Cloth/Operator/Simulate/hclSimulateOperator.h>
#include <Cloth/Cloth/Utilities/Instantiation/hclInstantiationUtil.h>

#include "Physics/havok/math_interop.h"

namespace Vixen {

template <> inline bool Core::Dict<NameProp, void*, BaseDict>::CompareKeys(const NameProp& knew, const NameProp& kdict)
{
	return String(knew).CompareNoCase(kdict) == 0;
}

template <> inline uint32 Core::Dict<NameProp, void*, BaseDict>::HashKey(const NameProp& np) const
{
	return HashStr((const TCHAR*) np);
}

/*
 * Main cloth setup starting point to link Havok Cloth objects with the corresponding Vixen object.
 * Each Vixen Mesh object that is controlled by Havok Cloth maps to a Havok hclBuffer object.
 * Similarly, each Vixen Skeleton that drives a Havok Cloth skin maps to a Havok hclTransformSet object.
 * This class links all the Havok Cloth buffers and transform sets with the corresponding Vixen
 * objects using the Havok Cloth infrastructure.
 */
class ClothSetup : public hclUserSetupRuntimeDisplayData, public hclUserSetupRuntimeSkeletonData
{
public:
	ClothSetup(Scene* vixenscene, const hkxScene* havokscene);
	virtual void	getDisplayBufferLayout(const char *objectName, hkUint32 section, hclBufferLayout &layoutOut) const;
	virtual void	getStaticDisplayBufferLayout(const char *objectName, hkUint32 section, hclBufferLayout &layoutOut) const;
	static Mesh*	FindMeshByName(const char* name, int section, Model* root);
	virtual const hclSetupMesh*	getMeshDataForObject(const char *objectName) const;
	virtual hclTransformSetSetupObject*	getTransformSetForObject(const char* skeletonName) const;

	Model*			FindModel(const char* name) const;
protected:
	const Mesh*		FindMesh(const char* name, int section) const;
	void			getBufferLayout(const Mesh* mesh, hclBufferLayout &dstlayout) const;

	const hkxScene*			m_HavokScene;
	Scene*					m_VixenScene;
	mutable PtrArray		m_Transforms;
	mutable NameDict<void*>	m_SetupMeshes;
};


/*
 * Links a Vixen Mesh to a Havok hclBuffer at run time.
 * Implements the Havok calls necessary for Havok to directly
 * modify the vertices of the mesh and read it's triangles.
 */
class ClothBuffer : public hclBuffer
{
public:
	ClothBuffer(Mesh* mesh);

	virtual hkResult notifyBeginAccess(const hclBufferUsage& bufferUsage);
	virtual hkResult notifyEndAccess();

	Ref<Mesh>	m_Mesh;
	int			m_SlotMap[4];	// Vixen vertex slot for positions, normals, tangents, bitangents
};

/*
 * Provides context necessary for cloth setup, in this case the Vixen Scene
 * and the Havok hkxScene that contain the Vixen meshes and the Havok meshes
 * and cloth setup data.
 */
struct ClothBufferInput : public hclInstantiationInput
{
	ClothBufferInput(const hkxScene* havok_scene, Scene* vixen_scene)
	: m_HavokScene(havok_scene), m_VixenScene(vixen_scene) { }

	const hkxScene* m_HavokScene;
	Scene*			m_VixenScene;
};

typedef	hclInstantiationOutput ClothBufferOutput;

/*
 * Provides input arguments for setting up cloth buffers.
 * This class assumes that static cloth buffers (read only) link only
 * to Havok meshes and that display  buffers (writeable) link only
 * to Vixen meshes.
 */
class ClothBufferUtil : public hclInstantiationUtil
{
public:

	// Instantiate cloth buffers
	ClothBufferUtil(ClothBufferInput& input, ClothBufferOutput& output)
	: hclInstantiationUtil(input, output) { }

private:
	virtual hclBuffer*	getUserDisplayBuffer(const hclBufferDefinition* bufferDefinition);
	virtual hclBuffer*	getUserStaticBuffer(const hclBufferDefinition* bufferDefinition);
	ClothBufferInput*	getInput() { return static_cast<ClothBufferInput*>(m_input); }
	ClothBufferOutput*	getOutput() { return static_cast<ClothBufferOutput*>(m_output); }

	NameDict<void*>		m_UniqueBuffers;
};

/*
 * Links a Havok Mesh to a set of Vixen Shape objects.
 * Each Havok MeshSection is linked to the Mesh object owned by a Vixen Shape.
 * Havok meshes with more than one section are represented as a Vixen Model
 * with a Shape child for each mesh section.
 * This class implements the Havok calls necessary to link the Vixen Shapes
 * representing the mesh to the corresponding Havok MeshSections.
 */
class ClothSetupMesh : public hclSetupMesh
{
public:
	ClothSetupMesh(const Model* meshroot, const hkMatrix4& worldFromMesh);
	~ClothSetupMesh();
	const char*	getMeshName() const	{ return m_Name; }
	hkUint32	getNumberOfMeshSections() const { return (hkUint32) m_Sections.GetSize(); }
	void		getWorldFromMeshTransform(hkMatrix4& t) const { m_WorldFromMesh.get4x4ColumnMajor((hkReal*) &t); }
	const hclSetupMeshSection* getSetupMeshSection(hkUint32 i) const { return (hclSetupMeshSection*) m_Sections.GetAt(i); }
	/*
	 * We take vertex selections, vertex weights, and skinning info. from the Havok toolchain,
	 * so the Vixen setup mesh does not need to implement the following functions
	 * (i.e. the hclSceneDataSetupMesh implementations of these are used, via the hclExtendedUserSetupMesh).
	 */
	hkUint32	getNumberOfVertexChannels () const { return 0; }
	const char*	getVertexChannelName(hkUint32 index) const { return HK_NULL; }
	hkUint32	getNumberOfTriangleChannels() const { return 0; }
	const char*	getTriangleChannelName(hkUint32 index) const { return HK_NULL; }
	hkBool		isSkinned() const { return false;  }
	hkUint32	getNumberOfBones()  const { return 0xffffffff; }
	const char*	getBoneName(hkUint32 boneIndex) const  { return HK_NULL; }
	void		getBoneFromSkinTransform(hkUint32 boneIndex, hkMatrix4& boneFromSkin) const { }
	hclSetupMesh::TriangleChannelType	getTriangleChannelType(hkUint32 index) const { return hclSetupMesh::HCL_TRIANGLE_CHANNEL_INVALID; }
	hclSetupMesh::VertexChannelType	getVertexChannelType (hkUint32 index) const { return hclSetupMesh::HCL_VERTEX_CHANNEL_INVALID; }

protected:
	Core::String	m_Name;
	PtrArray		m_Sections;
	hkMatrix4		m_WorldFromMesh;
};

/*
 * Links a Havok MeshSection to a Vixen Mesh object.
 * This class implements the Havok calls necessary for Havok to access
 * the index and vertex arrays of the Vixen Mesh so it may be used
 * to display the output of a Havok Cloth simulation.
 */
class ClothSetupMeshSection : public hclSetupMeshSection
{
public:
	ClothSetupMeshSection(const ClothSetupMesh* setupMesh, Mesh* mesh);
	~ClothSetupMeshSection();

	const char*		getMeshName() const		{ return m_Mesh->GetName(); }
	void			getWorldFromMeshTransform(hkMatrix4& transformOut) const;
	hkUint32		getNumberOfMeshSections () const;
	hkUint32		getNumberOfBoneInfluences(hkUint32 vIndex) const { return 0; }
	void			getBoneInfluence(hkUint32 vIndex, hkUint32 infIndex, hkUint32& boneIndex, hkReal& weight) const {}
	hkBool			hasNormals() const { return (m_Mesh->GetStyle() & VertexPool::NORMALS) != 0; }
	hkBool			hasTangents() const { return false;   } // TODO: implement this
	hkBool			hasBiTangents() const { return false;   } // TODO: implement this
	hkUint32		getNumberOfVertices() const	{ return (hkUint32) m_Mesh->GetNumVtx(); }
	hkResult		getVertexPosition(hkUint32 index, hkVector4& v) const;
	hkResult		getVertexNormal(hkUint32 index, hkVector4& v) const;
	hkResult		getTangent(hkUint32 index, hkVector4& v) const;
	hkResult		getBiTangent(hkUint32 index, hkVector4& v) const;
	hkUint32		getNumberOfTriangles() const	{ return (hkUint32) m_Mesh->GetNumFaces(); }
	hkResult		getTriangle(hkUint32 index, Triangle& triangle) const;
	hkResult		getVertexTangent(hkUint32 index, hkVector4& tangentOut) const { return HK_FAILURE; }
	hkResult		getVertexBiTangent(hkUint32 index, hkVector4& biTangentOut) const { return HK_FAILURE; }
	const hclSetupMesh*	getMeshInterface() const { return m_SetupMesh; }
	const hclSetupMeshSection* getSetupMeshSection(hkUint32 index) const;

protected:
	const ClothSetupMesh* m_SetupMesh;
	Ref<Mesh>			m_Mesh;
	VertexArray::Iter	m_Iter;
};


/*
 * Links a Vixen RagDoll (Havok physics-based Skeleton) to a Havok Cloth hclTransformSetupSet.
 * This class implements the Havok calls necessary for Havok to access the current
 * pose of the skeleton to drive a skinned animation.
 */
class ClothTransformSetup  : public hclTransformSetSetupObject 
{
	public:
		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CLOTH_SETUP );
		ClothTransformSetup(const RagDoll* skeleton);

		//  hclTransformSetSetupObject interface
		RagDoll*	getSkeleton() const							{ return m_Skeleton; }
		const char* getName() const								{ return m_SkelName; }
		const char* getSkeletonName() const						{ return m_SkelName; }
		hkUint32	getNumberOfTransforms () const				{ return m_Skeleton->GetNumBones(); }
		const char* getTransformName(hkUint32 index) const		{ return m_Skeleton->GetBoneName(index); }
		int			getTransformParentIndex(hkUint32 index) const;
		void		getSetupTransforms(hkArray<hkMatrix4>& transformsOut) const;
		hclTransformSetDefinition* _createTransformSetDefinition(const hclClothSetupExecution& executionData) const;

protected:
		Ref<RagDoll>	m_Skeleton;
		Core::String	m_SkelName;
};

/*
 * Provides Havok cloth setup process access to the Vixen and Havok scene hierarchy.
 */
ClothSetup::ClothSetup(Scene* vixenscene, const hkxScene* havokscene)
:	hclUserSetupRuntimeDisplayData(), m_VixenScene(vixenscene), m_HavokScene(havokscene)
{
	VX_ASSERT(vixenscene);
	VX_ASSERT(havokscene);
}

/*
 * Finds the Vixen Mesh corresponding to a Havok mesh section by name.
 * @param meshname	name of Havok mesh to find.
 * @param section	0-based index of Havok mesh section.
 * @param root		Vixen root Model for the mesh. If this is not null,
 *					it should point to the Vixen Model whose child Shapes
 *					represent Havok MeshSections or a single Vixen Shape
 *					(for a Havok Mesh with only one section.)
 *
 * A Havok Mesh with more than one section is represented by a Vixen Model
 * with a child Shape for each Havok MeshSection. The name of the Model
 * is the same as the name of the Havok Mesh. The name of each child Shape
 * is the Mesh name with a numeric suffix for the mesh section (e.g. mesh-0, mesh-1).
 * For a Havok mesh with one section, a single Vixen Shape of the same name is used.
 *
 * @return Vixen Mesh corresponding to Havok MeshSection or NULL if not found.
 */
const Mesh* ClothSetup::FindMesh(const char* meshName, int section) const
{
	return (const Mesh*) FindMeshByName(meshName, section, (Model*) m_VixenScene->GetModels());
}

Mesh* ClothSetup::FindMeshByName(const char* meshName, int section, Model* root)
{
	Shape*			shape;
	Core::String	name(TEXT('.'));
	Geometry*		geo;

	VX_ASSERT(root);
	name += meshName;
	root = (Model*) root->Find(name, Group::FIND_DESCEND | Group::FIND_END);
	if (root == NULL)
		VX_ERROR(("ClothSetup: FindMesh %s ERROR model not found", name), NULL);
	if (root->IsClass(VX_Shape))
		shape = (Shape*) root;
	else
	{
		name += TEXT('-');
		name += Core::String(section);
		shape = (Shape*) root->Find(name, Group::FIND_CHILD | Group::FIND_END);
		if ((shape == NULL) || !shape->IsClass(VX_Shape))
			VX_ERROR(("ClothSetup: FindMesh %s ERROR shape not found", name), NULL);
	}
	geo = shape->GetGeometry();
	if ((geo == NULL) || !geo->IsClass(VX_Mesh))
		VX_ERROR(("ClothSetup: FindMesh %s %d ERROR %s has no geometry", meshName, section, shape->GetName()), NULL);
	VX_TRACE(Physics::Debug > 1, ("ClothSetup: FindMesh %s %d -> %s %d verts, %d tris",
			meshName, section, shape->GetName(), geo->GetNumVtx(), geo->GetNumFaces()));
	return (Mesh*) geo;
}

/*
 * Finds the Vixen Model corresponding to a Havok Mesh by name.
 * @param meshname	name of Havok mesh to find.
 *
 * A Havok Mesh with more than one section is represented by a Vixen Model
 * with a child Shape for each Havok MeshSection. The name of the Model
 * is the same as the name of the Havok Mesh. For a Havok mesh with one section,
 * a single Vixen Shape of the same name is used.
 *
 * @return Vixen Model or Shape corresponding to Havok Mesh.
 */
Model* ClothSetup::FindModel(const char* meshname) const
{
	Model*		root = m_VixenScene->GetModels();
	Core::String		name(TEXT('.'));

	name += meshname;
	if (root)
		return (Model*) root->Find(meshname, Group::FIND_DESCEND | Group::FIND_END);
	return NULL;
}

/*
 * Describes the layout of a Vixen Mesh to Havok in terms of hclBufferLayout.
 * @param mesh		Vixen mesh to describe
 * @param dstlayout	Havok layout structure to get mesh description.
 */
void  ClothSetup::getBufferLayout(const Mesh* mesh, hclBufferLayout &dstlayout) const
{
	const DataLayout*	srclayout;
	const VertexArray*	verts;

	VX_ASSERT(mesh);
	verts = mesh->GetVertices();
	if (verts == NULL)
		return;
	verts->MakeLock();
	verts->Lock();
	srclayout = verts->GetLayout();
	dstlayout.m_triangleFormat = hclBufferLayout::TF_THREE_INT32S;
	dstlayout.m_numSlots = 1;
	dstlayout.m_slots[0].m_stride = (hkUint8) srclayout->Size * sizeof(float);
	dstlayout.m_slots[0].m_flags = hclBufferLayout::SF_16BYTE_ALIGNED_START;
	for (int i = 0; i < srclayout->NumSlots; ++i)
	{
		const LayoutSlot&	slot = srclayout->Slot[i];
		int					havokindex;

		if (slot.Name.Right(8) == TEXT("position"))
			havokindex = 0;
		else if (slot.Name.Right(6) == TEXT("normal"))
			havokindex = 1;
		else if (slot.Name.Right(7) == TEXT("bitangent"))
			havokindex = 2;
		else if (slot.Name.Right(6) == TEXT("tangent"))
			havokindex = 3;
		else
			continue;
		hclBufferLayout::BufferElement& dstelem = dstlayout.m_elementsLayout[havokindex];
		dstelem.m_vectorConversion = hclRuntimeConversionInfo::VC_FLOAT3;
		dstelem.m_vectorSize = 3 * sizeof(float);
		dstelem.m_slotId = 0;
		dstelem.m_slotStart = (hkUint8) slot.Offset * sizeof(float);
	}
	verts->Unlock();
}

/*
 * Describes the layout of the Vixen Mesh corresponding to a Havok MeshSection as hclBufferLayout.
 * The Vixen mesh will be used to display the output of a Havok Cloth simulation so it will be modified.
 * @param meshname	name of Havok mesh to find.
 * @param section	0-based index of Havok mesh section.
 * @param dstlayout	Havok layout structure to get mesh description.
 */
void  ClothSetup::getDisplayBufferLayout(const char *meshname, hkUint32 section, hclBufferLayout &dstlayout) const
{
	const Mesh*	mesh = FindMesh(meshname, (int) section);

	if (mesh)
	{
		getBufferLayout(mesh, dstlayout);
		return;
	}
	VX_ERROR_RETURN(("ClothSetup: ERROR mesh %s %d not found in Vixen scene", meshname, section));
}

/*
 * Describes the layout of a static buffer in terms of hclBufferLayout.
 * The buffer can come from either Vixen or Havok. If it is not found in the Vixen scene,
 * the Havok scene is then searched.
 * @param meshname	name of scene mesh to find.
 * @param section	0-based index of Havok mesh section.
 * @param dstlayout	Havok layout structure to get mesh description.
 */
void  ClothSetup::getStaticDisplayBufferLayout(const char *objectName, hkUint32 section, hclBufferLayout &layout) const
{
	const Mesh*	mesh = FindMesh(objectName, (int) section);

	// static buffer came from a Vixen mesh
	if (mesh)
	{
		getBufferLayout(mesh, layout);
		return;
	}
	// Static buffer came from a Havok mesh.
	const hkxNode* node = m_HavokScene->findNodeByName(objectName);
	if (node == NULL)
		VX_ERROR_RETURN(("ClothSetup: ERROR node %s not found in Havok scene", objectName));
	hclSceneDataBuffer::getSceneDataBufferLayout(node, section, layout);
}

/*
 * Describes a skeleton in the scene in terms of ClothTransformSetup.
 * @param skelname	name of skeleton to get pose for
 *
 * The skeleton is assumed to be in the Vixen scene. This function makes a ClothTransformSetup
 * object (a subclass of the Havok hclTransformSetSetupObject) which provides Havok access
 * to the structure of the skeleton and the current pose.
  */
hclTransformSetSetupObject* ClothSetup::getTransformSetForObject(const char* skelname) const
{
	Core::String	name(TEXT('.'));
	const RagDoll*	skel;
	
	name += skelname;
	name += TEXT(".skeleton");
	skel = (const RagDoll*) m_VixenScene->GetEngines()->Find(name, Group::FIND_DESCEND | Group::FIND_END);

	if (skel && skel->IsClass(VX_Skeleton))
	{
		ClothTransformSetup* transformSetSetup = new ClothTransformSetup(skel);

		skel->MakeLock();
		if (transformSetSetup->isValid())
		{
			void*	ptr = transformSetSetup;
			m_Transforms.Append(ptr);
			return transformSetSetup;
		}
		else
		{
			delete transformSetSetup;
			HK_ERROR(0x0, "Transform set setup invalid.");
		}
	}
	HK_WARN_ALWAYS(0x0, "Skeleton " << skelname << " not found, so cannot make a requested hclTransformSetSetupObject (setup execution will fail).");
	return NULL;
}

/*
 * Describes the set of Vixen Shapes for a scene object in terms of hclSetupMesh.
 * @param objname	name of scene object (a Havok Mesh)
 *
 * Constructs the Havok cloth setup infrastructure for a single Havok Mesh.
 * This allows Havok to read and write the vertices of the Vixen Shape
 * objects which are used to display the results of simulation on the mesh.
 */
const hclSetupMesh*  ClothSetup::getMeshDataForObject(const char *objname) const
{
	hclExtendedUserSetupMesh::Options	userSetupMeshOptions;
	const Model*				meshroot = FindModel(objname);
	const hkxNode*				node = m_HavokScene->findNodeByName(objname);
	hclSetupMesh*				setupMesh = HK_NULL;
	hclSceneDataSetupMesh*		havokSetupMesh;
	hclExtendedUserSetupMesh*	extendedUserSetupMesh;
	hkMatrix4					worldFromMesh;
	MatrixH						worldFromModel;
	hclSetupMesh*				vixenSetupMesh;
	void**						setupPtr;

	if ((meshroot == NULL) || (node == NULL))
		return NULL;
	setupPtr = m_SetupMeshes.Find(objname);
	if (setupPtr != NULL)
	{
		setupMesh = (hclSetupMesh*) *setupPtr;
		if (setupMesh != NULL)
			return setupMesh;
	}
	worldFromMesh.setIdentity();
	//m_HavokScene->getWorldFromNodeTransform(node, worldFromMesh, 0);
	//meshroot->TotalTransform(&worldFromModel);
	vixenSetupMesh = new ClothSetupMesh(meshroot, worldFromModel);
	havokSetupMesh = new hclSceneDataSetupMesh(node, worldFromMesh);
	userSetupMeshOptions.m_maxDistanceForMatch = 0.01f;  
	userSetupMeshOptions.m_userSetupMesh = vixenSetupMesh;
	userSetupMeshOptions.m_sceneDataSetupMesh = havokSetupMesh;
	userSetupMeshOptions.m_useUserNormalIds = false;
	extendedUserSetupMesh = hclExtendedUserSetupMesh::create(userSetupMeshOptions);
	vixenSetupMesh->removeReference();
	havokSetupMesh->removeReference();		
	setupMesh = extendedUserSetupMesh;
	m_SetupMeshes[objname] = setupMesh;
	return setupMesh;
}

/*
 * Creates a new hclBuffer for the given scene mesh.
 * @param bufferdef	Havok buffer definition with name of Mesh and index of MeshSection.
 * This function tries to find a Vixen Shape with the corresponding name.
 * If this is found, a ClothBuffer is made which informs Havok about the Vixen Mesh.
 * @return ClothBuffer to describe Vixen Mesh or NULL if mesh not found in Vixen scene
 */
hclBuffer* ClothBufferUtil::getUserDisplayBuffer(const hclBufferDefinition* bufferdef)
{
	Core::String	bufname(bufferdef->m_name);
	const char*		name;
	hclBuffer*		buffer = NULL;
	void**			bufptr;
	
	bufname += TEXT('-');
	bufname += Core::String(bufferdef->m_subType);
	name = bufname;
	bufptr = m_UniqueBuffers.Find(name);
	if (bufptr)
		buffer = (hclBuffer*) *bufptr;
	if (buffer)
	{
		buffer->addReference();
		return buffer;
	}

	ClothBufferInput* input = getInput();
	Mesh*			mesh = ClothSetup::FindMeshByName(bufferdef->m_name, bufferdef->m_subType, input->m_VixenScene->GetModels());

	if (mesh)			// Create buffer based on Vixen mesh
	{
		buffer = new ClothBuffer(mesh);
		m_UniqueBuffers[name] = buffer;
		buffer->addReference();
		return buffer;
	}
	else
	{
		HK_WARN_ALWAYS(0xabba56fe, "Could not find object in the Vixen scene " << bufferdef->m_name);
		getOutput()->m_result = HK_FAILURE;
	}
	return NULL;
}

/*
 * Creates a new hclBuffer for the given scene mesh.
 * @param bufferdef	Havok buffer definition with name of Mesh and index of MeshSection.
 * This function tries to find a Havok MeshSection in the Havok scene with the given name and index
 * and make a hclSceneDataBuffer from it.
 * Static meshes are exported via both the Vixen and Havok path.
 * Since they are the same, we use the Havok scene mesh for the static buffer.
 */
hclBuffer* ClothBufferUtil::getUserStaticBuffer(const hclBufferDefinition* bufferdef)
{
	ClothBufferInput*	input = getInput();
	Core::String		bufname(bufferdef->m_name);
	hkxNode*			node = input->m_HavokScene->findNodeByName(bufname);
	hkxMesh*			mesh;
	const char*			name;
	hclBuffer*			buffer = NULL;
	void**				bufptr;
	
	bufname += TEXT('-');
	bufname += Core::String(bufferdef->m_subType);
	name = bufname;
	bufptr = m_UniqueBuffers.Find(name);
	if (bufptr)
		buffer = (hclBuffer*) *bufptr;
	if (buffer)
	{
		buffer->addReference();
		return buffer;
	}
	if (node == NULL)
		VX_ERROR(("ClothBufferUtil: ERROR static buffer %s not found", name), NULL);
	mesh = hkxSceneUtils::getMeshFromNode(node);
	if (mesh == NULL)
		VX_ERROR(("ClothBufferUtil: ERROR static buffer %s has no geometry", name), NULL);
	VX_TRACE(Physics::Debug > 1, ("ClothBufferUtil: found Havok static buffer %s", bufname)); 
	buffer = new hclSceneDataBuffer(input->m_HavokScene, node, bufferdef->m_subType);
	m_UniqueBuffers[name] = buffer;
	buffer->addReference();
	return buffer;
}

/*
 * Provides Havok access to the vertices of a Vixen Mesh.
 * @param mesh	Vixen mesh.
 *
 * Constructs the Havok cloth setup infrastructure to allow Havok
 * to read and write the vertices of the Vixen Mesh.
 */
ClothBuffer::ClothBuffer(Mesh* mesh)
{
	hkUint32			nvtx = (hkUint32) mesh->GetNumVtx();
	const VertexArray*	verts = mesh->GetVertices(); VX_ASSERT(verts);
	const DataLayout*	layout = verts->GetLayout(); VX_ASSERT(layout);

	m_Mesh = mesh;
	verts->MakeLock();
	mesh->GetIndices()->MakeLock();
	m_positions.m_numElements = 0;
	m_normals.m_numElements = 0;
	m_tangents.m_numElements = 0;
	m_biTangents.m_numElements = 0;
	m_triangles.m_numElements = 0;
	m_positions.m_bufferStart = NULL;
	m_tangents.m_bufferStart = NULL;
	m_biTangents.m_bufferStart = NULL;
	m_triangles.m_bufferStart = NULL;
	m_SlotMap[0] = -1;
	m_SlotMap[1] = -1;
	m_SlotMap[2] = -1;
	m_SlotMap[3] = -1;

	for (int i = 0; i < layout->NumSlots; ++i)
	{
		const LayoutSlot&	slot = layout->Slot[i];

		if (slot.Name.Right(8) == TEXT("position"))
		{
			m_positions.m_numElements = nvtx;
			m_SlotMap[0] = i;
		}
		else if (slot.Name.Right(9) == TEXT("bitangent"))
		{
			m_biTangents.m_numElements = nvtx;
			m_SlotMap[2] = i;
		}
		else if (slot.Name.Right(7) == TEXT("tangent"))
		{
			m_tangents.m_numElements = nvtx;
			m_SlotMap[3] = i;
		}
		else if (slot.Name.Right(6) == TEXT("normal"))
		{
			m_normals.m_numElements = nvtx;
			m_SlotMap[1] = i;
		}
		else
			continue;
	}
	m_triangles.m_numElements = (hkUint32) mesh->GetNumFaces();
	VX_TRACE(Physics::Debug > 1, ("ClothBuffer: %s %d verts, %d tris",
			mesh->GetName(), nvtx, m_triangles.m_numElements));
}

/*
 * Called by Havok before accessing vertices of the Vixen mesh.
 * This function locks the vertex and index arrays associated with the mesh.
 */
hkResult ClothBuffer::notifyBeginAccess(const hclBufferUsage& bufferUsage)
{
	const VertexArray*	verts = m_Mesh->GetVertices(); VX_ASSERT(verts);
	const IndexArray*	inds = m_Mesh->GetIndices(); VX_ASSERT(inds);
	const DataLayout*	layout;
	const float*		vertdata;
	intptr				nvtx;
	int					stride;

	VX_TRACE(Physics::Debug > 2, ("ClothBuffer: %s lock", m_Mesh->GetName()));
	inds->Lock();
	verts->Lock();
	nvtx = verts->GetNumVtx();
	stride = verts->GetVtxSize() * sizeof(float);
	vertdata = verts->GetData();
	layout = verts->GetLayout();
	m_positions.m_bufferStart = HK_NULL;
	m_normals.m_bufferStart = HK_NULL;
	m_tangents.m_bufferStart = HK_NULL;
	m_biTangents.m_bufferStart = HK_NULL;
	m_positions.m_numElements = 0;
	m_normals.m_numElements = 0;
	m_tangents.m_numElements = 0;
	m_biTangents.m_numElements = 0;

	m_triangles.m_numElements = (hkUint32) inds->GetSize() / 3;
	m_triangles.m_bufferStart = (void*) inds->GetData();
	m_triangles.m_stride = 3 * sizeof(int32);
	m_triangles.m_use16BitsIndices = false;
	if (m_SlotMap[0] >= 0)				// POSITIONS
	{
		const LayoutSlot& slot = layout->Slot[m_SlotMap[0]];

		m_positions.m_bufferStart = (void*) vertdata;
		m_positions.m_numElements = (hkUint32) nvtx;
		m_positions.m_stride = (hkUint8) stride;
		m_positions.m_flags =  0;
	}	
	if (m_SlotMap[1] >= 0)				// NORMALS
	{
		const LayoutSlot& slot = layout->Slot[m_SlotMap[1]];

		m_normals.m_bufferStart = (void*) (vertdata + slot.Offset);
		m_normals.m_numElements = (hkUint32) nvtx;
		m_normals.m_stride = (hkUint8) stride;
		m_normals.m_flags =  0;
	}	
	if (m_SlotMap[2] >= 0)				// TANGENTS
	{
		const LayoutSlot& slot = layout->Slot[m_SlotMap[2]];
		m_tangents.m_bufferStart = (void*) (vertdata + slot.Offset);
		m_tangents.m_numElements = (hkUint32) nvtx;
		m_tangents.m_stride = (hkUint8) stride;
		m_tangents.m_flags =  0;
	}	
	if (m_SlotMap[3] >= 0)				// BITANGENTS
	{
		const LayoutSlot& slot = layout->Slot[m_SlotMap[3]];
		m_biTangents.m_bufferStart = (void*) (vertdata + slot.Offset);
		m_biTangents.m_numElements = (hkUint32) nvtx;
		m_biTangents.m_stride = (hkUint8) stride;
		m_biTangents.m_flags =  0;
	}
	return HK_SUCCESS;
}

/*
 * Called by Havok after accessing vertices of the Vixen mesh.
 * This function unlocks the vertex and index arrays associated with the mesh.
 */
hkResult ClothBuffer::notifyEndAccess()
{	
	const VertexArray*	verts = m_Mesh->GetVertices();
	const IndexArray*	inds = m_Mesh->GetIndices();

	verts->SetChanged(true);
	verts->Unlock();
	inds->Unlock();
	VX_TRACE(Physics::Debug > 2, ("ClothBuffer: %s unlock", m_Mesh->GetName()));
	return HK_SUCCESS;
}

ClothSetupMesh::ClothSetupMesh(const Model* meshroot, const hkMatrix4& worldFromMesh)
:	m_WorldFromMesh(worldFromMesh)
{
	/*
	 * Strip off the file name, isolating the mesh name
	 */
	m_Name = meshroot->GetName();
	/*
	 * Only one shape representing this object?
	 * Just make one mesh section.
	 */
	if (meshroot->IsClass(VX_Shape) && !meshroot->IsParent())
	{
		Mesh*	mesh = (Mesh*) ((Shape*) meshroot)->GetGeometry();
		VX_ASSERT(mesh && mesh->IsClass(VX_Mesh));
		m_Sections.SetAt(0, new ClothSetupMeshSection(this, mesh));
		return;
	}
	/*
	 * Multiple shapes represent this mesh? Make a different
	 * mesh section for each child. 
	 */
	Shape::Iter		iter(meshroot);
	Core::String	meshname(meshroot->GetName());
	Shape*			shape;
	size_t			len = meshname.GetLength();
	int				i = 0;

	while (shape = (Shape*) iter.Next())
	{
		Core::String childname(shape->GetName());
		if (!shape->IsClass(VX_Shape))
			continue;
		if (childname.Left(len) != meshname)
			continue;
		Mesh*	mesh = (Mesh*) shape->GetGeometry();
		shape->GetAppearance()->Set(Appearance::CULLING, 0);
		VX_ASSERT(mesh && mesh->IsClass(VX_Mesh));
		m_Sections.SetAt(i++, new ClothSetupMeshSection(this, mesh));
	}
}

ClothSetupMesh::~ClothSetupMesh()
{
	for (int i = 0; i < m_Sections.GetSize(); ++i)
	{
		delete m_Sections[i];
	}
}

ClothSetupMeshSection::ClothSetupMeshSection(const ClothSetupMesh* setupMesh, Mesh* mesh)
:	m_SetupMesh(setupMesh), m_Mesh(mesh), m_Iter(mesh->GetVertices())
{
	mesh->GetVertices()->Lock();
}

ClothSetupMeshSection::~ClothSetupMeshSection()
{
	m_Mesh->GetVertices()->Unlock();
}

hkResult ClothSetupMeshSection::getVertexPosition(hkUint32 index, hkVector4& dstpos) const
{
	Vec3* srcpos = (Vec3*) m_Iter.GetVtx(index);
	if (srcpos)
	{
		dstpos.set(srcpos->x, srcpos->y, srcpos->z);
		return HK_SUCCESS;
	}
	return HK_FAILURE;
}

hkResult ClothSetupMeshSection::getVertexNormal(hkUint32 index, hkVector4& normal) const
{
	Vec3* srcnml = (Vec3*) m_Iter.GetNormal(index);
	if (srcnml)
	{
		normal.set(srcnml->x, srcnml->y, srcnml->z);
		return HK_SUCCESS;
	}
	return HK_FAILURE;
}


hkResult ClothSetupMeshSection::getTriangle(hkUint32 index, Triangle& triangle) const
{
	const IndexArray*	inds = m_Mesh->GetIndices();
	intptr				numtris;

	if (inds == NULL)
		return HK_FAILURE;
	 numtris = inds->GetSize() / 3;

	if (index >= numtris)
	{
		HK_ASSERT(0x0, "index >= num. triangles in ClothSetupMeshSection::getTriangle()");
		return HK_FAILURE;
	}
	index *= 3;
	triangle.m_indices[0] = inds->GetAt(index);
	triangle.m_indices[1] = inds->GetAt(index + 1); 
	triangle.m_indices[2] = inds->GetAt(index + 2);
	return HK_SUCCESS;
}

ClothTransformSetup::ClothTransformSetup(const RagDoll* skeleton)
:	m_Skeleton(skeleton)
{
	int			p;
	const Pose*	pose = m_Skeleton->GetPose();

	pose->MakeLock();
	m_SkelName = skeleton->GetName();
	if (m_SkelName.Right(9) == TEXT(".skeleton"))
		m_SkelName = m_SkelName.Left(m_SkelName.GetLength() - 9);
	p = m_SkelName.Find(TEXT('.'));
	if (p > 0)
		m_SkelName = m_SkelName.Mid(p + 1);
}


int	ClothTransformSetup::getTransformParentIndex(hkUint32 index) const
{
	if (!m_Skeleton.IsNull())
		return m_Skeleton->GetParentBoneIndex(index);
	return -1;
}

void ClothTransformSetup::getSetupTransforms(hkArray<hkMatrix4>& transforms) const
{
	int			numBones = m_Skeleton->GetNumBones();
	const Pose*	pose = m_Skeleton->GetPose();
	ObjectLock	lock(pose);
	transforms.setSize(numBones);
	for (int b = 0; b < numBones; ++b)
	{
		MatrixH	mtx;
		
		pose->GetWorldMatrix(b, mtx);
		transforms[b] = mtx;
	}
}

// This hclTransformSetDefinition object is stored in the cloth data, and is meant to be used at runtime during cloth instantiation to 
// determine which Vixen skeleton should be associated with each hclTransformSet in the cloth instance.
// (Similarly to how the hclBufferDefinitions associate Vixen meshes with each hclBuffer in the cloth instance, at buffer instantiation time).
// In our case, we know there is only one skeleton needed, so this information is only used as a check.
hclTransformSetDefinition* ClothTransformSetup::_createTransformSetDefinition(const hclClothSetupExecution& executionData) const
{
	hclTransformSetDefinition* transformSetDefinition = new hclTransformSetDefinition();

	transformSetDefinition->m_name = m_SkelName;
	transformSetDefinition->m_type = HCL_TRANSFORMSET_TYPE_ANIMATION;
	transformSetDefinition->m_numTransforms = m_Skeleton->GetNumBones();	
	return transformSetDefinition;
}

ClothSkin* SetupCloth(Scene* vixenscene, hkxScene* havokscene, hclClothSetupObject* setupinfo)
{
	hclClothSetupExecutionReporter	reporter;
	hclClothSetupExecution::Options	opts;
	ClothSetup						setup(vixenscene, havokscene);
	hclClothData*					data;
	ClothSkin*						clothskin;
	Core::String					name(setupinfo->getName());

	opts.m_targetPlatform = hclClothData::HCL_PLATFORM_WIN32;
	opts.m_reporter = &reporter;
	hclClothSetupExecution exec(*setupinfo, setup, &setup, opts);
	data = exec.getClothData();
	if (data == NULL)
		VX_ERROR(("Havok Cloth Setup failed for %s", name), NULL);
	clothskin = new ClothSkin(data);
	name += TEXT(".clothskin");
	clothskin->SetName(name);
	return clothskin;
}

float	ClothSkin::ComputeTime(float t)
{
	return Engine::ComputeTime(t);
}

bool ClothSkin::OnStart()
{
	if ((m_ClothInstance == NULL) && m_ClothData)
	{
		Scene*				vixenscene = World3D::Get()->GetScene();
		ClothSim*			clothsim = (ClothSim*) Parent();
		hclWorld*			world;
		hclCollidable*		ground = NULL;
		ClothBufferOutput	output;
		hclClothData::InstanceOptions options;

		VX_ASSERT(clothsim);
		VX_ASSERT(clothsim->IsKindOf(&ClothSim::ClassInfo));
		ClothBufferInput input(clothsim->GetHavokScene(), vixenscene);
		m_ClothInstance = m_ClothData->createInstance(options);
		ground = clothsim->GetGroundCollide();
		if (ground != NULL)
		{
			for (int sc = 0; sc < m_ClothInstance->m_simClothInstances.getSize(); ++sc)
			{
				hclSimClothInstance* scinstance= m_ClothInstance->m_simClothInstances[sc];
				scinstance->addWorldCollidable(ground);
				VX_TRACE(ClothSim::Debug || ClothSkin::Debug, ("ClothSkin: %s adding ground plane", GetName()));
			}
		}
		input.m_clothInstance = m_ClothInstance;
		world = ClothSim::GetWorld();
		if (world == NULL)
			return false;
		VX_TRACE(Physics::Debug || ClothSim::Debug || ClothSkin::Debug, ("ClothSkin: %s creating cloth instance", GetName()));
		world->lockAll();
		world->setOnTheFlyNotificationsEnabled(true);
		world->addClothInstance(m_ClothInstance);
		for (int i = 0; i < m_ClothData->m_transformSetDefinitions.getSize(); ++i)
		{
			const hclTransformSetDefinition* tsdef = m_ClothData->m_transformSetDefinitions[i];
			Core::String		skelname(TEXT('.'));
			RagDoll*			skeleton;
			hclTransformSet*	tset = new hclTransformSet();
			
			skelname += tsdef->m_name;
			skelname += TEXT(".skeleton");
			skeleton = (RagDoll*) vixenscene->GetEngines()->Find(skelname, Group::FIND_DESCEND | Group::FIND_END);
			if (skeleton == NULL)
			{
				VX_WARNING(("ClothSkin: %s cannot find skeleton %s", GetName(), skelname));
				continue;
			}
			VX_ASSERT(skeleton->IsClass(VX_RagDoll));
			VX_ASSERT(tsdef->m_numTransforms == skeleton->GetNumBones());
			m_Skeletons.SetAt(i, skeleton);
			SetSkeleton(skeleton);
			skeleton->MakeLock();

			HavokPose bindpose(skeleton);
			bindpose.GetTransforms(tset);
			m_ClothInstance->setTransformSet(i, tset);
			VX_TRACE(ClothSim::Debug || ClothSkin::Debug, ("ClothSkin: %s linked to skeleton %s", GetName(), skeleton->GetName()));
		}
		ClothBufferUtil	makebuffers(input, output);
		makebuffers.instantiateBuffers();
		m_ClothData->removeReference();
		m_ClothData = HK_NULL;
		world->unlockAll();
	}
	return Engine::OnStart();
}


}	// end Vixen