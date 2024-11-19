#include "ILTimer.h"

using namespace std::chrono;

namespace Common
{
	ILTimer::ILTimer() noexcept
	{
		last = steady_clock::now();
	}

	float ILTimer::Mark() noexcept
	{
		const auto old = last;
		last = steady_clock::now();
		const duration<float> frameTime = last - old;
		return frameTime.count();
	}

	float ILTimer::Peek() const noexcept
	{
		return duration<float>(steady_clock::now() - last).count();
	}
}