#include "ILTimer.h"

namespace Common
{
	ILTimer::ILTimer() noexcept(!IS_DEBUG)
	{
		m_last = steady_clock::now();
	}

	float ILTimer::Mark() noexcept(!IS_DEBUG)
	{
		const auto old = m_last;
		m_last = steady_clock::now();
		const duration<float> frameTime = m_last - old;
		return frameTime.count();
	}

	float ILTimer::Peek() const noexcept(!IS_DEBUG)
	{
		return duration<float>(steady_clock::now() - m_last).count();
	}
}