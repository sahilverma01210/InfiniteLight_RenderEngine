#include "Sink.h"

namespace Renderer
{
	Sink::Sink(std::string registeredNameIn)
		:
		m_registeredName(std::move(registeredNameIn))
	{
		if (m_registeredName.empty())
		{
			throw RG_EXCEPTION("Empty output name");
		}
		const bool nameCharsValid = std::all_of(m_registeredName.begin(), m_registeredName.end(), [](char c) {
			return std::isalnum(c) || c == '_';
			});
		if (!nameCharsValid || std::isdigit(m_registeredName.front()))
		{
			throw RG_EXCEPTION("Invalid output name: " + m_registeredName);
		}
	}

	const std::string& Sink::GetRegisteredName() const noexcept(!IS_DEBUG)
	{
		return m_registeredName;
	}

	const std::string& Sink::GetPassName() const noexcept(!IS_DEBUG)
	{
		return m_passName;
	}

	const std::string& Sink::GetOutputName() const noexcept(!IS_DEBUG)
	{
		return m_outputName;
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
				throw RG_EXCEPTION("Invalid output name: " + m_registeredName);
			}
			this->m_passName = passName;
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
				throw RG_EXCEPTION("Invalid output name: " + m_registeredName);
			}
			this->m_outputName = outputName;
		}
	}
}