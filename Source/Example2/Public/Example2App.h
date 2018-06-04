#pragma once

#include "MoREApp.h"
#include <DirectXMath.h>

class Example2App : public MoREApp
{
public:
	Example2App( HINSTANCE hInstance );

	virtual void DrawScene();
	virtual void UpdateScene( double DeltaTime );
private:
	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;
};