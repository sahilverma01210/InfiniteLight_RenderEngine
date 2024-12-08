#include "Sink.h"

namespace Renderer
{
	Sink::Sink(std::string registeredNameIn)
		:
		registeredName(std::move(registeredNameIn))
	{
		if (registeredName.empty())
		{
			throw RG_EXCEPTION("Empty output name");
		}
		const bool nameCharsValid = std::all_of(registeredName.begin(), registeredName.end(), [](char c) {
			return std::isalnum(c) || c == '_';
			});
		if (!nameCharsValid || std::isdigit(registeredName.front()))
		{
			throw RG_EXCEPTION("Invalid output name: " + registeredName);
		}
	}

	const std::string& Sink::GetRegisteredName() const noexcept(!IS_DEBUG)
	{
		return registeredName;
	}

	const std::string& Sink::GetPassName() const noexcept(!IS_DEBUG)
	{
		return passName;
	}

	const std::string& Sink::GetOutputName() const noexcept(!IS_DEBUG)
	{
		return outputName;
	}

	void Sink::SetTarget(std::string passName, std::string outputName)
	{
		{
			if (passName.empty())
			{
				throw RG_EXCEPTION("Empty output name");
			}
			const bool nameCharsValid = std::all_of(passName.begin(), passName.end(), [](char c) {
				return std::isalnum(c) || c == '_';
				});
			if (passName != "$" && (!nameCharsValid || std::isdigit(passName.front())))
			{
				throw RG_EXCEPTION("Invalid output name: " + registeredName);
			}
			this->passName = passName;
		}
		{
			if (outputName.empty())
			{
				throw RG_EXCEPTION("Empty output name");
			}
			const bool nameCharsValid = std::all_of(outputName.begin(), outputName.end(), [](char c) {
				return std::isalnum(c) || c == '_';
				});
			if (!nameCharsValid || std::isdigit(outputName.front()))
			{
				throw RG_EXCEPTION("Invalid output name: " + registeredName);
			}
			this->outputName = outputName;
		}
	}
}