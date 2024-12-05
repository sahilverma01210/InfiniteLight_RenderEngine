#include "ShadowCameraBuffer.h"

namespace Renderer
{
	ShadowCameraBuffer::ShadowCameraBuffer(D3D12RHI& gfx, UINT rootParameterIndex)
		:
		pVcbuf{ std::make_unique<ConstantBuffer>(gfx,rootParameterIndex, sizeof(t), &t)}
	{
	}

	void ShadowCameraBuffer::Bind(D3D12RHI& gfx) noexcept
	{
		pVcbuf->Bind(gfx);
	}

	void ShadowCameraBuffer::Update(D3D12RHI& gfx)
	{
		t.ViewProj = XMMatrixTranspose(
			pCamera->GetMatrix() * pCamera->GetProjection()
		);
		pVcbuf->Update(gfx, &t);
	}

	void ShadowCameraBuffer::SetCamera(const Camera* p) noexcept
	{
		pCamera = p;
	}
}