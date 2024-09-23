#include "ILException.h"

namespace Common
{
	ILException::ILException(int line, const char* file) noexcept
		:
		line(line),
		file(file)
	{}

	const char* ILException::what() const noexcept
	{
		std::ostringstream oss;
		oss << GetType() << std::endl
			<< GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* ILException::GetType() const noexcept
	{
		return "IL Exception";
	}

	int ILException::GetLine() const noexcept
	{
		return line;
	}

	const std::string& ILException::GetFile() const noexcept
	{
		return file;
	}

	std::string ILException::GetOriginString() const noexcept
	{
		std::ostringstream oss;
		oss << "[File] " << file << std::endl
			<< "[Line] " << line;
		return oss.str();
	}
}