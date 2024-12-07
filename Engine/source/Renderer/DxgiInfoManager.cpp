#include "../Runtime/Window.h"
#include "../Runtime/WindowsException.h"

#include "DxgiInfoManager.h"
#include "D3D12RHI.h"
#include "D3D12RHIException.h"

namespace Renderer
{
	DxgiInfoManager::DxgiInfoManager()
	{
		// define function signature of DXGIGetDebugInterface
		typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

		// load the dll that contains the function DXGIGetDebugInterface
		const auto hModDxgiDebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (hModDxgiDebug == nullptr)
		{
			throw ILWND_LAST_EXCEPT();
		}

		// get address of DXGIGetDebugInterface in dll
		const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
			reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface"))
			);
		if (DxgiGetDebugInterface == nullptr)
		{
			throw ILWND_LAST_EXCEPT();
		}

		HRESULT hr;
		D3D12RHI_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pDxgiInfoQueue));
	}

	void DxgiInfoManager::Set() noexcept(!IS_DEBUG)
	{
		// set the index (next) so that the next all to GetMessages()
		// will only get errors generated after this call
		next = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
	}

	std::vector<std::string> DxgiInfoManager::GetMessages() const
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
}