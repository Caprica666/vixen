#pragma once


#include "dx9/vbufdx.h"

namespace Vixen {

class DeviceInfo;

struct D3DAdapterInfo;

/*!
 * @class DXRender
 *
 * The scene object contains global characteristics that define how
 * models are displayed. A scene describes how objects are
 * being viewed, the environment they are viewed in and
 * drives a simulation component which controls object behaviors.
 * DXRender is a subclass of the generic Scene that uses
 * DirectX as a geometry and rendering engine.
 *
 * @see GLScene Scene
 *
 * @ingroup vixenint
 * @internal
 */
class DXRenderer : public GeoSorter
{
	friend class DXDynamicVerts;
public:
//	Initializers
	DXRenderer(Scene* scene = NULL);
	DXRenderer(DXRenderer& src);

// Device Specific Accessors
	D3DDEVICE*		GetDevice()				{ return d3dDevice; }
	void			AddResource(Property* res);
	DXRenderer&		operator=(const DXRenderer& src);

// overrides
	virtual	SharedObj*	Clone() const;
	virtual	void	SetViewport(float l, float t, float r, float b);
	virtual	void	Begin(int changed, int frame);
	virtual	void	End(int frame);
	virtual	void	Exit();
	virtual	void	Flip(int frame);
	virtual	void	RenderMesh(const Geometry* geo, const Appearance* app, const Matrix* mtx);
	virtual void	RenderState(int32 stateindex);
	virtual	bool	Init(Scene* scene, Vixen::Window win, const TCHAR* options);
	virtual	intptr	AddLight(Light* light);
	bool			InitDX();
	bool			InitializeD3D();
	void			UpdateAppearance(const Appearance* app);
	void*			UpdateTexture(Texture* tex, int texunit, bool ismipmap);

// Public Data
	bool		IsFullScreen;	//!< true forces DX to run fullscreen
	int			NumTexUnits;
	int			MaxVerts;		//!< maximum vertex count
	bool		IsStereo;		//!< true if displaying stereo
	D3DFORMAT FontFormat;		//!< format for font textures

protected:
	void		SetViewMatrix();
	void		SetProjection();
	HRESULT		CreateDevice(HWND win, int adapterid, char* devdesc);
	D3DSURFACE*	ChangeRenderTarget(Bitmap*, D3DFORMAT, DWORD);
	HRESULT		BuildDeviceList(D3DAdapterInfo*, DWORD&);
	HRESULT		ConfirmDevice(D3DCAPS*, DWORD, D3DFORMAT);

	static bool	FindDepthStencilFormat(UINT iAdapter, D3DDEVTYPE DeviceType,
							D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat ); 

	D3DDEVICE*			d3dDevice;			// The D3D rendering device
	D3DFORMAT			m_DepthFormat;		// format of depth buffer
	D3DFORMAT			m_ColorFormat;		// format of color buffer
	DXRef<D3DSURFACE>	m_ColorSurface;		// color buffer render target
	DXRef<D3DSURFACE>	m_DepthSurface;		// depth buffer render target
	Box2				m_Viewport;			// display viewport


// Thread storage
	static const Appearance*	s_CurAppear;	// current appearance
	static D3DCAPS		d3dCaps;			// The device capabilities
	static	vint32		s_MaxAppIndex;		// maximum appearance index
	static IDirect3D9*	s_D3D;				// The main D3D object
	static DWORD		s_Adapter;			// display adapter to use
	static DWORD		s_MinDepthBits;		// Minimum number of bits needed in depth buffer
	static DWORD		s_MinStencilBits;	// Minimum number of bits needed in stencil buffer
	static D3DSURFACE*	s_ColorSurface;		// current color buffer
	static D3DSURFACE*	s_DepthSurface;		// current depth buffer
	static D3DPRESENT_PARAMETERS s_Present;	// present parameters
};

} // end Vixen