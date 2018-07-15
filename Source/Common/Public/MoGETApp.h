#pragma once

#include "Common.h"
#include "Platform.h"
#include <string>
#include <d3d11.h>
#include "GameTimer.h"

/**
 * Base class for windows, please note that this shouldn't be used in real production
 * cases as it's not complete
 */
class MoGETApp
{
public:
	COMMON_API MoGETApp( HINSTANCE hInstance );
	COMMON_API virtual ~MoGETApp();

	COMMON_API virtual bool Init();

	// Get the aspect ratio of the window
	COMMON_API float AspectRatio() const;

	COMMON_API int Run();
protected:
	MoGETApp() = delete; // Don't allow creating apps this way
		
	bool InitWindow();
	bool InitDirect3D();

	COMMON_API virtual void OnResize();
	COMMON_API virtual void UpdateScene( double DeltaTime );
	COMMON_API virtual void DrawScene();

	COMMON_API virtual void OnMouseDown( WPARAM BtnState, int x, int y ) {}
	COMMON_API virtual void OnMouseUp( WPARAM BtnState, int x, int y ) {}
	COMMON_API virtual void OnMouseMove( WPARAM BtnState, int x, int y ) {}

	void ProcessMessageQueue();
	void CalculateFrameStats();

	void EnumerateAdapters();

	void ReportLiveObjects();

	// Window message callback
	LRESULT CALLBACK MsgProc( HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam );
protected:
	friend LRESULT CALLBACK MainWndProc( HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam );
protected:
	// Bleh
	GameTimer mTimer;

	// Windows specific variables
	HINSTANCE mHInstance;
	HWND mHWND;
	struct IDXGIAdapter* mDesiredAdapter;

	// Platform independent window variables
	std::wstring mWindowCaption;
	int mClientWidth;
	int mClientHeight;

	int mNumVideoAdapters;
	int mNumMonitorsAttached;

	bool mMinimized;
	bool mMaximized;
	bool mResizing;
	bool mQuit;
	bool mAppPaused;

	// D3D11 variables
	struct ID3D11Device* mD3DDevice;
	struct ID3D11DeviceContext* mD3DImmediateContext;
	struct IDXGISwapChain* mSwapChain;

	struct ID3D11Texture2D* mDepthStencilBuffer;
	struct ID3D11RenderTargetView* mRenderTargetView;
	struct ID3D11DepthStencilView* mDepthStencilView;
	
	D3D11_VIEWPORT mScreenViewport;

	UINT m4xMsaaQuality;
	bool mEnable4xMSAA;

	bool mDisableAltEnter;
};