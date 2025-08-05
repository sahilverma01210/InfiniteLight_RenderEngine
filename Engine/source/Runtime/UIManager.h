#pragma once
#include "../Common/ImGUI_Includes.h"
#include "../Renderer/Renderer.h"

#ifndef UI_ENABLED
#define UI_ENABLED
#endif

using namespace Renderer;

namespace Runtime
{
	class UIManager
	{
	public:
		UIManager(ILRenderer* renderer);
		~UIManager();
		void StartUIFrame();
		void UpdateUIFrame();
		void EndUIFrame();
		bool HandleWindowResize();
		void EnableUIMouse();
		void DisableUIMouse();
		bool HandleUIMessages(UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		D3D12RHI& m_gfx;
	};
}