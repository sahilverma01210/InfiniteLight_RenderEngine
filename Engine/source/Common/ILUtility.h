#pragma once
#include "../_External/common.h"

namespace Common
{
	std::vector<std::string> TokenizeQuoted(const std::string& input);
	std::wstring ToWide(const std::string& narrow);
	std::string ToNarrow(const std::wstring& wide);

	template<class Iter>
	void SplitStringIter(const std::string& s, const std::string& delim, Iter out)
	{
		if (delim.empty())
		{
			*out++ = s;
		}
		else
		{
			size_t a = 0, b = s.find(delim);
			for (; b != std::string::npos;
				a = b + delim.length(), b = s.find(delim, a))
			{
				*out++ = std::move(s.substr(a, b - a));
			}
			*out++ = std::move(s.substr(a, s.length() - a));
		}
	}
	std::vector<std::string> SplitString(const std::string& s, const std::string& delim);
	bool StringContains(std::string_view haystack, std::string_view needle);

	inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
	{
		if (path == nullptr)
		{
			throw std::exception();
		}

		DWORD size = GetModuleFileName(nullptr, path, pathSize);
		if (size == 0 || size == pathSize)
		{
			// Method failed or path was truncated.
			throw std::exception();
		}

		WCHAR* lastSlash = wcsrchr(path, L'\\');
		if (lastSlash)
		{
			*(lastSlash + 1) = L'\0';
		}
	}
}