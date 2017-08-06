#pragma once
#include "util/vxquadtree.h"
#include "util/vxterrain.h"

namespace Vixen {

/*!
 * @class TerrainCollider
 * @brief Constrains the camera to a fixed distance above a terrain mesh.
 *
 * All of the terrain must be in a single mesh. The mesh is spatially sorted
 * into an QuadTree used to quickly determine the distance from the ground
 * at a given point. The position of the target model is adjusted each frame to keep
 * it above the ground.
 *
 * @see QuadRoot QuadTree
 */
class TerrainCollider : public Engine
{
public:
	VX_DECLARE_CLASS(TerrainCollider);
	TerrainCollider();

	QuadTerrain*	BuildTerrain(const Shape *model, int thresh = 5, int max_depth = 5);
	void			SetTerrain(QuadTerrain *terrain);
	QuadTerrain*	GetTerrain() { return m_TerrainObject; }
	void			SetHeight(float h);

	// overrides
	virtual bool	Eval(float t);
	virtual bool    OnStart();
	virtual bool	Copy(const SharedObj*);
	virtual bool	Do(Messenger& s, int op);

	enum Opcode
	{
		TERRCOLL_SetHeight = Engine::ENG_NextOp,
		TERRCOLL_BuildTerrain,
		TERRCOLL_SetTerrain,
		TERRCOLL_NextOp = Engine::ENG_NextOp + 10,
	};

protected:
	Ref<QuadTerrain>	m_TerrainObject;
	bool                m_evalReady;
	Vec3				m_normal;
	float				m_height;
	Vec3				m_start;
};

} // end Vixen
