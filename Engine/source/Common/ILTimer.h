#pragma once
#include "../_External/common.h"

using namespace std::chrono;

namespace Common
{
	class ILTimer
	{
	public:
		ILTimer() noexcept(!IS_DEBUG);
		float Mark() noexcept(!IS_DEBUG);
		float Peek() const noexcept(!IS_DEBUG);

	private:
		std::chrono::steady_clock::time_point last;
	};
}