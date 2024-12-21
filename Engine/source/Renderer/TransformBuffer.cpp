#include "TransformBuffer.h"

namespace Renderer
{
	TransformBuffer::TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex) : m_rootParameterIndex(rootParameterIndex)
	{
	}

	TransformBuffer::TransformBuffer(const TransformBuffer& transformBuffer)
	{
		m_scale = transformBuffer.m_scale;
		m_transform = transformBuffer.m_transform;
		m_rootParameterIndex = transformBuffer.m_rootParameterIndex;
	}

	TransformBuffer::TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex, float scale)
		:
		TransformBuffer(gfx, rootParameterIndex)
	{
		m_scale = scale;
	}

	void TransformBuffer::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		const auto model = gfx.GetTransform();
		const auto modelView = model * gfx.GetCamera();

		/*
		* Convert all XMMATRIX or XMFLOAT4X4 which are Row - major into Column - major matrix which is used by HLSL by default.
		* Use XMMatrixTranspose() to achieve this.
		*/ 
		m_transform = {
			XMMatrixTranspose(model),
			XMMatrixTranspose(modelView) * XMMatrixScaling(m_scale,m_scale,m_scale),
			XMMatrixTranspose(
				modelView *
				gfx.GetProjection()
			) * XMMatrixScaling(m_scale,m_scale,m_scale)
		};

		// Using Root Parameter Index to bind this buffer to Root Parameter having correct Shader Register which is used in HLSL.
		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRoot32BitConstants(m_rootParameterIndex, sizeof(m_transform) / 4, &m_transform, 0));
	}

	SkyboxTransformBuffer::SkyboxTransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex) : m_rootParameterIndex(rootParameterIndex)
	{
	}

	void SkyboxTransformBuffer::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		m_transform = {
			XMMatrixTranspose(gfx.GetCamera() * gfx.GetProjection())
		};

		// Using Root Parameter Index to bind this buffer to Root Parameter having correct Shader Register which is used in HLSL.
		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRoot32BitConstants(m_rootParameterIndex, sizeof(m_transform) / 4, &m_transform, 0));
	}
}