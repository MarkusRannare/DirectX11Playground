#include "MoREApp.h"
#include "Platform.h"
#include "DX11Utils.h"
#include <cassert>

static MoREApp* fmoREApp = nullptr;
static std::wstring fWndClassName( TEXT("MoREWndClassName") );

// Windows window proc
static LRESULT CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	assert( fmoREApp != nullptr );

	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return fmoREApp->MsgProc( hwnd, msg, wParam, lParam );
}

LRESULT CALLBACK MoREApp::MsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		// WM_DESTROY is sent when the window is being destroyed
		case WM_DESTROY:
			PostQuitMessage( 0 );
			return 0;
		// WM_SIZE is sent when the user resizes the window
		case WM_SIZE:
			mClientWidth = LOWORD(lParam);
			mClientHeight = HIWORD(lParam);
			if( mD3DDevice )
			{
				if( wParam == SIZE_MINIMIZED )
				{
					// @todo: Pause app
					mMinimized = true;
					mMaximized = true;
				}
				else if( wParam == SIZE_MAXIMIZED )
				{
					// @todo: Resume app
					mMaximized = true;
					mMinimized = false;
					OnResize();
				}
				else if( wParam == SIZE_RESTORED )
				{
					// Restoring from a minimized state
					if( mMinimized )
					{
						// @todo: Resume app
						mMinimized = false;
						assert(mMaximized == false);
						OnResize();
					}
					// Restoring from a maximized state
					else if( mMaximized )
					{
						mMaximized = false;
						OnResize();
					}
					else if( mResizing )
					{
						// If user is dragging the resize bars, we do not resize 
						// the buffers here because as the user continuously 
						// drags the resize bars, a stream of WM_SIZE messages are
						// sent to the window, and it would be pointless (and slow)
						// to resize for each WM_SIZE message received from dragging
						// the resize bars.  So instead, we reset after the user is 
						// done resizing the window and releases the resize bars, which 
						// sends a WM_EXITSIZEMOVE message.
					}
					// API call such as SetWindowPos or mSwapChain->SetFullscreenState
					else
					{
						OnResize();
					}
				}
			}
			return 0;
		// WM_ENTERSIZEMOVE is sent when the user grabs the resize bars.
		case WM_ENTERSIZEMOVE:
			// @todo Pause app?
			mResizing = true;
			return 0;
		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions
		case WM_EXITSIZEMOVE:
			// @todo: Resume app?
			mResizing = false;
			OnResize();
			return 0;
		// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			// @todo: Make these configurable
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 400;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 400;
			return 0;

	}

	return DefWindowProc( hwnd, msg, wParam, lParam );
}

MoREApp::MoREApp( HINSTANCE hInstance ) :
	mHInstance( hInstance ),
	mWindowCaption( TEXT("MoRE =)") ),
	mClientWidth( 800 ),
	mClientHeight( 600 ),
	mMinimized( false ),
	mMaximized( false ),
	mResizing( false ),
	mD3DDevice( nullptr ),
	mD3DImmediateContext( nullptr ),
	mSwapChain( nullptr ),
	mDepthStencilBuffer( nullptr ),
	mRenderTargetView( nullptr ),
	mDepthStencilView( nullptr ),
	m4xMsaaQuality( 0 ),
	mEnable4xMSAA( false )
{
	assert( fmoREApp == nullptr && "Can only have one MoRE app" );
	fmoREApp = this;
}

bool MoREApp::InitWindow()
{
	WNDCLASS WC;
	WC.style = CS_HREDRAW | CS_VREDRAW;
	WC.lpfnWndProc = MainWndProc;
	WC.cbClsExtra = 0;
	WC.cbWndExtra = 0;
	WC.hInstance = mHInstance;
	WC.hIcon = LoadIcon( 0, IDI_APPLICATION );
	WC.hCursor = LoadCursor( 0, IDC_ARROW );
	WC.hbrBackground = (HBRUSH)GetStockObject( NULL_BRUSH );
	WC.lpszMenuName = 0;
	WC.lpszClassName = fWndClassName.c_str();

	if( !RegisterClass( &WC ) )
	{
		MessageBox( 0, TEXT("RegisterClass Failed."), 0, 0 );
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = {0, 0, mClientWidth, mClientHeight};
	AdjustWindowRect( &R, WS_OVERLAPPEDWINDOW, false );
	int Width = R.right - R.left;
	int Height = R.bottom - R.top;

	// @todo: Make config to not allow windows to Minimize/Maximize or resize
	// @todo: Make config to be able to create a fullscreen window
	// @todo: Make config to be able to create borderless fullscreen mode
	mHWND = CreateWindow( fWndClassName.c_str(), mWindowCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, 0, 0, mHInstance, 0 );

	if( !mHWND )
	{
		MessageBox( 0, TEXT("CreateWindow Failed."), 0, 0 );
		return false;
	}

	ShowWindow( mHWND, SW_SHOW );
	UpdateWindow( mHWND );

	return true;
}

bool MoREApp::InitDirect3D()
{
	UINT CreateDeviceFlags = 0;

	// @todo: Make dependent on other preprocessor flag
	#if defined(DEBUG) || defined(_DEBUG)
		CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT Hr = D3D11CreateDevice(
		0,							// Adapter
		D3D_DRIVER_TYPE_HARDWARE,	// Driver type
		0,							// Software device module
		CreateDeviceFlags,			
		0, 0,						// Feature level array
		D3D11_SDK_VERSION,			
		&mD3DDevice,				// OUT, created device
		&featureLevel,				// OUT, feature level created on
		&mD3DImmediateContext );	// OUT, the context we can do direct operations on the device with

	if( FAILED(Hr) )
	{
		MessageBox( 0, TEXT("D3D11CreateDeviceFailed"), 0, 0 );
		return false;
	}

	if( featureLevel != D3D_FEATURE_LEVEL_11_0 )
	{
		MessageBox( 0, TEXT("Graphics Processessor with DirectX 11 support required for MoRE"), 0, 0 );
		return false;
	}

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.
	HR(mD3DDevice->CheckMultisampleQualityLevels( DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality ));
	assert( m4xMsaaQuality > 0 );

	// @todo: Allow backbuffers in different dimensions than the client rect
	// Create swap chain
	DXGI_SWAP_CHAIN_DESC SCD;
	SCD.BufferDesc.Width = mClientWidth;
	SCD.BufferDesc.Height = mClientHeight;
	// Refresh rate
	SCD.BufferDesc.RefreshRate.Numerator = 60;	// @todo: Make config
	SCD.BufferDesc.RefreshRate.Denominator = 1;
	SCD.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SCD.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// I can't find any useful information what this means

	if( mEnable4xMSAA )
	{
		SCD.SampleDesc.Count = 4;	// @todo: Make config
		SCD.SampleDesc.Quality = m4xMsaaQuality;
	}
	else
	{
		SCD.SampleDesc.Count = 1;
		SCD.SampleDesc.Quality = 0;
	}

	SCD.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SCD.BufferCount = 1;	// Double buffering (2 = triplebuffering)	// @todo: Make config
	SCD.OutputWindow = mHWND;
	SCD.Windowed = true;
	SCD.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SCD.Flags = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* DXGIDevice = nullptr;
	HR( mD3DDevice->QueryInterface( __uuidof(IDXGIDevice), (void**)&DXGIDevice ) );

	IDXGIAdapter* DXGIAdapter = nullptr;
	HR( DXGIDevice->GetParent( __uuidof(IDXGIAdapter), (void**)&DXGIAdapter ) );

	IDXGIFactory* DXGIFactory = nullptr;
	HR( DXGIAdapter->GetParent( __uuidof(IDXGIFactory), (void**)&DXGIFactory ) );

	HR( DXGIFactory->CreateSwapChain( mD3DDevice, &SCD, &mSwapChain ) );

	ReleaseCOM( DXGIDevice );
	ReleaseCOM( DXGIAdapter );
	ReleaseCOM( DXGIFactory );

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
	OnResize();

	return true;
}

void MoREApp::OnResize()
{
	assert(mD3DImmediateContext);
	assert(mD3DDevice);
	assert(mSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	ReleaseCOM( mRenderTargetView );
	ReleaseCOM( mDepthStencilView );
	ReleaseCOM( mDepthStencilBuffer );

	// Resize swap chain and the render target view
	HR( mSwapChain->ResizeBuffers( 1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0 ) );
	ID3D11Texture2D* BackBuffer = nullptr;
	HR( mSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer) ) );
	HR( mD3DDevice->CreateRenderTargetView( BackBuffer, 0, &mRenderTargetView ) );
	ReleaseCOM( BackBuffer );

	// Create the depth/stencil buffer and view
	D3D11_TEXTURE2D_DESC DepthStencilDesc;

	DepthStencilDesc.Width = mClientWidth;
	DepthStencilDesc.Height = mClientHeight;
	DepthStencilDesc.MipLevels = 1;
	DepthStencilDesc.ArraySize = 1;
	DepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if( mEnable4xMSAA )
	{
		DepthStencilDesc.SampleDesc.Count = 4;
		DepthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		DepthStencilDesc.SampleDesc.Count = 1;
		DepthStencilDesc.SampleDesc.Quality = 0;
	}

	DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthStencilDesc.CPUAccessFlags = 0;
	DepthStencilDesc.MiscFlags = 0;

	HR( mD3DDevice->CreateTexture2D( &DepthStencilDesc, 0, &mDepthStencilBuffer ) );
	HR( mD3DDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView ) );

	// Bind the render target view and depth/stencil view to the pipeline
	mD3DImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView );

	// Set the viewport transform

	mScreenViewport.TopLeftX	= 0;
	mScreenViewport.TopLeftY	= 0;
	mScreenViewport.Width		= static_cast<float>(mClientWidth);
	mScreenViewport.Height		= static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth	= 0.0f;
	mScreenViewport.MaxDepth	= 1.0f;
	
	mD3DImmediateContext->RSSetViewports( 1, &mScreenViewport );
}

bool MoREApp::Init()
{
	if( !InitWindow() )
	{
		return false;
	}

	if( !InitDirect3D() )
	{
		return false;
	}

	return true;
}

int MoREApp::Run()
{
	MSG Msg = {0};
	
	while( Msg.message != WM_QUIT )
	{
		// If there are Window messages then process them.
		if( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &Msg );
			DispatchMessage( &Msg );
		}
	}

	return -1;
}