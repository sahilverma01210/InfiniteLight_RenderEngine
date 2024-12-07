#pragma once
#include "../_External/common.h"

namespace Renderer
{
	class DxgiInfoManager
	{
	public:
		DxgiInfoManager();
		DxgiInfoManager(const DxgiInfoManager&) = delete;
		~DxgiInfoManager() = default;
		DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;
		void Set() noexcept(!IS_DEBUG);
		std::vector<std::string> GetMessages() const;

	private:
		unsigned long long next = 0u;
		ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
	};
}