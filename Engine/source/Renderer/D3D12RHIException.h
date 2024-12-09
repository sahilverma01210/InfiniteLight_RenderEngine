#pragma once
#include "../_External/dx12/directX12.h"
#include "../Common/ILException.h"

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
	class DxgiInfoManager
	{
	public:
		DxgiInfoManager()
		{
			// define function signature of DXGIGetDebugInterface
			typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

			// load the dll that contains the function DXGIGetDebugInterface
			const auto hModDxgiDebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
			if (hModDxgiDebug == nullptr)
			{
				throw IL_LAST_EXCEPT();
			}

			// get address of DXGIGetDebugInterface in dll
			const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
				reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface"))
				);
			if (DxgiGetDebugInterface == nullptr)
			{
				throw IL_LAST_EXCEPT();
			}

			HRESULT hr;
			D3D12RHI_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pDxgiInfoQueue));
		}
		DxgiInfoManager(const DxgiInfoManager&) = delete;
		~DxgiInfoManager() = default;
		DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;
		void Set() noexcept(!IS_DEBUG)
		{
			// set the index (next) so that the next all to GetMessages()
			// will only get errors generated after this call
			next = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		}
		std::vector<std::string> GetMessages() const
		{
			std::vector<std::string> messages;
			const auto end = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
			for (auto i = next; i < end; i++)
			{
				HRESULT hr;
				SIZE_T messageLength;
				// get the size of message i in bytes
				D3D12RHI_THROW_NOINFO(pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength));
				// allocate memory for message
				auto bytes = std::make_unique<byte[]>(messageLength);
				auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
				// get the message and push its description into the vector
				D3D12RHI_THROW_NOINFO(pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength));
				messages.emplace_back(pMessage->pDescription);
			}
			return messages;
		}

	private:
		unsigned long long next = 0u;
		ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
	};
}