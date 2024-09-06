#pragma once

#include "Bindable.h"
#include "ShaderResourceView.h"

namespace Renderer
{
	class TextureBuffer : public Bindable
	{
	public:
		TextureBuffer(D3D12RHI& gfx, UINT rootParameterIndex, const WCHAR* filename);
		void CreateView(D3D12RHI& gfx);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_rootParameterIndex;
		ComPtr<ID3D12Resource> m_texureBuffer;
		std::unique_ptr<ShaderResourceView> m_shaderResourceView;
	};
}