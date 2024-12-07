#include "ILException.h"

namespace Common
{
	ILException::ILException(int line, const char* file) noexcept(!IS_DEBUG)
		:
		line(line),
		file(file)
	{}

	const char* ILException::what() const noexcept(!IS_DEBUG)
	{
		std::ostringstream oss;
		oss << GetType() << std::endl
			<< GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* ILException::GetType() const noexcept(!IS_DEBUG)
	{
		return "IL Exception";
	}

	int ILException::GetLine() const noexcept(!IS_DEBUG)
	{
		return line;
	}

	const std::string& ILException::GetFile() const noexcept(!IS_DEBUG)
	{
		return file;
	}

	std::string ILException::GetOriginString() const noexcept(!IS_DEBUG)
	{
		std::ostringstream oss;
		oss << "[File] " << file << std::endl
			<< "[Line] " << line;
		return oss.str();
	}
}