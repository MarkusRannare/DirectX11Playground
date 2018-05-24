#pragma once

#include "Common.h"
#include "Platform.h"
#include <string>

class MoREApp
{
public:
	MoREApp( HINSTANCE hInstance );

	bool Init();

	int Run();
protected:
	MoREApp() = delete; // Don't allow creating apps this way
		
	bool InitWindow();

	// Window message callback
	LRESULT CALLBACK MsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
protected:
	friend LRESULT CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
protected:
	HINSTANCE mHInstance;
	HWND mHWND;
	std::wstring mWindowCaption;
	int mClientWidth;
	int mClientHeight;
};