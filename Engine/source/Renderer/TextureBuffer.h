#pragma once
#include "Bindable.h"
#include "ShaderResourceView.h"

namespace Renderer
{
	class TextureBuffer : public Bindable
	{
	public:
		TextureBuffer(D3D12RHI& gfx, const WCHAR* filename);
		ID3D12Resource* GetBuffer();
		bool HasAlpha() const noexcept;
		bool HasAlphaChannel(const DirectX::Image& image);
		static std::shared_ptr<TextureBuffer> Resolve(D3D12RHI& gfx, const WCHAR* filename);
		static std::string GenerateUID(const WCHAR* filename);
		std::string GetUID() const noexcept override;
	private:
		bool hasAlpha = false;
		const WCHAR* m_filename;
		ComPtr<ID3D12Resource> m_texureBuffer;
	};
}