#include "Source.h"

namespace Renderer
{
	Source::Source(std::string nameIn)
		:
		name(std::move(nameIn))
	{
		if (name.empty())
		{
			throw RG_EXCEPTION("Empty output name");
		}
		const bool nameCharsValid = std::all_of(name.begin(), name.end(), [](char c) {
			return std::isalnum(c) || c == '_';
			});
		if (!nameCharsValid || std::isdigit(name.front()))
		{
			throw RG_EXCEPTION("Invalid output name: " + name);
		}
	}

	std::shared_ptr<Bindable> Source::YieldBindable()
	{
		throw RG_EXCEPTION("Output cannot be accessed as bindable");
	}

	std::shared_ptr<BufferResource> Source::YieldBuffer()
	{
		throw RG_EXCEPTION("Output cannot be accessed as buffer");
	}

	std::vector<std::shared_ptr<BufferResource>> Source::YieldBufferBucket()
	{
		throw RG_EXCEPTION("Output cannot be accessed as bindable");
	}

	const std::string& Source::GetName() const noexcept(!IS_DEBUG)
	{
		return name;
	}
}