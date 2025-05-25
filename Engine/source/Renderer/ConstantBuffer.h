#pragma once
#include "BindableCodex.h"
#include "TechniqueProbe.h"

namespace Renderer
{
	class ConstantBuffer : public D3D12Resource, public Bindable
	{
	public:
		ConstantBuffer(D3D12RHI& gfx, UINT dataSize, const void* pData);
		~ConstantBuffer() = default;
		void Update(D3D12RHI& gfx, const void* pData) noexcept(!IS_DEBUG) override;
		void Accept(TechniqueProbe& probe) override;

	private:
		bool m_selected = false;
		UINT m_constantBufferSize;
		//std::optional<Buffer> m_dataBuffer = std::nullopt;
		ComPtr<ID3D12Resource> m_constantUploadBuffer;
	};
}