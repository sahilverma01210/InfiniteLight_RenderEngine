#include "LayoutCodex.h"

namespace Renderer
{
	CookedLayout LayoutCodex::Resolve(RawLayout&& layout)
	{
		auto sig = layout.GetSignature();
		auto& map = Get_().map;
		const auto i = map.find(sig);
		// idential layout already exists
		if (i != map.end())
		{
			// input layout is expected to be cleared after Resolve
			// so just throw away the layout tree
			layout.ClearRoot();
			return { i->second };
		}
		// otherwise add layout root element to map
		auto result = map.insert({ std::move(sig),layout.DeliverRoot() });
		// return layout with additional reference to root
		return { result.first->second };
	}

	LayoutCodex& LayoutCodex::Get_() noexcept(!IS_DEBUG)
	{
		static LayoutCodex codex;
		return codex;
	}
}