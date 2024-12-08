#include "Application.h"

namespace Runtime
{
	Application::Application() : window((LONG)WIDTH, (LONG)HEIGHT, TITLE)
	{}

	int Application::Run()
	{
		while (true)
		{
			// process all messages pending, but to not block for new messages
			if (const auto ecode = Window::ProcessMessages())
			{
				// if return optional has value, means we're quitting so return exit code
				return *ecode;
			}
			UpdateFrame();
		}
	}

	void Application::UpdateFrame()
	{
		// Rotating Cube following Mouse Left Click.
		window.UpdateWindow(timer.Mark());
	}
}