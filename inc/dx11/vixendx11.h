#pragma once

/*!
 * @file vixencput.h
 * @brief Visual Experience Engine Bucky port
 * *
 * To use the CPUT version of Vixen, define VIXEN_CPUTKY in
 * your C++ preprocessor variables and link with vixencput.lib.
 *
 * @author Nola Donato
 */
#include "base/vxbase.h"

#ifdef _WIN32
#include "win32/vixenw.h"
#else
#include "linux/vixenx.h"
#endif
namespace Vixen
{
	#define	VX_VEC3_SIZE	4
	#define	IMAGE_MaxTexUnits	8
	typedef IntArray IndexArray;
	typedef uint32	VertexIndex;
};

#include "vxexport.h"
#include <d3d11.h>
#include <d3dcompiler.h>

#define	D3DDEVICE			ID3D11Device
#define	D3DCONTEXT			ID3D11DeviceContext
#define	D3DBUFFER			ID3D11Buffer
#define	D3DSHADERVIEW		ID3D11ShaderResourceView
#define	D3DINPUTLAYOUT		ID3D11InputLayout
#define	D3DTEXTURE2D		ID3D11Texture2D
#define	D3DSAMPLER			ID3D11SamplerState

#define	D3D_BUFFERDESC		D3D11_BUFFER_DESC
#define	D3D_DYNAMIC			D3D11_USAGE_DYNAMIC
#define	D3D_DEFAULT			D3D11_USAGE_DEFAULT
#define	D3D_CONSTANTBUFFER	D3D11_BIND_CONSTANT_BUFFER
#define	D3D_VERTEXBUFFER	D3D11_BIND_VERTEX_BUFFER
#define	D3D_INDEXBUFFER		D3D11_BIND_INDEX_BUFFER

#include "dxref.h"
#include "vbufdx.h"
#include "renderdx.h"

