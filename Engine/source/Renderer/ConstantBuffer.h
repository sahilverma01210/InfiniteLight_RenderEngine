#pragma once
#include "BindableCodex.h"
#include "TechniqueProbe.h"

namespace Renderer
{
	class ConstantBuffer : public Bindable
	{
	public:
		ConstantBuffer(D3D12RHI& gfx, UINT rootParameterIndex, UINT dataSize, const void* pData);
		ConstantBuffer(D3D12RHI& gfx, UINT rootParameterIndex, Buffer dataBuffer);
		void Update(D3D12RHI& gfx, const void* pData) noexcept(!IS_DEBUG) override;
		void Update(D3D12RHI& gfx, Buffer dataBuffer) noexcept(!IS_DEBUG) override;
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		Buffer GetBuffer() const noexcept(!IS_DEBUG);
		void SetBuffer(const Buffer& buf_in);
		void Accept(TechniqueProbe& probe) override;
		std::shared_ptr<ConstantBuffer> Resolve(D3D12RHI& gfx, UINT rootParameterIndex, UINT dataSize, const void* pData)
		{
			return Codex::Resolve<ConstantBuffer>(gfx, rootParameterIndex, dataSize, pData);
		}
		template<typename...Ignore>
		static std::string GenerateUID(UINT rootParameterIndex, Ignore&&...ignore)
		{
			using namespace std::string_literals;
			return typeid(ConstantBuffer).name() + "#"s + std::to_string(rootParameterIndex);
		}
		std::string GetUID() const noexcept(!IS_DEBUG) override
		{
			return GenerateUID(m_rootParameterIndex);
		}

	private:
		bool dirty = false;
		UINT m_rootParameterIndex;
		UINT m_constantBufferSize;
		std::optional<Buffer> m_dataBuffer = std::nullopt;
		ComPtr<ID3D12Resource> m_constantBuffer;
		ComPtr<ID3D12Resource> m_constantUploadBuffer;
	};
}