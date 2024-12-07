#pragma once
#include "../Common/ILException.h"

#include "DxgiInfoManager.h"

using namespace Common;

// HRESULT hr should exist in the local scope for these macros to work

#define D3D12RHI_EXCEPT_NOINFO(hr) IL_EXCEPT( hr )
#define D3D12RHI_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw IL_EXCEPT( hr )

// macro for importing infomanager into local scope
// this.GetInfoManager(Graphics& gfx) must exist
#ifdef NDEBUG // Release Mode
#define INFOMAN(gfx) HRESULT hr
#define INFOMAN_NOHR(gfx)
#else
#define INFOMAN(gfx) HRESULT hr; Renderer::DxgiInfoManager& infoManager = GetInfoManager((gfx))
#define INFOMAN_NOHR(gfx) DxgiInfoManager& infoManager = GetInfoManager((gfx))
#endif

#ifdef NDEBUG // Release Mode
#define D3D12RHI_EXCEPT(hr) D3D12RHI_EXCEPT_NOINFO(hr)
#define D3D12RHI_THROW_INFO(hrcall) D3D12RHI_THROW_NOINFO(hrcall)
#define D3D12RHI_DEVICE_REMOVED_EXCEPT(hr) Renderer::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define D3D12RHI_THROW_INFO_ONLY(call) (call)
#else
#define D3D12RHI_EXCEPT(hr) IL_MSG_EXCEPT(hr, infoManager.GetMessages())
#define D3D12RHI_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw D3D12RHI_EXCEPT(hr)
#define D3D12RHI_DEVICE_REMOVED_EXCEPT(hr) Renderer::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define D3D12RHI_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw Renderer::InfoException( __LINE__,__FILE__,v);}}
#endif

namespace Renderer
{
    class InfoException : public ILException
    {
    public:
        InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept(!IS_DEBUG)
            :
            ILException(line, file)
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