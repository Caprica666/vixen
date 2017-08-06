#include "vixen.h"
#include "ogl/vxrendergl.h"
#include "ogl/vxocull.h"

namespace Vixen {

CullList::CullList()
  :	GLRenderer()
{
	const char* glext = (const char*) glGetString(GL_EXTENSIONS);
	if (strstr(glext, "GL_HP_occlusion_test"))
	{
		SetOptions(GeoSorter::NoStateSort | GeoSorter::Flatten);
		HasOcclusion = true;
	}
	else
		HasOcclusion = false;
}

/****
 *
 * CullList::Cull
 * Determines whether the given mesh is visible
 * (based on OGL occlusion check of its bounding box)
 *
 ****/
bool CullList::Cull(const Geometry* g) const
{
	Box3		b;
	GLboolean	result;

	if (g->IsSet(GEO_Culled))
		return true;
	if (!g->GetBound(&b))
		return true;
	RenderBox(b);					// render bounding box
	glGetBooleanv(GL_OCCLUSION_TEST_RESULT_HP, &result);
	if (!result)					// was not visible?
		g->SetFlags(GEO_Culled);	// mark as culled
	return true;
}

/****
 *
 * CullList_Append
 * Appends a primitive onto the list for this shape only if it is
 * visible (based on OGL occlusion check of its bounding box)
 *
 ****/
int CullList::AddShape(Shape* shape, const Matrix* mtx, int32 lightmask)
{
	Box3			b;
	GLboolean	result;
	int			rc = -1;

	if (shape->WasRendered())						// already rendered?
		return -1;									// don't render
	glDepthMask(GL_FALSE);							// disable drawing
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_OCCLUSION_TEST_HP);					// enable occlusion test
	if (shape->IsParent() && shape->GetBound(&b, Model::LOCAL))
	{
		RenderBox(b);								// render hierarchy bounds
		glGetBooleanv(GL_OCCLUSION_TEST_RESULT_HP, &result);
		if (result && GeoSorter::AddShape(shape, mtx, lightmask))	// something visible?
			rc = 1;
	}
	glDisable(GL_OCCLUSION_TEST_HP);				// disable occlusion test
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);// enable depth, color buffers
	glDepthMask(GL_TRUE);
	return rc;										// render, something was visible
}

void CullList::RenderBox(Box3& b) const
{
	float	verts[8 * 3] =
	{
			b.min.x, b.min.y, b.max.z,
			b.min.x, b.max.y, b.max.z,
			b.max.x, b.max.y, b.max.z,
			b.max.x, b.min.y, b.max.z,
			b.min.x, b.min.y, b.min.z,
			b.min.x, b.max.y, b.min.z,
			b.max.x, b.max.y, b.min.z,
			b.max.x, b.min.y, b.min.z,
	};

	float	idx[6][4] =
	{
		4, 5, 6, 7,                 /* back */
		3, 7, 6, 2,                 /* right */
		3, 2, 1, 0,                 /* front */
		0, 1, 5, 4,                 /* left */
		6, 5, 1, 2,                 /* top */
		4, 7, 3, 0,                 /* bottom */
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 3, verts);
	for (int i = 0; i < 6; ++i)
		glDrawElements(GL_TRIANGLE_FAN, 4, VXGLIndex, &idx[i][0]);
}

/****
 *
 * CullList_Active
 * Renders the primitives accumulated and rendered last frame
 * and them empties the list. Takes advantage of frame to frame
 * coherency and tries to draw occluders first.
 *
 ****/
void CullList::Reset()
{
	for (int s = GeoSorter::Opaque; s < m_States.GetSize(); ++s)
		GeoSorter::RenderState(s);
}		

/****
 *
 * CullList_Render
 * Renders transparent primitives. Requests to render opaque
 * primitives (GeoSorter::Opaque) are ignored because these
 * were rendered during AddShape as part of the culling process
 *
 ****/
void CullList::Render(int32 opts)
{
	if (opts & GeoSorter::Opaque)	// ignore opaque stuff
		m_IsEmpty = true;			// but leave it there	
	GeoSorter::Render(opts);		
}

}	// end Vixen