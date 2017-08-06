#include "vixen.h"
#include "scene/vxoctree.h"

namespace Vixen {

VX_IMPLEMENT_CLASSID(Octree, Shape, VX_Octree);

Octree::Octree(const Box3* bound) : Shape()
{
	if (bound)
		SetBound(bound);
//	SetHints(Model::STATIC);
}

/*!
 * @fn bool Octree::IsInside(const Mesh* mesh) const
 * @param mesh	mesh to check
 *
 * Determines whether or not the given mesh is within the
 * bounds of this octree node. The result is computed by
 * comparing the bounding box of the mesh against the
 * bounds of this node. Individual triangles are not checked.
 * This routine assumes the local coordinate system of the
 * mesh vertices is world coordinates!
 *
 * @return  true if mesh inside this node, else  false
 *
 * @see Octree::MakeWorldCoords
 */
bool Octree::IsInside(const Geometry* mesh) const
{
	Box3		b;
	Box3		bnd;

	if (mesh == NULL)
		return false;
	if (!mesh->IsClass(VX_Mesh))
		return false;
	if (!GetBound(&bnd, NONE))
		return false;
	if (!mesh->GetBound(&b))			// no bounding volume?
		return false;					// ignore this one
	if (bnd.Contains(b.min) && bnd.Contains(b.max))
		return true;					// within input bounds
	return false;
}


/*!
 * @fn Octree* Octree::Find(const Vec3& loc) const
 * @param loc	location to find within octree
 *
 * Finds the lowest level octree node which contains the
 * given location. The spatially sorted structure of the
 * octree makes this efficient, allowing for fast
 * collision detection and picking
 */

const Octree* Octree::Find(const Vec3& loc) const
{
	Box3	bound;

	if (!GetBound(&bound, NONE) ||
		!bound.Contains(loc))
		return NULL;

	GroupIter<Octree> iter(this, Group::CHILDREN);
	const Octree* node;
	const Octree* found;

	while (node = iter.Next())
	{
		if (node->IsClass(VX_Octree) &&
			(found = node->Find(loc)))
			return found;
	}
	return this;
}

/*!
 * @fn bool Octree::BuildTree(ObjArray* surfaces, const Box3& bound)
 * @param surfaces	array of surfaces to spatially sort
 * @param bound		bounding volume for octree
 *
 * Spatially sorts the input array of surfaces by building an octree
 * that arranges the meshes according to 3D location in space.
 * The octree itself is a scene graph and can be added to the
 * hierarchy of a scene. It arranges the meshes at each level
 * based on which octant of 3D space they occupy. Only the surfaces
 * of leaf nodes have meshes in them. Upper level nodes subdivide
 * space but render nothing. This spatial organization optimizes
 * operations like picking and collision detection which try to
 * determine relationships between objects in space.
 *
 * The technique only works for a scene graph that does not
 * contain dynamic objects whose matrix changes over time
 * (like billboards or animated characters). The input surfaces
 * must be converted to world coordinates before they can be used
 * to make an octree.
 *
 * @returns  true if octree construction successful,
 *			 false if octree was not made
 *
 */
bool Octree::BuildTree(ObjArray& surfaces, const Box3& bound)
{
	Box3		limit;
	float		limitvol;
	Matrix	rootmtx;
	int			nmeshes;
	ObjArray	collected;

	SetBound(&bound);
	nmeshes = CollectMeshes(surfaces, collected, limit, false);
	if (nmeshes <= 0)
		return false;
	limitvol = limit.Width() * limit.Height() * limit.Depth();
	Subdivide(collected, limitvol, 1);
	return true;
}

/*
 * @fn bool Octree::Subdivide(ObjArray& meshes, float limitvol, int level)
 * @param limitvol	minimum volume for subdivision
 * @param level		tree depth
 *
 * Subdivides this octree node, creating 8 children for each
 * octant of the original bounding volume.
 *
 * @return  true if children created,  false if subdivision below volume limit
 */
bool Octree::Subdivide(ObjArray& meshes, float limitvol, int level)
{
	Octree*	node;
	Box3		minvol;
	Box3		bound;

	if (meshes.GetSize() == 0)
		return false;			// no meshes and no children?
	if (!GetBound(&bound, NONE))
		return false;			// empty bounds?

	float			newvol = bound.Width() * bound.Height() * bound.Depth();
	Vec3			center = bound.Center();
	Box3			b;
	TCHAR			namebuf[VX_MaxName];
	const TCHAR*	name = GetName();
	TCHAR*			p;

	newvol /= 8;
	if (newvol < limitvol)				// smaller than limit volume?
		return false;					// don't subdivide
	++level;
	VX_ASSERT(name);					// isolate first part of name
	STRCPY(namebuf, name);
	p = STRCHR(namebuf, '-');
	if (p)
		*p = 0;

	b.Set(Vec3(bound.min.x, bound.min.y, bound.min.z), center);
	node = MakeNode(meshes, &b, level, namebuf, limitvol, 1);
	b.Set(Vec3(bound.max.x, bound.min.y, bound.min.z), center);
	node = MakeNode(meshes, &b, level, namebuf, limitvol, 2);
	b.Set(Vec3(bound.min.x, bound.max.y, bound.min.z), center);
	node = MakeNode(meshes, &b, level, namebuf, limitvol, 3);
	b.Set(Vec3(bound.max.x, bound.max.y, bound.min.z), center);
	node = MakeNode(meshes, &b, level, namebuf, limitvol, 4);
	b.Set(Vec3(bound.min.x, bound.min.y, bound.max.z), center);
	node = MakeNode(meshes, &b, level, namebuf, limitvol, 5);
	b.Set(Vec3(bound.max.x, bound.min.y, bound.max.z), center);
	node = MakeNode(meshes, &b, level, namebuf, limitvol, 6);
	b.Set(Vec3(bound.min.x, bound.max.y, bound.max.z), center);
	node = MakeNode(meshes, &b, level, namebuf, limitvol, 7);
	b.Set(Vec3(bound.max.x, bound.max.y, bound.max.z), center);
	node = MakeNode(meshes, &b, level, namebuf, limitvol, 8);
	return true;		
}

Octree* Octree::MakeNode(ObjArray& inputmeshes, const Box3* bound, int level, const TCHAR* nameprefix, float limitvol, int count)
{
	TCHAR		namebuf[VX_MaxName];
	TCHAR*		p;
	Box3		minvol;
	int			nmeshes;
	Octree*	node = new Octree(bound);
	ObjArray	outputmeshes;

	VX_ASSERT(nameprefix);
	nmeshes = node->CollectMeshes(inputmeshes, outputmeshes, minvol, true);
	if (nmeshes == 0)
	{
		node->Delete();
		return NULL;
	}
	STRCPY(namebuf, nameprefix);
	p = namebuf + STRLEN(namebuf);
	SPRINTF(p, TEXT("-%d.%d"), level, count);
	node->SetName(namebuf);
	node->Subdivide(outputmeshes, limitvol, level);	
	Append(node);
	if (nmeshes == 1)					// only one mesh?
		node->SetGeometry((Geometry*) (SharedObj*) outputmeshes[0]);
	else								// multiple meshes
	{
		ObjArray::Iter iter(outputmeshes);
		Geometry* geo;
		Shape* shape;
		while (geo = (Geometry*) iter.Next())		// add each mesh as a child shape
		{
			shape = new Shape();
			shape->SetGeometry(geo);
			node->Append(shape);
		}
	}
	return node;
}

/*!
 * @fn int Octree::CollectMeshes(const ObjArray& input, ObjArray& output, Box3& minvol)
 * @param input		input array of meshes
 * @param output	output array of meshes
 * @param minvol	gets bounding volume of smallest mesh
 * @param remove	true to remove the collected meshes from the input array, false to leave them
 *
 * Collects all of the meshes from the input array of surfaces that fit within
 * the bounds of this node and returns them in the output array.
 * This routine assumes all of the surfaces in the hierarchy have been
 * converted to world coordinates. 
 *
 * @return number of meshes collected
 */
int Octree::CollectMeshes(const ObjArray& input, ObjArray& output, Box3& minvol, bool remove)
{
	ObjArray::Iter	iter(input);
	const Geometry*	surf;
	int					nmeshes = 0;

	minvol.Empty();
	while (surf = (const Geometry*) iter.Next())	// for each mesh in the array
	{
		if (CalcMinVol(surf, minvol))				// is it in the box?
		{
			output.Append(surf);
			if (remove)
				iter.Remove();
			nmeshes++;
		}
	}
	return nmeshes;
}

/*!
 * @fn bool Octree::CalcMinVol(const Geometry* mesh, Box3& minbox)
 * @param mesh		input mesh to check
 * @param minbox	gets bounding volume of mesh
 *
 * Collects all of the meshes from the input hierarchy that fit within
 * the input bound and adds them to the surface of this shape.
 *
 * @return true if mesh is inside box, else false
 */
bool Octree::CalcMinVol(const Geometry* mesh, Box3& minbox)
{
	float			minvol = minbox.Width() * minbox.Height() * minbox.Depth();
	Box3			b;
	float			vol;

	if (!IsInside(mesh))
		return false;
	if (!GetBound(&b, NONE))
		return false;
	vol = b.Width() * b.Height() * b.Depth();
	if ((minvol == 0) || (minvol > vol))
	{
		minbox = b;			// new minimum volume box
		minvol = vol;
	}
	return true;
}


bool Octree::Hit(Ray& ray, float& distance) const
{
	Box3 bound;

	if (GetBound(&bound, NONE))
		return bound.Hit(ray, distance, NULL);
	return false;
}

intptr Octree::Cull(const Matrix* trans, Scene* scene)
{
	Box3 box;

	if (m_NoBounds)
		return DISPLAY_NONE;
	if (m_NoCull || !DoCulling)				// do not cull this one
		return DISPLAY_ALL;
	if (Parent() == NULL)					// never cull the root
		return DISPLAY_ALL;
	if (GetBound(&box, NONE) && scene->GetCamera()->IsVisible(box))
	{
		Geometry* geo = GetGeometry();
		if ((geo == NULL) || !geo->Cull(trans, scene))
			return DISPLAY_ALL;				// shape is visible
	}
	SceneStats* g = scene->GetStats();
	Core::InterlockAdd(&(g->CulledVerts), (int) m_Verts);
	Core::InterlockAdd(&(g->TotalVerts), (int) m_Verts);
	Core::InterlockInc(&(g->CulledModels));
	return DISPLAY_NONE;					// model culled
}

}	// end Vixen