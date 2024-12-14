#include "ILException.h"

namespace Common
{
	ILException::ILException(int line, const char* file) noexcept(!IS_DEBUG)
		:
		m_line(line),
		m_file(file)
	{}

	const char* ILException::what() const noexcept(!IS_DEBUG)
	{
		std::ostringstream oss;
		oss << GetType() << std::endl
			<< GetOriginString();
		m_whatBuffer = oss.str();
		return m_whatBuffer.c_str();
	}

	const char* ILException::GetType() const noexcept(!IS_DEBUG)
	{
		return "IL Exception";
	}

	int ILException::GetLine() const noexcept(!IS_DEBUG)
	{
		return m_line;
	}

	const std::string& ILException::GetFile() const noexcept(!IS_DEBUG)
	{
		return m_file;
	}

	std::string ILException::GetOriginString() const noexcept(!IS_DEBUG)
	{
		std::ostringstream oss;
		oss << "[File] " << m_file << std::endl
			<< "[Line] " << m_line;
		return oss.str();
	}
	std::string ILException::TranslateErrorCode(HRESULT hr) noexcept(!IS_DEBUG)
	{
		char* pMsgBuf = nullptr;
		// windows will allocate memory for err string and make our pointer point to it
		const DWORD nMsgLen = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
		);
		// 0 string length returned indicates a failure
		if (nMsgLen == 0)
		{
			return "Unidentified error code";
		}
		// copy error string from windows-allocated buffer to std::string
		std::string errorString = pMsgBuf;
		// free windows buffer
		LocalFree(pMsgBuf);
		return errorString;
	}
}