/****
 *
 *
 * These are the OpenGL implementations of geometry rendering
 *	TriMesh::Render	Mesh::Render
 *
 * These routines assume that the vertices are stored in AOS or
 * "array of structures" format. Each vertex occupies a fixed size
 * contiguous chunk of a large memory block containing all vertices.
 *
 * See: vtxaos.cpp vxvtxaos.h
 *
 ****/
#include "vixen.h"
#include "ogl/vxrendergl.h"
#include "ogl/vxshadergl.h"
#include "ogl/vbufgl.h"
	
namespace Vixen {

void	GLRenderer::RenderMesh(const Geometry* geo, const Appearance* app, const Matrix* mtx)
{
	const Mesh*			geomesh = (Mesh*) geo;
	const VertexArray*	verts;
	int					changed = m_Changed;
	bool				meshchanged = geomesh->HasChanged();
	int					rendertype;

	VX_ASSERT(geo->IsClass(VX_Mesh));
	verts = geomesh->GetVertices();
	if (verts == NULL)
		return;
	/*
	 * Update the GL program based on changes to the Appearance
	 */
	if (app == NULL)
		app = s_DefaultAppear;
	switch (app->Get(Appearance::SHADING))
	{
		case Appearance::POINTS: rendertype = GL_POINTS; break;
		case Appearance::WIRE:	rendertype = GL_LINES; break;
		default:				rendertype = GL_TRIANGLES; break;
	}
	if (app != s_CurAppear)			// appearance changed?
	{
		s_CurAppear = app;
		if (UpdateAppearance(app, verts->GetStyle()))
			changed = -1;
		if (m_CurProgram == 0)
			return;
		InitPerFrame(m_CurProgram, changed);
	}
	/*
	 * Update vertex and index buffers if changed
	 * and draw the mesh
	 */
	ObjectLock			lock2(geomesh);
	const IndexArray*	inds = geomesh->GetIndices();
	GLVertexBuf&		glvbuf = (GLVertexBuf&) verts->DevHandle;
	GLuint				mtxloc = glGetUniformLocation(m_CurProgram, TEXT("WorldMatrix"));
	GLint				err;
	GLint				start = (GLint) geomesh->GetStartVtx();
	GLint				end = (GLint) geomesh->GetEndVtx();
	GLint				nverts = (GLint) geomesh->GetNumVtx();

	VX_ASSERT(geomesh->GetNumVtx() < INT_MAX);
	VX_ASSERT(geomesh->GetStartVtx() < INT_MAX);
	VX_ASSERT(geomesh->GetEndVtx() < INT_MAX);
	if (start < 0)
		start = 0;
	if (end > 0)
		nverts = end - start;
	else
		end = nverts - 1;

	geo->SetChanged(false);
	if (mtxloc > 0)
		glUniformMatrix4fv(mtxloc, 1, false, mtx->GetMatrix());
	if (!glvbuf.HasBuffer() || verts->HasChanged() || meshchanged)
	{
		glvbuf.Update(this, verts, verts->IsSet(VertexPool::MORPH));
		verts->SetChanged(false);
	}
	else
		glBindBuffer(GL_ARRAY_BUFFER, glvbuf.GetBuffer());
	VX_TRACE(Debug > 1, ("GLRenderer::RenderMesh using vertex buffer #%d\n", glvbuf.GetBuffer()));
	glvbuf.EnableAttribs(verts->GetLayout(), m_CurProgram);
	if (inds)
	{
		GLIndexBuf& ibuf = (GLIndexBuf&) geomesh->DevHandle;
		GLuint		nidx = (GLuint) inds->GetSize();
		GLuint		glibuf = ibuf.GetBuffer();

		VX_ASSERT(nidx < INT_MAX);
		if (!glibuf || meshchanged)
		{
			ibuf.Update(this, inds, 0);
			inds->SetChanged(false);
		}
		else
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glibuf);
		VX_TRACE(Debug > 1, ("GLRenderer::RenderMesh using index buffer #%d\n", glibuf));
		glDrawRangeElements(rendertype, start, end, nidx, VXGLIndex, 0);
	}
	else
		glDrawArrays(rendertype, start, nverts);
#ifdef _DEBUG
	err = glGetError();
	if (err)
		VX_WARNING(("GLRenderer::RenderMesh GL ERROR %x\n", err));
#endif
	glvbuf.DisableAttribs(verts->GetLayout(), m_CurProgram);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}	// end Vixen
