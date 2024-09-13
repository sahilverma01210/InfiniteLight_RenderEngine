#pragma once
#include "Object.h"

namespace Renderer
{
	class SolidSphere : public Object<SolidSphere>
	{
	public:
		SolidSphere(D3D12RHI& gfx, float radius);
		void SetPos(XMFLOAT3 pos) noexcept;
		const UINT GetNumIndices() const noexcept override;
		XMMATRIX GetTransformXM() const noexcept override;
	private:
		UINT m_numIndices;
		XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
	};
}