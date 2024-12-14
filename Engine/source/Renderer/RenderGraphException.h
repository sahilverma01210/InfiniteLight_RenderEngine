#pragma once
#include "../Common/ILException.h"

#define RG_EXCEPTION( message ) RenderGraphException(__LINE__, __FILE__, (message))

using namespace Common;

namespace Renderer
{
	class RenderGraphException : public ILException
	{
	public:
		RenderGraphException(int line, const char* file, std::string message) noexcept(!IS_DEBUG);
		const char* what() const noexcept(!IS_DEBUG) override;
		const char* GetType() const noexcept(!IS_DEBUG) override;
		const std::string& GetMessage() const noexcept(!IS_DEBUG);

	private:
		std::string m_message;
	};
}