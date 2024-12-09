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
}