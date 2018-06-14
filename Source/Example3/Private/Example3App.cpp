#include "Example3App.h"
#include "DX11Utils.h"
#include <string>
#include "MoMath.h"
#include "MoFile.h"

// @todo: Make a MoMath.h
template<typename T>
T Clamp( T Val, T MinVal, T MaxVal )
{
	return max( min( Val, MaxVal ), MinVal );
}

Example3App::Example3App( HINSTANCE hInstance ) :
	MoREApp( hInstance ),
	mPhi( 0.25f * (float)M_PI ),
	mTheta( 1.5f * (float)M_PI ),
	mRadius( 7.0f ),
	mRasterState( nullptr ),
	mVertexShader( nullptr ),
	mPixelShader( nullptr ),
	mInputLayout( nullptr ),
	mVertexShaderBytecode( nullptr )
{
	mWindowCaption = std::wstring( TEXT( "Example 3 - MoRE" ) );

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	DirectX::XMMATRIX Ident = DirectX::XMMatrixIdentity();
	DirectX::XMStoreFloat4x4( &mWorld, Ident );
	DirectX::XMStoreFloat4x4( &mView, Ident );
	DirectX::XMStoreFloat4x4( &mProj, Ident );
}

Example3App::~Example3App()
{
	ReleaseCOM(mConstantBuffer);
	ReleaseCOM(mRasterState);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mVertexShader);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
}

bool Example3App::Init()
{
	if( !MoREApp::Init() )
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

void Example3App::CreateConstantBuffer()
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

void Example3App::OnResize()
{
	MoREApp::OnResize();

	// @todo: Make a portable version of M_PI
	// @todo: Make NearZ and FarZ config variables
	DirectX::XMMATRIX Proj = DirectX::XMMatrixPerspectiveFovLH( 
		0.25f * (float)M_PI,
		AspectRatio(),
		1.0f, // NearZ
		1000.0f ); // FarZ
	XMStoreFloat4x4( &mProj, Proj );
}

void Example3App::DrawScene()
{
	MoREApp::DrawScene();

	assert( mD3DImmediateContext );
	assert( mSwapChain );

	mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue) );
	mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

	mD3DImmediateContext->IASetInputLayout( mInputLayout );
	mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	UINT Stride = sizeof(Vertex);
	UINT Offset = 0;
	mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mBoxVB, &Stride, &Offset );
	mD3DImmediateContext->IASetIndexBuffer( mBoxIB, DXGI_FORMAT_R32_UINT, 0 );

	DirectX::XMMATRIX World = DirectX::XMLoadFloat4x4( &mWorld );
	DirectX::XMMATRIX View = DirectX::XMLoadFloat4x4( &mView );
	DirectX::XMMATRIX Proj = DirectX::XMLoadFloat4x4( &mProj );
	DirectX::XMMATRIX WorldViewProj = World * View * Proj;

	// Need to transpose matrices, as we want them packed Column major, while CPU size, they are Row major
	WorldViewProj = DirectX::XMMatrixTranspose(WorldViewProj);

	VS_ConstantBuffer ConstantBuffer;
	DirectX::XMStoreFloat4x4(&ConstantBuffer.WorldViewProj, WorldViewProj);

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

void Example3App::UpdateScene( double DeltaTime )
{
	MoREApp::UpdateScene( DeltaTime );

	// Convert Spherical to Cartesian coordinates
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf( mPhi ) * sinf( mTheta );
	float y = mRadius * cosf( mPhi );

	// Build view matrix
	DirectX::XMVECTOR Pos = DirectX::XMVectorSet( x, y, z, 1.0f );
	DirectX::XMVECTOR Target = DirectX::XMVectorZero();
	DirectX::XMVECTOR Up = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH( Pos, Target, Up );
	DirectX::XMStoreFloat4x4( &mView, View );
}

void Example3App::BuildGeometryBuffers()
{
	// Create vertex buffer
	Vertex Vertices[] =
	{
		{ DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4( (const float*)&Colors::White )	},
		{ DirectX::XMFLOAT3( -1.0f, +1.0f, -1.0f ), DirectX::XMFLOAT4( (const float*)&Colors::Black )	},
		{ DirectX::XMFLOAT3( +1.0f, +1.0f, -1.0f ), DirectX::XMFLOAT4( (const float*)&Colors::Red )		},
		{ DirectX::XMFLOAT3( +1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT4( (const float*)&Colors::Green )	},
		{ DirectX::XMFLOAT3( -1.0f, -1.0f, +1.0f ), DirectX::XMFLOAT4( (const float*)&Colors::Blue )	},
		{ DirectX::XMFLOAT3( -1.0f, +1.0f, +1.0f ), DirectX::XMFLOAT4( (const float*)&Colors::Yellow )	},
		{ DirectX::XMFLOAT3( +1.0f, +1.0f, +1.0f ), DirectX::XMFLOAT4( (const float*)&Colors::Cyan )	},
		{ DirectX::XMFLOAT3( +1.0f, -1.0f, +1.0f ), DirectX::XMFLOAT4( (const float*)&Colors::Magenta ) }
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
	UINT Indices[] =
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
	IBD.ByteWidth = 36 * sizeof(UINT);
	IBD.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBD.CPUAccessFlags = 0;
	IBD.MiscFlags = 0;
	IBD.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA IInitData;
	IInitData.pSysMem = Indices;
	
	HR( mD3DDevice->CreateBuffer( &IBD, &IInitData, &mBoxIB ) );
}

void Example3App::BuildShaders()
{
	assert(mD3DDevice);

	assert(mVertexShaderBytecode == nullptr);
	bool ReadFileResult = MoRE::ReadFileContent( "..\\..\\Example3\\ShaderBinaries\\VertexShader.cso", "rb", &mVertexShaderBytecode, mVertexShaderBytecodeSize );
	assert(ReadFileResult);

	HR( mD3DDevice->CreateVertexShader( mVertexShaderBytecode, mVertexShaderBytecodeSize, nullptr, &mVertexShader ) );

	long PixelShaderSize = 0;
	char* PixelShaderBytecode = nullptr;
	ReadFileResult = MoRE::ReadFileContent( "..\\..\\Example3\\ShaderBinaries\\PixelShader.cso", "rb", &PixelShaderBytecode, PixelShaderSize );
	assert(ReadFileResult);

	HR( mD3DDevice->CreatePixelShader( PixelShaderBytecode, PixelShaderSize, nullptr, &mPixelShader ) );

	delete[] PixelShaderBytecode;
}

void Example3App::BuildVertexLayout()
{
	// Create the vertex input layout
	D3D11_INPUT_ELEMENT_DESC VertexDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0, },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the input layout
	HR( mD3DDevice->CreateInputLayout( VertexDesc, 2,
		mVertexShaderBytecode, mVertexShaderBytecodeSize,
		&mInputLayout ) );

	// @todo: Make material system where the shader blobs stick around untill everything is initialized
	delete[] mVertexShaderBytecode;
	mVertexShaderBytecodeSize = -1;
}

void Example3App::OnMouseMove( WPARAM BtnState, int x, int y )
{
	if( (BtnState & MK_LBUTTON) != 0 )
	{
		float DX = DirectX::XMConvertToRadians( 0.25f * static_cast<float>( x - mLastMousePos.x ) );
		float DY = DirectX::XMConvertToRadians( 0.25f * static_cast<float>( y - mLastMousePos.y ) );

		mTheta += DX;
		mPhi += DY;

		mPhi = Clamp( mPhi, 0.1f, (float)M_PI - 0.1f );
	}
	else if( (BtnState & MK_RBUTTON) != 0 )
	{
		float DX = 0.005f * static_cast<float>(x - mLastMousePos.x );
		float DY = 0.005f * static_cast<float>(y - mLastMousePos.y );

		mRadius += DX - DY;

		mRadius = Clamp( mRadius, 3.0f, 15.0f );
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void Example3App::OnMouseDown( WPARAM BtnState, int x, int y )
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mHWND);
}

void Example3App::OnMouseUp( WPARAM BtnState, int x, int y )
{
	ReleaseCapture();
}