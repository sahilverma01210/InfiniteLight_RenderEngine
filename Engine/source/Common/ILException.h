#pragma once
#include "../_External/common.h"

namespace Common
{
	class ILException : public std::exception
	{
	public:
		ILException(int line, const char* file) noexcept(!IS_DEBUG);
		const char* what() const noexcept(!IS_DEBUG) override;
		virtual const char* GetType() const noexcept(!IS_DEBUG);
		int GetLine() const noexcept(!IS_DEBUG);
		const std::string& GetFile() const noexcept(!IS_DEBUG);
		std::string GetOriginString() const noexcept(!IS_DEBUG);

	private:
		int line;
		std::string file;
	protected:
		mutable std::string whatBuffer;
	};
}