#pragma once
#include "RenderMath.h"

#include "IndexedTriangleList.h"

namespace Renderer
{
	class Plane
	{
	public:
		static IndexedTriangleList Make(std::optional<VertexLayout> layout = std::nullopt)
		{
			using Type = VertexLayout::ElementType;
			if (!layout)
			{
				layout = VertexLayout{};
				layout->Append(VertexLayout::Position2D);
			}

			VertexRawBuffer vertices(std::move(*layout));
			vertices.Resize(4);
			vertices[0].Attr<Type::Position2D>() = { -1,1 };
			vertices[1].Attr<Type::Position2D>() = { 1,1 };
			vertices[2].Attr<Type::Position2D>() = { -1,-1 };
			vertices[3].Attr<Type::Position2D>() = { 1,-1 };
			return{
				std::move(vertices),{ 0,1,2,1,3,2 }
			};
		}
	};

	class Cube
	{
	public:
		static IndexedTriangleList Make(std::optional<VertexLayout> layout = std::nullopt)
		{
			using Type = VertexLayout::ElementType;
			if (!layout)
			{
				layout = VertexLayout{};
				layout->Append(Type::Position3D);
			}
			constexpr float side = 1.0f / 2.0f;
			VertexRawBuffer vertices(std::move(*layout));
			vertices.Resize(8);
			vertices[0].Attr<Type::Position3D>() = { -side,-side,-side };
			vertices[1].Attr<Type::Position3D>() = { side,-side,-side };
			vertices[2].Attr<Type::Position3D>() = { -side,side,-side };
			vertices[3].Attr<Type::Position3D>() = { side,side,-side };
			vertices[4].Attr<Type::Position3D>() = { -side,-side,side };
			vertices[5].Attr<Type::Position3D>() = { side,-side,side };
			vertices[6].Attr<Type::Position3D>() = { -side,side,side };
			vertices[7].Attr<Type::Position3D>() = { side,side,side };
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
		static IndexedTriangleList MakeIndependent(VertexRawBuffer layout)
		{
			using Type = VertexLayout::ElementType;

			constexpr float side = 1.0f / 2.0f;

			VertexRawBuffer vertices(std::move(layout));
			vertices.Resize(24u);
			vertices[0].Attr<Type::Position3D>() = { -side,-side,-side };// 0 near side
			vertices[1].Attr<Type::Position3D>() = { side,-side,-side };// 1
			vertices[2].Attr<Type::Position3D>() = { -side,side,-side };// 2
			vertices[3].Attr<Type::Position3D>() = { side,side,-side };// 3
			vertices[4].Attr<Type::Position3D>() = { -side,-side,side };// 4 far side
			vertices[5].Attr<Type::Position3D>() = { side,-side,side };// 5
			vertices[6].Attr<Type::Position3D>() = { -side,side,side };// 6
			vertices[7].Attr<Type::Position3D>() = { side,side,side };// 7
			vertices[8].Attr<Type::Position3D>() = { -side,-side,-side };// 8 left side
			vertices[9].Attr<Type::Position3D>() = { -side,side,-side };// 9
			vertices[10].Attr<Type::Position3D>() = { -side,-side,side };// 10
			vertices[11].Attr<Type::Position3D>() = { -side,side,side };// 11
			vertices[12].Attr<Type::Position3D>() = { side,-side,-side };// 12 right side
			vertices[13].Attr<Type::Position3D>() = { side,side,-side };// 13
			vertices[14].Attr<Type::Position3D>() = { side,-side,side };// 14
			vertices[15].Attr<Type::Position3D>() = { side,side,side };// 15
			vertices[16].Attr<Type::Position3D>() = { -side,-side,-side };// 16 bottom side
			vertices[17].Attr<Type::Position3D>() = { side,-side,-side };// 17
			vertices[18].Attr<Type::Position3D>() = { -side,-side,side };// 18
			vertices[19].Attr<Type::Position3D>() = { side,-side,side };// 19
			vertices[20].Attr<Type::Position3D>() = { -side,side,-side };// 20 top side
			vertices[21].Attr<Type::Position3D>() = { side,side,-side };// 21
			vertices[22].Attr<Type::Position3D>() = { -side,side,side };// 22
			vertices[23].Attr<Type::Position3D>() = { side,side,side };// 23

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
			using Type = VertexLayout::ElementType;

			auto itl = MakeIndependent(std::move(VertexLayout{}
				.Append(Type::Position3D)
				.Append(Type::Normal)
				.Append(Type::Texture2D)
			));

			itl.vertices[0].Attr<Type::Texture2D>() = { 0.0f,0.0f };
			itl.vertices[1].Attr<Type::Texture2D>() = { 1.0f,0.0f };
			itl.vertices[2].Attr<Type::Texture2D>() = { 0.0f,1.0f };
			itl.vertices[3].Attr<Type::Texture2D>() = { 1.0f,1.0f };
			itl.vertices[4].Attr<Type::Texture2D>() = { 0.0f,0.0f };
			itl.vertices[5].Attr<Type::Texture2D>() = { 1.0f,0.0f };
			itl.vertices[6].Attr<Type::Texture2D>() = { 0.0f,1.0f };
			itl.vertices[7].Attr<Type::Texture2D>() = { 1.0f,1.0f };
			itl.vertices[8].Attr<Type::Texture2D>() = { 0.0f,0.0f };
			itl.vertices[9].Attr<Type::Texture2D>() = { 1.0f,0.0f };
			itl.vertices[10].Attr<Type::Texture2D>() = { 0.0f,1.0f };
			itl.vertices[11].Attr<Type::Texture2D>() = { 1.0f,1.0f };
			itl.vertices[12].Attr<Type::Texture2D>() = { 0.0f,0.0f };
			itl.vertices[13].Attr<Type::Texture2D>() = { 1.0f,0.0f };
			itl.vertices[14].Attr<Type::Texture2D>() = { 0.0f,1.0f };
			itl.vertices[15].Attr<Type::Texture2D>() = { 1.0f,1.0f };
			itl.vertices[16].Attr<Type::Texture2D>() = { 0.0f,0.0f };
			itl.vertices[17].Attr<Type::Texture2D>() = { 1.0f,0.0f };
			itl.vertices[18].Attr<Type::Texture2D>() = { 0.0f,1.0f };
			itl.vertices[19].Attr<Type::Texture2D>() = { 1.0f,1.0f };
			itl.vertices[20].Attr<Type::Texture2D>() = { 0.0f,0.0f };
			itl.vertices[21].Attr<Type::Texture2D>() = { 1.0f,0.0f };
			itl.vertices[22].Attr<Type::Texture2D>() = { 0.0f,1.0f };
			itl.vertices[23].Attr<Type::Texture2D>() = { 1.0f,1.0f };

			return itl;
		}
	};

	class Sphere
	{
	public:
		static IndexedTriangleList MakeTesselated(VertexRawBuffer layout, int latDiv, int longDiv)
		{
			assert(latDiv >= 3);
			assert(longDiv >= 3);

			constexpr float radius = 1.0f;
			const auto base = XMVectorSet(0.0f, 0.0f, radius, 0.0f);
			const float lattitudeAngle = PI / latDiv;
			const float longitudeAngle = 2.0f * PI / longDiv;

			VertexRawBuffer vb{ std::move(layout) };
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
					vb.EmplaceBack(calculatedPos);
				}
			}

			// add the cap vertices
			const auto iNorthPole = (unsigned short)vb.Size();
			{
				XMFLOAT3 northPos;
				XMStoreFloat3(&northPos, base);
				vb.EmplaceBack(northPos);
			}
			const auto iSouthPole = (unsigned short)vb.Size();
			{
				XMFLOAT3 southPos;
				XMStoreFloat3(&southPos, XMVectorNegate(base));
				vb.EmplaceBack(southPos);
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
		static IndexedTriangleList Make(std::optional<VertexLayout> layout = std::nullopt)
		{
			using Element = VertexLayout::ElementType;
			if (!layout)
			{
				layout = VertexLayout{}.Append(Element::Position3D);
			}
			return MakeTesselated(std::move(*layout), 12, 24);
		}
	};

	class Pyramid
	{
	public:
		// If Commented parts uncomented then not actually a Pyramid but a Pyramid with extra Triangle. Used for Camera Indicator
		static IndexedTriangleList Make(std::optional<VertexLayout> layout = std::nullopt)
		{
			using Element = VertexLayout::ElementType;
			if (!layout)
			{
				layout = VertexLayout{}.Append(Element::Position3D);
			}

			VertexRawBuffer vertices{ std::move(*layout) };
			{
				const float x = 4.0f / 3.0f * 0.75f;
				const float y = 1.0f * 0.75f;
				const float z = -2.0f;
				//const float thalf = x * 0.5f;
				//const float tspace = y * 0.2f;
				vertices.EmplaceBack(XMFLOAT3{ -x,y,0.0f });
				vertices.EmplaceBack(XMFLOAT3{ x,y,0.0f });
				vertices.EmplaceBack(XMFLOAT3{ x,-y,0.0f });
				vertices.EmplaceBack(XMFLOAT3{ -x,-y,0.0f });
				vertices.EmplaceBack(XMFLOAT3{ 0.0f,0.0f,z });
				//vertices.EmplaceBack(XMFLOAT3{ -thalf,y + tspace,0.0f });
				//vertices.EmplaceBack(XMFLOAT3{ thalf,y + tspace,0.0f });
				//vertices.EmplaceBack(XMFLOAT3{ 0.0f,y + tspace + thalf,0.0f });
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
				indices.push_back(0);
				indices.push_back(4);
				indices.push_back(1);
				indices.push_back(4);
				indices.push_back(2);
				indices.push_back(4);
				indices.push_back(3);
				indices.push_back(4);
				indices.push_back(2);
				indices.push_back(3);
				//indices.push_back(5);
				//indices.push_back(6);
				//indices.push_back(6);
				//indices.push_back(7);
				//indices.push_back(7);
				//indices.push_back(5);
			}

			return { std::move(vertices),std::move(indices) };
		}
	};

	class Frustum
	{
	public:
		static IndexedTriangleList MakeTesselated(VertexRawBuffer layout, float width, float height, float nearZ, float farZ)
		{
			VertexRawBuffer vertices{ std::move(layout) };
			{
				const float zRatio = farZ / nearZ;
				const float nearX = width / 2.0f;
				const float nearY = height / 2.0f;
				const float farX = nearX * zRatio;
				const float farY = nearY * zRatio;
				vertices.EmplaceBack(XMFLOAT3{ -nearX,nearY,nearZ });
				vertices.EmplaceBack(XMFLOAT3{ nearX,nearY,nearZ });
				vertices.EmplaceBack(XMFLOAT3{ nearX,-nearY,nearZ });
				vertices.EmplaceBack(XMFLOAT3{ -nearX,-nearY,nearZ });
				vertices.EmplaceBack(XMFLOAT3{ -farX,farY,farZ });
				vertices.EmplaceBack(XMFLOAT3{ farX,farY,farZ });
				vertices.EmplaceBack(XMFLOAT3{ farX,-farY,farZ });
				vertices.EmplaceBack(XMFLOAT3{ -farX,-farY,farZ });
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
		static IndexedTriangleList Make(float width, float height, float nearZ, float farZ, std::optional<VertexLayout> layout = std::nullopt)
		{
			using Element = VertexLayout::ElementType;
			if (!layout)
			{
				layout = VertexLayout{}.Append(Element::Position3D);
			}
			return MakeTesselated(std::move(*layout), width, height, nearZ, farZ);
		}
	};
}
