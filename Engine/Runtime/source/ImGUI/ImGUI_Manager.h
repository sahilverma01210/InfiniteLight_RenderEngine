#pragma once
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "../Renderer/D3D12RHI.h"

namespace Renderer
{
	class ImGUI_Manager
	{
	public:
		void InitImGUI(D3D12RHI& gfx);
		void StartImGUIFrame(D3D12RHI& gfx);
		void EndImGUIFrame(D3D12RHI& gfx);
		void DestroyImGUI(D3D12RHI& gfx);
	};
}