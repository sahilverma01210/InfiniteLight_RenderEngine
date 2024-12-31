#pragma once
#include "../Common/ImGUI_Includes.h"
#include "../Renderer/Renderer.h"

using namespace Renderer;

namespace Runtime
{
	class UIManager
	{
	public:
		UIManager(HWND hWnd, ILRenderer* renderer);
		~UIManager();
		void StartUIFrame();
		void UpdateUIFrame();
		void EndUIFrame();
		bool HandleWindowResize();
		void EnableUIMouse();
		void DisableUIMouse();
		bool HandleUIMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_hWnd;
		D3D12RHI& m_gfx;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	};
}