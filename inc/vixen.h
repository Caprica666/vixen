#pragma once

#ifdef _MANAGED
#pragma managed(push, off)
#endif


/*!
 * @mainpage VIXEN the Visual Experience Engine
 * @image html vixen_intel_logo.png
 *
 * The Visual Experience Engine is an object-oriented, extensible API callable
 * from C++ or C# for building and manipulating 3D scenes.
 * Vixen can use either OpenGL, Direct3D or Bucky as its underlying graphics pipeline,
 * providing an easy to use, high level interface for working with real time graphics.
 * It can run on both Windows and Linux.
 *
 * Vixen is designed to perform efficiently on low-end devices like smartphones.
 * Because it is multi-threaded internally, Vixen can take advantage of multiple
 * processors if they are available. In multi-processor mode, Vixen
 * maintains separate threads for loading content, performing
 * real-time simulation and scene traversal, and rendering 3D graphics.
 *
 * Vixen builds on the infrastructure in the Core Layer for architecture,
 * memory management and network communication.
 *
 * Summary of features:
 *	- OpenGL and DirectX rendering
 *	- multi-threaded traversal, rendering and loading
 *	- loading of 3D files and textures from HTTP server
 *	- distributed object infrastructure
 *	- portal visibility tools for fast rendering of interior scenes
 *	- 3D picking at sphere, box or mesh level (ray casting)
 *	- terrain navigation
 *	- multimedia support for video textures on 3D objects
 *	- Maya animation and geometry export
 *	- facial tracking and animation
 *	- Kinect body tracking
 *	- skinning and deformation
 *	- Havok cloth simulation
 *
 * @see vcore
 * @author Nola Donato
 *
 */

/*!
 * @defgroup vixen Visual Experience Engine
 *
 * The Visual Experience Engine is an object-oriented, extensible API callable
 * from C++ or C# for building and manipulating 3D scenes.
 * Vixen can use either OpenGL, Direct3D  or Buckyas its underlying graphics pipeline,
 * providing an easy to use, high level interface for working with real time graphics.
 * It runs under both Windows and Linux.
 *
 * Vixen builds on the infrastructure in the Core layer for operating system independence,
 * memory management and network communication.
 *
 * @see vcore
 * @author Nola Donato
 *
 */

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(VIXEN_OGL) || defined(VIXEN_GLES2)
		//
		// OpenGL for rendering
		//
		#pragma warning(disable : 64) // Does not declare anything
//		#pragma warning(disable : 1125)
//		#pragma warning(disable : 1224)
//		#pragma warning(disable : 1595) // Does not declare anything
		#pragma message("OpenGL version of Vixen")
		#include "ogl/vixengl.h"
	#elif defined(VIXEN_DX9)
		//
		// DirectX for rendering
		//
		#pragma message("DirectX 9 version of Vixen")
		#include "dx9/vixendx.h"

	#elif defined(VIXEN_DX11)
		//
		// DirectX for rendering
		//
		#pragma message("DirectX 11 version of Vixen")
		#include "dx11/vixendx11.h"

	#else
		//
		// no rendering component
		//
		#include "base/vxbase.h"
		namespace Vixen
		{
			#define	IMAGE_MaxTexUnits	8
			typedef IntArray IndexArray;
			typedef int32	VertexIndex;
		}	// end Vixen
		#include "vxexport.h"
		#ifdef _WIN32
		#include "win32/vixenw.h"
		#else
		#include "linux/vixenx.h"
		#endif
	#endif


#ifdef _MANAGED
#pragma managed(pop)
#endif