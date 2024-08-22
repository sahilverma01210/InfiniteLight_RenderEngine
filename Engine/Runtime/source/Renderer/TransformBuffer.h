#pragma once
#include "Bindable.h"
#include "Drawable.h"

namespace Renderer
{
	class TransformBuffer : public Bindable
	{
	public:
		TransformBuffer(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx) noexcept;
	private:
		XMMATRIX m_transform;
	};
}