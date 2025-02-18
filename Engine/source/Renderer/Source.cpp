#include "Source.h"

namespace Renderer
{
	Source::Source(std::string nameIn)
		:
		m_name(std::move(nameIn))
	{
		if (m_name.empty())
		{
			throw RG_EXCEPTION("Empty output name");
		}
		const bool nameCharsValid = std::all_of(m_name.begin(), m_name.end(), [](char c) {
			return std::isalnum(c) || c == '_';
			});
		if (!nameCharsValid || std::isdigit(m_name.front()))
		{
			throw RG_EXCEPTION("Invalid output name: " + m_name);
		}
	}

	std::shared_ptr<Bindable> Source::YieldBindable()
	{
		throw RG_EXCEPTION("Output cannot be accessed as bindable");
	}

	std::shared_ptr<RenderGraphResource> Source::YieldBuffer()
	{
		throw RG_EXCEPTION("Output cannot be accessed as buffer");
	}

	std::vector<std::shared_ptr<RenderGraphResource>> Source::YieldBufferBucket()
	{
		throw RG_EXCEPTION("Output cannot be accessed as bindable");
	}

	const std::string& Source::GetName() const noexcept(!IS_DEBUG)
	{
		return m_name;
	}
}