#pragma once
#include "D3D12Buffer.h"
#include "D3D12Texture.h"
#include "Shapes.h"

namespace Renderer
{
	class ILMesh
	{
	public:
		struct DrawData
		{
			std::vector<VertexStruct> vertices;
			std::vector<USHORT> indices;
			UINT vertexSizeInBytes;
			UINT indexSizeInBytes;
			UINT vertexStrideInBytes;
			UINT indexStrideInBytes;
			std::shared_ptr<D3D12Buffer> vertexBuffer;
			std::shared_ptr<D3D12Buffer> indexBuffer;
			std::shared_ptr<D3D12Buffer> drawIndirectBuffer;
		};
		struct Transforms
		{
			Matrix meshMat;
			Matrix meshInvMat;
		};

	public:
		Transforms& GetTransforms() const { return m_transforms; }
		DrawData& GetDrawData() { return m_drawData; }

	protected:
		DrawData m_drawData{};
		mutable Transforms m_transforms{};
	};
}