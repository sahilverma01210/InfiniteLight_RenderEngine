#include "TransformBuffer.h"

namespace Renderer
{
	TransformBuffer::TransformBuffer(D3D12RHI& gfx)
	{
	}

	void TransformBuffer::Bind(D3D12RHI& gfx) noexcept
	{
		m_transform = XMMatrixTranspose(
			gfx.GetTransform() *
			gfx.GetCamera() *
			gfx.GetProjection()
		);

		GetCommandList(gfx)->SetGraphicsRoot32BitConstants(0, sizeof(m_transform) / 4, &m_transform, 0);
	}
}