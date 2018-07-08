#pragma once

#include "MoREApp.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::PackedVector::XMCOLOR Color;
	DirectX::XMFLOAT3 Normal;
};

struct VS_ConstantBuffer
{
	DirectX::XMFLOAT4X4 WorldViewProj;
};

class Example3App : public MoREApp
{
public:
	Example3App( HINSTANCE hInstance );
	virtual ~Example3App();

	virtual bool Init() override;

	virtual void DrawScene() override;
	virtual void UpdateScene( double DeltaTime ) override;

	virtual void OnMouseMove( WPARAM BtnState, int x, int y ) override;
	virtual void OnMouseUp( WPARAM BtnState, int x, int y ) override;
	virtual void OnMouseDown( WPARAM BtnState, int x, int y ) override;
protected:
	float GetHeight( float X, float Z ) const;
	DirectX::XMFLOAT3 GetNormal( float X, float Z ) const;

	virtual void OnResize() override;

	void CreateConstantBuffers();
	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();
private:
	// @todo: Make camera class
	// Camera variables
	DirectX::XMFLOAT4X4 mWorld;
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	// For rotating on the shell of a sphere
	POINT mLastMousePos;
	float mPhi;
	float mTheta;
	float mRadius;
	DirectX::XMFLOAT3 mCameraLocation;

	// Landscape variables
	struct ID3D11Buffer* mLandscapeVB;
	struct ID3D11Buffer* mLandscapeIB;
	struct ID3D11Buffer* mConstantBuffer;
	struct ID3D11Buffer* mLightingBuffer;
	UINT mNumIndices;

	// Shader variables
	struct ID3D11RasterizerState* mRasterState;
	struct ID3D11DepthStencilState* mDepthStencilState;
	struct ID3D11VertexShader* mVertexShader;
	struct ID3D11PixelShader* mPixelShader;
	struct ID3D11InputLayout* mInputLayout;
	// These will just live through the initialization stage, they shouldn't really be members, but haxx for
	// now to simplify getting everything up and running
	char* mVertexShaderBytecode;
	long mVertexShaderBytecodeSize;
};