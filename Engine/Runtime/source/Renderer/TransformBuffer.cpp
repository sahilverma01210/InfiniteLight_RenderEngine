#include "TransformBuffer.h"

namespace Renderer
{
	TransformBuffer::TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex) : m_rootParameterIndex(rootParameterIndex)
	{
	}

	void TransformBuffer::Update(D3D12RHI& gfx, const void* pData) noexcept
	{
	}

	void TransformBuffer::Bind(D3D12RHI& gfx) noexcept
	{
		m_transform = {
			XMMatrixTranspose(gfx.GetTransform() * gfx.GetCamera()),
			XMMatrixTranspose(
				gfx.GetTransform() *
				gfx.GetCamera() *
				gfx.GetProjection()
			)
		};

		GetCommandList(gfx)->SetGraphicsRoot32BitConstants(m_rootParameterIndex, sizeof(m_transform) / 4, &m_transform, 0);
	}
}