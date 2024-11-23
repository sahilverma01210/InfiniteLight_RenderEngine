#pragma once
#include "Bindable.h"
#include "ConstantBuffer.h"

namespace Renderer
{
	class Camera;

	class ShadowCameraBuffer : public Bindable
	{
	protected:
		struct Transform
		{
			DirectX::XMMATRIX ViewProj;
		} t;
	public:
		ShadowCameraBuffer(D3D12RHI& gfx, UINT rootParameterIndex);
		void Bind(D3D12RHI& gfx) noexcept override;
		void Update(D3D12RHI& gfx);
		void SetCamera(const Camera* pCamera) noexcept;
	private:
		std::unique_ptr<ConstantBuffer> pVcbuf;
		const Camera* pCamera = nullptr;
	};
}