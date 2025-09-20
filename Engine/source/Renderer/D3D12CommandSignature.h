#pragma once
#include "GraphicsResource.h"

namespace Renderer
{
	class D3D12CommandSignature : public GraphicsResource
	{
	public:
		D3D12CommandSignature(D3D12RHI& gfx, ID3D12RootSignature* rootSignature = nullptr);
		ID3D12CommandSignature* GetCommandSignature() const noexcept(!IS_DEBUG) { return m_commandSignature.Get(); }
	private:
		D3D12RHI& m_gfx;
		ComPtr<ID3D12CommandSignature> m_commandSignature;
	};
}