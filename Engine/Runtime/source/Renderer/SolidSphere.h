#pragma once
#include "Object.h"

namespace Renderer
{
	class SolidSphere : public Object<SolidSphere>
	{
	public:
		SolidSphere(D3D12RHI& gfx, float radius);
		void Update(float dt) noexcept override;
		void SetPos(XMFLOAT3 pos) noexcept;
		XMMATRIX GetTransformXM() const noexcept override;
	private:
		XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
	};
}