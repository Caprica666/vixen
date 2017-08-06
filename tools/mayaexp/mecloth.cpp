#include "vix_all.h"
#include <maya/MFnAttribute.h>
#include <maya/MFnDagNode.h>
#include <maya/MAnimUtil.h>
#include <maya/MFnGeometryFilter.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MItGeometry.h>
#include <maya/MItDependencyGraph.h>


/*!
 * @fn SharedObj* meConvCloth::Make()
 *
 * Creates a Vixen Morph engine from the Maya blend shape node.
 * The Vixen engine is empty upon return from this routine.
 *
 * @return Deformer engine created for the Maya node, NULL if Maya node cannot be converted
 *
 * @see meConvEng::Convert
 */
SharedObj* meConvCloth::Make()
{
	MFnDagNode		cloth(m_MayaObj);
	MeshAnimator*	meshanim;
	Core::String	name(GetMayaName());
	meConnIter		iter(m_MayaObj, "inputMesh", true);
	MPlug			plug;
	MObject			obj;

	meConvObj::PrintPlugs(m_MayaObj);
	while (iter.Next(obj, MFn::kMesh))
	{
		m_RestMesh = obj;			// found the input mesh
		break;
	}
	iter.Reset(m_MayaObj, "outputMesh", false);
	while (iter.Next(plug))
	{
		obj = plug.node();
		if (obj.hasFn(MFn::kMesh))			// is a mesh?
		{
			m_OutMesh = obj;				// found the output mesh
			break;
		}
		if (obj.hasFn(MFn::kGeometryFilt))	// multiple geometry outputs
		{
			MFnGeometryFilter gf(obj);
			MObjectArray meshes;

			gf.getOutputGeometry(meshes);
			if (meshes.length() > 0)
			{
				m_OutMesh = meshes[0];
				break;
			}
		}
		iter.Reset(obj, "outputGeometry", false);
		meConvObj::PrintPlugs(obj);
	}
	if (m_OutMesh.isNull())
		ME_ERROR(("%s - cannot find Maya output mesh to deform", m_MayaName), NULL);	
	if (m_RestMesh.isNull())
		ME_ERROR(("%s - cannot find input mesh with rest positions", m_MayaName), NULL);
	m_VXObj = meshanim = new MeshAnimator();
	name += ".meshanim";
	SetName(meshanim, name);
	Exporter->SimRoot->Append(meshanim);
	return meshanim;
}

int meConvCloth::Link(meConvObj* convparent)
{
	MFnDagNode		cloth(m_MayaObj);
	MeshAnimator*	meshanim = (MeshAnimator*) (SharedObj*) m_VXObj;
	MFnDagNode		dagMesh(m_OutMesh);
	MDagPath		dagPath;
	ConvRef			cvref;
	meConvObj*		conv = NULL;
	Shape*			shape = NULL;
	meConvMesh*		meshconv;
	int				rc = 0;

	if (convparent)
		meConvEng::Link(convparent);
	/*
	 * Find the output mesh and make it the target of the deformer.
	 */
	dagMesh.getPath(dagPath);
	cvref = Exporter->MayaNodes[dagPath];
	conv = cvref;
	meLog(2, "%s: found output geometry %s", GetMayaName(), dagPath.partialPathName().asChar());
	if (conv != NULL)
		shape = (Shape*) (SharedObj*) conv->GetVXObj();
	if (shape && shape->IsClass(VX_Shape))
	{
		meshconv = dynamic_cast<meConvMesh*>( conv );
		if (meshconv)
		{
			LinkTarget(meshconv);
			rc = 1;
		}
	}
	if (!rc)
		ME_ERROR(("%s - cannot find Vixen output mesh to deform", m_MayaName), 0);	
	/*
	 * Find the mesh with the rest positions.
	 */
	if (!MakeRestVerts())
		ME_ERROR(("%s - cannot make rest vertices", m_MayaName), 0);	
	return 1;
 }

VertexArray* meConvCloth::MakeRestVerts()
{
	VertexArray*	restverts = NULL;
	MeshAnimator*	meshanim = (MeshAnimator*) (SharedObj*) m_VXObj;
	MFnMesh			mesh(m_RestMesh);
	int				n = mesh.numVertices();
	MDagPath		dagPath;
	Core::String	meshname(mesh.name().asChar());
	Core::String	tmp(meshname.Right(5));

	if (tmp == "Shape")
		meshname = meshname.Left(meshname.GetLength() - 5);
	meshname += ".meshanim";
	meshanim->SetName(meshname);
	restverts = new VertexArray("position float 3", n);
	SetName(restverts, meshname);
	restverts->SetNumVtx(n);
	meshanim->SetRestPose(restverts);
	mesh.getPath(dagPath);
	meLog(2, "%s: found input geometry %s", GetMayaName(), meshname);

	VertexArray::Iter	diter(restverts);
	for (int i = 0; i < n; ++i)
	{
		Vec3*	dst = (Vec3*) diter.Next();
		MPoint	p;

		mesh.getPoint(i, p);
		dst->Set(p.x, p.y, p.z);
	}
	return restverts;
}

