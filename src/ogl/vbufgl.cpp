#include "vixen.h"
#include "ogl/vxrendergl.h"
#include "ogl/vbufgl.h"

namespace Vixen
{
int	GLBuffer::s_NumBuffers = 0;

/*
 * Make a D3D buffer of the given size.
 */
GLuint GLBuffer::MakeBuffer(GLRenderer* render, size_t nbytes, int type) 
{
	GLuint	bufid = m_GLBuffer;
	GLenum	err;

	if (bufid)
		Release();
	bufid = 0;
	glGenBuffers(1, &bufid);
	err = glGetError();
	if (err && (bufid == 0))
		VX_ERROR(("GLBuffer::MakeBuffer cannot make buffer %d bytes %x\n", nbytes, err), 0);
	m_GLBuffer = bufid;
	return bufid;
}

void GLBuffer::Release()
{
	if (m_GLBuffer)
	{
		glDeleteBuffers(1, &m_GLBuffer);
		if (m_GLBuffer == s_NumBuffers)
			--s_NumBuffers;
	}
	m_GLBuffer = 0;
}


bool GLBuffer::Update(const DeviceBuffer* databuf, GLuint program, const TCHAR* prefix)
{
	const DataLayout*	layout = databuf->GetLayout();
	const float*		data = (float*) databuf->GetData();
	TCHAR				name[256];

	VX_ASSERT(layout);
	VX_ASSERT(data);
	for (int i = 0; i < layout->NumSlots; ++i)
	{
		const LayoutSlot& slot = layout->Slot[i];
		GLint			attrib;
		bool			isint = (slot.Style & VertexPool::INTEGER) != 0;
		const float*	v = data + slot.Offset;

		if (prefix)
		{
			STRCPY(name, prefix);
			STRCAT(name, slot.Name);
		}
		else
			STRCPY(name, slot.Name);
		attrib = glGetUniformLocation(program, name);
		if (attrib >= 0)
			switch (slot.Size)
			{
				case 1:		if (isint) glUniform1iv(attrib, 1, (const GLint*) v); else glUniform1fv(attrib, 1, (const GLfloat*) v); break;
				case 2:		if (isint) glUniform2iv(attrib, 1, (const GLint*) v); else glUniform2fv(attrib, 1, (const GLfloat*) v); break;
				case 3:		if (isint) glUniform3iv(attrib, 1, (const GLint*) v); else glUniform3fv(attrib, 1, (const GLfloat*) v); break;
				case 4:		if (isint) glUniform4iv(attrib, 1, (const GLint*) v); else glUniform4fv(attrib, 1, (const GLfloat*) v); break;
				case 16:	VX_ASSERT(isint == false); glUniformMatrix4fv(attrib, 1, false, (const GLfloat*) v);
				case 9:		VX_ASSERT(isint == false); glUniformMatrix3fv(attrib, 1, false, (const GLfloat*) v);
				default:
				VX_ERROR(("GLBuffer::Update ERROR invalid uniform vector size %d\n", slot.Size), false);
			}
		else
		{
			VX_TRACE(GLRenderer::Debug > 1, ("GLBuffer::Update element %s not found\n", (const TCHAR*) name));
		}
	}
#ifdef _DEBUG
	GLint err = glGetError();
	if (err && (GLRenderer::Debug > 1))
		VX_ERROR(("GLBuffer::Update ERROR failed to update some buffer elements %x\n", err), false);
#endif
	return true;
}

void GLBuffer::ReleaseAll()
{
	for (GLuint bufid = 1; bufid != s_NumBuffers; ++bufid)
		glDeleteBuffers(1, &bufid);
	s_NumBuffers = 0;
}

/*
 * Copy the indices from the input array into the D3D index buffer.
 * The previous contents of the index buffer are discarded.
 * This routine will create a D3D index buffer large enough to hold the
 * input data if one does not already exist.
 */
bool GLIndexBuf::Update(GLRenderer* render, const IndexArray* inds, bool dynamic) 
{
	ObjectLock		lock(inds);
	const int32*	data;
	size_t			size = inds->GetSize() * sizeof(VXGLIndex);
	GLuint			bufid = m_GLBuffer;

	if (size == 0)
		return false;
	data = inds->GetData();
	VX_ASSERT(data);
	if (bufid == 0)
	{
		bufid = MakeBuffer(render, size, GL_ELEMENT_ARRAY_BUFFER);
		if (bufid == 0)
			return false;
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufid);
	if (VXGLIndex == GL_UNSIGNED_SHORT)
	{
		int16*	tmpdata = (int16*) Core::ThreadAllocator::Get()->Alloc(size);
		for (intptr i = 0; i < inds->GetSize(); ++i)
			tmpdata[i] = (int16) data[i];
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, tmpdata, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		Core::ThreadAllocator::Get()->Free(tmpdata);
	}
	else glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#ifdef _DEBUG
	GLenum err = glGetError();
	if (err)
		VX_ERROR(("GLIndexBuf::Update ERROR cannot update buffer %x\n", err), 0);
#endif
	VX_TRACE(GLRenderer::Debug, ("GLIndexBuf::Update #%d %d bytes\n", bufid, size));
	return true;
}

/*
 * Copy all of the vertices in the input array into the vertex buffer.
 * The previous contents of the vertex buffer are overwritten.
 */
bool GLVertexBuf::Update(GLRenderer* render, const VertexArray* verts, bool dynamic)
{
	ObjectLock	lock(verts);
	size_t		buffsize = verts->GetVtxSize() * verts->GetNumVtx() * sizeof(float);
	const void*	data = verts->GetData();
	GLuint		bufid = GetBuffer();

	if (buffsize == 0)
		return false;
	VX_ASSERT(data);
	if (bufid == 0)
	{
		bufid = MakeBuffer(render, buffsize);
		if (bufid == 0)
			return false;
	}
	glBindBuffer(GL_ARRAY_BUFFER, bufid);
	glBufferData(GL_ARRAY_BUFFER, buffsize, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#ifdef _DEBUG
	GLenum	err = glGetError();
	if (err)
		VX_ERROR(("GLVertexBuf::Update ERROR cannot update buffer %x\n", err), 0);
#endif
	VX_TRACE(GLRenderer::Debug, ("GLVertexBuf::Update #%d %d bytes\n", bufid, buffsize));
	return true;
}

void GLVertexBuf::BindAttribs(const DataLayout* layout, GLuint glprogram)
{
	int		stride = layout->Size * sizeof(float);

	for (int i = 0; i < layout->NumSlots; ++i)
	{
		const LayoutSlot& slot = layout->Slot[i];

		glBindAttribLocation(glprogram, i, slot.Name);
	}
}

void GLVertexBuf::EnableAttribs(const DataLayout* layout, GLuint glprogram)
{
	int		stride = layout->Size * sizeof(float);

	for (int i = 0; i < layout->NumSlots; ++i)
	{
		const LayoutSlot& slot = layout->Slot[i];
		GLuint	gltype = (slot.Style & VertexPool::INTEGER) ? GL_INT : GL_FLOAT;

		glVertexAttribPointer(i, slot.Size, gltype, false, stride, (const void*) (slot.Offset * sizeof(float)));
		glEnableVertexAttribArray(i);
#ifdef _DEBUG
		{
			GLint err = glGetError();
			if (err)
				VX_TRACE(GLRenderer::Debug, ("GLRenderer::RenderMesh Cannot update vertex attribute %s %x\n", (const TCHAR*) slot.Name, err));
		}
#endif
	}
}

void GLVertexBuf::DisableAttribs(const DataLayout* layout, GLuint glprogram)
{
	for (int i = 0; i < layout->NumSlots; ++i)
	{
		const LayoutSlot& slot = layout->Slot[i];

		glDisableVertexAttribArray(i);
	}
	glGetError();
}

} // end Vixen