#include "Example2App.h"
#include "DX11Utils.h"
#include <string>
#include "MoMath.h"
#include "MoFile.h"

using namespace MoGET;
using namespace DirectX;

Example2App::Example2App( HINSTANCE hInstance ) :
	MoGETApp( hInstance ),
	mPhi( 0.25f * (float)Math::Pi ),
	mTheta( 1.5f * (float)Math::Pi ),
	mRadius( 7.0f ),
	mRasterState( nullptr ),
	mVertexShader( nullptr ),
	mPixelShader( nullptr ),
	mInputLayout( nullptr ),
	mVertexShaderBytecode( nullptr )
{
	mWindowCaption = std::wstring( TEXT( "Example 2 - MoGET" ) );

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX Ident = XMMatrixIdentity();
	XMStoreFloat4x4( &mWorld, Ident );
	XMStoreFloat4x4( &mView, Ident );
	XMStoreFloat4x4( &mProj, Ident );
}

Example2App::~Example2App()
{
	ReleaseCOM(mPixelShader);
	ReleaseCOM(mDepthStencilState);
	ReleaseCOM(mConstantBuffer);
	ReleaseCOM(mRasterState);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mVertexShader);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
}

bool Example2App::Init()
{
	if( !MoGETApp::Init() )
	{
		return false;
	}

	BuildGeometryBuffers();
	BuildShaders();
	BuildVertexLayout();
	CreateConstantBuffer();

	// @todo: Create a default raster state
	// Setup raster state
	D3D11_RASTERIZER_DESC RasterDesc;
	ZeroMemory( &RasterDesc, sizeof(D3D11_RASTERIZER_DESC) );
	RasterDesc.FillMode = D3D11_FILL_SOLID;
	RasterDesc.CullMode = D3D11_CULL_BACK;
	RasterDesc.DepthClipEnable = true;

	HR( mD3DDevice->CreateRasterizerState( &RasterDesc, &mRasterState ) );

	// @todo: Create a default DepthStencil state
	D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
	ZeroMemory( &DepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC) );
	DepthStencilDesc.DepthEnable = true;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DepthStencilDesc.StencilEnable = FALSE;
	DepthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	DepthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	HR( mD3DDevice->CreateDepthStencilState( &DepthStencilDesc, &mDepthStencilState ) );

	// @todo: Make a default blend-state

	return true;
}

void Example2App::CreateConstantBuffer()
{
	// Setup constant buffer
	D3D11_BUFFER_DESC CBDesc;
	CBDesc.ByteWidth = sizeof( VS_ConstantBuffer );
	CBDesc.Usage = D3D11_USAGE_DYNAMIC;
	CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	CBDesc.MiscFlags = 0;
	CBDesc.StructureByteStride = 0;

	HR( mD3DDevice->CreateBuffer( &CBDesc, nullptr, &mConstantBuffer ) );
}

void Example2App::OnResize()
{
	MoGETApp::OnResize();

	// @todo: Make NearZ and FarZ config variables
	XMMATRIX Proj = XMMatrixPerspectiveFovLH( 
		0.25f * (float)Math::Pi,
		AspectRatio(),
		1.0f, // NearZ
		1000.0f ); // FarZ
	XMStoreFloat4x4( &mProj, Proj );
}

void Example2App::DrawScene()
{
	MoGETApp::DrawScene();

	assert( mD3DImmediateContext );
	assert( mSwapChain );

	mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue) );
	mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

	mD3DImmediateContext->IASetInputLayout( mInputLayout );
	mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	UINT Stride = sizeof(Vertex);
	UINT Offset = 0;
	mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mBoxVB, &Stride, &Offset );
	mD3DImmediateContext->IASetIndexBuffer( mBoxIB, DXGI_FORMAT_R16_UINT, 0 );

	XMMATRIX World = XMLoadFloat4x4( &mWorld );
	XMMATRIX View = XMLoadFloat4x4( &mView );
	XMMATRIX Proj = XMLoadFloat4x4( &mProj );
	XMMATRIX WorldViewProj = World * View * Proj;

	// Need to transpose matrices, as we want them packed Column major, while CPU size, they are Row major
	WorldViewProj = XMMatrixTranspose(WorldViewProj);

	VS_ConstantBuffer ConstantBuffer;
	XMStoreFloat4x4(&ConstantBuffer.WorldViewProj, WorldViewProj);

	// Update the constant buffer
	D3D11_MAPPED_SUBRESOURCE Content;
	mD3DImmediateContext->Map( mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Content );
	memcpy( Content.pData, &ConstantBuffer, sizeof( VS_ConstantBuffer ) );
	mD3DImmediateContext->Unmap( mConstantBuffer, 0 );

	mD3DImmediateContext->VSSetConstantBuffers( 0, 1, &mConstantBuffer );
	mD3DImmediateContext->VSSetShader( mVertexShader, nullptr, 0 );
	mD3DImmediateContext->PSSetShader( mPixelShader, nullptr, 0 );

	mD3DImmediateContext->RSSetState( mRasterState );
	mD3DImmediateContext->OMSetDepthStencilState( mDepthStencilState, 0 );
	mD3DImmediateContext->DrawIndexed( 36, 0, 0 );

	HR( mSwapChain->Present( 0, 0 ) );
}

void Example2App::UpdateScene( double DeltaTime )
{
	MoGETApp::UpdateScene( DeltaTime );

	// Convert Spherical to Cartesian coordinates
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf( mPhi ) * sinf( mTheta );
	float y = mRadius * cosf( mPhi );

	// Build view matrix
	XMVECTOR Pos = XMVectorSet( x, y, z, 1.0f );
	XMVECTOR Target = XMVectorZero();
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	XMMATRIX View = XMMatrixLookAtLH( Pos, Target, Up );
	XMStoreFloat4x4( &mView, View );
}

void Example2App::BuildGeometryBuffers()
{
	// Create vertex buffer
	// @todo: Do something more gracious about ArgbToAbgr. Al through gives the correct result, we have the data in the wrong component in the XMCOLOR
	Vertex Vertices[] =
	{
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), Math::ArgbToAbgr( PackedVector::XMCOLOR( (const float*)&Colors::White ) )		},
		{ XMFLOAT3( -1.0f, +1.0f, -1.0f ), Math::ArgbToAbgr( PackedVector::XMCOLOR( (const float*)&Colors::Black ) )		},
		{ XMFLOAT3( +1.0f, +1.0f, -1.0f ), Math::ArgbToAbgr( PackedVector::XMCOLOR( (const float*)&Colors::Red ) )		},
		{ XMFLOAT3( +1.0f, -1.0f, -1.0f ), Math::ArgbToAbgr( PackedVector::XMCOLOR( (const float*)&Colors::Green ) )		},
		{ XMFLOAT3( -1.0f, -1.0f, +1.0f ), Math::ArgbToAbgr( PackedVector::XMCOLOR( (const float*)&Colors::Blue ) )		},
		{ XMFLOAT3( -1.0f, +1.0f, +1.0f ), Math::ArgbToAbgr( PackedVector::XMCOLOR( (const float*)&Colors::Yellow ) )		},
		{ XMFLOAT3( +1.0f, +1.0f, +1.0f ), Math::ArgbToAbgr( PackedVector::XMCOLOR( (const float*)&Colors::Cyan ) )		},
		{ XMFLOAT3( +1.0f, -1.0f, +1.0f ), Math::ArgbToAbgr( PackedVector::XMCOLOR( (const float*)&Colors::Magenta ) )	}
	};

	D3D11_BUFFER_DESC VBD;
	VBD.Usage = D3D11_USAGE_IMMUTABLE;
	VBD.ByteWidth = sizeof(Vertex) * 8;
	VBD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBD.CPUAccessFlags = 0;
	VBD.MiscFlags = 0;
	VBD.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA VInitData;
	VInitData.pSysMem = Vertices;
	HR(mD3DDevice->CreateBuffer( &VBD, &VInitData, &mBoxVB ) );

	// Create the index buffer
	unsigned short Indices[] =
	{
		// Front face
		0, 1, 2,
		0, 2, 3,
		// Back face
		4, 6, 5,
		4, 7, 6,
		// Left face
		4, 5, 1,
		4, 1, 0,
		// Right face
		3, 2, 6,
		3, 6, 7,
		// Top face
		1, 5, 6,
		1, 6, 2,
		// Bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC IBD;
	IBD.Usage = D3D11_USAGE_IMMUTABLE;
	IBD.ByteWidth = 36 * sizeof(unsigned short);
	IBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBD.CPUAccessFlags = 0;
	IBD.MiscFlags = 0;
	IBD.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA IInitData;
	IInitData.pSysMem = Indices;
	
	HR( mD3DDevice->CreateBuffer( &IBD, &IInitData, &mBoxIB ) );
}

void Example2App::BuildShaders()
{
	assert(mD3DDevice);

	assert(mVertexShaderBytecode == nullptr);
	bool ReadFileResult = ReadFileContent( "..\\..\\Example2\\ShaderBinaries\\VertexShader.cso", "rb", &mVertexShaderBytecode, mVertexShaderBytecodeSize );
	assert(ReadFileResult);

	HR( mD3DDevice->CreateVertexShader( mVertexShaderBytecode, mVertexShaderBytecodeSize, nullptr, &mVertexShader ) );

	long PixelShaderSize = 0;
	char* PixelShaderBytecode = nullptr;
	ReadFileResult = ReadFileContent( "..\\..\\Example2\\ShaderBinaries\\PixelShader.cso", "rb", &PixelShaderBytecode, PixelShaderSize );
	assert(ReadFileResult);

	HR( mD3DDevice->CreatePixelShader( PixelShaderBytecode, PixelShaderSize, nullptr, &mPixelShader ) );

	delete[] PixelShaderBytecode;
}

void Example2App::BuildVertexLayout()
{
	// Create the vertex input layout
	D3D11_INPUT_ELEMENT_DESC VertexDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0, },
		{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the input layout
	HR( mD3DDevice->CreateInputLayout( VertexDesc, sizeof(VertexDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		mVertexShaderBytecode, mVertexShaderBytecodeSize,
		&mInputLayout ) );

	// @todo: Make material system where the shader blobs stick around until everything is initialized
	delete[] mVertexShaderBytecode;
	mVertexShaderBytecodeSize = -1;
}

void Example2App::OnMouseMove( WPARAM BtnState, int x, int y )
{
	if( (BtnState & MK_LBUTTON) != 0 )
	{
		float DX = XMConvertToRadians( 0.25f * static_cast<float>( x - mLastMousePos.x ) );
		float DY = XMConvertToRadians( 0.25f * static_cast<float>( y - mLastMousePos.y ) );

		mTheta += DX;
		mPhi += DY;

		mPhi = Math::Clamp( mPhi, 0.1f, (float)Math::Pi - 0.1f );
	}
	else if( (BtnState & MK_RBUTTON) != 0 )
	{
		float DX = 0.005f * static_cast<float>(x - mLastMousePos.x );
		float DY = 0.005f * static_cast<float>(y - mLastMousePos.y );

		mRadius += DX - DY;

		mRadius = Math::Clamp( mRadius, 3.0f, 15.0f );
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void Example2App::OnMouseDown( WPARAM BtnState, int x, int y )
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mHWND);
}

void Example2App::OnMouseUp( WPARAM BtnState, int x, int y )
{
	ReleaseCapture();
}