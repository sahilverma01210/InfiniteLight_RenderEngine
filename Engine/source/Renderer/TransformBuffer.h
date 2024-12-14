#pragma once
#include "BindableCodex.h"

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
		~TransformBuffer() = default;
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG);

	private:
		float m_scale = 1.0f;
		Transforms m_transform;
		UINT m_rootParameterIndex;
	};
}