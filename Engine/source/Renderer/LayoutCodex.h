#pragma once
#include "DynamicConstant.h"

namespace Renderer
{
	class LayoutCodex
	{
	public:
		static CookedLayout Resolve(RawLayout&& layout);
	private:
		static LayoutCodex& Get_() noexcept;

	private:
		std::unordered_map<std::string, std::shared_ptr<LayoutElement>> map;
	};
}