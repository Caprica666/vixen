#pragma once


namespace Vixen
{
class GLRenderer;

class GLBuffer
{
public:
	GLBuffer() : m_GLBuffer(0) { }

	GLBuffer&	operator=(GLuint bufid);
	bool		HasBuffer() const	{ return m_GLBuffer != 0; }
	GLuint		MakeBuffer(GLRenderer* render, size_t nbytes, int type = GL_ARRAY_BUFFER);
	GLuint		GetBuffer()	{ return m_GLBuffer; }
	void		Release();
	
	static bool		Update(const DeviceBuffer* devbuf, GLuint program, const TCHAR* prefix = NULL);	
	static	void	ReleaseAll();
protected:
	GLuint			m_GLBuffer;

	static int		s_NumBuffers;
};


/*!
 * @class GLIndexBuf
 *
 * Encapsulates a Direct3D index buffer for a mesh.
 * This property is attached to the index buffer when a D3D representation
 * of the index data is created.
 *
 *
 * @see IndexArray GLBuffer
 */
class GLIndexBuf : public GLBuffer
{
public:
	GLIndexBuf();

	bool	Update(GLRenderer*, const IndexArray*, bool dynamic = false);
};

/*!
 * @class GLVertexBuf
 *
 * Encapsulates a Direct3D vertex buffer for a mesh.
 * This property is attached to a vertex array when a D3D representation
 * of the vertex data is created.
 *
 *
 * @see VertexArray GLBuffer
 */
class GLVertexBuf : public GLBuffer
{
public:
	GLVertexBuf();

	bool			Update(GLRenderer*, const VertexArray*, bool dynamic = false);
	static	void	BindAttribs(const DataLayout* layout, GLuint glprogram);
	static	void	EnableAttribs(const DataLayout* layout, GLuint glprogram);
	static	void	DisableAttribs(const DataLayout* layout, GLuint glprogram);
};

}	// end Vixen