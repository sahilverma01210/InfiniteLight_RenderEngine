#pragma once
#include "../_External/framework.h"

#include "Drawable.h"
#include "CommonBindables.h"
#include "CommonShapes.h"

namespace Renderer
{
	class SolidSphere : public Drawable
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