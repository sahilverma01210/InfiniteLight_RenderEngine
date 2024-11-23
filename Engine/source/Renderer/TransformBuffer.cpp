#include "TransformBuffer.h"

#include "Drawable.h"

namespace Renderer
{
	TransformBuffer::TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex) : m_rootParameterIndex(rootParameterIndex)
	{
	}

	TransformBuffer::TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex, float scale)
		:
		TransformBuffer(gfx, rootParameterIndex)
	{
		m_scale = scale;
	}

	void TransformBuffer::Bind(D3D12RHI& gfx) noexcept
	{
		assert(pParent != nullptr);
		const auto model = pParent->GetTransformXM();
		const auto modelView = model * gfx.GetCamera();

		m_transform = {
			//XMMatrixTranspose(model),
			XMMatrixTranspose(modelView) * XMMatrixScaling(m_scale,m_scale,m_scale),
			XMMatrixTranspose(
				modelView *
				gfx.GetProjection()
			) * XMMatrixScaling(m_scale,m_scale,m_scale)
		};

		GetCommandList(gfx)->SetGraphicsRoot32BitConstants(m_rootParameterIndex, sizeof(m_transform) / 4, &m_transform, 0);
	}

	void TransformBuffer::InitializeParentReference(const Drawable& parent) noexcept
	{
		pParent = &parent;
	}

	std::unique_ptr<CloningBindable> TransformBuffer::Clone() const noexcept
	{
		return std::make_unique<TransformBuffer>(*this);
	}
}