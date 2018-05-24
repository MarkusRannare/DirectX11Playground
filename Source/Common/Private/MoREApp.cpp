#include "MoREApp.h"
#include "Platform.h"
#include <cassert>

static MoREApp* fmoREApp = nullptr;

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
	}	

	return DefWindowProc( hwnd, msg, wParam, lParam );
}

MoREApp::MoREApp( HINSTANCE hInstance ) :
	mHInstance( hInstance ),
	mClientWidth( 800 ),
	mClientHeight( 600 ),
	mWindowCaption( L"MoRE =)" )
{
	assert( fmoREApp == nullptr && "Can only have one MoRE app" );
	fmoREApp = this;
}

bool MoREApp::InitWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mHInstance;
	wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
	wc.hCursor = LoadCursor( 0, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( NULL_BRUSH );
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if( !RegisterClass( &wc ) )
	{
		MessageBox( 0, L"RegisterClass Failed.", 0, 0 );
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = {0, 0, mClientWidth, mClientHeight};
	AdjustWindowRect( &R, WS_OVERLAPPEDWINDOW, false );
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mHWND = CreateWindow( L"D3DWndClassName", mWindowCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mHInstance, 0 );

	if( !mHWND )
	{
		MessageBox( 0, L"CreateWindow Failed.", 0, 0 );
		return false;
	}

	ShowWindow( mHWND, SW_SHOW );
	UpdateWindow( mHWND );

	return true;
}

bool MoREApp::Init()
{
	if( !InitWindow() )
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