#pragma once
#include "Bindable.h"

namespace Renderer
{
	class TransformBuffer : public Bindable
	{
	public:
		struct Transforms
		{
			XMMATRIX modelViewProj;
			XMMATRIX model;
		};

		TransformBuffer(D3D12RHI& gfx, UINT rootParameterIndex);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept;
	private:
		Transforms m_transform;
		UINT m_rootParameterIndex;
	};
}