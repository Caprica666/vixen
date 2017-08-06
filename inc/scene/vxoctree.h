#pragma once
namespace Vixen {

/*!
 * @brief Scene graph spatially sorted as an Octree.
 *
 * @internal
 */
class Octree : public Shape
{
public:
	Octree(const Box3* bound = NULL);
	~Octree() {}
	VX_DECLARE_CLASS(Octree);

	//! Build octree from array of surfaces
	bool			BuildTree(ObjArray& surfaces, const Box3& bound);

	//! Returns  true if input mesh within bounding box of this node
	bool			IsInside(const Geometry* mesh) const;

	//! Find octree descendant which contains input point
	const Octree*	Find(const Vec3& loc) const;

	virtual	bool	Hit(Ray& ray, float& distance) const; 
	virtual	intptr	Cull(const Matrix* trans, Scene* scene);

protected:
	//! Subdivide space and build tree below this node
	bool			Subdivide(ObjArray& meshes, float limitvol, int level);

	//! Make a child node with the given bounds
	Octree*			MakeNode(ObjArray& meshes, const Box3* bound, int level, const TCHAR* nameprefix, float limitvol, int count);

	//! Collect meshes from the array that are within the given volume and adds them to this node
	int				CollectMeshes(const ObjArray& inmeshes, ObjArray& outmeshes, Box3& minvol, bool remove = false);

	//! Collect meshes from the given surface within the bounds of this node
	bool			CalcMinVol(const Geometry* mesh, Box3& minvol);
};

} // end Vixen