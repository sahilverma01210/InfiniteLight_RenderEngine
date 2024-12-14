#include "RenderGraphException.h"

namespace Renderer
{
	RenderGraphException::RenderGraphException(int line, const char* file, std::string message) noexcept(!IS_DEBUG)
		:
		ILException(line, file),
		m_message(std::move(message))
	{}
	const char* RenderGraphException::what() const noexcept(!IS_DEBUG)
	{
		std::ostringstream oss;
		oss << ILException::what() << std::endl;
		oss << "[message]" << std::endl;
		oss << m_message;
		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}
	const char* RenderGraphException::GetType() const noexcept(!IS_DEBUG)
	{
		return "Render Graph Compile Exception";
	}
	const std::string& RenderGraphException::GetMessage() const noexcept(!IS_DEBUG)
	{
		return m_message;
	}
}