#pragma once
#include "Bindable.h"
#include "BindableCodex.h"

namespace Renderer
{
	class ConstantBuffer : public Bindable
	{
	public:
		ConstantBuffer(D3D12RHI& gfx, UINT rootParameterIndex, UINT dataSize, const void* pData);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
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
		std::string GetUID() const noexcept override
		{
			return GenerateUID(m_rootParameterIndex);
		}
	private:
		UINT m_rootParameterIndex;
		UINT m_constantBufferSize;
		ComPtr<ID3D12Resource> m_constantBuffer;
		ComPtr<ID3D12Resource> m_constantUploadBuffer;
	};
}