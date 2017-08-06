#pragma once

namespace Vixen {

/*!
 * @file vxterrain.h
 * @brief Terrain classes
 *
 * Implements hit-testing of a point against a 3D terrain model.
 *
 * @author Dennis Crowley
 * @ingroup vixen
 *
 * @see vxquadtree.h vxterrcoll.h
 */

/*!
 * @class QuadTerrain
 * @brief Speeds up hit testing against terrain.
 *
 * It uses spatial sorting using quadtrees to "hit" the surface in
 * roughly O(logN) as opposed to the O(N) of "naive brute force".
 *
 * A quadtree terrain is created from a model at run-time.
 * It may be shared by multiple engines that need access to the same terrain.
 * This class is primarily intended for outdoor terrain in which the altitude 
 * changes, however, it may also be used to test if a point is inside a "floor plan."
 *
 * <B>Content Creation</B>
 *
 * Models for use by QuadTerrain must be a single mesh object.
 * Seperate pieces should be unified by using "attach" to make one object.  Note
 * that these seperate pieces do not actually need to be touching.
 *
 * The terrain when viewed from above should have no overlapping sections
 * or vertical sections.  For example, a box aligned with the axes is inapprpriate
 * for two reasons; One, the sides are vertical, and two, the top and the bottom
 * overlap in plan view.
 *	
 * @see TerrainCollider VXTerrain QuadTree QuadRoot
 */
class QuadTerrain : public QuadRoot
{
public:
	VX_DECLARE_CLASS(QuadTerrain);
	QuadTerrain(const Shape *model = NULL, int threshold = 5, int max_depth = 5);

	bool	HitInfo(Vec3 &location, Vec3 &normal);
	bool	HitTest(Vec3& test_location);
	bool	IsValid();
	bool	SetModel(const Shape*);
	void	SetCamera(const Camera*);

protected:
	// internal functions
	bool	HitTriangle(Vec3 &point, int v[]);
	bool	GetSurfaceData(Vec3 &loc, Vec3 &norm, int v[]);

	// override pure virtual function in QuadRoot
	bool	BoundCheck(const Box2 &bound, int index);

	bool	AddMesh(const TriMesh* ingeo, TriMesh* outgeo);

	// data members
	Ref<Camera>	m_camera;		// camera terrain is viewed thru
	Ref<Shape>	m_model;		// model to use for terrain
	VertexArray		m_vData;		// world coordinate vertices of model's meshes
	IntArray		m_triData;		// vertex indices for each triangle of each mesh
};

} // end Vixen