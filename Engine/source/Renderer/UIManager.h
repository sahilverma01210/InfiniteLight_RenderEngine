#pragma once
#include "../_External/imgui/backends/imgui_impl_win32.h"
#include "../_External/imgui/backends/imgui_impl_dx12.h"

#include "D3D12RHI.h"

namespace Renderer
{
	class UIManager
	{
	public:
		void InitUI(D3D12RHI& gfx);
		void StartUIFrame(D3D12RHI& gfx);
		void UpdateUIFrame(D3D12RHI& gfx);
		void EndUIFrame(D3D12RHI& gfx);
		void DestroyUI(D3D12RHI& gfx);
		bool HandleWindowResize(D3D12RHI& gfx);
	};
}