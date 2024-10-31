#pragma once
#include "D3D12RHI.h"

namespace Renderer
{
	class Drawable;
	class TechniqueProbe;

	class Bindable
	{
	public:
		virtual ~Bindable() = default;
		virtual void Update(D3D12RHI& gfx, const void* pData) noexcept = 0;
		virtual void Bind(D3D12RHI& gfx) noexcept = 0;
		virtual void InitializeParentReference(const Drawable&) noexcept
		{}
		virtual void Accept(TechniqueProbe&)
		{}
		virtual std::string GetUID() const noexcept
		{
			assert(false);
			return "";
		}
	protected:
		static UINT GetWidth(D3D12RHI& gfx) noexcept;
		static UINT GetHeight(D3D12RHI& gfx) noexcept;
		static ID3D12Device* GetDevice(D3D12RHI& gfx) noexcept;
		static ID3D12CommandQueue* GetCommandQueue(D3D12RHI& gfx) noexcept;
		static ID3D12CommandAllocator* GetCommandAllocator(D3D12RHI& gfx) noexcept;
		static ID3D12GraphicsCommandList* GetCommandList(D3D12RHI& gfx) noexcept;
		static void InsertFence(D3D12RHI& gfx) noexcept;
		static DxgiInfoManager& GetInfoManager(D3D12RHI& gfx);
	};

	class CloningBindable : public Bindable
	{
	public:
		virtual std::unique_ptr<CloningBindable> Clone() const noexcept = 0;
	};
}