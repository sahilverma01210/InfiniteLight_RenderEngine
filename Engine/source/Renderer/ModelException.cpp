#include "ModelException.h"

namespace Renderer
{
	ModelException::ModelException(int line, const char* file, std::string note) noexcept
		:
		ILException(line, file),
		note(std::move(note))
	{}

	const char* ModelException::what() const noexcept
	{
		std::ostringstream oss;
		oss << ILException::what() << std::endl
			<< "[Note] " << GetNote();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* ModelException::GetType() const noexcept
	{
		return "IL Model Exception";
	}

	const std::string& ModelException::GetNote() const noexcept
	{
		return note;
	}
}