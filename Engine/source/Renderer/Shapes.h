#pragma once
#include "RenderMath.h"
#include "Primitives.h"

namespace Renderer
{
	class Plane
	{
	public:
		static IndexedTriangleList Make()
		{
			std::vector<VertexStruct> vertices;
			vertices.resize(4);
			vertices[0].position = { -1, 1, 0 };
			vertices[1].position = { 1, 1, 0 };
			vertices[2].position = { -1, -1, 0 };
			vertices[3].position = { 1, -1, 0 };
			return{
				std::move(vertices),{ 0,1,2,1,3,2 }
			};
		}
	};

	class Cube
	{
	public:
		static IndexedTriangleList Make()
		{
			constexpr float side = 1.0f / 2.0f;
			std::vector<VertexStruct> vertices;
			vertices.resize(8);
			vertices[0].position = { -side,-side,-side };
			vertices[1].position = { side,-side,-side };
			vertices[2].position = { -side,side,-side };
			vertices[3].position = { side,side,-side };
			vertices[4].position = { -side,-side,side };
			vertices[5].position = { side,-side,side };
			vertices[6].position = { -side,side,side };
			vertices[7].position = { side,side,side };
			return{
				std::move(vertices),{
					0,2,1, 2,3,1,
					1,3,5, 3,7,5,
					2,6,3, 3,6,7,
					4,5,7, 4,7,6,
					0,4,2, 2,4,6,
					0,1,4, 1,5,4
				}
			};
		}
		static IndexedTriangleList MakeIndependent()
		{
			constexpr float side = 1.0f / 2.0f;

			std::vector<VertexStruct> vertices;
			vertices.resize(24u);
			vertices[0].position = { -side,-side,-side };// 0 near side
			vertices[1].position = { side,-side,-side };// 1
			vertices[2].position = { -side,side,-side };// 2
			vertices[3].position = { side,side,-side };// 3
			vertices[4].position = { -side,-side,side };// 4 far side
			vertices[5].position = { side,-side,side };// 5
			vertices[6].position = { -side,side,side };// 6
			vertices[7].position = { side,side,side };// 7
			vertices[8].position = { -side,-side,-side };// 8 left side
			vertices[9].position = { -side,side,-side };// 9
			vertices[10].position = { -side,-side,side };// 10
			vertices[11].position = { -side,side,side };// 11
			vertices[12].position = { side,-side,-side };// 12 right side
			vertices[13].position = { side,side,-side };// 13
			vertices[14].position = { side,-side,side };// 14
			vertices[15].position = { side,side,side };// 15
			vertices[16].position = { -side,-side,-side };// 16 bottom side
			vertices[17].position = { side,-side,-side };// 17
			vertices[18].position = { -side,-side,side };// 18
			vertices[19].position = { side,-side,side };// 19
			vertices[20].position = { -side,side,-side };// 20 top side
			vertices[21].position = { side,side,-side };// 21
			vertices[22].position = { -side,side,side };// 22
			vertices[23].position = { side,side,side };// 23

			return{
				std::move(vertices),{
					0,2, 1,    2,3,1,
					4,5, 7,    4,7,6,
					8,10, 9,  10,11,9,
					12,13,15, 12,15,14,
					16,17,18, 18,17,19,
					20,23,21, 20,22,23
				}
			};
		}
		static IndexedTriangleList MakeIndependentTextured()
		{
			auto itl = MakeIndependent();

			itl.vertices[0].texCoord = { 0.0f,0.0f };
			itl.vertices[1].texCoord = { 1.0f,0.0f };
			itl.vertices[2].texCoord = { 0.0f,1.0f };
			itl.vertices[3].texCoord = { 1.0f,1.0f };
			itl.vertices[4].texCoord = { 0.0f,0.0f };
			itl.vertices[5].texCoord = { 1.0f,0.0f };
			itl.vertices[6].texCoord = { 0.0f,1.0f };
			itl.vertices[7].texCoord = { 1.0f,1.0f };
			itl.vertices[8].texCoord = { 0.0f,0.0f };
			itl.vertices[9].texCoord = { 1.0f,0.0f };
			itl.vertices[10].texCoord = { 0.0f,1.0f };
			itl.vertices[11].texCoord = { 1.0f,1.0f };
			itl.vertices[12].texCoord = { 0.0f,0.0f };
			itl.vertices[13].texCoord = { 1.0f,0.0f };
			itl.vertices[14].texCoord = { 0.0f,1.0f };
			itl.vertices[15].texCoord = { 1.0f,1.0f };
			itl.vertices[16].texCoord = { 0.0f,0.0f };
			itl.vertices[17].texCoord = { 1.0f,0.0f };
			itl.vertices[18].texCoord = { 0.0f,1.0f };
			itl.vertices[19].texCoord = { 1.0f,1.0f };
			itl.vertices[20].texCoord = { 0.0f,0.0f };
			itl.vertices[21].texCoord = { 1.0f,0.0f };
			itl.vertices[22].texCoord = { 0.0f,1.0f };
			itl.vertices[23].texCoord = { 1.0f,1.0f };

			return itl;
		}
	};

	class Sphere
	{
	public:
		static IndexedTriangleList MakeTesselated(int latDiv, int longDiv)
		{
			assert(latDiv >= 3);
			assert(longDiv >= 3);

			constexpr float radius = 0.1f;
			const auto base = XMVectorSet(0.0f, 0.0f, radius, 0.0f);
			const float lattitudeAngle = PI / latDiv;
			const float longitudeAngle = 2.0f * PI / longDiv;

			std::vector<VertexStruct> vb;
			for (int iLat = 1; iLat < latDiv; iLat++)
			{
				const auto latBase = XMVector3Transform(
					base,
					XMMatrixRotationX(lattitudeAngle * iLat)
				);
				for (int iLong = 0; iLong < longDiv; iLong++)
				{
					XMFLOAT3 calculatedPos;
					auto v = XMVector3Transform(
						latBase,
						XMMatrixRotationZ(longitudeAngle * iLong)
					);
					XMStoreFloat3(&calculatedPos, v);
					auto& vertex = vb.emplace_back();
					vertex.position = calculatedPos;
				}
			}

			// add the cap vertices
			const auto iNorthPole = (unsigned short)vb.size();
			{
				XMFLOAT3 northPos;
				XMStoreFloat3(&northPos, base);
				auto& vertex = vb.emplace_back();
				vertex.position = northPos;
			}
			const auto iSouthPole = (unsigned short)vb.size();
			{
				XMFLOAT3 southPos;
				XMStoreFloat3(&southPos, XMVectorNegate(base));
				auto& vertex = vb.emplace_back();
				vertex.position = southPos;
			}

			const auto calcIdx = [latDiv, longDiv](USHORT iLat, USHORT iLong)
				{ return iLat * longDiv + iLong; };
			std::vector<USHORT> indices;
			for (USHORT iLat = 0; iLat < latDiv - 2; iLat++)
			{
				for (USHORT iLong = 0; iLong < longDiv - 1; iLong++)
				{
					indices.push_back(calcIdx(iLat, iLong));
					indices.push_back(calcIdx(iLat + 1, iLong));
					indices.push_back(calcIdx(iLat, iLong + 1));
					indices.push_back(calcIdx(iLat, iLong + 1));
					indices.push_back(calcIdx(iLat + 1, iLong));
					indices.push_back(calcIdx(iLat + 1, iLong + 1));
				}
				// wrap band
				indices.push_back(calcIdx(iLat, longDiv - 1));
				indices.push_back(calcIdx(iLat + 1, longDiv - 1));
				indices.push_back(calcIdx(iLat, 0));
				indices.push_back(calcIdx(iLat, 0));
				indices.push_back(calcIdx(iLat + 1, longDiv - 1));
				indices.push_back(calcIdx(iLat + 1, 0));
			}

			// cap fans
			for (USHORT iLong = 0; iLong < longDiv - 1; iLong++)
			{
				// north
				indices.push_back(iNorthPole);
				indices.push_back(calcIdx(0, iLong));
				indices.push_back(calcIdx(0, iLong + 1));
				// south
				indices.push_back(calcIdx(latDiv - 2, iLong + 1));
				indices.push_back(calcIdx(latDiv - 2, iLong));
				indices.push_back(iSouthPole);
			}
			// wrap triangles
			// north
			indices.push_back(iNorthPole);
			indices.push_back(calcIdx(0, longDiv - 1));
			indices.push_back(calcIdx(0, 0));
			// south
			indices.push_back(calcIdx(latDiv - 2, 0));
			indices.push_back(calcIdx(latDiv - 2, longDiv - 1));
			indices.push_back(iSouthPole);

			return { std::move(vb),std::move(indices) };
		}
		static IndexedTriangleList Make()
		{
			return MakeTesselated( 12, 24);
		}
	};

	class Pyramid
	{
	public:
		// If Commented parts uncomented then not actually a Pyramid but a Pyramid with extra Triangle. Used for Camera Indicator
		static IndexedTriangleList Make()
		{
			std::vector<VertexStruct> vertices;
			vertices.resize(5);
			{
				const float x = 4.0f / 3.0f * 0.75f;
				const float y = 1.0f * 0.75f;
				const float z = -2.0f;
				//const float thalf = x * 0.5f;
				//const float tspace = y * 0.2f;
				vertices[0].position = XMFLOAT3{ -x,y,0.0f };
				vertices[1].position = XMFLOAT3{ x,y,0.0f };
				vertices[2].position = XMFLOAT3{ x,-y,0.0f };
				vertices[3].position = XMFLOAT3{ -x,-y,0.0f };
				vertices[4].position = XMFLOAT3{ 0.0f,0.0f,z };
				//vertices[5].position = XMFLOAT3{ -thalf,y + tspace,0.0f };
				//vertices[6].position = XMFLOAT3{ thalf,y + tspace,0.0f };
				//vertices[7].position = XMFLOAT3{ 0.0f,y + tspace + thalf,0.0f };
			}
			std::vector<USHORT> indices;
			{
				indices.push_back(0);
				indices.push_back(2);
				indices.push_back(1);
				indices.push_back(0);
				indices.push_back(3);
				indices.push_back(2);
				indices.push_back(0);
				indices.push_back(1);
				indices.push_back(4);
				indices.push_back(1);
				indices.push_back(2);
				indices.push_back(4);
				indices.push_back(2);
				indices.push_back(3);
				indices.push_back(4);
				indices.push_back(3);
				indices.push_back(0);
				indices.push_back(4);
				//indices.push_back(5);
				//indices.push_back(6);
				//indices.push_back(7);
			}

			return { std::move(vertices),std::move(indices) };
		}
	};

	class Frustum
	{
	public:
		static IndexedTriangleList MakeTesselated(float width, float height, float nearZ, float farZ)
		{
			std::vector<VertexStruct> vertices;
			vertices.resize(8);
			{
				const float zRatio = farZ / nearZ;
				const float nearX = width / 2.0f;
				const float nearY = height / 2.0f;
				const float farX = nearX * zRatio;
				const float farY = nearY * zRatio;
				vertices[0].position = XMFLOAT3{ -nearX,nearY,nearZ };
				vertices[1].position = XMFLOAT3{ nearX,nearY,nearZ };
				vertices[2].position = XMFLOAT3{ nearX,-nearY,nearZ };
				vertices[3].position = XMFLOAT3{ -nearX,-nearY,nearZ };
				vertices[4].position = XMFLOAT3{ -farX,farY,farZ };
				vertices[5].position = XMFLOAT3{ farX,farY,farZ };
				vertices[6].position = XMFLOAT3{ farX,-farY,farZ };
				vertices[7].position = XMFLOAT3{ -farX,-farY,farZ };
			}

			std::vector<unsigned short> indices;
			{
				indices.push_back(0);
				indices.push_back(1);
				indices.push_back(1);
				indices.push_back(2);
				indices.push_back(2);
				indices.push_back(3);
				indices.push_back(3);
				indices.push_back(0);
				indices.push_back(4);
				indices.push_back(5);
				indices.push_back(5);
				indices.push_back(6);
				indices.push_back(6);
				indices.push_back(7);
				indices.push_back(7);
				indices.push_back(4);
				indices.push_back(0);
				indices.push_back(4);
				indices.push_back(1);
				indices.push_back(5);
				indices.push_back(2);
				indices.push_back(6);
				indices.push_back(3);
				indices.push_back(7);
			}

			return { std::move(vertices),std::move(indices) };
		}
		static IndexedTriangleList Make(float width, float height, float nearZ, float farZ)
		{
			return MakeTesselated(width, height, nearZ, farZ);
		}
	};
}
