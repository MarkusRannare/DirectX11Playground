#include "Example2App.h"
#include "DX11Utils.h"
#include <string>

Example2App::Example2App( HINSTANCE hInstance ) :
	MoREApp( hInstance )
{
	mWindowCaption = std::wstring( TEXT( "Example 2 - MoRE" ) );

	DirectX::XMMATRIX Ident = DirectX::XMMatrixIdentity();
	DirectX::XMStoreFloat4x4( &mWorld, Ident );
	DirectX::XMStoreFloat4x4( &mView, Ident );
	DirectX::XMStoreFloat4x4( &mProj, Ident );
}

void Example2App::DrawScene()
{
	MoREApp::DrawScene();

	assert( mD3DImmediateContext );
	assert( mSwapChain );

	mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue) );
	mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

	HR( mSwapChain->Present( 0, 0 ) );
}

void Example2App::UpdateScene( double DeltaTime )
{
	MoREApp::UpdateScene( DeltaTime );

	
}