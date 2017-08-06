#include "vix_all.h"

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MDagPath.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnAttribute.h>

/*!
 * @fn meConvMesh::meConvMesh
 * Constructor for polygon mesh converter base.
 * This class maintains information that relates the mesh
 * to each Maya shader that is used with it and the Vixen
 * geometry and appearance for that shader.
 *
 * A single Maya mesh may become more than one
 * Vixen mesh if multiple shaders are used.
 * Different shaders may have different UV mappings
 * so a separate set of texture coordinates is kept
 * for each Vixen mesh.
 *
 * This base class also queries Maya to determine
 * the shaders used by each face of the polygon
 * and the names of each UV set in the Maya mesh.
  */
meConvMesh::meConvMesh(const MDagPath& dagpath)
: meConvGeo(dagpath), m_Mesh(dagpath, &m_Status)
{
    m_NumFaces = 0;
    m_Instance = 0;
    m_NumShaders = 1;
    if (!m_Status)
    {
		NurbsToMesh();
		return;
    }
	DoScan = true;
    m_MayaObj = m_Mesh.object();
    m_NumFaces = m_Mesh.numPolygons();	// # of polygons
    m_Mesh.getUVSetNames(m_UVNames);	// get texture coord set names
	m_MayaVerts = MakeOrigVerts();		// accumulate Maya verts into vertex array
	m_VertexCache.SetLocations(m_MayaVerts);
    FindShaders();						// find attached shaders
	if (Exporter->DupVerts && (m_NumShaders > 1))
		m_HasDups = true;
}

/*!
 * @fn int meConvMesh::Link(meConvObj* convparent)
 * Determines which Maya UV sets are associated with which textures on this mesh.
 *
 * @see meConvGeo::LinkUVMappers
 */
int meConvMesh::Link(meConvObj* convparent)
{
    int			nuvsets = m_Mesh.numUVSets();
	int			rc = meConvGeo::Link(convparent);
	meConnIter	iter(m_MayaObj, "inMesh", true);
	MObject		obj;

	while (iter.Next(obj))
    {
		ConvRef		cvref = Exporter->MayaObjects[obj];
		meConvObj*	conv = cvref;
		meConvEng*	econv = dynamic_cast<meConvEng*>( conv );
		if (econv)
			Exporter->linkOne(obj);
	}
    for (int s = 0; s < m_NumShaders; ++s)	// link UV mappers for each shader
        LinkUVMappers(&m_States[s], nuvsets);
	meLog(1, "%s: Linking mesh with %d UV sets", GetMayaName(), nuvsets);
    return rc;
}

meConvDeform* meConvMesh::FindDeformer()
{
  	meConnIter	iter(m_MayaObj, "inMesh", true);
	MObject		obj;

	while (iter.Next(obj))
    {
		ConvRef			cvref = Exporter->MayaObjects[obj];
		meConvObj*		conv = cvref;
		meConvDeform*	dconv = dynamic_cast<meConvDeform*>( conv );
		if (dconv)
			return dconv;
	}
	return NULL;
}

/*!
 * @fn bool meConvMesh::NurbsToMesh()
 * Converts a Maya nurbs surface to a Maya mesh.
 * This creates an additional Maya DAG node representing the mesh under
 * the parent of the nurbs surface. This node is deleted when this converter
 * is destroyed.
 *
 * @return true if successfully converted, else false
 */
bool meConvMesh::NurbsToMesh()
{
    MTesselationParams tess;
    MFnNurbsSurface nurbs(m_DagPath, &m_Status);
    MDagPath newpath;
    MString newname = nurbs.name() + "_mesh";
    MObject parent = m_DagPath.transform();	// transform node parent

	if (!m_Status)							// is it nurbs?
		return false;

	PrintPlugs(nurbs.object());
	if (GetShaderGroup(nurbs.object(), m_States[0].MayaShader))
		m_NumShaders = 1;
	meLog(1, "%s: Converting Nurbs surface to mesh", GetMayaName());
	tess.setFormatType(MTesselationParams::kGeneralFormat);
	tess.setOutputType(MTesselationParams::kTriangles);
	m_MayaObj = nurbs.tesselate(tess, parent, &m_Status);
	if (!m_Status)
		ME_ERROR(("ERROR: Cannot convert nurbs to polygon mesh %s", GetMayaName()), false);
	if (!MDagPath::getAPathTo(m_MayaObj, newpath) ||
		!m_Mesh.setObject(newpath))
		ME_ERROR(("ERROR: Cannot attach nurbs to polygon mesh %s", GetMayaName()), false);
	m_Mesh.setIntermediateObject(true);
	m_NumFaces = m_Mesh.numPolygons();	// # of polygons
	m_Mesh.getUVSetNames(m_UVNames);	// get texture coord set names
	m_Mesh.setName(newname);
	m_MayaVerts = MakeOrigVerts();		// accumulate Maya verts into vertex array
	m_VertexCache.SetLocations(m_MayaVerts);
	return true;
}

/*!
 * @fn meConvMesh::~meConvMesh()
 * Destructor for base mesh converter.
 * if the current Maya object associated with this converter is intermediate,
 * it was made by converting a Maya node to another format. We delete these
 * Maya nodes made during conversion from the DAG.
 *
 * @see meConvMesh::NurbsToMesh
 */
meConvMesh::~meConvMesh()
{
    MFnDagNode node(m_MayaObj, &m_Status);

    if (!m_Status)
		return;
    if (!node.isIntermediateObject())
		return;
    for (uint32 p = 0; p < node.parentCount(); ++p)
    {
		MObject parobj = node.parent(p);
		MFnDagNode parnode(parobj, &m_Status);
		if (m_Status && node.isChildOf(parobj))
			parnode.removeChild(m_MayaObj);
    }
}

/*!
 * @fn int meConvMesh::FindVertexIndex(MItMeshPolygon& polyIter, MPoint& findme)
 * @param polyIter	Maya polygon iterator
 * @param findme	point to find in the vertex list
 *
 * Find the index of the first vertex in the polygon with the specified location.
 */
int meConvMesh::FindVertexIndex(MItMeshPolygon& polyIter, MPoint& findme)
{
    for (uint32 i = 0; i < polyIter.polygonVertexCount(); ++i)
    {
		MPoint p = polyIter.point(i);
		if (p == findme)
	       return i;
    }
    return -1;
}


/*!
 * @fn bool meConvMesh::FindShaders()
 * Finds the shaders associated with each face of the Maya mesh.
 * Upon return m_FaceShade has the index of the shader for each face.
 */
bool meConvMesh::FindShaders()
{
    MObjectArray shaders;

    m_NumShaders = 1;
    if (!m_Mesh.getConnectedShaders(m_Instance, shaders, m_FaceShade))
		{ ME_ERROR(("WARNING: %s has no shader, using default appearance\n", GetMayaName()), false); }
    else
		m_NumShaders = shaders.length();	// how many shaders?
    if (m_NumShaders < 1)
    {
		m_NumShaders = 1;
		return false;
    }
    if (m_NumShaders > ME_MAX_SHADERS)		// too many for us to handle?
    {
		meLog(1, "ERROR: %s uses %d shaders, maximum of %d - ignoring the rest\n",
			  GetMayaName(), m_NumShaders, ME_MAX_SHADERS);
		m_NumShaders = ME_MAX_SHADERS;
    }
    for (int i = 0; i < m_NumShaders; ++i)
    {
		VX_ASSERT(m_States[i].Offset == 0);
		m_States[i].Offset = 0;
		m_States[i].Size = 0;
		m_States[i].ShaderIndex = i;
		m_States[i].MayaShader = shaders[i];	// save Maya shader handles
    }
    return true;
}

void meConvMesh::MakeLayoutDesc(Core::String& vtxdesc)
{
	vtxdesc = TEXT("float");
	vtxdesc += Core::String(VX_VEC3_SIZE) + TEXT(" position, float3 normal");
	for (int j = 0; j < m_Mesh.numUVSets(); ++j)
		vtxdesc += ", float2 texcoord" + Core::String(j);
}

/*!
 * @fn bool meConvMesh::GetShaderGroup(MDagPath& dagpath, MObject& shadergroup)
 * @param dagpath		Maya DAG path for Nurbs surface to get shaders for
 * @param shadergroup	Gets Maya shader group for surface upon return
 *
 * Gets the Maya shader group used to shade the given node. If there are multiple
 * shaders only the first one is returned.
 *
 * @return true if one and only one shader was found, else false
 *
 * @see meConvSurf::FindShaders
 */
bool meConvMesh::GetShaderGroup(MObject& surface, MObject& shadergroup)
{
	MStatus		status;
	MFnDependencyNode surfnode(surface);
	MItDependencyGraph iter(surface, MFn::kShadingEngine, MItDependencyGraph::kDownstream, MItDependencyGraph::kDepthFirst, MItDependencyGraph::kNodeLevel, &status);
	for(; !iter.isDone(); iter.next())
	{
		shadergroup = iter.currentItem();
		MFnDependencyNode shadernode(shadergroup);
		meLog(1, "%s: Nurbs surface linked to shader %s\n", surfnode.name().asChar(), shadernode.name().asChar());
		return true;
	}
 	return false;
}

/*!
 * @fn meConvMesh*  meConvMesh::MakeConverter(const MDagPath& dagPath)
 * @param dagPath Maya DAG path to node to make converter for
 *
 * Makes a converter to convert a polygon mesh into a Vixen hierarchy.
 *
 * @return pointer to node converter to convert polygon mesh
 *
 * @see meConvPoly meConvPolyOpt meConvNode::MakeConverter
 */
meConvMesh*  meConvMesh::MakeConverter(const MDagPath& dagPath)
{
	meConvPoly*	conv = new meConvPoly(dagPath);
	MDagPath	shapePath(dagPath);
	shapePath.extendToShape();
	Exporter->MayaNodes.Set(shapePath, conv);		// shape dagPath -> converter
	return conv;
}

  
VertexArray* meConvMesh::MakeOrigVerts()
{
	MFnMesh			mesh(m_MayaObj);
	int				n = mesh.numVertices();
	VertexArray*	skinverts;
	char			buf[128];

	if (n == 0)
		return NULL;
	sprintf(buf, "float%d position, float3 normal, float%d blendweight, int%d blendindex",
			VX_VEC3_SIZE, meConvSkin::MAX_BONES_PER_VERTEX, meConvSkin::MAX_BONES_PER_VERTEX);
	skinverts = new VertexArray(buf, n);
	skinverts->SetNumVtx(n);
	VertexArray::Iter iter(skinverts);
	float*	vtx;
	int		i = 0;

	while (vtx = iter.Next())
	{
		MPoint	pos;
		MVector nml;
		int32*	iptr = (int32*) (vtx + 6 + meConvSkin::MAX_BONES_PER_VERTEX);

		mesh.getPoint(i, pos);
		mesh.getVertexNormal(i, nml);
		++i;
		vtx[0] = (float) pos.x; vtx[1] = (float) pos.y; vtx[2] = (float) pos.z; vtx[3] = 1.0f;
		vtx[VX_VEC3_SIZE] = (float) nml.x; vtx[VX_VEC3_SIZE + 1] = (float) nml.y; vtx[VX_VEC3_SIZE + 2] = (float) nml.z;
		vtx += VX_VEC3_SIZE + 3;
		for (int j = 0; j < meConvSkin::MAX_BONES_PER_VERTEX; ++j)
		{
			*vtx++ = 0.0f;
			*iptr++ = -1;
		}
	}
	return skinverts;
}

/*!
 * @fn meConvPoly::meConvPoly
 * Constructor for polygon mesh converter.
 * Checks the conversion options and sets the internal options to match.
 */
meConvPoly::meConvPoly(const MDagPath& dagpath, int opts)
: meConvMesh(dagpath)
{
    m_Options = opts;
	m_VixenVerts = NULL;
}

meConvPoly::~meConvPoly()
{
}

/*!
 * @fn SharedObj* meConvPoly::Make()
 * Makes a Vixen shape node with a TriMesh geometry object
 * for each different Maya shader used on the Maya mesh.
 *
 * @return Shape or NULL if Maya object is not a polygon mesh
 *
 * @see meConvPoly::MakeGeometry
 */
SharedObj* meConvPoly::Make()
{
	Matrix localmtx;

	Shape* shape = new Shape();
    m_VXObj = shape;					// node that owns the geometry
    SetName();							// set name of Shape
	Exporter->Root->Append(shape);
	if (GetMatrix(localmtx))
		shape->SetTransform(&localmtx);
    MakeGeometry(shape);				// make geometry for each shader
	PrintPlugs(m_MayaObj);
    return m_VXObj;
}


/*!
 * @fn int meConvPoly::MakeGeometry()
 * Makes a Vixen geometry object (TriMesh) for each different shader used on the mesh.
 * The geometry is attached to the Shape for this converter.
 * The meshes all share the same locations, normals and texture coordinates but
 * use different index tables to access them.
 * Each mesh will reference a different appearance that describes
 * the shader for the faces contained in the mesh.
 *
 * @see meConvGeo::MakeAppearance
 */
int meConvPoly::MakeGeometry(Shape* root)
{
    int				primverts = CountPrims();		// count verts per primitive
	Shape*			shape = NULL;
	VertexArray*	verts;							// vertex array shared by meshes
	Core::String	vtxdesc;

	/*
	 * Add a set of UV coordinates to the vertex array for all UV sets used by all shaders
	 */
	assert(primverts == 3);						// make sure we have triangles
	MakeLayoutDesc(vtxdesc);
	verts = new VertexArray(vtxdesc);			// make a vertex array
	m_VixenVerts = verts;
	m_VertexCache.SetVertices(verts);
	/*
	 * Make a Vixen triangle mesh for each shader used by the Maya polygon.
	 * All triangle meshes share the same vertex array
	 */
    for (int i = 0; i < m_NumShaders; ++i)		// make geometry for each shader
    {
		meConvState*	stateconv;
		TriMesh*		mesh;					// current mesh
		int				nidx = m_States[i].Size;// number of indices
		Core::String	name(root->GetName());

		m_States[i].Geom = (Mesh*) NULL;
		if (nidx == 0)							// no polygons for this shader?
			continue;
		name += "-";
		name += Core::String(i);				// append shader number to shape name
		shape = new Shape();					// make a new shape
		shape->SetName(name);
		root->Append(shape);
	    mesh = new TriMesh();					// make a new mesh
		shape->SetGeometry(mesh);				// add the mesh to the shape
		mesh->SetVertices(verts);				// share vertex array
		mesh->SetStartVtx(-1);
		mesh->SetIndices(new IndexArray(nidx)); // add index array
		MakeAppearance(&m_States[i]);			// make appearance from the shader
		stateconv = m_States[i].ShaderConv;
		if (stateconv)
			shape->SetAppearance((Appearance*) stateconv->GetVXObj()); // associate appearance with shape
		m_States[i].Shape = shape;				// remember the shape
		m_States[i].Geom = mesh;				// remember the mesh
		m_States[i].ShaderIndex = i;			// and which shader it was used with
    }
	/*
	 * If there is only one child, consolidate
	 */
	if (root->GetSize() == 1)
	{
		shape = (Shape*) root->First();
		root->SetAppearance(shape->GetAppearance());
		root->SetGeometry(shape->GetGeometry());
		shape->Remove(true);
	}
    return m_NumShaders;
}

/*!
 * @fn int meConvPoly::Convert(meConvObj*)
 *
 * Converts a Maya indexed polygon mesh into one or more TriMesh
 * containing non-indexed polygons. There will be a separate TriMesh made
 * for each different Maya shader used on the mesh.
 *
 * @see meConvGeo::MakeGeoState meConvPoly::MakeGeoSets
 */
int meConvPoly::Convert(meConvObj*)
{
    if (m_VXObj == NULL)
		return -1;
	m_DagPath.extendToShape();
	meLog(1, "%s: Converting polygon mesh of %d faces",
		  m_DagPath.partialPathName().asChar(), m_NumFaces);
	if (!Exporter->DupVerts || ((m_NumShaders < 2) && (m_Mesh.numUVSets() < 2)))
	{
		AddVertices();		// add Maya vertices to mesh without reordering
		AddFaces();			// add Maya faces to mesh without reordering
	}
	else					// this path does not preserve original Maya vertex or face ordering
	{
		AddPolygons();
	}
   return 1;
}

/*!
 * @fn int meConvPoly::CountPrims() const
 *
 * Counts the number of indices needed for all polygons in the mesh.
 * Used to determine maximum size of index tables for each geoset.
 *
 * @return number of vertices per primitive upon return
 *	   3 for triangles, 4 for quads, -1 for different sized polygons
 */
int meConvPoly::CountPrims()
{
    MItMeshPolygon polyIter(m_MayaObj);
    int primverts = 0;

    for ( ; !polyIter.isDone(); polyIter.next() )	// for each polygon
    {
        int n = polyIter.polygonVertexCount();
		int shaderidx = 0;

		if (m_NumShaders > 1)				// get index of shader used
			shaderidx = m_FaceShade[polyIter.index()];	// by this face
		if (shaderidx >= ME_MAX_SHADERS)		// exceeded maximum shader count?
		{
			meLog(1, "ERROR: %s shader index %d > %d, defaulting to 0",
			GetMayaName(), shaderidx, ME_MAX_SHADERS - 1);
			shaderidx = 0;
		}
		assert(shaderidx < m_NumShaders);
		if (!polyIter.hasValidTriangulation())
			continue;
		else
		{
			polyIter.numTriangles(n);		// use triangulated face info
			n *= 3;
			m_States[shaderidx].Size += n;	// number of indices for this shader
			primverts = 3;
		}
	}
    return primverts;
}


int meConvPoly::AddVertices()
{
    MFnMesh mesh(m_MayaObj);
	int nuvs = m_Mesh.numUVSets();

	VX_ASSERT(nuvs < 2);
	if (m_NumShaders > 1);
		ME_WARNING(("WARNING: Multiple shaders on mesh - duplicate vertices needed! %s", GetMayaName()));
	for (int i = 0; i < mesh.numVertices(); ++i)
	{
		MPoint	loc;
		MVector nml;
		float	vtx[VertexPool::MAX_VTX_SIZE];

		mesh.getPoint(i, loc);
		mesh.getVertexNormal(i, nml);
		vtx[0] = (float) loc.x; vtx[1] = (float) loc.y; vtx[2] = (float) loc.z;
		vtx[3] = (float) nml.x; vtx[4] = (float) nml.y; vtx[5] = (float) nml.z;
		if (nuvs)
		{
			vtx[6] = 0.0f;
			vtx[7] = 0.0f;
		}
		m_VertexCache.AddVertex(vtx);
	}
	return mesh.numVertices();
}

int meConvPoly::AddFaces()
{
	MItMeshPolygon	polyIter(m_MayaObj, &m_Status);
	meConvMesh::ShaderInfo& state = m_States[0];
	TriMesh*		mesh = (TriMesh*) (Geometry*) state.Geom;
	VertexArray*	verts = m_VixenVerts;
	int				hasuvs = m_Mesh.numUVSets();
	meConvUV*		uvconv = state.UVMappers[0];
	VertexArray::Iter	iter(verts);
    for ( ; !polyIter.isDone(); polyIter.next() )
    {											// for each polygon
		MPointArray mayaverts;
		MIntArray   mayaindices;

		if (!polyIter.hasValidTriangulation())	// skip the bad ones
			continue;
		polyIter.getTriangles(mayaverts, mayaindices, m_Space);
		for (uint32 j = 0; j < mayaindices.length(); j++)
		{
			int		vtxindex = mayaindices[j];	// index of Maya vertex
			MPoint	mayaloc = mayaverts[j];
			float*	vixloc = (float*) iter.GetLoc(vtxindex);
			Vec3	l((float) mayaloc.x, (float) mayaloc.y, (float) mayaloc.z);
			int		i = FindVertexIndex(polyIter, mayaloc);// find triangle vertex in polygon

			mesh->AddIndex(vtxindex);					// add this face to the mesh
			VX_ASSERT(vixloc);
			vixloc += 6;								// skip position, normal
			for (int u = 0; u < m_Mesh.numUVSets(); ++u)// for each UV Set
			{
				float2	uv;

				if (polyIter.getUV(i, uv, &m_UVNames[0]))
				{
					Vec2 v(uv[0], uv[1]);
					if (uvconv)							// apply UV mapping
						uvconv->MapUV(&v, &l, &m_Center);
					*vixloc++ = v.x;					// set the UV in the vertex
					*vixloc++ = v.y;
				}
			}
		}
	}
	return mesh->GetNumFaces();
}
  
/*!
 * @fn int meConvPoly::AddPolygons()
 *
 * Converts the Maya indexed polygons for this mesh into Vixen format and
 * adds them to one or more Vixen Shapes.
 */
int meConvPoly::AddPolygons()
{
    MItMeshPolygon polyIter(m_MayaObj, &m_Status);

    for ( ; !polyIter.isDone(); polyIter.next() )
    {											// for each polygon
		int	  faceidx = polyIter.index();		// polygon index
		int	  shaderidx = 0;					// index of shader/GeoSet
		TriMesh* mesh;

		if (m_NumShaders > 1)
			shaderidx = m_FaceShade[faceidx];
		if (shaderidx >= m_NumShaders)
			shaderidx = 0;
		mesh = (TriMesh*) (Geometry*) m_States[shaderidx].Geom;
		if (mesh == NULL)						// no polygons for this shader
			continue;
		if (Exporter->LogLevel >= 4)
		{
			meConvState*   state = m_States[shaderidx].ShaderConv;

			if (state)
				meLog(4, "%s %d", state->GetMayaName(), faceidx);
		}
		if (polyIter.hasValidTriangulation())
			DoTriangles(polyIter, mesh, m_States[shaderidx]);
		VX_ASSERT(m_States[shaderidx].Offset <= m_States[shaderidx].Size);
    }
    return 1;
}

int meConvPoly::DoTriangles(MItMeshPolygon& polyIter, TriMesh* mesh, meConvMesh::ShaderInfo& state)
{
	MPointArray mayaverts;
    MIntArray   mayaindices;
	int	        nuvsets = state.NumUVSets;	// how many sets of UVs?
    float		vertex[VertexPool::MAX_VTX_SIZE];
    int			ofs = state.Offset;
    int			idx, ntris;
	int			nverts;
	int			minidx = 0, maxidx = 0;

    assert(VertexPool::MAX_VTX_SIZE >= 3 + 3 + 2 * ME_MAX_TEXTURES);
    polyIter.numTriangles(ntris);
    polyIter.getTriangles(mayaverts, mayaindices, m_Space);
	nverts = ntris *= 3;
    assert(mayaindices.length() == nverts);
    for (int j = 0; j < nverts; j++)
    {
		idx = FindVertexIndex(polyIter, mayaverts[j]);	// find triangle vertex in polygon
		if (idx < 0)
		{
			meLog(3, "ERROR: %s Triangulated vertex %d not found in original mesh", GetMayaName());
			idx = 0;
		}
		MPoint	loc = polyIter.point(idx, m_Space);	// vertex for this polygon
		MVector nml;
		float*	vtxptr = vertex;

		*vtxptr++ = (float) loc.x;
		*vtxptr++ = (float) loc.y;
		*vtxptr++ = (float) loc.z;				// save location
		m_Status = polyIter.getNormal(idx, nml, m_Space);
		assert(m_Status);
		*vtxptr++ = (float) nml.x;
		*vtxptr++ = (float) nml.y;
		*vtxptr++ = (float) nml.z;				// save normal
		for (int i = 0; i < nuvsets; ++i)		// for each UV Set
		{
			int mayauv = state.UVSets[i];		// get Maya UVset
			meConvUV* uvconv = state.UVMappers[i];
			if (mayauv >= 0)					// have UVs?
			{
				float2 uv;
				m_Status = polyIter.getUV(idx, uv, &m_UVNames[mayauv]);
				vtxptr[0] = uv[0];
				vtxptr[1] = uv[1];
			}
			if (uvconv)							// apply UV mapping
				uvconv->MapUV((Vec2*) vtxptr, (Vec3*) vertex, &m_Center);
			vtxptr += 2;
		}
		int vxidx = m_VertexCache.AddVertex(vertex);// add the vertex to Vixen

		mesh->SetIndex(ofs, vxidx);			// add index of vertex to index table
		if (vxidx > mesh->GetEndVtx())
			mesh->SetEndVtx(vxidx);			// update maximum vertex index used
		if ((mesh->GetStartVtx() < 0) || (vxidx < mesh->GetStartVtx()))
			mesh->SetStartVtx(vxidx);
		++ofs;
    }
    state.Offset = ofs;						// offset of next index
    return state.Offset;
}


