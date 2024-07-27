#pragma once

#include <chrono>

#include "Common/Windows/Window.h"

namespace Runtime
{
	// Viewport dimensions.
	#define WIDTH 1280.0f
	#define HEIGHT 800.0f

	// Window title.
	#define TITLE L"Infinite Light Render Engine"

	class Timer
	{
	public:
		Timer() noexcept;
		float Mark() noexcept;
		float Peek() const noexcept;
	private:
		std::chrono::steady_clock::time_point last;
	};

	class Application
	{
	public:
		Application();
		int Run();
	private:
		Window window;
		Timer timer;

		void UpdateFrame();
	};
}
