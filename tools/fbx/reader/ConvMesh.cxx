#include "vixen.h"
#include "ConvNode.h"
#include "ConvMesh.h"
#include "FBXReader.h"

namespace Vixen { namespace FBX {

ConvMesh::ConvMesh(FbxNode* pNode)
  :	ConvModel(pNode),
	mMesh(pNode->GetMesh()),
	mHasNormal(false),
	mHasUV(false),
	mAllByControlPoint(true)
{
}

/*
 * Generate one or more Vixen Shape objects (in a hierarchy) for a single FBX mesh.
 * If the FBX mesh uses a single material, only one Vixen Shape is created.
 * Otherwise, a Vixen Shape is made for the geometry for each separate FBX material.
 *
 * If the FBX control points, normals and texcoords do not share the same index array,
 * the Vixen vertex array will be different than the FBX controls points because vertices
 * will need to be duplicated if they have different normals or texcoords.
 */
Model* ConvMesh::MakeShapes(const TCHAR* filebase)
{
    FbxNode*		pNode = m_FBXNode;
    const int		numtris = mMesh->GetPolygonCount();
	VertexArray*	vtxarray;
	Shape*			shape;
	Mesh*			mesh;
	Model*			root = NULL;
	int				nshapes = 0;
	int				mtlcount = m_FBXNode->GetMaterialCount();
    FbxLayerElementArrayTemplate<int>* MaterialMap = NULL;
    FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;

	if (mMesh->GetElementMaterial())
    {
        MaterialMap = &mMesh->GetElementMaterial()->GetIndexArray();
        lMaterialMappingMode = mMesh->GetElementMaterial()->GetMappingMode();
	}
	mVertices = vtxarray = MakeVerts();
	if (mtlcount == 0)
		mtlcount = 1;
	/*
	 * MakeVertices has already made the vertex array and separated the vertices
	 * according to each material. mNumVerts gives the number of vertices for
	 * each different material used in the mesh. mStartVert gives the starting
	 * vertex for the polygons which use that material.
	 */
	for (int i = 0; i < mtlcount; ++i)
	{
		FbxSurfaceMaterial* lMaterial = pNode->GetMaterial(i);
		Appearance*	appear;

		shape = new Shape();
		shape->SetName(m_Name + Core::String(TEXT('-')) + Core::String(i));
		if (lMaterial)
		{
			appear = (Appearance*) lMaterial->GetUserDataPtr();
			if (appear == NULL)
			{
				ConvAppear fbxmtl(lMaterial, Core::String(filebase) + lMaterial->GetName());
				fbxmtl.MakeAppearance();
				appear = (Appearance*) fbxmtl.Convert();
			}
			if (appear)
			{
				VX_TRACE(FBXReader::Debug, ("Linking Appearance %s -> Shape %s\n", appear->GetName(), shape->GetName()));
				shape->SetAppearance(appear);
			}
		}
		mesh = (TriMesh*) new TriMesh();
		mesh->SetIndices(new IndexArray());
		mesh->SetVertices(vtxarray);
		mesh->SetStartVtx(INT_MAX);
		shape->SetGeometry(mesh);
        mShapes.Append(shape);
	}
/*
 * mAllByControlPoint indicates the Vixen vertices correspond exactly to the FBX vertices.
 * Every vertex has a normal and texcoord and they use the same index arrays
 * (as opposed to one array per channel).
 */
	if (mAllByControlPoint)
	{
		shape = (Shape*) (SharedObj*) mShapes[0];
		mesh = (Mesh*) shape->GetGeometry();
		for (int polyindex = 0; polyindex < numtris; ++polyindex)
		{
			for (int v = 0; v < 3; ++v)
			{
				const int srcindex = mMesh->GetPolygonVertex(polyindex, v);
				mesh->AddIndex(srcindex);
			}
		}
	}
	else
	{
		mVertexCache.SetLocations(vtxarray);
		OptimizeVerts(vtxarray, MaterialMap, lMaterialMappingMode);
	}
	/*
	 * Prune out the shapes with no faces
	 */
	nshapes = 0;
	root = new Model();
	for (int i = 0; i < mtlcount; ++i)
	{
		shape = (Shape*) (SharedObj*) mShapes[i];
		mesh = (Mesh*) shape->GetGeometry();
		if ((mesh == NULL) || (mesh->GetStartVtx() == INT_MAX) || (mesh->GetNumVtx() == 0))
		{
			shape->Delete();
			mShapes[i] = NULL;
		}
		else
		{
			++nshapes;
			root->Append(shape);
			VX_TRACE(FBXReader::Debug, ("Converted mesh %s to %d verts %d faces\n", shape->GetName(), mesh->GetNumVtx(), mesh->GetNumIdx() / 3));
		}
	}
	if (nshapes == 0)
	{
		root->Delete();
		return NULL;
	}
	if (nshapes == 1)
	{
		shape = (Shape*) root->First();
		shape->Remove(Group::UNLINK_NOFREE);
		shape->SetName(m_Name);
		root->Delete();
		root = shape;
	}
	m_VixNode = root;
    return root;
}

/*
 * Construct a Vixen VertexArray from FBX control points. If the descriptor is supplied, the
 * Vixen VertexArray will only have the locations from the FBX mesh. Otherwise, the
 * normals and texcoords are copied as well.
 */
VertexArray* ConvMesh::MakeVerts(const TCHAR* descriptor)
{
    FbxGeometryElement::EMappingMode	lNormalMappingMode = FbxGeometryElement::eNone;
    FbxGeometryElement::EMappingMode	lUVMappingMode = FbxGeometryElement::eNone;
	const FbxGeometryElementUV*			lUVElement = NULL;
	const FbxGeometryElementNormal*		lNormalElement = NULL;
	int									numverts = mMesh->GetControlPointsCount();
	Core::String						vtxdesc = TEXT("float4 position");

    // Congregate all the data of a mesh to be cached in VBOs.
    // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
    mHasNormal = mMesh->GetElementNormalCount() > 0;
    mHasUV = mMesh->GetElementUVCount() > 0;
	if (descriptor != NULL)
	{
		mHasUV = false;
		mHasNormal = false;
		mAllByControlPoint = true;
		vtxdesc = descriptor;
	}
	else
	{
		if (mHasNormal)
		{
			lNormalMappingMode = mMesh->GetElementNormal(0)->GetMappingMode();
			if (lNormalMappingMode == FbxGeometryElement::eNone)
				mHasNormal = false;
			if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
				mAllByControlPoint = false;
			if (mHasNormal)
			{
				vtxdesc += TEXT(", float4 normal");
				lNormalElement = mMesh->GetElementNormal(0);
			}
		}
		if (mHasUV)
		{
			lUVElement = mMesh->GetElementUV(0);
			lUVMappingMode = lUVElement->GetMappingMode();
			if (lUVMappingMode == FbxGeometryElement::eNone)
				mHasUV = false;
			if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
				mAllByControlPoint = false;
			if (mHasUV)
			{
				for (int i = 0; i < mMesh->GetElementUVCount(); ++i)
				{
					vtxdesc += TEXT(", float2 texcoord");
					vtxdesc += TEXT('0') + i;
				}
			}
		}
	}

	VertexArray*		vtxarray = new VertexArray(vtxdesc, numverts);
    const FbxVector4*	srcvertices = mMesh->GetControlPoints();

	vtxarray->SetNumVtx(numverts);
	vtxarray->SetName(m_Name + TEXT(".verts"));
	CopyVerts(vtxarray, lNormalElement, lUVElement);
    return vtxarray;
}

/*
 * Copy vertices from FBX mesh to Vixen VertexArray.
 * Vertices are copied directly from the FBX control point into
 * the Vixen vertex array in the same order.
 */
void ConvMesh::CopyVerts(VertexArray* vtxarray, const FbxGeometryElementNormal* normal_elem, const FbxGeometryElementUV* uv_elem)
{
	VertexArray::Iter	iter(vtxarray);
    const FbxVector4*	srcvertices = mMesh->GetControlPoints();
	int					numverts = mMesh->GetControlPointsCount();

	VX_TRACE(FBXReader::Debug, ("Copying %d vertices\n", vtxarray->GetNumVtx()));
	/*
	 * If using control points, populate the Vixen vertex array directly
	 * from the control vertices.
	 */
    for (int i = 0; i < numverts; ++i)
    {
        // Save the vertex position.
		Vec4*		dstvtx =  (Vec4*) iter.Next();
        FbxVector4	srcloc = srcvertices[i];

        dstvtx->x = static_cast<float>(srcloc[0]);
        dstvtx->y = static_cast<float>(srcloc[1]);
        dstvtx->z = static_cast<float>(srcloc[2]);
		dstvtx->w = 1.0f;
		dstvtx++;

        // Save the normal.
        if (mHasNormal)
        {
            int lNormalIndex = i;
            if (normal_elem->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                lNormalIndex = normal_elem->GetIndexArray().GetAt(i);
            FbxVector4 srcnml = normal_elem->GetDirectArray().GetAt(lNormalIndex);
            dstvtx->x = static_cast<float>(srcnml[0]);
            dstvtx->y = static_cast<float>(srcnml[1]);
            dstvtx->z = static_cast<float>(srcnml[2]);
			dstvtx->w = 0.0f;
            dstvtx++;
        }

        // Save the UV.
        if (mHasUV)
        {
            int lUVIndex = i;
            if (uv_elem->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                lUVIndex = uv_elem->GetIndexArray().GetAt(i);
            FbxVector2 srcuv = uv_elem->GetDirectArray().GetAt(lUVIndex);
            dstvtx->x = static_cast<float>(srcuv[0]);
            dstvtx->y = static_cast<float>(srcuv[1]);
        }
    }
}

/*
 * Optimize shared vertex usage across a multi-material mesh.
 * @param vtxarray			Vixen vertex array to get the optimized vertices
 * @param materialMap		FBX material map
 * @param materialMapMode	FBX material mapping mode
 *
 * Vixen vertex arrays have a position, normal and texture coordinate per vertex.
 * There is one index buffer for the entire vertex array - not an index buffer per channel.
 * This function produces a vertex array with vertices in the order they are encountered
 * when rendering the triangles. A single FBX control point may correspond to multiple
 * Vixen vertices if that vertex is shared by multiple polygons with different normals or texcoords.
 *
 * A single FBX mesh can have polygons with several different materials.
 * A Vixen mesh can only have a single material. One FBX mesh can produce
 * a hierarchy of several Vixen shapes all sharing the same vertex array.
 * Each mesh has a different index array.
 */
void ConvMesh::OptimizeVerts(VertexArray* vtxarray, FbxLayerElementArrayTemplate<int>* materialMap, FbxGeometryElement::EMappingMode materialMapMode)
{
    const FbxVector4*	srcvertices = mMesh->GetControlPoints();
	const int			numtris = mMesh->GetPolygonCount();
    FbxStringList		lUVNames;
	
	VX_TRACE(FBXReader::Debug, ("Optimizing %d triangles, %d vertices\n", numtris, vtxarray->GetNumVtx()));
	mMesh->GetUVSetNames(lUVNames);
	mVertexCache.SetLocations(vtxarray);
	mVertexCache.SetVertices(vtxarray);
	vtxarray->SetNumVtx(0);
    for (int polyindex = 0; polyindex < numtris; ++polyindex)
    {
        int mtlindex = 0;

        if (materialMap && materialMapMode == FbxGeometryElement::eByPolygon)
            mtlindex = materialMap->GetAt(polyindex);
        for (int v = 0; v < 3; ++v)
        {
            const int		srcindex = mMesh->GetPolygonVertex(polyindex, v);
			FbxVector4		srcloc = srcvertices[srcindex];
			Shape*			shape = (Shape*) (SharedObj*) mShapes.GetAt(mtlindex);
			Mesh*			mesh;
			int				tmp = 0;
			float			vtxbuf[VertexPool::MAX_VTX_SIZE];
			Vec4*			dstvtx = (Vec4*) vtxbuf;
			int				dstindex;
			int				startvtx;
			int				endvtx;

			VX_ASSERT(shape);
			mesh = (TriMesh*) shape->GetGeometry();
			VX_ASSERT(mesh);
			dstvtx->x = static_cast<float>(srcloc[0]);
			dstvtx->y = static_cast<float>(srcloc[1]);
			dstvtx->z = static_cast<float>(srcloc[2]);
			dstvtx->w = 1.0f;
			++dstvtx;
			if (mHasNormal)
            {
				FbxVector4 srcnml;
                mMesh->GetPolygonVertexNormal(polyindex, v, srcnml);
                dstvtx->x = static_cast<float>(srcnml[0]);
                dstvtx->y = static_cast<float>(srcnml[1]);
                dstvtx->z = static_cast<float>(srcnml[2]);
                dstvtx->w = 0.0f;
				++dstvtx;
            }
            if (mHasUV)
			{
				Vec2* dstuv = (Vec2*) dstvtx;
				for (int i = 0; i < lUVNames.GetCount(); ++i)
				{
					bool lUnmappedUV;
					FbxVector2 srcuv;

					mMesh->GetPolygonVertexUV(polyindex, v, lUVNames[i], srcuv, lUnmappedUV);
					dstuv->x = static_cast<float>(srcuv[0]);
					dstuv->y = static_cast<float>(srcuv[1]);
					++dstuv;
				}
			}
			dstindex = mVertexCache.AddVertex(vtxbuf);
			VX_ASSERT(dstindex >= 0);
			mesh->AddIndex(dstindex);
			startvtx = mesh->GetStartVtx();
			if (startvtx > dstindex)
				mesh->SetStartVtx(dstindex);
			endvtx = mesh->GetEndVtx();
			if (endvtx < dstindex)
				mesh->SetEndVtx(dstindex);
        }
    }
}

} }	// end FBX
