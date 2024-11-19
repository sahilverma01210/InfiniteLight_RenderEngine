#pragma once
#include "Bindable.h"

namespace Renderer
{
	class Drawable;

	class TransformBuffer : public CloningBindable
	{
	public:
		struct Transforms
		{
			XMMATRIX modelViewProj;
			XMMATRIX model;
		};

		TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex);
		TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex, float scale);
		void Bind(D3D12RHI& gfx) noexcept;
		void InitializeParentReference(const Drawable& parent) noexcept override;
		std::unique_ptr<CloningBindable> Clone() const noexcept override;
	private:
		float m_scale = 1.0f;
		Transforms m_transform;
		UINT m_rootParameterIndex;
		const Drawable* pParent = nullptr;
	};
}