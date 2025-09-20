#include "PointLightIndicator.h"

namespace Renderer
{
	PointLightIndicator::PointLightIndicator(D3D12RHI& gfx, float radius)
	{
		m_indexedList = Sphere::Make();		
		m_indexedList.Transform(XMMatrixScaling(radius, radius, radius)); // deform vertices of model by linear transformation

		m_drawData.vertices = m_indexedList.vertices;
		m_drawData.indices = m_indexedList.indices;
		m_drawData.vertexSizeInBytes = m_drawData.vertices.size() * sizeof(m_drawData.vertices[0]);
		m_drawData.indexSizeInBytes = m_drawData.indices.size() * sizeof(m_drawData.indices[0]);
		m_drawData.vertexStrideInBytes = sizeof(VertexStruct);

		m_drawData.vertexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_drawData.vertices.data(), m_drawData.vertexSizeInBytes));
		m_drawData.indexBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, m_drawData.indices.data(), m_drawData.indexSizeInBytes));

		D3D12_DRAW_INDEXED_ARGUMENTS drawArgs = {};
		drawArgs.IndexCountPerInstance = m_drawData.indices.size();
		drawArgs.InstanceCount = 1;
		drawArgs.StartIndexLocation = 0;
		drawArgs.BaseVertexLocation = 0;
		drawArgs.StartInstanceLocation = 0;

		m_drawData.drawIndirectBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, &drawArgs, sizeof(drawArgs)));
	}

	void PointLightIndicator::Update(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		m_transforms.meshMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
		m_transforms.meshInvMat = XMMatrixInverse(nullptr, m_transforms.meshMat);
	}
}