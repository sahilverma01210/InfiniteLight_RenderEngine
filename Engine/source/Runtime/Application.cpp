#include "Application.h"

using namespace std::chrono;

namespace Runtime
{
	Timer::Timer() noexcept
	{
		last = steady_clock::now();
	}

	float Timer::Mark() noexcept
	{
		const auto old = last;
		last = steady_clock::now();
		const duration<float> frameTime = last - old;
		return frameTime.count();
	}

	float Timer::Peek() const noexcept
	{
		return duration<float>(steady_clock::now() - last).count();
	}

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