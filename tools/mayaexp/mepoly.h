#pragma once

#include <maya/MFnSet.h>
#include <maya/MFloatArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>

#include "megeo.h"

#define ME_OPT_STRIP	4	// optimize by outputting triangle strips

class meConvUV;
class meConvState;
class meConvDeform;


/*!
 * @class meConvMesh
 * @brief Base converter for polygonal meshes.
 *
 * This class encapsulates common code to convert indexed polygon meshes from Maya
 * into into Vixen TriMesh objects. The base class manages per-face shaders and UV mapping.
 * Subclasses must implement geometry conversion.
 *
 * Export and logging options are controlled by the pfbExporter:
 * - vixExporter::LogLevel	controls log output of conversion process
 *				0 = user level messsages about node conversion
 *				1 = outputs Vixen call descriptions
 *				> 1 = outputs vertex information
 * - vixExporter::DoTextures	if true, textures and texture coordinates are exported
 *				if false, geometry has material properties only
 *
 * @see meConvGeo meConvPoly meConvPolyOpt meConvPolyInd meConvPolyOptInd
 */
class meConvMesh : public meConvGeo
{
public:
    meConvMesh(const MDagPath& mpath);
    ~meConvMesh();

    static meConvMesh*	MakeConverter(const MDagPath& dagPath);
    static int			FindVertexIndex(MItMeshPolygon& polyIter, MPoint& findme);
    virtual int			Link(meConvObj* convparent = NULL);
	meConvDeform*		FindDeformer();
	static bool			GetShaderGroup(MObject& geometry, MObject& shadergroup);
	VertexArray*		GetMayaVerts() const	{ return m_MayaVerts; }
	IntArray*			GetVertexMap() const	{ return m_VertexCache.GetVertexMap(); }
	bool				HasDuplicates() const	{ return m_HasDups; }

protected:
    virtual bool	FindShaders();	// Find Maya shaders that affect this mesh
	void			MakeLayoutDesc(Core::String& vtxdesc);
    bool			NurbsToMesh();
	VertexArray*	MakeOrigVerts();

	Ref<VertexArray>	m_MayaVerts;	// original Maya vertices in Vixen format
	VertexCache			m_VertexCache;
    MFnMesh				m_Mesh;			// mesh interface
	bool				m_HasDups;
    int					m_NumShaders;	// number of shaders used by mesh
    int					m_NumFaces;		// number of faces
    MIntArray			m_FaceShade;	// shader index per polygon
    MStringArray		m_UVNames;		// Maya UV set names
    ShaderInfo			m_States[ME_MAX_SHADERS];	// geometry info for each shader
};

/*!
 * @class meConvPoly
 * @brief Node converter for non-indexed polygonal meshes.
 *
 * Converts an indexed polygon mesh from Maya into one or more Vixen TriMesh objects.
 * This converter does preserve the Maya index tables but instead generates
 * non-indexed polygons. This mesh cannot be deformed at run time using animation
 * information extracted from Maya. In the case where the mesh has a single appearance
 * across all faces, one pfGeoSet is produced containing unoptimized polygons.
 * If multiple appearances are used, the exporter produces a pfGeoSet for each
 * unique appearance. The pfGeoSets produced for a single Maya mesh do not share 
 * vertex, normal, color or texture coordinate information.
 *
 * Export and logging options are controlled by the vixExporter:
 * - vixExporter::LogLevel	controls log output of conversion process
 *				0 = user level messsages about node conversion
 *				1 = outputs Vixen call descriptions
 *				> 1 = outputs vertex information
 * - vixExporter::DoTextures	if true, textures and texture coordinates are exported
 *				if false, geometry has material properties only
 * - vixExporter::LogLevel	controls log output of conversion process
 *
 * @see meConvNode
 */
class meConvPoly : public meConvMesh
{
public:
    meConvPoly(const MDagPath& mpath, int opts = 0);
    ~meConvPoly();

	int					GetOptions() const		{ return m_Options; }
    void				SetOptions(int opts)	{ m_Options = opts; }
	virtual int			Convert(meConvObj* = NULL);
    virtual SharedObj*	Make();
	VertexArray*		GetVertices() const		{ return m_VixenVerts; }

protected:
    int			MakeGeometry(Shape*);
    int			CountPrims();
	int			AddPolygons();
	int			AddVertices();
	int			AddFaces();
    int			DoTriangles(MItMeshPolygon& polyIter, TriMesh* mesh, meConvMesh::ShaderInfo& state);

    int					m_Options;
	Ref<VertexArray>	m_VixenVerts;
};
