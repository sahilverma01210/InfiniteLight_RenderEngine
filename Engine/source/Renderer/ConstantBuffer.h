#pragma once
#include "BindableCodex.h"
#include "TechniqueProbe.h"

namespace Renderer
{
	class ConstantBuffer : public Bindable
	{
	public:
		ConstantBuffer(D3D12RHI& gfx, UINT dataSize, const void* pData);
		~ConstantBuffer() = default;
		void Update(D3D12RHI& gfx, const void* pData) noexcept(!IS_DEBUG) override;
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		ID3D12Resource* GetBuffer() const noexcept(!IS_DEBUG);
		Buffer GetDynamicBuffer() const noexcept(!IS_DEBUG);
		void SetBuffer(const Buffer& buf_in);
		void Accept(TechniqueProbe& probe) override;

	private:
		bool m_selected = false;
		UINT m_constantBufferSize;
		std::optional<Buffer> m_dataBuffer = std::nullopt;
		ComPtr<ID3D12Resource> m_constantBuffer;
		ComPtr<ID3D12Resource> m_constantUploadBuffer;
	};
}