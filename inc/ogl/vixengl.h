#pragma once

/*!
 * @file vixengl.h
 * @brief Visual Experience Engine OpenGL port.
 *
 * The OpenGL port of the scene manger can use either OpenGL 1.1
 * or OpenGL ES 2.0. The OpenGL 1.1 version relies on the fixed function
 * pipeline whereas the 2.0 version uses shaders.
 *
 * Vertex indices can either be GL_UNSIGNED_INT or GL_UNSIGNED_SHORT.
 * The GLRenderer will convert 32 bit indices to 16 bit if necessary.
 *
 * VIXEN_OGL	if defined, enables OpenGL 1.1 version
 * VIXEN_GLES2	if defined, enables OpenGL ES 2.0 version
 *
 * @author Nola Donato
 * @see vixendx9.h vixen
 */


#ifdef VIXEN_GLES2
#include "ogl/vixen_egl.h"

#else
#include "base/vxbase.h"

namespace Vixen
{
	#define	IMAGE_MaxTexUnits	8
	typedef	IntArray		IndexArray;
	typedef	int32			VertexIndex;
	#define	VXGLIndex		GL_UNSIGNED_INT
}
#include "vxexport.h"
#include "ogl/vxgdev.h"

#if defined(_WIN32)
#include "win32/vixenw.h"

#else
#include "linux/vixenx.h"
#endif

#endif


