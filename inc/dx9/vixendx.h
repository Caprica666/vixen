#pragma once

/*!
 * @file vixendx.h
 * @brief Visual Experience Engine DirectX port.
 *
 * The DirectX port of the scene manager uses 16 bits
 * to describe vertex indices and can support a
 * maximum of 8 simultaneous textures. It requires
 * DirectX 8 or 9 and will use multi-texturing hardware
 * efficiently if it is available.
 *
 * To use the DirectX version of Vixen, define VIXEN_DX9 in
 * your C++ preprocessor variables and link with vixendx9.lib.
 *
 *
 * @author Judith Stanley, Nola Donato
 * @see vixengl.h vixen
 */
#define word WORD
#define	DIRECTINPUT_VERSION	0x0900

#include "d3d9.h"
#include "dinput.h"

#define D3D_OVERLOADS

#include "base/vxbase.h"

namespace Vixen
{
	#define	IMAGE_MaxTexUnits	8

	#define D3DDEVICE		IDirect3DDevice9
	#define D3DVERTEXBUFFER IDirect3DVertexBuffer9
	#define D3DINDEXBUFFER	IDirect3DIndexBuffer9
	#define D3DSURFACE		IDirect3DSurface9
	#define D3DCAPS			D3DCAPS9
	#define D3DTEXTURE		IDirect3DTexture9
	#define D3DVIEWPORT		D3DVIEWPORT9
	#define D3DCreate		Direct3DCreate9

	typedef IntArray IndexArray;
	typedef uint32	VertexIndex;
} // end Vixen

#include "vxexport.h"
#include "win32/vixenw.h"
#include "dx9/dxref.h"

