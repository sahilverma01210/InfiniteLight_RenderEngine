#include "ILTimer.h"

namespace Common
{
	ILTimer::ILTimer() noexcept(!IS_DEBUG)
	{
		last = steady_clock::now();
	}

	float ILTimer::Mark() noexcept(!IS_DEBUG)
	{
		const auto old = last;
		last = steady_clock::now();
		const duration<float> frameTime = last - old;
		return frameTime.count();
	}

	float ILTimer::Peek() const noexcept(!IS_DEBUG)
	{
		return duration<float>(steady_clock::now() - last).count();
	}
}