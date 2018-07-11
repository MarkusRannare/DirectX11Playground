#include "GeometryGenerator.h"

using namespace DirectX;

namespace MoGET
{
	namespace GeometryGenerator
	{
		void CreateGrid( float Width, float Depth, UINT VertsX, UINT VertsZ, MeshData& Out_Mesh )
		{
			const UINT VertexCount = VertsX * VertsZ;
			const UINT FaceCount = (VertsX - 1) * (VertsZ - 1) * 2;

			const float HalfWidth = 0.5f * Width;
			const float HalfDepth = 0.5f * Depth;

			const float DX = Width / (VertsX - 1);
			const float DZ = Depth / (VertsZ - 1);

			const float DU = 1.0f / (VertsX - 1);
			const float DV = 1.0f / (VertsZ - 1);

			// Generate vertices
			Out_Mesh.Vertices.SetNum(VertexCount, false);
			for( UINT IdxZ = 0; IdxZ < VertsZ; ++IdxZ )
			{
				const float Z = HalfDepth - IdxZ * DZ;
				for( UINT IdxX = 0; IdxX < VertsX; ++IdxX )
				{
					const float X = -HalfWidth + IdxX * DX;

					Vertex& Vert = Out_Mesh.Vertices[IdxZ * VertsX + IdxX];

					Vert.Position = XMFLOAT3( X, 0.0f, Z );
					Vert.Normal = XMFLOAT3( 0.0f, 1.0f, 0.0f );
					Vert.Tangent = XMFLOAT3( 1.0f, 0.0f, 0.0f );
					Vert.Tex = XMFLOAT2( IdxX * DU, IdxZ * DV );
				}
			}

			// Generate indices
			Array<UINT>& Indices = Out_Mesh.Indices;
			Indices.SetNum( FaceCount * 3, false ); // 3 indices per face
			// Compute indices for each quad
			UINT Idx = 0;
			for( UINT IdxZ = 0; IdxZ < VertsZ - 1; ++IdxZ )
			{
				for( UINT IdxX = 0; IdxX < VertsX - 1; ++IdxX )
				{
					Indices[Idx + 0] = IdxZ * VertsX + IdxX;
					Indices[Idx + 1] = IdxZ * VertsX + IdxX + 1;
					Indices[Idx + 2] = ( IdxZ + 1 ) * VertsX + IdxX;
					Indices[Idx + 3] = ( IdxZ + 1 ) * VertsX + IdxX;
					Indices[Idx + 4] = IdxZ * VertsX + IdxX + 1;
					Indices[Idx + 5] = ( IdxZ + 1 ) * VertsX + IdxX + 1;

					Idx += 6;
				}
			}
		}
	}
}