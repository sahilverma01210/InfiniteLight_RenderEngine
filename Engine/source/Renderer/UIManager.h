#pragma once
#include "../_External/imgui/backends/imgui_impl_win32.h"
#include "../_External/imgui/backends/imgui_impl_dx12.h"

#include "D3D12RHI.h"

namespace Renderer
{
	class UIManager
	{
	public:
		UIManager(D3D12RHI& gfx);
		~UIManager();
		void StartUIFrame(D3D12RHI& gfx);
		void UpdateUIFrame(D3D12RHI& gfx);
		void EndUIFrame(D3D12RHI& gfx);
		bool HandleWindowResize(D3D12RHI& gfx);

	private:
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	};
}