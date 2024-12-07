#include "RenderGraphException.h"

namespace Renderer
{
	RenderGraphException::RenderGraphException(int line, const char* file, std::string message) noexcept(!IS_DEBUG)
		:
		ILException(line, file),
		message(std::move(message))
	{}
	const char* RenderGraphException::what() const noexcept(!IS_DEBUG)
	{
		std::ostringstream oss;
		oss << ILException::what() << std::endl;
		oss << "[message]" << std::endl;
		oss << message;
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}
	const char* RenderGraphException::GetType() const noexcept(!IS_DEBUG)
	{
		return "Render Graph Compile Exception";
	}
	const std::string& RenderGraphException::GetMessage() const noexcept(!IS_DEBUG)
	{
		return message;
	}
}