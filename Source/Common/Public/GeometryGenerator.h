#pragma once

// For XMFLOAT3
#include "Common.h"
#include "DX11Utils.h"
#include <vector>

struct COMMON_API DirectX::XMFLOAT2;
struct COMMON_API DirectX::XMFLOAT3;

namespace MoRE
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
			/**
			 * \param Out_Vertices - the vertices of the mesh
			 * \return number of vertices
			 */
			COMMON_API size_t GetVertices( Vertex*& Out_Vertices );
			
			/**
			 * \param Out_Vertices - the indices of the mesh
			 * \return number of indices
			 */
			COMMON_API size_t GetIndices( UINT*& Out_Indices );

			std::vector<Vertex>	Vertices;
			std::vector<UINT>	Indices;
		};

		/**
		 * @param m - number 
		 */
		COMMON_API void CreateGrid( float Width, float Depth, UINT VertsX, UINT VertsZ, MeshData& Out_Mesh );
	}
}



//class COMMON_API std::vector<UINT>;
//class COMMON_API std::vector<MoRE::GeometryGenerator::Vertex>;