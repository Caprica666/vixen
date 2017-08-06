#include "vixen.h"
#include "dx9/renderdx.h"

namespace Vixen {

/*
 * Error codes
 */
enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF, MSGWARN_LOGERROR  };

#define D3DAPPERR_NODIRECT3D          0x82000001
#define D3DAPPERR_NOWINDOW            0x82000002
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
#define D3DAPPERR_NODESKTOPHAL        0x82000008
#define D3DAPPERR_NOHALTHISMODE       0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_MEDIANOTFOUND       0x8200000b
#define D3DAPPERR_RESIZEFAILED        0x8200000c
#define D3DAPPERR_NEEDSTRUECOLOR      0x8200000d
#define D3DAPPERR_MAXLIGHTS		      0x8200000E


inline int FindColorDepth(D3DFORMAT fmt)
{
	switch (fmt)
	{
		default: return 0;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		return 32;

		case D3DFMT_R8G8B8:
		return 24;

		case D3DFMT_X1R5G5B5:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_R5G6B5:
		return 16;
	}
}

/*
 * @fn SortModesCallback()
 * Callback function for sorting display modes (used by BuildDeviceList).
 */
static int _cdecl SortModesCallback( const VOID* arg1, const VOID* arg2 )
{
    D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
    D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

    if( p1->Format > p2->Format )   return -1;
    if( p1->Format < p2->Format )   return +1;
    if( p1->Width  < p2->Width )    return -1;
    if( p1->Width  > p2->Width )    return +1;
    if( p1->Height < p2->Height )   return -1;
    if( p1->Height > p2->Height )   return +1;

    return 0;
}


/*
 * @class D3DModeInfo
 * Structure for holding information about a display mode
 */
struct D3DModeInfo
{
    DWORD      Width;      // Screen width in this mode
    DWORD      Height;     // Screen height in this mode
    D3DFORMAT  Format;     // Pixel format in this mode
    DWORD      dwBehavior; // Hardware / Software / Mixed vertex processing
    D3DFORMAT  DepthStencilFormat; // Which depth/stencil format to use with this mode
};

/*
 * @class D3DDeviceInfo
 * Structure for holding information about a Direct3D device, including
 * a list of modes compatible with this device
 */
struct D3DDeviceInfo
{
    D3DDEVTYPE   DeviceType;		// Reference, HAL, etc.
    D3DCAPS	     d3dCaps;			// Capabilities of this device
    const TCHAR* strDesc;			// Name of this device
    DWORD        dwNumModes;		// Modes for this device
    D3DModeInfo  modes[150];
    DWORD        dwCurrentMode;		// Current state
    BOOL         bWindowed;
	BOOL		 bHasHAL;
    D3DMULTISAMPLE_TYPE MultiSampleType;
};

/*
 * @class D3DAdapterInfo
 * Structure for holding information about an adapter, including a list
 * of devices available on this adapter
 */
struct D3DAdapterInfo
{
    D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;	// Adapter data
    D3DDISPLAYMODE	d3ddmDesktop;					// Desktop display mode for this adapter
    DWORD			dwNumDevices;					// Devices for this adapter
    DWORD			dwCurrentDevice;				// Current state
	DWORD			dwPriority;						// adapter priority
    D3DDeviceInfo	devices[5];
};


bool DXRenderer::InitializeD3D()
{
	HRESULT hr;

	if (s_D3D)
		return true;
	s_D3D = D3DCreate(D3D_SDK_VERSION);
    if (s_D3D == NULL)
      VX_ERROR(("Scene: ERROR cannot create Direct3D context"), false);

/*
 * Build a list of Direct3D adapters, modes and devices. The
 * ConfirmDevice() callback is used to confirm that only devices that
 * meet the app's requirements are considered.
 */
	D3DAdapterInfo	adapters[10];
	UINT			count = s_D3D->GetAdapterCount();
	World3D*		w = World3D::Get();
	
	if (BackWidth == 0)
		 BackWidth = ::GetSystemMetrics(SM_CXSCREEN);
	if (BackHeight == 0)
		 BackHeight = ::GetSystemMetrics(SM_CYSCREEN);
	if (count > 10)
		count = 10;
	if (BackWidth < (int) w->WinRect.Width())
		BackWidth = (int) w->WinRect.Width();
	if (BackHeight < (int) w->WinRect.Height())
		BackHeight = (int) w->WinRect.Height();
	s_Adapter = 0;
	if (FAILED( hr = BuildDeviceList(adapters, s_Adapter)))
    {
		s_D3D->Release();
        VX_ERROR(("Scene: ERROR cannot find compatible Direct 3D device"), false);
    }
/*
 * Set presentation parameters from chosen adapter
 */
	D3DAdapterInfo*	adapter = adapters + s_Adapter;
    D3DDeviceInfo*  pDeviceInfo  = &adapter->devices[adapter->dwCurrentDevice];
    D3DModeInfo*    pModeInfo    = &pDeviceInfo->modes[pDeviceInfo->dwCurrentMode];
	char			devdesc[1024];

	d3dCaps = pDeviceInfo->d3dCaps;
    ZeroMemory(&s_Present, sizeof(s_Present));
    s_Present.BackBufferCount        = 1;
    s_Present.MultiSampleType        = pDeviceInfo->MultiSampleType;
    s_Present.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    s_Present.EnableAutoDepthStencil = TRUE;
    s_Present.AutoDepthStencilFormat = pModeInfo->DepthStencilFormat;
#ifdef _DEBUG
	s_Present.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
#else
	s_Present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#endif

    if (pDeviceInfo->bWindowed)
    {
		s_Present.Windowed         = true;
		s_Present.SwapEffect = D3DSWAPEFFECT_COPY;
        s_Present.BackBufferWidth  = BackWidth;
        s_Present.BackBufferHeight = BackHeight;
        s_Present.BackBufferFormat = adapter->d3ddmDesktop.Format;
    }
    else
    {
		s_Present.Windowed         = false;
        s_Present.BackBufferWidth  = BackWidth = pModeInfo->Width;
        s_Present.BackBufferHeight = BackHeight = pModeInfo->Height;
        s_Present.BackBufferFormat = pModeInfo->Format;
    }
	Scene::DeviceDepth = FindColorDepth(s_Present.BackBufferFormat);
	sprintf(devdesc, "%s %d x %d x %d %s %s\n", adapter->d3dAdapterIdentifier.Description,
			s_Present.BackBufferWidth, s_Present.BackBufferHeight, Scene::DeviceDepth,
			pDeviceInfo->bWindowed ? " window" : "",
			(pDeviceInfo->DeviceType == D3DDEVTYPE_HAL) ? "HAL" : "REF");
	VX_PRINTF( (devdesc) );
	return true;
}


/*!
 * This routine is called by the rendering thread to initialize a DirectX
 * device for display. It assumes the scene has already been attached to a
 * window and is not already in use.
 *
 * @return \b true if DirectX successfully initialized, else \b false
 *
 * @see DXScene::InitThread DXScene::EndThread Scene::SetWindow
 */
bool DXRenderer::Init(Scene* scene, Vixen::Window window, const TCHAR* options)
{
	RECT		rcViewport;
	HRESULT		hr;
	DWORD		behavior = 0; //D3DCREATE_MULTITHREADED;
	D3DDEVTYPE	devtype = D3DDEVTYPE_HAL;
	float		w = (float) BackWidth;
	float		h = (float) BackHeight;
	char		devinfobuf[1024];
	UINT		count;
	DeviceInfo*	dinfo;

	if (!Renderer::Init(scene, window, options))
		return false;
	if (window == NULL)
		return false;
	IsStereo = scene->GetCamera()->GetEyeSep() > 0;
	dinfo = (DeviceInfo*) scene->GetDevInfo();
	VX_ASSERT(dinfo);
	if (dinfo->Device != NULL)
	{
		d3dDevice = (D3DDEVICE*) dinfo->Device;
		m_ColorFormat = s_Present.BackBufferFormat;
		m_DepthFormat = s_Present.AutoDepthStencilFormat;
		NumTexUnits = d3dCaps.MaxSimultaneousTextures;
		MaxLights = d3dCaps.MaxActiveLights;
		return true;
	}
	InitializeD3D();
	count = s_D3D->GetAdapterCount();
	s_ColorSurface = NULL;
	s_DepthSurface = NULL;
	devinfobuf[0] = 0;
	dinfo->DC = ::GetDC(dinfo->WinHandle);
	if (!dinfo->DC)
		VX_ERROR(("Scene::SetWindow: Null Device Context"), false);
	::GetClientRect(dinfo->WinHandle, &rcViewport);
	if (!IsFullScreen)
	{
		w = float(rcViewport.right - rcViewport.left);
		h = float(rcViewport.bottom - rcViewport.top);
		if (w && (BackWidth == 0))
			s_Present.BackBufferWidth = BackWidth = (int) w;
		if (h && (BackHeight == 0))
			s_Present.BackBufferHeight = BackHeight = (int) h;
		m_Scene->SetViewport(0, w, 0, h);
	}
	s_Present.hDeviceWindow = dinfo->WinHandle;
	hr = CreateDevice(dinfo->WinHandle, s_Adapter, devinfobuf);
	if (hr != D3D_OK)
		return false;
	dinfo->Device = d3dDevice;
/*
 * Store back buffer description and references to
 * color and depth buffer render targets
 */
	D3DSURFACE* depthsurf;
	D3DSURFACE* colorsurf;

	m_ColorFormat = s_Present.BackBufferFormat;
	m_DepthFormat = s_Present.AutoDepthStencilFormat;
	d3dDevice->GetDepthStencilSurface(&depthsurf);
	d3dDevice->GetRenderTarget(0, &colorsurf);
	m_ColorSurface = colorsurf;
	m_DepthSurface = depthsurf;
	s_ColorSurface = colorsurf;
	s_DepthSurface = depthsurf;
/*
 * Initialize D3D state and store capabilities in scene variables
 */		
	d3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	d3dDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	d3dDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	d3dDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	d3dDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	d3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	NumTexUnits = d3dCaps.MaxSimultaneousTextures;
	MaxLights = d3dCaps.MaxActiveLights;
/*
 * Check for render target types supported for shadow and environment maps
 */		
static	D3DFORMAT	depth_types[6] = {	D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D16, D3DFMT_D15S1 };
static	D3DFORMAT	font_types[5] = {	D3DFMT_A4L4, D3DFMT_A8L8, D3DFMT_A4R4G4B4,
										D3DFMT_A1R5G5B5, D3DFMT_A8R8G8B8 };
static	D3DFORMAT	compressed[3] = { D3DFMT_DXT1, D3DFMT_DXT3, D3DFMT_DXT5 };
	D3DFORMAT display_format = s_Present.BackBufferFormat;
	m_DepthFormat = D3DFMT_UNKNOWN;
	D3DFORMAT	t = D3DFMT_UNKNOWN;
	int			i;

// Find highest resolution depth buffer supported by this device
	for (i = 0; i < 6; ++i)
		if (SUCCEEDED(s_D3D->CheckDeviceFormat(s_Adapter, devtype, display_format,
							D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, depth_types[i])))
		{
			m_DepthFormat = depth_types[i];
			break;
		}
// Find most optimal font formats supported by this device
	for (i = 0; i < 5; ++i)
		if (SUCCEEDED(s_D3D->CheckDeviceFormat(s_Adapter, devtype, display_format,
							0, D3DRTYPE_TEXTURE, font_types[i])))
		{
			FontFormat = font_types[i];
			break;
		}
// See if device supports compressed textures
//	for (i = 0; i < 3; ++i)
//		if (SUCCEEDED(s_D3D->CheckDeviceFormat(s_Adapter, devtype, display_format,
//							0, D3DRTYPE_TEXTURE, compressed[i])))
//		{
//			SupportDDS = true;
//			break;
//		}
	if (SUCCEEDED(s_D3D->CheckDeviceFormat(s_Adapter, devtype, display_format,
							0, D3DRTYPE_TEXTURE, D3DFMT_DXT1)))
		Scene::SupportDDS = true;

// Find highest resolution RGBA texture formats supported by this device
	if (SUCCEEDED(s_D3D->CheckDeviceFormat(s_Adapter, devtype, display_format,
							0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
		Bitmap::RGBADepth = 32;
	else
		Bitmap::RGBADepth = 16;
// Find highest resolution RGB texture formats supported by this device
	if (SUCCEEDED(s_D3D->CheckDeviceFormat(s_Adapter, devtype, display_format,
							0, D3DRTYPE_TEXTURE, D3DFMT_R8G8B8)))
		Bitmap::RGBDepth = 24;
	else if (Bitmap::RGBADepth == 32)
		Bitmap::RGBDepth = 32;
	else
		Bitmap::RGBDepth = 16;
	return true;
}

HRESULT DXRenderer::CreateDevice(HWND window, int adapterid, char* devdesc)
{
	HRESULT		hr;
	DWORD		behavior = 0; //D3DCREATE_MULTITHREADED;
	D3DDEVTYPE	devtype = D3DDEVTYPE_HAL;
	char*		vtxproc = devdesc + strlen(devdesc);

/* 
 * Choose vertex processing behavior based on the capabilities of the adapter
 */
	devdesc += strlen(devdesc);
	if (d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		strcpy(vtxproc, " hwvtx\n");
		behavior |= D3DCREATE_MIXED_VERTEXPROCESSING;
	}
	else
	{
		behavior |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		strcpy(vtxproc, " swvtx\n");
	}
#ifdef _DEBUG
	 behavior |= D3DCREATE_MULTITHREADED;
#endif
/*
 * Try to create a hardware accelerated device with hardware lights first
 * Then try reference rasterizer and give up if that fails
 */
	hr = s_D3D->CreateDevice(adapterid, devtype, window, behavior, &s_Present, &d3dDevice);
	if (hr == D3D_OK)
	{
		hr = d3dDevice->GetDeviceCaps(&d3dCaps);
		if (d3dCaps.MaxActiveLights >= 2)
			return hr;
		hr = D3DAPPERR_MAXLIGHTS;
	}
	VX_WARNING(("Scene: WARNING cannot create hardware accelerated Direct 3D device %s", devdesc));
	devtype = D3DDEVTYPE_REF;
//	behavior = D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	behavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	strcpy(vtxproc, " swvtx\n");
	hr = s_D3D->CreateDevice(adapterid, devtype, window, behavior, &s_Present, &d3dDevice);
	if (hr != D3D_OK)
		VX_ERROR(("Scene: ERROR Cannot create Direct 3D device %s", devdesc), hr);
	VX_WARNING(("Scene: WARNING switching to reference rasterizer"));
	return hr;
}

HRESULT DXRenderer::BuildDeviceList(D3DAdapterInfo* Adapters, DWORD& dwAdapter)
{
    const DWORD dwNumDeviceTypes = 2;
    const TCHAR* strDeviceDescs[] = { TEXT("HAL"), TEXT("REF") };
    const D3DDEVTYPE DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };
	DWORD dwNumAdapters = 0;
    BOOL bHALExists = FALSE;
    BOOL bHALIsWindowedCompatible = FALSE;
    BOOL bHALIsDesktopCompatible = FALSE;
    BOOL bHALIsSampleCompatible = FALSE;
	UINT minwidth = BackWidth ? BackWidth : 800;
	UINT minheight = BackHeight ? BackHeight : 600;
	DWORD maxpriority = 0;
	UINT count = s_D3D->GetAdapterCount();
	DWORD m, f;



    // Loop through all the adapters on the system (usually, there's just one
    // unless more than one graphics card is present).
    for (UINT iAdapter = 0; iAdapter < count; iAdapter++)
    {
        // Fill in adapter info
        D3DAdapterInfo* pAdapter  = &Adapters[dwNumAdapters];
        s_D3D->GetAdapterIdentifier( iAdapter, 0, &pAdapter->d3dAdapterIdentifier );
        s_D3D->GetAdapterDisplayMode( iAdapter, &pAdapter->d3ddmDesktop );
        pAdapter->dwNumDevices    = 0;
        pAdapter->dwCurrentDevice = 0;
		pAdapter->dwPriority = 0;

        // Enumerate all display modes on this adapter
        D3DDISPLAYMODE modes[100];
        D3DFORMAT      formats[20];
        DWORD dwNumFormats      = 0;
        DWORD dwNumModes        = 0;
        DWORD dwNumAdapterModes = s_D3D->GetAdapterModeCount( iAdapter, pAdapter->d3ddmDesktop.Format );
        // Add the adapter's current desktop format to the list of formats
        formats[dwNumFormats++] = pAdapter->d3ddmDesktop.Format;

        for (UINT iMode = 0; iMode < dwNumAdapterModes; iMode++)
        {
            // Get the display mode attributes
            D3DDISPLAYMODE DisplayMode;
            s_D3D->EnumAdapterModes(iAdapter, pAdapter->d3ddmDesktop.Format, iMode, &DisplayMode);
            // Filter out low-resolution modes
            if (DisplayMode.Width  < minwidth || DisplayMode.Height < minheight)
                continue;

            // Check if the mode already exists (to filter out refresh rates)
            for( m=0L; m<dwNumModes; m++ )
            {
                if( ( modes[m].Width  == DisplayMode.Width  ) &&
                    ( modes[m].Height == DisplayMode.Height ) &&
                    ( modes[m].Format == DisplayMode.Format ) )
                    break;
            }

            // If we found a new mode, add it to the list of modes
            if( m == dwNumModes )
            {
                modes[dwNumModes].Width       = DisplayMode.Width;
                modes[dwNumModes].Height      = DisplayMode.Height;
                modes[dwNumModes].Format      = DisplayMode.Format;
                modes[dwNumModes].RefreshRate = 0;
                dwNumModes++;

                // Check if the mode's format already exists
                for( f=0; f<dwNumFormats; f++ )
                {
                    if( DisplayMode.Format == formats[f] )
                        break;
                }

                // If the format is new, add it to the list
                if( f== dwNumFormats )
                    formats[dwNumFormats++] = DisplayMode.Format;
            }
        }

        // Sort the list of display modes (by format, then width, then height)
        qsort(modes, dwNumModes, sizeof(D3DDISPLAYMODE), SortModesCallback);

        // Add devices to adapter
        for (UINT iDevice = 0; iDevice < dwNumDeviceTypes; iDevice++)
        {
            // Fill in device info
            D3DDeviceInfo* pDevice;
            pDevice                 = &pAdapter->devices[pAdapter->dwNumDevices];
            pDevice->DeviceType     = DeviceTypes[iDevice];
            s_D3D->GetDeviceCaps( iAdapter, DeviceTypes[iDevice], &pDevice->d3dCaps );
            pDevice->strDesc        = strDeviceDescs[iDevice];
            pDevice->dwNumModes     = 0;
            pDevice->dwCurrentMode  = 0;
            pDevice->bWindowed      = FALSE;
            pDevice->MultiSampleType = D3DMULTISAMPLE_NONE;

            // Examine each format supported by the adapter to see if it will
            // work with this device and meets the needs of the application.
            BOOL  bFormatConfirmed[20];
            DWORD dwBehavior[20];
            D3DFORMAT fmtDepthStencil[20];

            for (f = 0; f < dwNumFormats; f++)
            {
                bFormatConfirmed[f] = FALSE;
                fmtDepthStencil[f] = D3DFMT_UNKNOWN;

                // Skip formats that cannot be used as render targets on this device
                if (FAILED(s_D3D->CheckDeviceType(iAdapter, pDevice->DeviceType, formats[f], formats[f], FALSE)))
                    continue;
                if (pDevice->DeviceType == D3DDEVTYPE_HAL)
                {
                    bHALExists = TRUE;						// This system has a HAL device
                     bHALIsWindowedCompatible = TRUE;	// HAL can run in a window for some mode
                     if (f == 0)
                         bHALIsDesktopCompatible = TRUE;
                }

                // Confirm the device/format for HW vertex processing
                if (pDevice->d3dCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT )
                {
                    if (pDevice->d3dCaps.DevCaps&D3DDEVCAPS_PUREDEVICE)
                    {
                        dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
                        if (SUCCEEDED(ConfirmDevice(&pDevice->d3dCaps, dwBehavior[f], formats[f])))
                            bFormatConfirmed[f] = TRUE;
                    }
                    if (!bFormatConfirmed[f])
                    {
                        dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;
                        if (SUCCEEDED(ConfirmDevice(&pDevice->d3dCaps, dwBehavior[f], formats[f])))
						{
                            bFormatConfirmed[f] = TRUE;
							pAdapter->dwPriority |= 2;
						}
                    }
                    if (!bFormatConfirmed[f])
                    {
                        dwBehavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;
                        if (SUCCEEDED(ConfirmDevice(&pDevice->d3dCaps, dwBehavior[f], formats[f])))
						{
                            bFormatConfirmed[f] = TRUE;
							pAdapter->dwPriority |= 2;
						}
                    }
                }

                // Confirm the device/format for SW vertex processing
                if (!bFormatConfirmed[f])
                {
                    dwBehavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
                    if (SUCCEEDED(ConfirmDevice(&pDevice->d3dCaps, dwBehavior[f], formats[f])))
                        bFormatConfirmed[f] = TRUE;
                }

                // Find a suitable depth/stencil buffer format for this device/format
                if (bFormatConfirmed[f])
                    bFormatConfirmed[f] = FindDepthStencilFormat(iAdapter, pDevice->DeviceType, formats[f], &fmtDepthStencil[f]);

				// Confirm windowed display if needed
				if (bFormatConfirmed[f])
				{
					pDevice->bWindowed = TRUE;
				}
            }
			            // Add all enumerated display modes with confirmed formats to the
            // device's list of valid modes
            for (m = 0L; m < dwNumModes; m++)
            {
                for (f = 0; f < dwNumFormats; f++)
                {
                    if (modes[m].Format == formats[f])
                    {
                        if (bFormatConfirmed[f])
                        {
                            // Add this mode to the device's list of valid modes
                            pDevice->modes[pDevice->dwNumModes].Width      = modes[m].Width;
                            pDevice->modes[pDevice->dwNumModes].Height     = modes[m].Height;
                            pDevice->modes[pDevice->dwNumModes].Format     = modes[m].Format;
                            pDevice->modes[pDevice->dwNumModes].dwBehavior = dwBehavior[f];
                            pDevice->modes[pDevice->dwNumModes].DepthStencilFormat = fmtDepthStencil[f];
                            pDevice->dwNumModes++;
                            if (pDevice->DeviceType == D3DDEVTYPE_HAL)
                                bHALIsSampleCompatible = TRUE;
                        }
                    }
                }
            }

            // Select true color resolution of minimum dimensions as default mode
            for (m = 0; m < pDevice->dwNumModes; m++)
            {
                if (pDevice->modes[m].Width >= minwidth &&
					pDevice->modes[m].Height >= minheight)
                {
                    pDevice->dwCurrentMode = m;
                    if (pDevice->modes[m].Format == D3DFMT_R5G6B5 ||
                        pDevice->modes[m].Format == D3DFMT_X1R5G5B5 ||
                        pDevice->modes[m].Format == D3DFMT_A1R5G5B5 || 
						pDevice->modes[m].Format == D3DFMT_X8R8G8B8)
						break;
                }
            }

			pDevice->bHasHAL = bHALExists;
			if (!bHALIsSampleCompatible && !bHALIsDesktopCompatible)
				pDevice->bHasHAL = false;
			if (!IsFullScreen && !bHALIsWindowedCompatible)
				pDevice->bHasHAL = false;
			if (pDevice->bHasHAL)
				pAdapter->dwPriority |= 1;

            // If valid modes were found, keep this device
            if( pDevice->dwNumModes > 0 )
                pAdapter->dwNumDevices++;
        }

        // If valid devices were found, keep this adapter
        if( pAdapter->dwNumDevices > 0 )
		{
			if (pAdapter->dwPriority > maxpriority)
			{
				maxpriority = pAdapter->dwPriority;
				dwAdapter = iAdapter;
			}
            dwNumAdapters++;
		}
    }

    // Return an error if no compatible devices were found
    if( 0L == dwNumAdapters )
        return D3DAPPERR_NOCOMPATIBLEDEVICES;
	return S_OK;
}


/*
 * @fn DXScene::ConfirmDevice()
 * Called during device intialization, this code checks the device
 * for some minimum set of capabilities
 */
HRESULT DXRenderer::ConfirmDevice(D3DCAPS* pCaps, DWORD dwBehavior, D3DFORMAT Format )
{
    if (dwBehavior & D3DCREATE_PUREDEVICE)
        return E_FAIL;			// GetTransform doesn't work on PUREDEVICE
	int depth = FindColorDepth(Format);
	if (depth < 16)
        return E_FAIL;			// GetTransform doesn't work on PUREDEVICE
    if (pCaps->TextureCaps & D3DPTEXTURECAPS_ALPHA)
        return S_OK;			// make sure it does alpha
    return E_FAIL;
}

/*
 * @fn DXScene::FindDepthStencilFormat()
 * Finds a depth/stencil format for the given device that is compatible
 * with the render target format and meets the needs of the app.
 */
bool DXRenderer::FindDepthStencilFormat(UINT iAdapter, D3DDEVTYPE DeviceType,
    D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat)
{
    if( s_MinDepthBits <= 16 && s_MinStencilBits == 0 )
    {
        if( SUCCEEDED( s_D3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
        {
            if( SUCCEEDED( s_D3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D16 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D16;
                return TRUE;
            }
        }
    }

    if( s_MinDepthBits <= 15 && s_MinStencilBits <= 1 )
    {
        if( SUCCEEDED( s_D3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1 ) ) )
        {
            if( SUCCEEDED( s_D3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D15S1 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D15S1;
                return TRUE;
            }
        }
    }

    if( s_MinDepthBits <= 24 && s_MinStencilBits == 0 )
    {
        if( SUCCEEDED( s_D3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) )
        {
            if( SUCCEEDED( s_D3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24X8 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24X8;
                return TRUE;
            }
        }
    }

    if( s_MinDepthBits <= 24 && s_MinStencilBits <= 8 )
    {
        if( SUCCEEDED( s_D3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) )
        {
            if( SUCCEEDED( s_D3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24S8 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24S8;
                return TRUE;
            }
        }
    }

    if( s_MinDepthBits <= 24 && s_MinStencilBits <= 4 )
    {
        if( SUCCEEDED( s_D3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) )
        {
            if( SUCCEEDED( s_D3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24X4S4 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24X4S4;
                return TRUE;
            }
        }
    }

    if( s_MinDepthBits <= 32 && s_MinStencilBits == 0 )
    {
        if( SUCCEEDED( s_D3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) )
        {
            if( SUCCEEDED( s_D3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D32 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D32;
                return TRUE;
            }
        }
    }

    return FALSE;
}
}	// end Vixen