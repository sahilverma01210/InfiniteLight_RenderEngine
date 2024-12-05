#pragma once
#include "Bindable.h"
#include "ConstantBuffer.h"
#include "Camera.h"

namespace Renderer
{
	class ShadowCameraBuffer : public Bindable
	{
	protected:
		struct Transform
		{
			XMMATRIX ViewProj;
		};

	public:
		ShadowCameraBuffer(D3D12RHI& gfx, UINT rootParameterIndex);
		void Bind(D3D12RHI& gfx) noexcept override;
		void Update(D3D12RHI& gfx);
		void SetCamera(const Camera* pCamera) noexcept;

	protected:
		Transform t;
	private:
		std::unique_ptr<ConstantBuffer> pVcbuf;
		const Camera* pCamera = nullptr;
	};
}