#include "Example1App.h"
#include "DX11Utils.h"
#include <string>

Example1App::Example1App( HINSTANCE hInstance ) :
	MoREApp( hInstance )
{
	mWindowCaption = std::wstring(TEXT("Example 1 - MoGET"));
}

void Example1App::DrawScene()
{
	MoREApp::DrawScene();

	assert( mD3DImmediateContext );
	assert( mSwapChain );

	mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue) );
	mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

	HR( mSwapChain->Present( 0, 0 ) );
}