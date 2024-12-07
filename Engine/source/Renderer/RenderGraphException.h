#pragma once
#include "../Common/ILException.h"

#define RGC_EXCEPTION( message ) RenderGraphException( (message),__LINE__,__FILE__ )

using namespace Common;

namespace Renderer
{
	class RenderGraphException : public ILException
	{
	public:
		RenderGraphException(std::string message, int line, const char* file) noexcept(!IS_DEBUG);
		const char* what() const noexcept(!IS_DEBUG) override;
		const char* GetType() const noexcept(!IS_DEBUG) override;
		const std::string& GetMessage() const noexcept(!IS_DEBUG);

	private:
		std::string message;
	};
}