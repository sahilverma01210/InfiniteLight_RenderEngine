#pragma once
#include "Window.h"

namespace Runtime
{
	class Application
	{
	public:
		Application();
		int Run();
	private:
		void UpdateFrame();

	private:
		Window m_window;
	};
}
