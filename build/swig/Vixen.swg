/*
 * SWIG wrapper for Vixen API
 */
%module Vixen
%nspace Vixen;
#pragma SWIG nowarn=325,503,516
#define VIXEN_MEDIA_CV
#define FORCE_INLINE inline
#define	ALIGN16
#undef THREAD_LOCAL
#define THREAD_LOCAL

typedef unsigned int	uint32;     // 32 bit unsigned
typedef int	            int32;      // 32 bit signed
typedef int				vint32;		// volatile 32 bit signed
typedef unsigned char   uchar;      // 8 bit unsigned
typedef unsigned short  uint16;     // 16 bit unsigned
typedef short           int16;      // 16 bit signed
typedef unsigned char	uint8;
typedef unsigned char	ubyte;
typedef char			int8;
typedef long long		int64;		// 64 bit integer 
typedef char			TCHAR;
typedef wchar_t			WCHAR;
#if sizeof(void*) > sizeof(int)
typedef int				intptr;
#else
typedef long long		intptr;
#endif

%include "typemaps.i"
%include "enums.swg"
%include "wchar.i"
%include "attribute.i"
%include "windows.i"

#ifdef SWIGCSHARP
%include "arrays_csharp.i"
#endif


%{
#pragma warning(disable:4197)
#include "vixen.h"
#include "vxutil.h"
#include "physics/havok/vxphysics.h"
#include "physics/havok/vxcloth.h"

using namespace Vixen;
using namespace Vixen::Core;

%}

%apply float INPUT[]	{ const float* floatArray }
%apply float INPUT[]	{ float* floatArray }
%apply float INPUT[]	{ float* floatArray1 }
%apply float INPUT[]	{ float* floatArray2 }
%apply float INPUT[]	{ float* floatArray3 }
%apply float OUTPUT[]	{ float* dstArray }
%apply float INPUT[]	{ float [4][4] }
%apply int	INPUT[]		{ int* intArray }
%apply int	INPUT[]		{ int32* intArray }

%ignore operator!;
%ignore	operator<<;
%ignore	operator>>;
%ignore operator[];
%ignore	operator new;
%ignore	operator delete;

namespace  Vixen
{
	typedef intptr	Window;		// window handle
	typedef void	Renderer;

	%ignore operator SharedObj*;
	%ignore	Core::Stream;
	%ignore Messenger;
	%ignore VXOpcode;
	%ignore BuMessenger;
	%ignore	CompareKeys;
	%ignore	HashKey;
	%ignore	PROP_MaxScript;
	%ignore	GetTime;
	%ignore VX_USE_SCRIPTOR;
	%ignore VX_MAX_LAYOUT_SLOTS;
	%ignore VX_VEC3_SIZE;
	%rename(Assign)	operator=;
}

%include "scene/vxtypes.h"
%include "VCore.swg"
%include "VString.swg"
%include "VXObj.swg"
%include "VXArray.swg"
%include "VXMath.swg"
%include "VXBox.swg"
%include "VXGroup.swg"
%include "VXModel.swg"
%include "VXTexture.swg"
%include "VXAppear.swg"
%include "VXMesh.swg"
%include "VXShape.swg"
%include "VXAnim.swg"
%include "VXScene.swg"
%include "VXWorld.swg"
%include "VXUtil.swg"
%include "VXEvent.swg"
%include "VXBody.swg"
%include "Viewer.swg"
%include "VXPhysics.swg"

#ifdef VIXEN_MEDIA_CV
%include "VXMedia.swg"
#endif


