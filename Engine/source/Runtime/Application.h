#pragma once
#include "Window.h"

namespace Runtime
{
	class Application
	{
	public:
		Application(json appConfig);
		int Run();
		void Restart();
	private:
		void UpdateApp(bool& triggerRestart, bool& exit);
		void ExitApp();

	private:
		Window m_window;
	};
}
