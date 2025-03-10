#pragma once
#include "../_External/common.h"

#define IL_EXCEPT( hr ) Common::HrException( __LINE__,__FILE__,hr )
#define IL_MSG_EXCEPT(hr, msgs) Common::HrException( __LINE__,__FILE__,(hr),msgs )
#define IL_LAST_EXCEPT() Common::HrException( __LINE__,__FILE__,GetLastError() )

namespace Common
{
    class ILException : public std::exception
    {
    public:
        ILException(int line, const char* file) noexcept(!IS_DEBUG);
        const char* what() const noexcept(!IS_DEBUG) override;
        virtual const char* GetType() const noexcept(!IS_DEBUG);
        int GetLine() const noexcept(!IS_DEBUG);
        const std::string& GetFile() const noexcept(!IS_DEBUG);
        std::string GetOriginString() const noexcept(!IS_DEBUG);
        static std::string TranslateErrorCode(HRESULT hr) noexcept(!IS_DEBUG);

    private:
        int m_line;
        std::string m_file;
    protected:
        mutable std::string m_whatBuffer;
    };

    class HrException : public ILException
    {
    public:
        HrException(int line, const char* file, HRESULT hr, std::vector<std::string> msgs = {}) noexcept(!IS_DEBUG)
            :
            ILException(line, file),
            m_hResult(hr),
            m_msgs(msgs)
        {
            // join all info messages with newlines into single string
            for (const auto& m : msgs)
            {
                m_info += m;
                m_info.push_back('\n');
            }
            // remove final newline if exists
            if (!m_info.empty())
            {
                m_info.pop_back();
            }
        }
        const char* what() const noexcept(!IS_DEBUG) override
        {
            std::ostringstream oss;
            oss << GetType() << std::endl
                << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
                << std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
                << "[Description] " << GetErrorDescription() << std::endl;
            if (!m_info.empty())
            {
                oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
            }
            oss << GetOriginString();
            m_whatBuffer = oss.str();
            return m_whatBuffer.c_str();
        }
        const char* GetType() const noexcept(!IS_DEBUG) override
        {
            return "IL D3D12 Exception";
        }
        HRESULT GetErrorCode() const noexcept(!IS_DEBUG)
        {
            return m_hResult;
        }
        std::string GetErrorDescription() const noexcept(!IS_DEBUG)
        {
            return TranslateErrorCode(m_hResult);
        }
        std::string GetErrorInfo() const noexcept(!IS_DEBUG)
        {
            return m_info;
        }

    private:
        HRESULT m_hResult;
        std::vector<std::string> m_msgs;
        std::string m_info;
    };
}