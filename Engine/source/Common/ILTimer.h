#pragma once
#include "../_External/common.h"

namespace Common
{
	class ILTimer
	{
	public:
		ILTimer() noexcept;
		float Mark() noexcept;
		float Peek() const noexcept;

	private:
		std::chrono::steady_clock::time_point last;
	};
}