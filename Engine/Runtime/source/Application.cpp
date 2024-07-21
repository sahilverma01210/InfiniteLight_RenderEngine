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

	Application::Application() : window(WIDTH, HEIGHT, TITLE)
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
		// Core Module Test Function.
		Test();

		//// Rotating Cube.
		//const float c = sin(timer.Peek()) / 2.0f + 0.5f;
		//window.UpdateWindow(timer.Peek(),
		//	0.0f,
		//	0.0f);

		// Rotating Cube following Mouse Left Click.
		window.UpdateWindow(timer.Peek(),
			window.mouse.GetPosX() / (WIDTH / 2) - 1.0f,
			-window.mouse.GetPosY() / (HEIGHT / 2) + 1.0f);
	}
}