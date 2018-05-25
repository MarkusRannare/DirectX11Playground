#pragma once

#include "Common.h"
#include "Platform.h"
#include <string>
#include <d3d11.h>

class MoREApp
{
public:
	MoREApp( HINSTANCE hInstance );

	bool Init();

	int Run();
protected:
	MoREApp() = delete; // Don't allow creating apps this way
		
	bool InitWindow();
	bool InitDirect3D();

	virtual void OnResize();

	// Window message callback
	LRESULT CALLBACK MsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
protected:
	friend LRESULT CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
protected:
	// Windows specific variables
	HINSTANCE mHInstance;
	HWND mHWND;

	// Platform independent window variables
	std::wstring mWindowCaption;
	int mClientWidth;
	int mClientHeight;
	bool mMinimized;
	bool mMaximized;
	bool mResizing;

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
};