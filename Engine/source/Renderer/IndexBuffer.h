#pragma once
#include "BindableCodex.h"

namespace Renderer
{
	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(D3D12RHI& gfx, std::vector<USHORT> pData);
		IndexBuffer(D3D12RHI& gfx, std::string tag, std::vector<USHORT> pData);
		~IndexBuffer() = default;
		void CreateView(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		static std::shared_ptr<IndexBuffer> Resolve(D3D12RHI& gfx, std::string tag, std::vector<USHORT> pData);
		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore)
		{
			return GenerateUID_(tag);
		}
		std::string GetUID() const noexcept(!IS_DEBUG) override;
		UINT GetNumOfIndices() {
			return m_numOfIndices;
		}
	private:
		static std::string GenerateUID_(const std::string& tag);

	private:
		std::string m_tag;
		UINT m_numOfIndices;
		UINT m_indexBufferSize;
		ComPtr<ID3D12Resource> m_indexBuffer;
		ComPtr<ID3D12Resource> m_indexUploadBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	};
}