#pragma once
#include "../_External/framework.h"

namespace Common
{
	class ILException : public std::exception
	{
	public:
		ILException(int line, const char* file) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept;
		int GetLine() const noexcept;
		const std::string& GetFile() const noexcept;
		std::string GetOriginString() const noexcept;
	private:
		int line;
		std::string file;
	protected:
		mutable std::string whatBuffer;
	};
}