#include "vxmfc.h"
#include "mfc/CameraDlg.h"

using namespace Vixen;

/****
 *
 * Name: MakeNormals
 *
 * Description:
 * Calculate normals for all of the geometry in the given hierarchy.
 * This routine exists until we revise the Studio MAX exporter to
 * calculate the normals instead of using the ones MAX generates
 *
 ****/
void MakeNormals(Model *root)
{
	GroupIter<Shape>	iter((Shape*) root);
	Shape*				pShape;
	Geometry*				pMesh;

	while (pShape = iter.Next())
	{
		if (!pShape->IsClass(VX_Shape))
			continue;
		if (!(pMesh = pShape->GetGeometry()))
			continue;
		if (pMesh->IsClass(VX_TriMesh))
			((TriMesh*) pMesh)->MakeNormals();
	}
}


void GetModelDirection(Model *pModel, Vec3 &dir)
{
	const Matrix *pTransform = pModel->GetTransform();

    dir.x = -1.0f * pTransform->Get(2, 0);
    dir.y = -1.0f * pTransform->Get(2, 1);
    dir.z = -1.0f * pTransform->Get(2, 2);
}

//
// ben's version of "lookat"
// doesn't garauntee any specific roll value, but will probably do what you want
//
void SetModelDirection(Model *pModel, Vec3 dir)
{
	pModel->Reset();

	// create orientation from difference in position
	float mag_xy = sqrtf(dir.x * dir.x + dir.y * dir.y);

	float angle1 = (float)atan2(mag_xy, dir.z);
	float angle2 = (float)atan2(dir.y, dir.x);
	pModel->Rotate(Model::XAXIS, angle1);
	pModel->Rotate(Model::ZAXIS, angle2 - (PI/2.0f));
}

