#include "MoREApp.h"
#include "Platform.h"
#include "DX11Utils.h"
#include <cassert>
#include <sstream>
#include <DirectXMath.h>

using namespace DirectX;

static MoREApp* fmoREApp = nullptr;
static std::wstring fWndClassName( TEXT("MoGETWndClassName") );

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
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active. 
		case WM_ACTIVATE:
			if( LOWORD(wParam) == WA_INACTIVE )
			{
				mAppPaused = true;
				mTimer.Stop();
			}
			else
			{
				mAppPaused = false;
				mTimer.Start();
			}
			return 0;
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
					mAppPaused = true;
					mMinimized = true;
					mMaximized = true;
				}
				else if( wParam == SIZE_MAXIMIZED )
				{
					mAppPaused = false;
					mMaximized = true;
					mMinimized = false;
					OnResize();
				}
				else if( wParam == SIZE_RESTORED )
				{
					// Restoring from a minimized state
					if( mMinimized )
					{
						mAppPaused = false;
						mMinimized = false;
						OnResize();
					}
					// Restoring from a maximized state
					else if( mMaximized )
					{
						mAppPaused = false;
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
			mAppPaused = true;
			mResizing = true;
			mTimer.Stop();
			return 0;
		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions
		case WM_EXITSIZEMOVE:
			mAppPaused = false;
			mResizing = false;
			mTimer.Start();
			OnResize();
			return 0;
		// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			// @todo: Make these configurable
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 400;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 400;
			return 0;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			OnMouseDown( wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
			return 0;
		
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			OnMouseUp( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
			return 0;

		case WM_MOUSEMOVE:
			OnMouseMove( wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
			return 0;
	}

	return DefWindowProc( hwnd, msg, wParam, lParam );
}

MoREApp::MoREApp( HINSTANCE hInstance ) :
	mHInstance( hInstance ),
	mDesiredAdapter( nullptr ),
	mWindowCaption( TEXT("MoGET =)") ),
	mClientWidth( 800 ),
	mClientHeight( 600 ),
	mNumVideoAdapters( 0 ),
	mNumMonitorsAttached( 0 ),
	mMinimized( false ),
	mMaximized( false ),
	mResizing( false ),
	mQuit( false ),
	mAppPaused( false ),
	mD3DDevice( nullptr ),
	mD3DImmediateContext( nullptr ),
	mSwapChain( nullptr ),
	mDepthStencilBuffer( nullptr ),
	mRenderTargetView( nullptr ),
	mDepthStencilView( nullptr ),
	m4xMsaaQuality( 0 ),
	mEnable4xMSAA( false ),
	mDisableAltEnter( false )
{
	assert( fmoREApp == nullptr && "Can only have one MoRE app" );
	fmoREApp = this;
}

MoREApp::~MoREApp()
{
	ReleaseCOM(mD3DImmediateContext);
	ReleaseCOM(mSwapChain);
	ReleaseCOM(mDepthStencilBuffer);
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	// Uncomment the following line to make it easier to find leaking resources
	//ReportLiveObjects();
	ReleaseCOM(mD3DDevice);
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

	EnumerateAdapters();

	// @todo: Make dependent on other preprocessor flag
	#if defined(DEBUG) || defined(_DEBUG)
		CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	// @todo: Add support for the application to disable Dx10 support
	const D3D_FEATURE_LEVEL SupportedFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	const int NumSupportedFeatureLevels = sizeof( SupportedFeatureLevels ) / sizeof( D3D_FEATURE_LEVEL );

	assert( mDesiredAdapter != nullptr );
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT Hr = D3D11CreateDevice(
		mDesiredAdapter,			// Adapter
		D3D_DRIVER_TYPE_UNKNOWN,	// Driver type ( https://msdn.microsoft.com/en-us/library/windows/desktop/ff476082(v=vs.85).aspx states that if Adapter is non-null, you then you need to pass driver type Unknown  )
		0,							// Software device module
		CreateDeviceFlags,			
		SupportedFeatureLevels,		
		NumSupportedFeatureLevels,	
		D3D11_SDK_VERSION,			
		&mD3DDevice,				// OUT, created device
		&featureLevel,				// OUT, feature level created on
		&mD3DImmediateContext );	// OUT, the context we can do direct operations on the device with

	if( FAILED(Hr) )
	{
		MessageBox( 0, TEXT("D3D11CreateDeviceFailed"), 0, 0 );
		return false;
	}

	// @todo: Store current feature level and disable some effects depending on the feature level or do them differently
	/*if( featureLevel != D3D_FEATURE_LEVEL_11_0 )
	{
		MessageBox( 0, TEXT("Graphics Processessor with DirectX 11 support required for MoRE"), 0, 0 );
		return false;
	}*/

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
	SCD.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

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

	if( mDisableAltEnter )
	{
		// Prevent DXGI from monitor the application message queue
		DXGIFactory->MakeWindowAssociation( mHWND, DXGI_MWA_NO_WINDOW_CHANGES );
	}

	ReleaseCOM( DXGIDevice );
	ReleaseCOM( DXGIAdapter );
	ReleaseCOM( DXGIFactory );

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
	OnResize();

	return true;
}

void MoREApp::EnumerateAdapters()
{	
	// @todo: Make some kind of intelligent choice of what adapter to use? Like, prefer non intel adapter if possible,
	// as intel adapters tends to be the slower ones when you have multiple ones installed

	IDXGIFactory* Factory = nullptr;
	HR( CreateDXGIFactory( __uuidof(IDXGIFactory), (void**)&Factory) );

	int AdapterIdx = 0;
	mNumVideoAdapters = 0;

	IDXGIAdapter* Adapter = nullptr;
	while( Factory->EnumAdapters( AdapterIdx++, &Adapter ) != DXGI_ERROR_NOT_FOUND )
	{
		LARGE_INTEGER Version;
		if( Adapter->CheckInterfaceSupport( __uuidof(ID3D11Device), &Version ) )
		{
			++mNumVideoAdapters;

			// First and default adapter, select it as our desired adapter
			if( mDesiredAdapter == nullptr )
			{
				// Testcode to skip intel graphics cards
				// @todo: Set this as low prio card, so that we can select desired other card as preferred
				// instead if the user has a NVidia/ATI card also installed
				/*DXGI_ADAPTER_DESC Desc;
				Adapter->GetDesc( &Desc );
				OutputDebugString( Desc.Description );

				if( wcsstr( Desc.Description, TEXT("Intel(R)") ) != nullptr )
				{
					continue;
				}*/

				mDesiredAdapter = Adapter;

				int MonitorIdx = 0;
				mNumMonitorsAttached = 0;

				IDXGIOutput* Monitor = nullptr;
				while( Adapter->EnumOutputs( MonitorIdx++, &Monitor ) != DXGI_ERROR_NOT_FOUND )
				{
					++mNumMonitorsAttached;
				}
			}
		}
	}

	// Case of laptop, then the intel card might have the monitor attached, while the NVidia card just does all
	// the rendering
	//assert(mNumMonitorsAttached > 0);
	ReleaseCOM(Factory);
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
	if( !XMVerifyCPUSupport() )
	{
		MessageBox( 0, TEXT( "MoGET requires SSE2 support to properly function" ), 0, 0 );
		return false;
	}

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

void MoREApp::ProcessMessageQueue()
{
	MSG Msg = {0};

	// If there are Window messages then process them, if WM_QUIT is posted, abort the message loop
	while( PeekMessage( &Msg, 0, 0, 0, PM_REMOVE ) && Msg.message != WM_QUIT )
	{
		TranslateMessage( &Msg );
		DispatchMessage( &Msg );
	}

	if( Msg.message == WM_QUIT )
	{
		mQuit = true;
	}
}

void MoREApp::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int FrameCnt = 0;
	static float TimeElapsed = 0.0f;

	FrameCnt++;

	// Compute averages over one second period.
	if( (mTimer.TotalTime() - TimeElapsed) >= 1.0f )
	{
		float FPS = (float)FrameCnt; // fps = frameCnt / 1
		float MSPF = 1000.0f / FPS;

		std::wostringstream Outs;
		Outs.precision( 8 );
		Outs << mWindowCaption << L"    "
			<< L"FPS: " << FPS << L"    "
			<< L"Frame Time: " << MSPF << L" (ms)";
		SetWindowText( mHWND, Outs.str().c_str() );

		// Reset for next average.
		FrameCnt = 0;
		TimeElapsed += 1.0f;
	}
}

void MoREApp::UpdateScene( double DeltaTime )
{
}

void MoREApp::DrawScene()
{
}

int MoREApp::Run()
{
	mTimer.Reset();
	
	while( !mQuit )
	{
		ProcessMessageQueue();

		mTimer.Tick();

		// ProcessMessageQueue might set mQuit to true, so check for it here
		if( !mAppPaused && !mQuit )
		{
			CalculateFrameStats();
			UpdateScene( mTimer.DeltaTime() );
			DrawScene();

			// Let other processes have a timeslice to not make the system unresponsive
			Sleep(0);
		}
		else
		{
			Sleep(100);
		}
	}
	

	return -1;
}

float MoREApp::AspectRatio() const
{
	return mClientWidth / (float)mClientHeight;
}

void MoREApp::ReportLiveObjects()
{
#ifdef _DEBUG
	ID3D11Debug* DebugDevice = nullptr;
	HR(mD3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)(&DebugDevice)));
	HR(DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL));
	
	ReleaseCOM(DebugDevice);
#endif
}
