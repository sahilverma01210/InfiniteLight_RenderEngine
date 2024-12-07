#include "ModelException.h"

namespace Renderer
{
	ModelException::ModelException(int line, const char* file, std::string note) noexcept(!IS_DEBUG)
		:
		ILException(line, file),
		note(std::move(note))
	{}

	const char* ModelException::what() const noexcept(!IS_DEBUG)
	{
		std::ostringstream oss;
		oss << ILException::what() << std::endl
			<< "[Note] " << GetNote();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* ModelException::GetType() const noexcept(!IS_DEBUG)
	{
		return "IL Model Exception";
	}

	const std::string& ModelException::GetNote() const noexcept(!IS_DEBUG)
	{
		return note;
	}
}