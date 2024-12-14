#pragma once
#include "../_External/dx12/directX12.h"
#include "../Common/ILException.h"

using namespace Common;

// HRESULT hr should exist in the local scope for these macros to work

#define D3D12RHI_EXCEPT_NOINFO(hResult) IL_EXCEPT( hResult )
#define D3D12RHI_THROW_NOINFO(hResultCall) if( FAILED( hResult = (hResultCall) ) ) throw IL_EXCEPT( hResult )

// macro for importing infomanager into local scope
// this.GetInfoManager(Graphics& gfx) must exist
#ifdef NDEBUG // Release Mode
#define INFOMAN(gfx) HRESULT hResult
#define INFOMAN_NOHR(gfx)
#else
#define INFOMAN(gfx) HRESULT hResult; Renderer::DxgiInfoManager& m_infoManager = GetInfoManager((gfx))
#define INFOMAN_NOHR(gfx) DxgiInfoManager& m_infoManager = GetInfoManager((gfx))
#endif

#ifdef NDEBUG // Release Mode
#define D3D12RHI_EXCEPT(hResult) D3D12RHI_EXCEPT_NOINFO(hResult)
#define D3D12RHI_THROW_INFO(hResultCall) D3D12RHI_THROW_NOINFO(hResultCall)
#define D3D12RHI_DEVICE_REMOVED_EXCEPT(hResult) Renderer::DeviceRemovedException( __LINE__,__FILE__,(hResult) )
#define D3D12RHI_THROW_INFO_ONLY(call) (call)
#else
#define D3D12RHI_EXCEPT(hResult) IL_MSG_EXCEPT(hResult, m_infoManager.GetMessages())
#define D3D12RHI_THROW_INFO(hResultCall) m_infoManager.Set(); if( FAILED( hResult = (hResultCall) ) ) throw D3D12RHI_EXCEPT(hResult)
#define D3D12RHI_DEVICE_REMOVED_EXCEPT(hResult) Renderer::DeviceRemovedException( __LINE__,__FILE__,(hResult),m_infoManager.GetMessages() )
#define D3D12RHI_THROW_INFO_ONLY(call) m_infoManager.Set(); (call); {auto v = m_infoManager.GetMessages(); if(!v.empty()) {throw Renderer::InfoException( __LINE__,__FILE__,v);}}
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
                << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
            oss << GetOriginString();
			m_whatBuffer = oss.str();
            return m_whatBuffer.c_str();
        }
        const char* GetType() const noexcept(!IS_DEBUG) override
        {
            return "Chili Graphics Info Exception";
        }
        std::string GetErrorInfo() const noexcept(!IS_DEBUG)
        {
            return m_info;
        }

    private:
        std::string m_info;
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
        std::string m_reason;
    };

	class DxgiInfoManager
	{
	public:
		DxgiInfoManager()
		{
			HRESULT hResult;

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

			D3D12RHI_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &m_pDxgiInfoQueue));
		}
		DxgiInfoManager(const DxgiInfoManager&) = delete;
		~DxgiInfoManager() = default;
		DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;
		void Set() noexcept(!IS_DEBUG)
		{
			// set the index (next) so that the next all to GetMessages()
			// will only get errors generated after this call
			m_next = m_pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		}
		std::vector<std::string> GetMessages() const
		{
			HRESULT hResult;

			std::vector<std::string> messages;
			const auto end = m_pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
			for (auto i = m_next; i < end; i++)
			{
				SIZE_T messageLength;
				// get the size of message i in bytes
				D3D12RHI_THROW_NOINFO(m_pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength));
				// allocate memory for message
				auto bytes = std::make_unique<byte[]>(messageLength);
				auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
				// get the message and push its description into the vector
				D3D12RHI_THROW_NOINFO(m_pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength));
				messages.emplace_back(pMessage->pDescription);
			}
			return messages;
		}

	private:
		unsigned long long m_next = 0u;
		ComPtr<IDXGIInfoQueue> m_pDxgiInfoQueue;
	};
}