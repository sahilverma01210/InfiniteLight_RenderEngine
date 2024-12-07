#pragma once

#define ILWND_EXCEPT( hr ) Runtime::HrException( __LINE__,__FILE__,hr )
#define ILWND_LAST_EXCEPT() Runtime::HrException( __LINE__,__FILE__,GetLastError() )
#define ILWND_NOGFX_EXCEPT() Runtime::NoGfxException( __LINE__,__FILE__ )

namespace Runtime
{
	class Exception : public ILException
	{
		using ILException::ILException;
	public:
		static std::string TranslateErrorCode(HRESULT hr) noexcept(!IS_DEBUG)
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
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept(!IS_DEBUG)
			:
			Exception(line, file),
			hr(hr)
		{
		}
		const char* what() const noexcept(!IS_DEBUG) override
		{
			std::ostringstream oss;
			oss << GetType() << std::endl
				<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
				<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
				<< "[Description] " << GetErrorDescription() << std::endl
				<< GetOriginString();
			whatBuffer = oss.str();
			return whatBuffer.c_str();
		}
		const char* GetType() const noexcept(!IS_DEBUG) override
		{
			return "Infinite Light Window Exception";
		}
		HRESULT GetErrorCode() const noexcept(!IS_DEBUG)
		{
			return hr;
		}
		std::string GetErrorDescription() const noexcept(!IS_DEBUG)
		{
			return Exception::TranslateErrorCode(hr);
		}

	private:
		HRESULT hr;
	};
	class NoGfxException : public Exception
	{
		using Exception::Exception;
	public:
		const char* GetType() const noexcept(!IS_DEBUG) override
		{
			return "Infinite Light Window Exception [No Graphics]";
		}
	};
}