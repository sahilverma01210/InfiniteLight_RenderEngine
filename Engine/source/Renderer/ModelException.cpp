#include "ModelException.h"

namespace Renderer
{
	ModelException::ModelException(int line, const char* file, std::string message) noexcept(!IS_DEBUG)
		:
		ILException(line, file),
		m_message(std::move(message))
	{}

	const char* ModelException::what() const noexcept(!IS_DEBUG)
	{
		std::ostringstream oss;
		oss << ILException::what() << std::endl
			<< "[Note] " << GetNote();
		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}

	const char* ModelException::GetType() const noexcept(!IS_DEBUG)
	{
		return "IL Model Exception";
	}

	const std::string& ModelException::GetNote() const noexcept(!IS_DEBUG)
	{
		return m_message;
	}
}