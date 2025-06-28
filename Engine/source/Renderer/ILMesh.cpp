#include "ILMesh.h"

namespace Renderer
{
	void ILMesh::ApplyMesh(D3D12RHI& gfx, VertexRawBuffer vertices, std::vector<USHORT> indices)
	{
		m_drawData.numIndices = indices.size();
		m_drawData.vertexSizeInBytes = UINT(vertices.SizeBytes());
		m_drawData.indexSizeInBytes = m_drawData.numIndices * sizeof(indices[0]);
		m_drawData.vertexStrideInBytes = (UINT)vertices.GetLayout().Size();

		m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, vertices.GetData(), m_drawData.vertexSizeInBytes));
		m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, indices.data(), m_drawData.indexSizeInBytes));
	}
}