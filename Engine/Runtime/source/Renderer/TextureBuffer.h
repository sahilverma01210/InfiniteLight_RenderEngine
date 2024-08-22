#pragma once

#include "Bindable.h"

namespace Renderer
{
	class TextureBuffer : public Bindable
	{
	public:
		TextureBuffer(D3D12RHI& gfx);
		void CreateView(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		ComPtr<ID3D12Resource> m_texureBuffer;
	};
}