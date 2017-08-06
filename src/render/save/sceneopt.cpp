#include "vixen.h"
#include "vxutil.h"
#include "util/sceneopt.h"
#include "scene/vxoctree.h"

/*!
 * @fn void PSSceneOptimize::UniqueAppearances(PSModel* root)
 * @param root	root of hierarchy to optimize appearances for
 *
 * Scans the hierarchy and computes an Appearances table
 * containing the unique Appearance objects used in the
 * hierarchy and changes all the meshes to use this
 * this Appearances table.
 *
 * This routine is a step in the process of static scenery
 * optimization. Sharing a single sppearances table allows
 * meshes to be sorted more efficiently and permits the
 * scene graph hierarchy to be more easily rearranged.
 *
 * @see PSOctree PSSceneOptimize::MakeWorldCoords
 */
PSAppearances* PSSceneOptimize::UniqueAppearances(PSModel* root)
{
	PSShape*			mod;
	PSSurface*			surf;
	PSAppearances*		apps;
	PSAppearances*		uniqapps = NULL;
	int32				tot_apps = 0;
	int32				tot_nodes = 0;
	int32				tot_shapes = 0;
	PSGroupIter<PSModel> iter(root, GROUP_DepthFirst);

	while (mod = (PSShape*) (PSModel*) iter.Next())
	{
		++tot_nodes;						// count this node
		if (!mod->IsClass(PSM_Shape))		// is it a shape?
			continue;						// no, on to the next
		++tot_shapes;						// count the shape
		apps = mod->GetAppearances();		// get Appearances table
		if (apps == uniqapps)				// no change needed
			continue;
		if (!apps)
		{
			mod->SetAppearances(uniqapps);	// use the global table
			continue;
		}
		if (uniqapps == NULL)				// share the first non-null appearance table
		{
			uniqapps = apps;
			continue;
		}
		mod->SetAppearances(uniqapps);		// use the global table
		surf = mod->GetSurface();
		if (surf == NULL)
			continue;
		for (int i = 0; i < apps->GetSize(); ++i)
		{
			PSGeometry* geo = surf->Find(i);
			if (geo == NULL)
				continue;

			Ref<PSAppearance> a = apps->GetAt(i);
			int32  n = uniqapps->Find(a);	// already have this one?
			if (n < 0)						// no, let's add it
			{
				n = uniqapps->Append(a);
				++tot_apps;					// another unique one
			}
			geo->SetAppIndex(n);
		}
	}
	PSM_PRINTF(("%d nodes, %d shapes, %d unique appearances", tot_nodes, tot_shapes, tot_apps));
	return uniqapps;
}


PSModel* PSSceneOptimize::OptimizeStatic(PSModel* root)
{
	PSObjArray*	statmods;
	PSModel*	statroot;
	const char*	name = root->GetName();

	m_StatRoots.SetSize(0);
	if (name == NULL)
		root->SetName("scene.root");
	CollectStatic(root, PSMatrix::GetIdentity(), 0);	// collect the static shapes
	if (m_StatRoots.GetSize() < 2)
		return NULL;
	statmods = (PSObjArray*) m_StatRoots.GetAt(1);
	root = BuildStatic(root, statmods);
	for (int i = 2; i < m_StatRoots.GetSize(); i += 2)
	{
		statroot = (PSModel*) m_StatRoots.GetAt(i);
		statmods = (PSObjArray*) m_StatRoots.GetAt(i + 1);
		if (statroot && statmods && !statmods->IsEmpty())	// make octree for each static root
			BuildStatic(statroot, statmods);
	}
	m_StatRoots.Empty();
	return root;
}

#if 1
PSModel* PSSceneOptimize::BuildStatic(PSModel* dynroot, const PSObjArray* statmods)
{
	PSObjArray::Iter iter(*statmods);
	PSShape*	shape;
	PSModel*	root;
	PSOctree*	octree = new PSOctree;		// root of spatially sorted shapes
	const char*	name = statmods->GetName();
	char*		p;
	char		namebuf[PSM_MaxString];
	PSObjArray	surfaces;
	PSBox3		bound;

	PSAppearances* apps = UniqueAppearances(dynroot);
	PSM_ASSERT(name);
	strcpy(namebuf, name);
	p = strrchr(namebuf, '.');
	if (p)
		*p = 0;
	strcpy(p, ".static");	
	while (shape = (PSShape*) iter.Next())	// remove static shapes from hierarchy
	{
		const PSMatrix* worldmtx = (const PSMatrix*) iter.Next();
		PSSurface* surf = shape->GetSurface();
		PSBox3 b;

		PSM_ASSERT(worldmtx && worldmtx->IsClass(PSM_Matrix));
		if (surf)
		{
			*surf *= *worldmtx;				// convert surface to world coords
			surf->CalcBound(&b);
			bound.Extend(b);				// compute overall bounding box
			surfaces.Append(surf);			// save the surface
			shape->SetSurface(NULL);		// disconnect surface we collected
		}
		while (shape && !shape->IsParent() && (shape != dynroot))
		{
			PSModel* mod = shape->Parent();
			shape->Remove(GROUP_Free);		// nuke empty parent models
			shape = (PSShape*) mod;
		}
			
	}

	octree->SetAppearances(apps);
	octree->SetName(namebuf);					// all statics in octree
	if (octree->BuildTree(&surfaces, bound))	// make octree from surfaces
	{
		root = new PSModel;
		root->SetName(dynroot->GetName());		// make new root with no matrix
		strcpy(p, ".dynamic");
		dynroot->SetName(namebuf);				// name the dynamic tree
		dynroot->Replace(root);					// replace the root
		root->Append(octree);
		root->Append(dynroot);
		return root;
	}
	octree->Delete();							// octree failed
	return NULL;
}

#else
/*
 * Extracts all static shapes into a separate scene graph but does not
 * do any spatial sorting
 */
PSModel* PSSceneOptimize::BuildStatic(PSModel* dynroot, const PSObjArray* statmods)
{
	PSObjArray::Iter iter(*statmods);
	PSShape*	shape;
	PSModel*	root;
	PSModel*	statroot = new PSModel;		// root of spatially sorted shapes

	const char*	name = statmods->GetName();
	char*		p;
	char		namebuf[PSM_MaxString];

	PSM_ASSERT(name);
	strcpy(namebuf, name);
	p = strrchr(namebuf, '.');
	if (p)
		*p = NULL;
	strcpy(p, ".static");	
	statroot->SetName(namebuf);				// name the static tree
	while (shape = (PSShape*) iter.Next())	// remove static shapes from hierarchy
	{
		const PSMatrix* worldmtx = (const PSMatrix*) iter.Next();
		PSModel* parent = shape->Parent();	// parent model
		PSSurface* surf = shape->GetSurface();

		if (!shape->IsParent())
		{
			PSM_ASSERT(worldmtx && worldmtx->IsClass(PSM_Matrix));
			*surf *= *worldmtx;				// convert surface to world coords
			shape->SetTransform((PSMatrix*) NULL);
			shape->Remove(GROUP_DontFree);	// remove if a leaf shape
			statroot->PutFirst(shape);
		}
		while (parent && !parent->IsParent() && (parent != dynroot))
		{
			PSModel* mod = parent->Parent();
			parent->Remove(GROUP_Free);		// nuke empty parent models
			parent = mod;
		}
			
	}
	root = new PSModel;
	root->SetName(dynroot->GetName());		// make new root with no matrix
	strcpy(p, ".dynamic");
	dynroot->SetName(namebuf);				// name the dynamic tree
	dynroot->Replace(root);					// replace the root
	root->Append(statroot);
	root->Append(dynroot);
	return root;
}
#endif

/*!
 * @fn bool PSSceneOptimize::CollectStatic(const PSModel* root, const PSMatrix* mtx, int level)
 * @param root		root of hierarchy to convert to world coordinates
 * @param mtx		initial matrix (usually identity)
 * @param level		0-based level index
 *
 * Collects all of the static geometry in the hierarchy and computes the
 * world matrix for each static surface.
 *
 * Billboards, sprites, level of detail nodes and other implementations
 * which update the local matrix or geometry are not collected.
 * External references, which load content dynamically are also not static.
 * Only shapes which have no parents that update the local matrix can be
 * kept in the static scene.
 *
 * This routine is useful for optimizing static scenery. By collecting the
 * surfaces, their meshes may be spatially sorted for more efficient rendering.
 * For simple scenes, all of the static scenery can be extracted and put into a
 * single octree. More complex scenes, such as those using portal visiblity,
 * may require multiple octrees.
 *
 * This routine only collects the surfaces - it does not change the
 * structure or contents of the input hierarchy.
 *
 * @return true this model and children are static, else false
 *
 * @see PSSceneOptimize::UniqueAppearances PSSceneOptimize::BuildStatic
 * @see PSRoomGroup PSOctree::BuildTree 
 */
bool PSSceneOptimize::CollectStatic(const PSModel* root, const PSMatrix* mtx, int level)
{
	const PSModel*	statroot;				// root of hierarchy
	PSObjArray*		statmods;				// static models for this hierarchy
	Ref<PSMatrix>	worldmtx = (PSMatrix*) NULL;

	if (level >= m_StatRoots.GetSize())		// need to add a new root?
	{
		statroot = root;					// remember this root
		m_StatRoots.Append(statroot);
		statmods = new PSObjArray;			// make new static models collection
		statmods->SetName(root->GetName());
		m_StatRoots.Append(statmods);		// remember it too
	}
	else									// use existing collection
	{
		statroot = (const PSModel*) m_StatRoots.GetAt(level);
		PSM_ASSERT(statroot && statroot->IsClass(PSM_Model));
		statmods = (PSObjArray*) m_StatRoots.GetAt(level + 1);
		PSM_ASSERT(statmods && statmods->IsClass(PSM_Array));
	}
	if (root->IsClass(PSM_RoomGroup))		// room group is a special case
	{
		PSRoomGroup*		roomgroup = (PSRoomGroup*) root;
		PSObjArray::Iter	iter(roomgroup->GetRooms());
		PSRoom*				room;

		roomgroup->RemoveMatrix();
		while (room = (PSRoom*) iter.Next())
		{
			Ref<PSModel> fixture = room->GetFixture();
			if (fixture != (const PSModel*) NULL)	// room has contents?
			{
				char	namebuf[PSM_MaxString];

				level += 2;					// make empty node to contain static models laster
				CollectStatic(fixture, mtx, level);
				statmods = (PSObjArray*) m_StatRoots.GetAt(level + 1);
				strcpy(namebuf, room->GetName());
				strcat(namebuf, ".static");
				statmods->SetName(namebuf);
			}
		}
		return false;
	}
	if (root->GetTransform()->IsIdentity())	// local matrix is identity
		worldmtx = mtx;
	else									// make a new world matrix
	{
		worldmtx = new PSMatrix(*mtx);
		root->CalcMatrix(worldmtx, PSGetScene());			// concatenate with input matrix
	}
	switch (root->ClassID())
	{
		case PSM_Shape:						// for a shape - transform geometry
		if (root->GetHints() & MOD_MorphBounds)
			return false;
		break;

		case PSM_Model:
		break;

		default:							// not a model or shape?
		return false;						// don't descend further
	}

	PSGroupIter<PSModel> iter(root, GROUP_TopOnly);
	const PSModel*	mod;

	while (mod = iter.Next())				// collect from children
		CollectStatic(mod, worldmtx, level);

	if (root->IsClass(PSM_Shape))
	{
		statmods->Append(root);				// save shape in static model list
		statmods->Append((const PSMatrix*) worldmtx);	// save its world matrix too
	}
	return true;
}

/*!
 * @fn void PSSceneOptimize::OptimizeVerts(PSSurface* surf)
 *
 * Sorts the indices and vertices of the meshes in the surface based on
 * the order the vertices are used by the primitives.
 * Duplicate vertices are coalesced so that the one vertex
 * array contaiing only unique vertices is shared by all meshes.
 *
 * This routine correctly optimizes multiple meshes sharing the same
 * vertex and/or index arrays.
 *
 * @see PSSurface PSTriangles
 */
void PSSceneOptimize::OptimizeVerts(PSSurface* surf)
{
	int32		i;
	int32		nvtx;			/* no of vertices */
	int32		nidx;			/* no of indices */
	int32		j = 0;
	
	PSSurface::Iter	siter(surf);
	PSTriMesh*		mesh;
	PSIndexArray	vtx_map;
	PSVtxArray*		new_vtx = NULL;
	PSIndexArray*	new_idx = NULL;

	while (mesh = (PSTriMesh*) siter.Next())
	{
		if (!mesh->IsClass(PSM_TriMesh))		/* not a triangle mesh */
			continue;
		if ((nvtx = mesh->GetNumVtx()) <= 0)	/* no vertices in mesh */
			continue;
		if ((nidx = mesh->GetNumIdx()) <= 0)	/* do we have indices? */
			continue;							/* no, skip this one */
		if (new_vtx == NULL)
			new_vtx = new PSVtxArray(mesh->GetStyle(), nvtx);
		else if (new_vtx->GetStyle() != mesh->GetStyle())
			continue;							/* vertex style incompatible */

		int firstvtx = vtx_map.GetSize();		/* set unused map cells to -1 */
		vtx_map.SetSize(firstvtx + nvtx);		/* enlarge vertex map */
		memset(vtx_map.GetData() + firstvtx * vtx_map.GetElemSize(), -1, nvtx * vtx_map.GetElemSize());
		if (new_idx == NULL)					/* allocate new index array */
			new_idx = new PSIndexArray(nidx);
		new_idx->SetSize(new_idx->GetSize() + nidx);
/*
 * Sort the vertices in the order they are used by the primitives.
 * Copy the vertices into a new array so that they are in
 * the same order as the primitives. Construct a mapping
 * array which gives the new location of each vertex (using
 * the old vertex location as an index). The vertex map entries
 * are initialized as -1 so that vertices that are mapped can
 * be detected and not remapped.
 */
		PSTriangles::PrimIter iter(mesh);
		const PSTriPrim* prim;

		while (prim = iter.Next())				/* for each primitive */
		{
			for (i = 0; i < prim->Size; ++i)	/* for each vertex */
			{
				int32 v = mesh->GetIndex(i + prim->VtxIndex);
				PSM_ASSERT(v >= 0);
				if (vtx_map[v] < 0)				/* has this one been added? */
				{
					float tmpvtx[VTX_MaxVtxSize];

					mesh->GetVertices()->GetVertex(v, tmpvtx);
					if ((j = new_vtx->Find(tmpvtx)) < 0)
						j = new_vtx->Append(tmpvtx);
					vtx_map.SetAt(v, PSVtxIndex(j));
				}
				PSM_ASSERT(v != PSVtxIndex(-1));
				new_idx->SetAt(j++, v);			/* add new index to table */
			}
		}
		mesh->SetVertices(new_vtx);				/* use new vertex array */
		mesh->SetIndices(new_idx);				/* use new index array */
		mesh->CompressPrims();					/* optimize primitive usage */
	}
}
