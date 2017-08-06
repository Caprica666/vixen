#include "vixen.h"
#include "physics/havok/vxphysics.h"
#include "physics/havok/vxcloth.h"

#include <Common/Base/KeyCode.h>
#include <Common/Base/hkBase.h>
#include <Common/Base/hkBase.h>
#include <Common/Base/Config/hkConfigVersion.h>
#include <Common/Base/Math/QsTransform/hkQsTransform.h>
#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>
#include <Common/SceneData/Graph/hkxNode.h>
#include <Common/SceneData/Scene/hkxScene.h>
#include <Animation/Animation/Rig/hkaSkeleton.h>

#include <Cloth/Setup/Execution/hclClothSetupExecution.h>
#include <Cloth/Setup/Execution/hclUserSetupRuntimeDisplayData.h>
#include <Cloth/Setup/Execution/hclUserSetupRuntimeSkeletonData.h>
#include <Cloth/Cloth/World/hclWorld.h>
#include <Cloth/Cloth/Cloth/hclClothInstance.h>
#include <Cloth/Cloth/Cloth/hclClothData.h>
#include <Cloth/Cloth/Collide/Shape/Plane/hclPlaneShape.h>
#include <Cloth/Cloth/Collide/hclCollidable.h>
#include <Cloth/Cloth/Container/hclClothContainer.h>
#include <Cloth/Cloth/SimCloth/hclSimClothInstance.h>
#include <Cloth/Cloth/Buffer/hclBufferDefinition.h>
#include <Cloth/Cloth/Buffer/hclBuffer.h>
#include <Cloth/Cloth/TransformSet/hclTransformSet.h>

#include "Physics/havok/math_interop.h"

#define NUM_CLOTH_THREADS 2	// TODO: figure out how to calculate this

namespace Vixen {

VX_IMPLEMENT_CLASSID(ClothSim, Engine, VX_ClothSim);
VX_IMPLEMENT_CLASSID(ClothSkin, Skin, VX_ClothSkin);

ClothSim*	ClothSim::s_OnlyOne = NULL;

/*!
 * @fn ClothSim::ClothSim()
 *
 * Constructs a Havok cloth simulation engine.
 */
ClothSim::ClothSim(hclWorld* world)
:	Engine(),
	m_ClothWorld(world),
	m_GroundPlane(0,0,0,0),
	m_GroundCollidable(NULL)
{
	if (world)
		world->addReference();
	m_TimeStep = 1.0f / 30.0f;
	SetControl(Engine::CONTROL_CHILDREN | CHILDREN_FIRST);
}


ClothSim::~ClothSim()
{
	Empty();
}

/*!
 * @fn void ClothSim::Empty()
 * Dereference the Haovk data structures used in cloth simulation
 * and dereference all child cloth skins.
 *
 * @see Physics::Clear ClothSkin::Empty
 */
void ClothSim::Empty()
{
	Engine::Empty();
	if (m_GroundCollidable && Physics::IsRunning())
		m_GroundCollidable->removeReference();
	m_GroundCollidable = NULL;
	m_ClothJobs = NULL;
	if (m_ClothWorld)
	{
		VX_TRACE(Physics::Debug || ClothSim::Debug, ("ClothSim: destroy cloth world %s", GetName()));
		if (Physics::IsRunning())
			m_ClothWorld->removeReference();
		m_ClothWorld = NULL;
	}
}

void ClothSim::Shutdown()
{
	if (s_OnlyOne)
	{
		s_OnlyOne->Empty();
		s_OnlyOne->Delete();
		s_OnlyOne = NULL;
	}
}

hkJobThreadPool* ClothSim::GetThreadPool() const
{
	Physics*	physics = (Physics*) Parent();
	
	if (physics && physics->IsClass(VX_Physics))
		return physics->GetThreadPool();
	return NULL;
}

void ClothSim::SetGroundPlane(const Vec4& gp)
{
	if (gp == m_GroundPlane)
		return;
	if (gp.IsEmpty())
	{
		if (m_GroundCollidable)
		{
			m_GroundCollidable->removeReference();
			m_GroundCollidable = NULL;
		}
		return;
	}
	hclPlaneShape* plane = new hclPlaneShape(hkVector4(gp.x, gp.y, gp.z, gp.w));
	m_GroundCollidable = new hclCollidable(plane);
	plane->removeReference();
}

/*
 * The root ClothSim engine is the only one which steps the cloth.
 * Child ClothSim engines are used for grouping only.
 */
bool ClothSim::OnStart()
{
	if (s_OnlyOne == NULL)
	{
		s_OnlyOne = this;
		s_OnlyOne->AddRef();
	}
	if (s_OnlyOne != this)
		return true;
	if (m_ClothWorld == NULL)
	{
		hclWorld::Options worldOptions;
		worldOptions.m_onTheFlyNotifications = true;
		m_ClothWorld = new hclWorld(worldOptions);
		m_ClothWorld->addReference();
		VX_TRACE(Physics::Debug || ClothSim::Debug, ("ClothSim: create cloth world %s", GetName()));
	}
	if (m_ClothJobs == NULL)
	{
		hkJobThreadPool* threads = GetThreadPool();

		if (threads)
		{
			hkJobQueueCinfo	info;

			info.m_jobQueueHwSetup.m_numCpuThreads = 1 + threads->getNumThreads();
			m_ClothJobs = new hkJobQueue(info);
			hclWorld::registerWithJobQueue(m_ClothJobs);
			VX_TRACE(Physics::Debug || ClothSim::Debug,
					("ClothSim: %s creating job queue %d threads", GetName(), info.m_jobQueueHwSetup.m_numCpuThreads));
		}
	}
	return true;
}

bool ClothSim::Eval(float t)
{
	int			bufferSize;	
	char*		buffer;
	hkJobThreadPool* threads;

	if ((m_ClothWorld == NULL) || (m_TimeStep == 0))
		return true;
	if (!m_ClothWorld->getOnTheFlyNotificationsEnabled())
		m_ClothWorld->triggerPreStepNotifications();
	threads = GetThreadPool();
	if (m_ClothJobs && threads)
	{
		int		numthreads = 1 + threads->getNumThreads();
		bufferSize = m_ClothWorld->calcStepBufferSize(numthreads);
		buffer = hkAllocate<char>(bufferSize, HK_MEMORY_CLASS_DEMO);
#if HAVOK_SDK_VERSION_NUMBER >= 20120200
		m_ClothWorld->startStepMultiThreaded(m_ClothJobs, numthreads, m_TimeStep, buffer);
#else
		m_ClothWorld->addClothJobs(m_ClothJobs, numthreads, m_TimeStep, buffer);
#endif
		threads->processAllJobs(m_ClothJobs);
		m_ClothJobs->processAllJobs();
		threads->waitForCompletion();
#if HAVOK_SDK_VERSION_NUMBER >= 20120200
		m_ClothWorld->finishStepMultiThreaded(m_TimeStep);
#endif
	}
	else
	{
		bufferSize = m_ClothWorld->calcStepBufferSize(1);	
		buffer = hkAllocate<char>(bufferSize, HK_MEMORY_CLASS_DEMO);
#if HAVOK_SDK_VERSION_NUMBER >= 20120200
		m_ClothWorld->stepSingleThreaded(m_TimeStep, buffer);
#else
		m_ClothWorld->singleThreadedStep(m_TimeStep, buffer);
#endif
	}
	hkDeallocate(buffer);
	if (!m_ClothWorld->getOnTheFlyNotificationsEnabled())
		m_ClothWorld->triggerPostStepNotifications();
	return true;
}

ClothSkin::ClothSkin()
: m_ClothData(NULL)
{
	m_Control = 0;
}

ClothSkin::ClothSkin(hclClothData* data)
: m_ClothData(data)
{
	if (data)
		data->addReference();
}

ClothSkin::~ClothSkin()
{
	Empty();
}

/*!
 * @fn void ClothSkin::Empty()
 * Dereference the Haovk data structures attached to this cloth skin
 * and dereference all child engines.
 *
 * @see Physics::Clear ClothSim::Empty
 */
void ClothSkin::Empty()
{
	if (Physics::IsRunning())
	{
		if (m_ClothInstance)
			m_ClothInstance->removeReference();
		if (m_ClothData)
			m_ClothData->removeReference();
	}
	m_ClothInstance = NULL;
	m_ClothData = NULL;
	Skin::Empty();
}

void	ClothSkin::SetTarget(SharedObj* obj)
{
	Engine::SetTarget(obj);
}

void ClothSkin::SetActive(bool active)
{
	hclWorld* world = ClothSim::GetWorld();

	if (world && m_ClothInstance)
	{
		if (active)
		{
			if (!IsActive())
				world->addClothInstance(m_ClothInstance);
		}
		else if (IsActive())
			world->removeClothInstance(m_ClothInstance);
	}
	Skin::SetActive(active);
}

bool	ClothSkin::Eval(float t)
{
	UpdateTransforms();
	return Engine::Eval(t);
}

void ClothSkin::UpdateTransforms()
{
	for (int i = 0; i < m_ClothInstance->m_transformSets.getSize(); ++i)
	{
		hclTransformSet*	tset = m_ClothInstance->m_transformSets[i];
		const Skeleton*		skeleton = m_Skeletons.GetAt(i);
		const HavokPose*	pose;
		
		if (skeleton == NULL)
			continue;
		pose = (const HavokPose*) skeleton->GetPose();
		if (pose == NULL)
			continue;
		VX_ASSERT(pose->IsKindOf(&HavokPose::ClassInfo));
		pose->GetTransforms(tset);
	}
}

}	// end Vixen