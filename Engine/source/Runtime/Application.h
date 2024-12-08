#pragma once
#include "../Common/ILTimer.h"

#include "Window.h"

// Viewport dimensions.
#define WIDTH 1280.0f
#define HEIGHT 720.0f

// Window title.
#define TITLE L"Infinite Light Render Engine"

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
		Window window;
		ILTimer timer;
	};
}
