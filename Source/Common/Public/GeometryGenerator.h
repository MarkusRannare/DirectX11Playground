#pragma once

// For XMFLOAT3
#include "Common.h"
#include "Array.h"
#include "DX11Utils.h"
#include <vector>

struct COMMON_API DirectX::XMFLOAT2;
struct COMMON_API DirectX::XMFLOAT3;

namespace MoGET
{
	namespace GeometryGenerator
	{
		struct COMMON_API Vertex
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 Tangent;
			DirectX::XMFLOAT2 Tex;

			Vertex( float px, float py, float pz,
					float nx, float ny, float nz,
					float tx, float ty, float tz,
					float u, float v ) :
				Position( px, py, pz ),
				Normal( nx, ny, nz ),
				Tangent( tx, ty, tz ),
				Tex( u, v )
			{
			}

			// No parameters leaves memory unitialized
			Vertex(){}
		};

		// @todo: Make this more flexible, so that you can specify different
		// attributes for the vertices and different sizes for the indices
		struct MeshData
		{
			MoGET::Array<Vertex>	Vertices;
			MoGET::Array<UINT>		Indices;
		};

		/**
		 * @param m - number 
		 */
		COMMON_API void CreateGrid( float Width, float Depth, UINT VertsX, UINT VertsZ, MeshData& Out_Mesh );
	}
}
