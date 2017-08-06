#include "vixen.h"
#include "physics/havok/vxphysics.h"
#include "physics/havok/vxcloth.h"
#include <Common/Base/KeyCode.h>
#include <Common/Base/KeyCode.cxx>

#define HK_EXCLUDE_FEATURE_CompoundShape
#define INCLUDE_HAVOK_PHYSICS_CLASSES 
#define INCLUDE_HAVOK_ANIMATION_CLASSES
#define INCLUDE_HAVOK_CLOTH_CLASSES

#define HK_FEATURE_REFLECTION_CLOTH_SETUP
#define HK_CLASSES_FILE <Common/Serialize/ClassList/hkKeyCodeClasses.h>

#include <Common/Base/Config/hkProductFeatures.cxx>

#include <Common/Base/Reflection/Registry/hkTypeInfoRegistry.h>
#include <Common/Base/System/Io/IStream/hkIstream.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Serialize/Util/hkLoader.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#if HAVOK_SDK_VERSION_NUMBER >= 20130000
#include <Physics2012/Utilities/Serialize/hkpPhysicsData.h>
#include <Physics2012/Dynamics/hkpDynamics.h>
#include <Physics2012/Dynamics/World/hkpWorld.h>
#include <Physics2012/Dynamics/Entity/hkpEntity.h>
#include <Physics2012/Dynamics/Entity/hkpRigidBody.h>
#include <Physics2012/Collide/Filter/Group/hkpGroupFilter.h>
#include <Physics2012/Collide/Filter/Group/hkpGroupFilterSetup.h>
#include <Physics2012/Collide/Dispatch/hkpAgentRegisterUtil.h>
#include <Physics2012/Utilities/Collide/Filter/GroupFilter/hkpGroupFilterUtil.h>
#else
#include <Physics/Utilities/Serialize/hkpPhysicsData.h>
#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpEntity.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Collide/Filter/Group/hkpGroupFilter.h>
#include <Physics/Collide/Filter/Group/hkpGroupFilterSetup.h>
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>
#include <Physics/Utilities/Collide/Filter/GroupFilter/hkpGroupFilterUtil.h>
#endif

#include "Physics/havok/math_interop.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Physics, Engine, VX_Physics);

hkMemoryRouter* Physics::t_Memory;

static const TCHAR* opnames[] = {
		TEXT("SetFileName"),
};


static void HK_CALL havokError(const char* msg, void* userArgGivenToInit)
{
	VX_ERROR_RETURN(("%s", msg));
}

Ref<Physics>	Physics::s_OnlyOne;
bool			Physics::s_IsRunning = false;

hkMemoryRouter*	Physics::GetMemory()		{ return hkMemoryRouter::getInstancePtr(); }

/*!
 * @fn Physics::Physics()
 *
 * Constructs a physics world whose children are Havok physics systems.
 */
Physics::Physics(hkpWorld* world)
: Engine(), m_World(world), m_TimeStep(0), m_PhysData(NULL), m_NumThreads(0)
{
	m_Threads = NULL;
	m_JobQueue = NULL;
	if (world)
		world->addReference();
}

Physics::Physics(hkpPhysicsData* physdata)
: Engine(), m_World(NULL), m_PhysData(physdata), m_TimeStep(0), m_NumThreads(0)
{
	m_Threads = NULL;
	m_JobQueue = NULL;
	if (physdata)
		physdata->addReference();
}

Physics::~Physics()
{
	KillLock();
	Clear();
}

void Physics::Clear()
{
	if (s_OnlyOne == NULL)
		return;
	s_OnlyOne = NULL;
	ObjectLock	lock(this);
	Stop(CONTROL_CHILDREN);
	if (m_PhysData)
	{
		m_PhysData->removeReference();
		m_PhysData = NULL;
	}
	if (m_Threads)
	{
		if (m_Threads->isProcessing())
			m_Threads->waitForCompletion();
	}
	GroupIterNotSafe<Engine> iter(this, Group::DEPTH_FIRST);
	Engine*			 e;
	while (e = (Engine*) iter.Next())
		e->Empty();
	Empty();
	if (m_World)
	{
		m_World->removeReference();
		m_World = NULL;
	}
	if (m_JobQueue)
	{
		delete m_JobQueue;
		m_JobQueue = NULL;
	}
	if (m_Threads)
	{
		delete m_Threads;
		m_Threads = NULL;
	}
}


/*
 * bool Physics::OnEvent(Event* ev)
 * Handles loading of scene files containing physics.
 * Vixen only permits one Physics instance per scene to control
 * the Havok thread pool. When a physics file is loaded, this
 * function is called to process the load event.
 * It removes all the physics engines from the scene and adds them
 * to this physics world.
 */
bool Physics::OnEvent(Event* ev)
{
	Scene*			scene;
	Ref<Physics>	physroot;

	VX_ASSERT(this == s_OnlyOne);
	if (ev->Code != Event::LOAD_SCENE)
		return false;
	scene = (Scene*) (SharedObj*) ((LoadSceneEvent*) ev)->Object;
	if (!scene || !scene->IsClass(VX_Scene))
		return true;
	physroot = (Physics*) scene->GetEngines();		// is the root a physics engine?
	if (physroot.IsNull() || !physroot->IsClass(VX_Physics))
		return true;

	VX_TRACE(Debug, ("Physics::OnEvent LOAD_SCENE %s", ((LoadSceneEvent*) ev)->FileName));
	ObjectLock		lock1((Physics*) physroot);
	ObjectLock		lock2(this);
	hkpWorld*		physworld = physroot->GetWorld();
	hkpPhysicsData*	physdata = physroot->GetPhysData();
	Ref<Engine>		eng = physroot->First();

	/*
	 * Transfer Havok physics systems from this physics root to the active world
	 */
	if (physworld)
	{
		if (m_World == NULL)
			m_World = physworld;	// just use this world
		else
			AddToWorld(physworld);	// transfer physics systems from new world to this world
	}
	else if (physdata)
	{
		if (m_World == NULL)
			m_World = physroot->CreateWorld(physdata);
		else
			AddToWorld(physdata);
	}
	else if (scene->GetModels() == NULL)
		return true;
	while (!eng.IsNull())					// transfer all child engines to this parent
	{
		Ref<Engine> next = eng->Next();

		VX_TRACE(Debug > 1, ("\t%s -> %s", GetName(), eng->GetName()));
		eng->Remove(Group::UNLINK_NOFREE);	// remove engine from loaded scene
		if (eng->IsClass(VX_ClothSim))
		{
			ClothSim* mastercloth = ClothSim::Get();
			if ((mastercloth != NULL) && (mastercloth != eng))
			{
				mastercloth->Append(eng);
				eng = next;
				continue;
			}
		}
		Append(eng);						// add it to this engine
		eng = next;							// get next child
	}
	scene->SetEngines(this);
	//World3D::Get()->GetLoader()->Unload(((LoadEvent*) ev)->FileName);
	return true;
}

bool Physics::Startup()
{
	hkMemoryRouter* router = GetMemory();
	FileLoader*	 load = World3D::Get()->GetLoader();

	if (router != NULL)
		return true;
	VX_TRACE(Debug, ("Physics:Startup initialize Havok memory system, enable HKT file load"));
	router = hkMemoryInitUtil::initDefault(hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(200000));
	hkBaseSystem::init(router, havokError);
	load->SetFileFunc(TEXT("hkx"), &Physics::ReadScene, Event::LOAD_SCENE);
	load->SetFileFunc(TEXT("hkt"), &Physics::ReadScene, Event::LOAD_SCENE);
	s_IsRunning = true;
	return true;
}

void Physics::Shutdown(int threadtype)
{
	VX_TRACE(Debug, ("Physics:Shutdown shut down Havok"));

	if ((threadtype & SCENE_SimThread) && !s_OnlyOne.IsNull())
	{
		s_OnlyOne->Clear();
		ClothSim::Shutdown();
		s_IsRunning = false;
		if (t_Memory != NULL)
		{
			hkMemoryRouter* router = t_Memory;
			hkBaseSystem::quitThread();
			hkMemorySystem::getInstance().threadQuit(*router);
			free(router);
			t_Memory = NULL;
		}
	}
	if (threadtype & SCENE_UIThread)
	{
		if (GetMemory())
		{
			hkBaseSystem::quit();
			hkMemoryInitUtil::quit();
		}
	}
}

/*!
 * @fn void Physics::SetFileName(const TCHAR* fname)
 * @param fname	name of the physics file to load
 *
 * Sets the name of the physics file to load. This function also
 * attempts to load the file.
 *
 */
void Physics::SetFileName(const TCHAR* fname)
{
	if (fname)
	{
		m_FileName = fname;
		World3D::Get()->LoadAsync(fname, this);
	}
}


bool Physics::Do(Messenger& s, int op)
{
	Vec3		p;
	Box3		b;
	TCHAR		fname[VX_MaxPath];

	switch (op)
	{
		case PHYSICS_SetFileName:
		s >> fname;
		SetFileName(fname);
		break;

		return Engine::Do(s, op);
	}
	return true;
}

DebugOut& Physics::Print(DebugOut& dbg, int opts) const
{
	Vec3	g;

	if ((opts & PRINT_Attributes) == 0)
		return Engine::Print(dbg, opts);
	Engine::Print(dbg, opts & ~PRINT_Trailer);
	if (!m_FileName.IsEmpty())
		endl(dbg << "<attr name='FileName'>" << m_FileName << "</attr>");
	return Engine::Print(dbg, opts & PRINT_Trailer);
}

bool Physics::Copy(const SharedObj* src_obj)
{
	ObjectLock dlock(this);
	ObjectLock slock(src_obj);
	if (!Engine::Copy(src_obj))
		return false;

	const Physics* src = (const Physics*) src_obj;
	if (src->IsClass(VX_Physics))
    {
		m_FileName = src->m_FileName;
    }
    return true;
}

int Physics::Save(Messenger& s, int opts) const
{
	int32		h;
	Vec3		p;
	h = Engine::Save(s, opts);

	if (h <= 0)
		return h;
	if (!m_FileName.IsEmpty())
		s << OP(VX_Physics, PHYSICS_SetFileName) << m_FileName << p;
	return h;
}

/*
 * Force first time initialization before any other Havok engines run
 */
float Physics::ComputeTime(float t)
{
	hkMemoryRouter*	router = t_Memory;

	if (router == NULL)
		Init();
	return Engine::ComputeTime(t);
}

void Physics::Init()
{
	VX_TRACE(Debug, ("Physics:Init initialize Havok physics thread"));
	void*			hack = malloc(sizeof(hkMemoryRouter));
	hkMemoryRouter*	router = new (hack) hkMemoryRouter();
	hkCpuJobThreadPoolCinfo	threadinfo;
	hkJobQueueCinfo			jobinfo;

	t_Memory = router;
	threadinfo.m_numThreads =  m_NumThreads;
	threadinfo.m_threadName = "hkPhysicsThread";
	jobinfo.m_jobQueueHwSetup.m_numCpuThreads = 1 + m_NumThreads;

	hkMemorySystem::getInstance().threadInit(*router, "hkSimThread");
	hkBaseSystem::initThread(router);
	VX_ASSERT(m_Threads == NULL);
	m_Threads = new hkCpuJobThreadPool(threadinfo);
	m_JobQueue = new hkJobQueue(jobinfo);
	m_World->registerWithJobQueue(m_JobQueue);
	VX_ASSERT(s_OnlyOne == NULL);
	s_OnlyOne = this;
	s_OnlyOne->AddRef();
	MakeLock();
	GetMessenger()->Observe(this, Event::LOAD_SCENE, NULL);
	m_TimeStep = 1.0f / 60.0f;
}


hkpWorld* Physics::CreateWorld(hkpPhysicsData* physdata)
{
	hkMemoryRouter*	router = t_Memory;
	hkpWorldCinfo	worldInfo;
	hkpWorld*		world;

	worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM);
	worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_CONTINUOUS;
//	worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
	worldInfo.m_gravity.set(0.0f, -9.8f, 0.0f);
	worldInfo.setBroadPhaseWorldSize(1000.0f);

	VX_ASSERT(router);	
	if (physdata)
	{
		hkpWorldCinfo*	temp =  physdata->getWorldCinfo();

		VX_TRACE(Debug, ("Physics::CreateWorld %s from hkpPhysicsData", GetName()));
		if (temp)
		{
			if (m_NumThreads > 0)
				temp->m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
			else
				temp->m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_CONTINUOUS;
			world = physdata->createWorld(true);
		}
		else
		{
			if (m_NumThreads > 0)
				worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
			physdata->setWorldCinfo(&worldInfo);
			world = physdata->createWorld(true);
		}
	}
	else
	{
		VX_TRACE(Debug, ("Physics::CreateWorld %s Default", GetName()));
		world = new hkpWorld(worldInfo);
	}
	if (world)
	{
		hkpGroupFilter* filter = new hkpGroupFilter();
		hkpGroupFilterSetup::setupGroupFilter(filter);
		world->lock();
		world->setCollisionFilter(filter);
		world->updateCollisionFilterOnWorld(HK_UPDATE_FILTER_ON_WORLD_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS); 
		world->unlock();
		filter->removeReference();
		m_TimeStep = 1.0f / 60.0f;
	}
	return world;
}


/*
 * Transfer all the physics systems from the hkpWorld in the new scene
 * to this hkpWorld. This puts all Vixen physics systems under a common
 * root engine so they will use a common thread pool.
 */
void Physics::AddToWorld(hkpPhysicsData* physdata)
{
	const hkArray<hkpPhysicsSystem*>& systems = physdata->getPhysicsSystems();

	m_World->lock();
	for (int i = 0; i < systems.getSize(); ++i)
	{
		hkpPhysicsSystem* physys = systems[i];

		VX_TRACE(Physics::Debug, ("Physics::AddToWorld %s adding physics system", GetName(), physys->getName()));
		physys->addReference();
		physdata->removePhysicsSystem(i);
		m_World->addPhysicsSystem(physys);
		physys->removeReference();
	}
	m_World->unlock();
}

void Physics::AddToWorld(hkpWorld* world)
{
	hkArray<hkpPhysicsSystem*> systems;

	m_World->lock();
	world->getWorldAsSystems(systems);
	for (int i = 0; i < systems.getSize(); ++i)
	{
		hkpPhysicsSystem* physys = systems[i];

		VX_TRACE(Physics::Debug, ("Physics::AddToWorld %s adding physics system", GetName(), physys->getName()));
		physys->addReference();
		world->removePhysicsSystem(physys);
		m_World->addPhysicsSystem(physys);
		physys->removeReference();
	}
	m_World->unlock();
}

void Physics::Compute(float t)
{
	Lock();
	bool		dont_control_kids = !(m_Control & CONTROL_CHILDREN);
	bool		eval_says_do_my_kids = true;
	float		eval_t = ComputeTime(t);
	Unlock();

	GroupIterNotSafe<Engine> iter(this, CHILDREN);
	RagDoll*	ragdoll;
	while (ragdoll = (RagDoll*) iter.Next())
	{
		if (ragdoll->IsClass(VX_RagDoll))
		{
			ragdoll->OnBoneChanged();
			if (ragdoll->GetControl() & RagDoll::DYNAMIC)
				ragdoll->DrivePhysics();
		}
	}
	if (eval_t >= 0)
		eval_says_do_my_kids = DoEval(eval_t);	// step physics
	if ((eval_says_do_my_kids && IsActive()) || dont_control_kids)
		ComputeChildren(t);						// update shape transforms from physics
}

bool Physics::Eval(float t)
{
	hkpWorldCinfo cinfo;

	if (m_TimeStep == 0)
		return false;
	if (m_World == NULL)
		m_World = CreateWorld(m_PhysData);
	if ((m_NumThreads > 0) && m_Threads)
	{
		VX_ASSERT(m_JobQueue);
		m_World->initMtStep(m_JobQueue, m_TimeStep);
		m_Threads->processAllJobs(m_JobQueue);		// Tell the worker threads to start processing all jobs
		m_JobQueue->processAllJobs( );				// Also use this thread to process all jobs
		m_Threads->waitForCompletion();				// Wait for the worker threads to complete
		m_World->finishMtStep(m_JobQueue, m_Threads);	// Finish the step, including performing continuous simulation
	}
	else
		m_World->stepDeltaTime(m_TimeStep);
	return true;
}

}	// end Vixen