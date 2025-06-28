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
			UINT numIndices;
			UINT vertexSizeInBytes;
			UINT indexSizeInBytes;
			UINT vertexStrideInBytes;
			std::shared_ptr<D3D12Buffer> vertexBuffer;
			std::shared_ptr<D3D12Buffer> indexBuffer;
		};
		struct Transforms
		{
			Matrix meshMat;
			Matrix meshInvMat;
		};

	public:
		void ApplyMesh(D3D12RHI& gfx, VertexRawBuffer vertices, std::vector<USHORT> indices);
		Transforms& GetTransforms() const { return m_transforms; }
		ResourceHandle GetMaterialIdx() const { return m_materialIdx; }
		DrawData& GetDrawData() { return m_drawData; }

	protected:
		DrawData m_drawData{};
		ResourceHandle m_materialIdx;
		mutable Transforms m_transforms{};
	};
}