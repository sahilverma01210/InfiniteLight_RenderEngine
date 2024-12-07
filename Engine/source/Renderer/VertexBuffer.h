#pragma once
#include "Bindable.h"
#include "BindableCodex.h"

namespace Renderer
{
	class VertexBuffer : public Bindable
	{
	public:
		VertexBuffer(D3D12RHI& gfx, const char* data, UINT size, UINT layoutSize);
		VertexBuffer(D3D12RHI& gfx, const std::string& tag, const char* data, UINT size, UINT layoutSize);
		void CreateView(D3D12RHI& gfx, UINT strides);
		void Update(D3D12RHI& gfx, const void* pData) noexcept(!IS_DEBUG) override;
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		static std::shared_ptr<VertexBuffer> Resolve(D3D12RHI& gfx, const std::string& tag, const char* data, UINT size, UINT layoutSize);
		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore)
		{
			return GenerateUID_(tag);
		}
		std::string GetUID() const noexcept(!IS_DEBUG) override;
	private:
		static std::string GenerateUID_(const std::string& tag);

	private:
		std::string tag;
		UINT m_vertexBufferSize;
		ComPtr<ID3D12Resource> m_vertexBuffer;
		ComPtr<ID3D12Resource> m_vertexUploadBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	};
}