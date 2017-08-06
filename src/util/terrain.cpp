#include "vixen.h"
#include "vxutil.h"

namespace Vixen {
VX_IMPLEMENT_CLASSID(QuadTerrain, SharedObj, VX_Terrain);


/*!
 * @fn QuadTerrain::QuadTerrain(const Model *model, int threshold, int maxdepth)
 * @param model			model describing navigable terrain
 * @param threshold		some way to tune the quadtree
 * @param maxdepth		maxmimum depth of collision hierarchy??
 *
 * The constructor takes a pointer to the model which will be the navigable terrain.
 * In the past, we have used a naming convention in the content so that we can identify the 
 * model at run time.  The model is only used by the
 * QuadTerrain to build its own database, so the model's reference count is not 
 * affected.  Also, the Model object may be destroyed without affecting the 
 * QuadTerrain.
 *
 * You may also supply values to the constructor for  threshold and  max_depth.
 * These parameters affect the topology of the resulting quadtree and can be
 * used to tune the efficiency of the object (memory vs. speed).  The default
 * values should be sufficient in most cases.
 *
 * @see QuadTerrain::IsValid
 */
QuadTerrain::QuadTerrain(const Shape *model, int threshold, int maxdepth)
: QuadRoot(threshold, maxdepth), m_vData(VertexPool::LOCATIONS)
{
	if (model)
		SetModel(model);
}

/*!
 * @fn bool QuadTerrain::SetModel(const Shape* shape)
 * @param shape	shape to use for terrain
 *
 * Establishes the model to be used for constructing the terrain.
 * This routine examines the meshes of this shape's surface and extracts
 * the triangle indices and vertices to later construct a quad tree.
 *
 * @return  true if model successfully converted to terrain, else  false
 */
bool QuadTerrain::SetModel(const Shape* shape)
{
	TriMesh* outmesh;
	Shape *child;

	m_model = shape;
	if (shape == NULL)
		return false;
	/*
	 * Make one big mesh out of all the terrain meshes in the model
	 */
	outmesh = new TriMesh();
	Shape::Iter iter(shape, Group::DEPTH_FIRST);
	TriMesh* trimesh = (TriMesh*) shape->GetGeometry();
	AddMesh(trimesh, outmesh);
	while (child = (Shape*) iter.Next())
	{
		if (child->IsClass(VX_Shape))
			AddMesh(trimesh, outmesh);
	}
	Matrix totalTransform;
	Box3   bound;

	m_model->TotalTransform(&totalTransform);	// Get total transform from model
	*(outmesh->GetVertices()) *= totalTransform;// map vertices into world coordinates
	m_model->GetBound(&bound, Model::WORLD);			// get model's bounding box to use for quadtree bound
	SetBound(bound);
	return m_Valid = BuildTree();
}

bool QuadTerrain::AddMesh(const TriMesh* ingeo, TriMesh* outgeo)
{
	intptr		current_offset = outgeo->GetNumVtx();
	const VertexArray* vtxSource;

	if ((ingeo == NULL) || !ingeo->IsClass(VX_Triangles))
		return false;
	vtxSource = ingeo->GetVertices();
	if (!vtxSource)
		return false;
	current_offset = outgeo->GetNumVtx();			// new vertex offset

	VertexPool::ConstIter citer(vtxSource);		// add vertices from mesh
	const float* p;								// to our combined vertex list
	while (p = citer.Next())
		outgeo->AddVertices(p, 1);

	TriMesh::TriIter	triter(ingeo);			// add mesh indices to combined list
	intptr	i0, i1, i2;
	while (triter.Next(i0, i1, i2))				// for each triangle
	{
		outgeo->AddIndex(i0 + current_offset);	// save indices, adjust for vertex offset
		outgeo->AddIndex(i1 + current_offset);
		outgeo->AddIndex(i2 + current_offset);
	}
	return true;
}

#define MAKE_LINE(p0x,p0y,p1x,p1y,L)\
    (L)->x=(p1y)-(p0y);\
    (L)->y=(p0x)-(p1x);\
    (L)->z=(p0x)*(L)->x+(p0y)*(L)->y;

#define POINT_TO_LINE(px,py,L)\
    ((px)*(L)->x+(py)*(L)->y - (L)->z)


bool QuadTerrain::BoundCheck(const Box2 &box, int index) 
{ 
	//        from index, extract vertices from m_vData using m_triData.
	//        Apply Planify to these verts, then test the resulting planar
	//        triangle against the bound.

	Vec3 line;
	bool test;
	int i, ii;

	if (index > m_Size || index < 0) return false;
	index *= 3;

	//--- get triangle vertices in "plane of terrain" ---
	Vec2 tv[3];
	VertexPool::ConstIter iter(m_vData);
	for (i=0; i<3; i++)
	{
		int t = m_triData.GetAt(index + i);
		const Vec3* p = iter.GetLoc(t);
		VX_ASSERT(p);
		tv[i] = Planify(*p);
	}

    //-- for each box edge --
    //-- 0 --
    MAKE_LINE(box.min.x ,box.min.y ,box.min.x ,box.max.y ,&line);
    test = false;
    //-- for each triangle vertex --
    for(i=0;i<3;i++)
    {
        //-- if vertex inside or on edge
        if (POINT_TO_LINE(tv[i].x, tv[i].y, &line)>=0)
        {
            test = true;
            break;
        }
    }
    if (test == false) return false;
    //-- 1 --
    MAKE_LINE(box.min.x ,box.max.y ,box.max.x ,box.max.y ,&line);
    test = false;
    //-- for each triangle vertex --
    for(i=0;i<3;i++)
    {
        //-- if vertex inside
        if (POINT_TO_LINE(tv[i].x, tv[i].y, &line)>0) // don't include line here
        {
            test = true;
            break;
        }
    }
    if (test == false) return false;
    //-- 2 --
    MAKE_LINE(box.max.x ,box.max.y ,box.max.x ,box.min.y ,&line);
    test = false;
    //-- for each triangle vertex --
    for(i=0;i<3;i++)
    {
       //-- if vertex inside
        if (POINT_TO_LINE(tv[i].x, tv[i].y, &line)>0) // don't include line here
        {
            test = true;
            break;
        }
    }
    if (test == false) return false;
    //-- 3 --
    MAKE_LINE(box.max.x ,box.min.y ,box.min.x ,box.min.y ,&line);
    test = false;
    //-- for each triangle vertex --
    for(i=0;i<3;i++)
    {
        //-- if vertex inside or on edge
        if (POINT_TO_LINE(tv[i].x, tv[i].y, &line)>=0)
        {
            test = true;
            break;
        }
    }
    if (test == false) return false;
    //-- for each triangle edge --
    for(i=0;i<3;i++)
    {
    ii = (i+1)%3;
    MAKE_LINE(tv[i].x , tv[i].y , tv[ii].x , tv[ii].y ,&line);
    test = false;
        //-- for each box vertex --
        //-- 0 --
        //-- if vertex inside or on edge --
        if (POINT_TO_LINE(box.min.x, box.min.y, &line)>=0)
        {
            test = true;
            goto Cont00;
        }
        //-- 1 --
        //-- if vertex inside or on edge --
        if (POINT_TO_LINE(box.min.x, box.max.y, &line)>=0)
        {
            test = true;
            goto Cont00;
        }
        //-- 2 --
        //-- if vertex inside or on edge --
        if (POINT_TO_LINE(box.max.x, box.max.y, &line)>=0)
        {
            test = true;
            goto Cont00;
        }
        //-- 3 --
        //-- if vertex inside or on edge --
        if (POINT_TO_LINE(box.max.x, box.min.y, &line)>=0)
        {
            test = true;
        }
        Cont00:
        if (test == false) return false;
    }
	return true;
}

bool QuadTerrain::HitTriangle(Vec3 &point, int v[])
{
	int i, ii;
	bool test;
	static Vec3 line;  //--- avoid calling c'tor every call
	static Vec2 loc2D; //--- ditto

	//--- get triangle vertices in "plane of terrain" ---
	Vec2 tv[3];
	VertexPool::ConstIter iter(m_vData);
	for (i=0; i<3; i++)
		tv[i] = Planify(*iter.GetLoc(v[i]));

	loc2D = Planify(point); //--- Inefficient !!!! only needs to be called once for whole list.

	//--- test if point is inside all three edges ---
	test = true;
	for (i=0; i<3; i++)
	{
		ii = (i+1)%3;
		MAKE_LINE(tv[i].x, tv[i].y, tv[ii].x, tv[ii].y, &line);
		if (POINT_TO_LINE(loc2D.x, loc2D.y, &line) < 0)
		{
			test = false;
			break;
		}
	}
	return test;
}


bool QuadTerrain::GetSurfaceData(Vec3 &loc, Vec3 &norm, int v[])
{
	//--- TODO: ---
	//        adjust loc to the surface of the triangle given by
	//        the indices in v, and fill norm with the normal.
	//        If there is any way to fail, return false on failure.

	//--- get triangle vertices ---
	Vec3 tv[3];
	VertexPool::ConstIter iter(m_vData);
	for (int i=0; i<3; i++)
		tv[i] = *iter.GetLoc(v[i]);

	//--- get edge vectors from first vertex ---
	Vec3 V0(tv[1] - tv[0]);
	Vec3 V1(tv[2] - tv[0]);

	//--- get normal to triangle ---
	norm = V0.Cross(V1);
	norm.Normalize();

	//--- normal vector and w together define a plane ---
	float w = norm.Dot(tv[0]);

	//--- NOTE:
	//        This section of code hard wires the y-axis to be the vertical

	if (norm.y != 0) //--- vertical terrain if norm.y == 0 ---
	{
		loc.y = (w - norm.x * loc.x - norm.z * loc.z) / norm.y; 
	}

	return true;
}

/*++++
 *
 * @fn bool QuadTerrain::HitInfo(Vec3 &loc, Vec3 &norm)
 * @param location	(input and output) location in world space to be tested.
 *					If a hit is detected, this point's Y-value will be
 *					modified to move the point to the level of the terrain.
 *					If a hit is not detected, the value will be unaffected.
 * @param normal	if a hit is detected, this will be the
 *					normal of the surface at  location.
 *
 * Performs a hit test on a location in world space and supplies
 * the user with surface information (location and normal).
 *
 * @return  true if point overlaps with terrain in plan view, else  false
 *
 * @see TerrainCollider QuadTerrain::HitTest
 */
bool QuadTerrain::HitInfo(Vec3 &loc, Vec3 &norm)
{
	int			v[3], tmp, n; 
	const IntArray* hitlist = GetHitList(loc);

	if (hitlist == NULL)
		return false;
	n = (int) hitlist->GetSize();
	for (int i = 0; i < n; i++)
	{
		tmp = hitlist->GetAt(i) * 3;		// get an array of indices to the triangle verts

		v[0] = m_triData.GetAt(tmp++);
		v[1] = m_triData.GetAt(tmp++);
		v[2] = m_triData.GetAt(tmp);
		if (HitTriangle(loc, v))
		{
				//--- adjust the output parameters ---
			GetSurfaceData(loc, norm, v);
			return true;
		}
	}
	return false;
}


/*!
 * @fn bool QuadTerrain::HitTest(Vec3& test_loc)
 * @param test_loc	location to be tested in world space.
 *
 * Tests if the location in world space is within the  floor  plan
 * of the terrain.  No other surface information is generated,
 * so this function is faster than  HitTerrain if this is
 * all the information you need.
 *
 * @return  true if point overlaps with terrain in plan view, else  false
 *
 * @see TerrainCollider
 */
bool QuadTerrain::HitTest(Vec3& test_loc)
{
	const IntArray* hitlist = GetHitList(test_loc);
	if (hitlist == NULL)
		return false;
	for (int i = 0; i < hitlist->GetSize(); i++)
	{
		int tmp = hitlist->GetAt(i) * 3;		// get an array of indices to the triangle verts
		int	v[3];

		v[0] = m_triData.GetAt(tmp++);
		v[1] = m_triData.GetAt(tmp++);
		v[2] = m_triData.GetAt(tmp);
		if (HitTriangle(test_loc, v))
			return true;
	}
	return false;
}


/*!
 * @fn bool QuadTerrain::IsValid()
 *
 * Once an object is created, the possibility exists that the terrain created is
 * not valid. (for example, if the provided shape had no geometry) Before using
 * the QuadTerrain, a validity check should be made.  An  invalid QuadTerrain 
 * will return false to all hit test queries.  Use the following function to verify
 * that a QuadTerrain successfully turned the supplied model into a valid 
 * terrain database.
 *
 * @see QuadTerrain::QuadTerrain QuadTerrain::HitTest
 */
bool QuadTerrain::IsValid()
{ 
	return m_Valid; 
}

}	// end Vixen