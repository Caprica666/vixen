#pragma once
#pragma managed(push, off)
/*!
 * @file vxcloth.h
 * @brief Havok cloth simulation wrappers.
 *
 * Encapsulates Havok cloth  related engines
 *
 * @author Nola Donato
 * @ingroup vixen
 */
#pragma once
class hclWorld;
class hclClothData;
class hclClothInstance;
class hclCollidable;
class hclNamedSetupMesh;
class hclTransformSet;
class hclUserSetupRuntimeDisplayData;
class hclClothSetupObject;
class hkJobQueue;
class hkJobThreadPool;
class hkxScene;

namespace Vixen {

/*!
 * @class ClothSim
 *
 * @brief Engine which contains one or more related cloth simulations.
 *
 * This class controls the overall properties of cloth simulation,
 * performs cloth setup and handles multithreading and load balancing
 * for cloth. All of the Vixen engines which relate to Havok cloth
 * must be children of the single ClothSim engine.
 *
 * @see Physics ClothSkin
 * @ingroup vixen
 */
class ClothSim : public Engine
{
public:
	VX_DECLARE_CLASS(ClothSim);
	ClothSim(hclWorld* world = NULL);
	~ClothSim();

	//! Establish ground plane for cloth collision.
	void				SetGroundPlane(const Vec4& groundplane);

	//! Get ground plane for cloth collision.
	const Vec4&			GetGroundPlane() const	{ return m_GroundPlane; }

	//! Get Havok ground collidable.
	hclCollidable*		GetGroundCollide() const{ return m_GroundCollidable; }

	//! Get the one and only ClothSim instance.
	static	ClothSim*	Get()					{ return s_OnlyOne; }

	//! Get the one and only Havok cloth world.
	static hclWorld*	GetWorld()				{ if (s_OnlyOne) return s_OnlyOne->m_ClothWorld; else return NULL; }
	
	//! Get the Havok thread pool for cloth simulation.	
	hkJobThreadPool*	GetThreadPool() const;

	hkxScene*			GetHavokScene() const	{ return m_Scene; }

	//! Shut down cloth simulation.
	static void			Shutdown();

//
// Overrides
//
	virtual bool		Eval(float);
	virtual	bool		OnStart();
	virtual	void		Empty();

protected:
	Plane				m_GroundPlane;
	hclWorld*			m_ClothWorld;
	hclCollidable*		m_GroundCollidable;
	hkJobQueue*			m_ClothJobs;
	hkxScene*			m_Scene;
	float				m_TimeStep;
	static ClothSim*	s_OnlyOne;
};

/*!
 * @class ClothSkin
 * Deforms a target mesh based on Havok cloth simulation.
 * A ClothSkin can only be created by loading a Havok physics file
 * which contains a cloth simulation.
 * Havok cloth simulation directly updates the vertices in the target mesh
 * when this engine is running. It must have a ClothSim engine as a parent.
 *
 * @see Physics ClothSim Skin RagDoll
 * @ingroup vixen
 */
class ClothSkin : public Skin
{
public:
	VX_DECLARE_CLASS(ClothSkin);
	ClothSkin();
	ClothSkin(hclClothData* data);
	~ClothSkin();

//
// Overrides
//
	virtual	void		SetActive(bool);
	virtual bool		Eval(float);
	virtual	bool		OnStart();
	virtual void		SetTarget(SharedObj*);
	virtual	void		Empty();
	virtual float		ComputeTime(float);

protected:
	void				UpdateTransforms();

	RefArray<Skeleton>	m_Skeletons;
	hclClothData*		m_ClothData;
	hclClothInstance*	m_ClothInstance;
};

/*!
 * Perform Havok cloth setup.
 * This function is called when loading a Havok physics file which contains cloth.
 */
extern ClothSkin* SetupCloth(Scene* vixenscene, hkxScene* havokscene, hclClothSetupObject* setupinfo);

}	// end Vixen

#pragma managed(pop)