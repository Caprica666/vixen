#include "dx11/vixendx11.h"

namespace Vixen
{

/*!
 * @fn bool DXRenderer::Init(Scene* scene, Vixen::Window window, const TCHAR* options)
 * This routine is called by the rendering thread to initialize a DirectX
 * device for display. It assumes the scene has already been attached to a
 * window and is not already in use.
 *
 * @return \b true if DirectX successfully initialized, else \b false
 *
 * @see Scene::InitThread Scene::EndThreadScene::SetWindow
 */
bool DXRenderer::Init(Scene* scene, Vixen::Window window, const TCHAR* options)
{
	World3D*	w = World3D::Get();
	int			width = (int) w->WinRect.Width();
	int			height = (int) w->WinRect.Height();

	if (!Renderer::Init(scene, window, options))
		return false;
#ifdef _WIN32
	HWND	win = (HWND) window;
	RECT	r;

	::GetClientRect(win, &r);
	if (width == 0)
		width = abs(r.right - r.left);
	if (height == 0)
		height = abs(r.bottom - r.top);
#else
	Window		win = (Window) window;
#endif
	Bitmap::RGBADepth = 32;
	Bitmap::RGBDepth = 32;
	m_ColorSurface = NULL;
	m_DepthSurface = NULL;
	if (!CreateDevice(win, width, height))
		return false;
	if (!CreateBuffers(width, height))
		return false;
	MakeDefaultShaders();
	VX_TRACE(Debug, ("DXRenderer::Init successfully initialized DX11\n"));
	Messenger::SysVecSize = 4;
	return true;
}

void DXRenderer::FreeBuffers()
{
	ID3D11RenderTargetView* colorsurf = (ID3D11RenderTargetView*) m_ColorSurface;
	ID3D11DepthStencilView* depthsurf = (ID3D11DepthStencilView*) m_DepthSurface;
	D3DTEXTURE2D* depthtex = (D3DTEXTURE2D*) m_DepthTexture;

	if (colorsurf)
		colorsurf->Release();
	m_ColorSurface = NULL;
	if (depthtex)
		depthtex->Release();
	if (depthsurf)
		depthsurf->Release();
	m_DepthSurface = NULL;
}

bool DXRenderer::CreateBuffers(int w, int h)
{
	/*
	 * Store back buffer description and references to
	 * color and depth buffer render targets
	 */
	HRESULT	hr;
	D3DTEXTURE2D*		pBackBuffer;
	ID3D11RenderTargetView* colorsurf = NULL;
	D3D11_RENDER_TARGET_VIEW_DESC colordesc;

	hr = m_SwapChain->GetBuffer(0, __uuidof(D3DTEXTURE2D), (LPVOID*) &pBackBuffer);
	if (FAILED(hr))
		{ VX_ERROR(("InitDX: Cannot get back buffer\n"), false); }
	/*
	 * Set up render target for color buffer
	 */
	hr = m_Device->CreateRenderTargetView(pBackBuffer, NULL, &colorsurf);
	pBackBuffer->Release();
	if (SUCCEEDED(hr))
	{
		colorsurf->GetDesc(&colordesc);
		m_ColorFormat = colordesc.Format;
		m_ColorSurface = colorsurf;
	}
	else
	{
		VX_ERROR(("InitDX: Cannot create render target view\n"), FALSE);
	}
	/*
	 * Create depth stencil buffer and bind it to the device
	 */
	D3DTEXTURE2D* depthtex = NULL;
	D3D11_TEXTURE2D_DESC texdesc;
	ID3D11DepthStencilView* depthsurf = NULL;
	UINT sampleRate = 1;

	texdesc.Width = w;
	texdesc.Height = h;
	texdesc.MipLevels = 1;
	texdesc.ArraySize = 1;
	texdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texdesc.SampleDesc.Count = 1;
	texdesc.SampleDesc.Quality = 0;
	texdesc.Usage = D3D11_USAGE_DEFAULT;
	texdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texdesc.CPUAccessFlags = 0;
	texdesc.MiscFlags = 0;

	hr = m_Device->CreateTexture2D(&texdesc, NULL, &depthtex);
	if (FAILED(hr))
		{ VX_ERROR(("InitDX: Cannot create depth stencil buffer texture\n"), false); }
	m_DepthTexture = depthtex;
	hr = m_Device->CreateDepthStencilView(depthtex, NULL, &depthsurf);
	if (FAILED(hr))
		{ VX_ERROR(("InitDX: Cannot create depth stencil view\n"), FALSE); }
	m_DepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_DepthSurface = depthsurf;
	m_Context->OMSetRenderTargets(1, &colorsurf, depthsurf);
	BackWidth = w;
	BackHeight = h;
	return true;
}


bool DXRenderer::CreateDevice(HWND win, int width, int height)
{
	UINT			enum_flags = 0;
	HRESULT			hr;
	UINT			outnum = 0;
	bool			rc = false;
	UINT			numModes;
	IDXGIAdapter*	adapter = NULL;
	IDXGIOutput*	output;
	IDXGIDevice*	dxgi;
	IDXGIFactory*	factory;
	DXGI_MODE_DESC*	pModeList;

	DXGI_MODE_DESC modeDesc = {0};
	DXGI_MODE_DESC actualMode = {0};
	DXGI_OUTPUT_DESC outputDesc = {0};
	DXGI_SWAP_CHAIN_DESC swapDesc;
	int					flags = 0;

#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    ZeroMemory(&swapDesc, sizeof(swapDesc));
    swapDesc.BufferCount = 1;
    swapDesc.BufferDesc.Width = width;
    swapDesc.BufferDesc.Height = height;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = win;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
//	swapDesc.Windowed = IsFullScreen ? FALSE : TRUE;
	swapDesc.Windowed = TRUE;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	D3D_FEATURE_LEVEL	features[5] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1 };
	D3D_FEATURE_LEVEL	f;
	/*
	 * Make the D3D Device
	 */
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, features, 3,
										D3D11_SDK_VERSION, &swapDesc, &m_SwapChain, &m_Device, &f, &m_Context);
	if (SUCCEEDED(hr))
		return true;
	if (flags)
	{
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, features, 3,
											D3D11_SDK_VERSION, &swapDesc, &m_SwapChain, &m_Device, &f, &m_Context);
		if (SUCCEEDED(hr))
			return true;
	}
	hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, features, 5, D3D11_SDK_VERSION, &m_Device, &f, &m_Context);
	if (FAILED(hr))
		{ VX_ERROR(("InitDX: cannot create DX11 device\n"), false); }
	hr = m_Device->QueryInterface(__uuidof(*dxgi), (void**) &dxgi);
	if (SUCCEEDED(hr))
	{
		hr = dxgi->GetAdapter(&adapter);
		dxgi->Release();
		if (SUCCEEDED(hr))
			hr = adapter->GetParent(__uuidof(*factory), (void**) &factory);
	}
	if (FAILED(hr))
	{
		adapter->Release();
		VX_ERROR(("InitDX: cannot get adapter\n"), false);
	}
	dxgi->Release();
	/*
	 * Find the best backbuffer format
	 */
	factory->MakeWindowAssociation(swapDesc.OutputWindow, 0);
	output = NULL;
	while (SUCCEEDED(adapter->EnumOutputs(outnum++, &output)))
	{
		hr = output->GetDesc(&outputDesc);
		if (output)
			output->Release();
		if (FAILED(hr))
		{
			VX_WARNING(("InitDX: failed to get output device descriptor\n"));
			continue;
		}
		modeDesc.Width = width;
		modeDesc.Height = height;
		modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		hr = output->FindClosestMatchingMode(&modeDesc, &actualMode, NULL);
		if (SUCCEEDED(hr))
		{
			swapDesc.BufferDesc = actualMode;
			rc = true;
			break;
		}
		hr = output->GetDisplayModeList(swapDesc.BufferDesc.Format, enum_flags, &numModes, NULL);
		if (FAILED(hr) || numModes == 0)
		{
			VX_WARNING(("InitDX: cannot get display modes for device\n"));
			continue;
		}
		try
		{
			pModeList = new DXGI_MODE_DESC[numModes];
		}
		catch (...)
		{
			pModeList = NULL;
		}
		if (pModeList == NULL)
			return false;
		hr = output->GetDisplayModeList(swapDesc.BufferDesc.Format, enum_flags, &numModes, pModeList);
		if (FAILED(hr) || numModes == 0)
			{ VX_ERROR(("InitDX: cannot get display modes for device\n"), false); }
		swapDesc.Windowed = FALSE;
		rc = true;
		break;
	}
	output->Release();
	factory->Release();
	adapter->Release();
	return rc;
}


void DXRenderer::FreeResources()
{
	for (int i = 0; i < 16; ++i)
		VertexShaders[i] = NULL;
	for (int i = 0; i <= CBUF_MAX; ++i)
		m_ConstantBuffers[i] = NULL;
	if (m_Device)
	{
		m_Device->Release();
		m_Device = NULL;
	}
	if (m_Context)
	{
		m_Context->Release();
		m_Context = NULL;
	}
}

}