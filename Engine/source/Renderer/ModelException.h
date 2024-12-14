#pragma once
#include "../Common/ILException.h"

#define MDL_EXCEPTION( message ) ModelException(__LINE__, __FILE__, (message))

using namespace Common;

namespace Renderer
{
	class ModelException : public ILException
	{
	public:
		ModelException(int line, const char* file, std::string message) noexcept(!IS_DEBUG);
		const char* what() const noexcept(!IS_DEBUG) override;
		const char* GetType() const noexcept(!IS_DEBUG) override;
		const std::string& GetNote() const noexcept(!IS_DEBUG);

	private:
		std::string m_message;
	};
}