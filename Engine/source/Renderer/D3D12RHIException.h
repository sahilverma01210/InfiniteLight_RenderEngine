#pragma once
#include "../Common/ILException.h"

#include "DxgiInfoManager.h"

// HRESULT hr should exist in the local scope for these macros to work

#define D3D12RHI_EXCEPT_NOINFO(hr) Renderer::HrException( __LINE__,__FILE__,(hr) )
#define D3D12RHI_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Renderer::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define D3D12RHI_EXCEPT(hr) Renderer::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define D3D12RHI_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw D3D12RHI_EXCEPT(hr)
#define D3D12RHI_DEVICE_REMOVED_EXCEPT(hr) Renderer::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define D3D12RHI_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw Renderer::InfoException( __LINE__,__FILE__,v);}}
#else
#define D3D12RHI_EXCEPT(hr) Renderer::HrException( __LINE__,__FILE__,(hr) )
#define D3D12RHI_THROW_INFO(hrcall) D3D12RHI_THROW_NOINFO(hrcall)
#define D3D12RHI_DEVICE_REMOVED_EXCEPT(hr) Renderer::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define D3D12RHI_THROW_INFO_ONLY(call) (call)
#endif

// macro for importing infomanager into local scope
// this.GetInfoManager(Graphics& gfx) must exist
#ifdef NDEBUG
#define INFOMAN(gfx) HRESULT hr
#else
#define INFOMAN(gfx) HRESULT hr; Renderer::DxgiInfoManager& infoManager = GetInfoManager((gfx))
#endif

#ifdef NDEBUG
#define INFOMAN_NOHR(gfx)
#else
#define INFOMAN_NOHR(gfx) DxgiInfoManager& infoManager = GetInfoManager((gfx))
#endif

using namespace Common;

namespace Renderer
{
    class Exception : public ILException
    {
        using ILException::ILException;
    public:
        static std::string TranslateErrorCode(HRESULT hr) noexcept(!IS_DEBUG)
        {
            char* pMsgBuf = nullptr;

            // windows will allocate memory for err string and make our pointer point to it
            DWORD nMsgLen = FormatMessageA(
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
        HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept(!IS_DEBUG)
            :
            Exception(line, file),
            hr(hr)
        {
            // join all info messages with newlines into single string
            for (const auto& m : infoMsgs)
            {
                info += m;
                info.push_back('\n');
            }
            // remove final newline if exists
            if (!info.empty())
            {
                info.pop_back();
            }
        }
        const char* what() const noexcept(!IS_DEBUG) override
        {
            std::ostringstream oss;
            oss << GetType() << std::endl
                << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
                << std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
                << "[Description] " << GetErrorDescription() << std::endl;
            if (!info.empty())
            {
                oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
            }
            oss << GetOriginString();
            whatBuffer = oss.str();
            return whatBuffer.c_str();
        }
        const char* GetType() const noexcept(!IS_DEBUG) override
        {
            return "IL D3D12 Exception";
        }
        HRESULT GetErrorCode() const noexcept(!IS_DEBUG)
        {
            return hr;
        }
        std::string GetErrorDescription() const noexcept(!IS_DEBUG)
        {
            return TranslateErrorCode(hr);
        }
        std::string GetErrorInfo() const noexcept(!IS_DEBUG)
        {
            return info;
        }

    private:
        HRESULT hr;
        std::string info;
    };
    class InfoException : public Exception
    {
    public:
        InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept(!IS_DEBUG)
            :
            Exception(line, file)
        {
            // join all info messages with newlines into single string
            for (const auto& m : infoMsgs)
            {
                info += m;
                info.push_back('\n');
            }
            // remove final newline if exists
            if (!info.empty())
            {
                info.pop_back();
            }
        }
        const char* what() const noexcept(!IS_DEBUG) override
        {
            std::ostringstream oss;
            oss << GetType() << std::endl
                << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
            oss << GetOriginString();
            whatBuffer = oss.str();
            return whatBuffer.c_str();
        }
        const char* GetType() const noexcept(!IS_DEBUG) override
        {
            return "Chili Graphics Info Exception";
        }
        std::string GetErrorInfo() const noexcept(!IS_DEBUG)
        {
            return info;
        }

    private:
        std::string info;
    };
    class DeviceRemovedException : public HrException
    {
        using HrException::HrException;
    public:
        const char* GetType() const noexcept(!IS_DEBUG) override
        {
            return "Infinite Light D3D12 Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
        }

    private:
        std::string reason;
    };
}