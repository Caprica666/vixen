/*!
 * @class VXSceneOptimize
 * 
 * Optimizer for a scene graph that can make appearance
 * and geometry usage more efficient. It can also spatially sort
 * static scenery for faster rendering, picking and collision detection.
 *
 * @see VXOctree
 */
#pragma once

class VXSceneOptimize
{
public:

	//! Optimizes the vertex usage among multiple triangle meshes.
	void			OptimizeVerts(PSSurface* surf);

	//! Optimize appearance usage for a hierarchy.
	VXAppearances*	UniqueAppearances(VXModel* root);

	//! Build static hierarchy from objects in input hierarchy which are not dynamic
	VXModel*		OptimizeStatic(VXModel* root);

protected:
	VXModel*		BuildStatic(VXModel* root, const VXObjArray* statmods);
	bool			CollectStatic(const VXModel* root, const VXMatrix* mtx, int level);

	VXObjArray	m_StatRoots;	// roots of static hierarchies
};


