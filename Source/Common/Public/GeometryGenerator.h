#pragma once

// For XMFLOAT3
#include "DX11Utils.h"
#include <vector>

namespace MoRE
{
	namespace GeometryGenerator
	{
		struct Vertex
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
			std::vector<Vertex>	Vertices;
			std::vector<UINT>	Indices;
		};

		/**
		 * @param m - number 
		 */
		void CreateGrid( float Width, float Depth, UINT VertsX, UINT VertsZ, MeshData& Out_Mesh );
	}
}