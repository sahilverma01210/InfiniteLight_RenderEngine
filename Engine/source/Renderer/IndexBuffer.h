#pragma once
#include "Bindable.h"

namespace Renderer
{
	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(D3D12RHI& gfx, UINT dataSize, std::vector<USHORT> pData);
		IndexBuffer(D3D12RHI& gfx, std::string tag, UINT dataSize, std::vector<USHORT> pData);
		void CreateView(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx) noexcept override;
		static std::shared_ptr<IndexBuffer> Resolve(D3D12RHI& gfx, std::string tag, UINT dataSize, std::vector<USHORT> pData);
		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore)
		{
			return GenerateUID_(tag);
		}
		std::string GetUID() const noexcept override;
		UINT GetNumOfIndices() {
			return m_numOfIndices;
		}
	private:
		static std::string GenerateUID_(const std::string& tag);
	private:
		std::string tag;
		UINT m_numOfIndices;
		UINT m_indexBufferSize;
		ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	};
}