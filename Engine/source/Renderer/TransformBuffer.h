#pragma once
#include "Bindable.h"

namespace Renderer
{
	class TransformBuffer : public Bindable
	{
	public:
		struct Transforms
		{
			XMMATRIX model;
			XMMATRIX modelView;
			XMMATRIX modelViewProj;
		};

	public:
		TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex);
		TransformBuffer(const TransformBuffer& transformBuffer);
		TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex, float scale);
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void InitializeParentReference(const Drawable& parent) noexcept(!IS_DEBUG) override;

	private:
		float m_scale = 1.0f;
		Transforms m_transform;
		UINT m_rootParameterIndex;
		const Drawable* pParent = nullptr;
	};
}