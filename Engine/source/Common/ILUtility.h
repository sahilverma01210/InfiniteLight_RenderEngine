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

	enum AssetType
	{
		Shader
	};

	inline std::wstring GetAssetsPath(AssetType assetType)
	{
		WCHAR path[512];

		DWORD size = GetModuleFileName(nullptr, path, _countof(path));
		if (size == 0 || size == _countof(path))
		{
			// Method failed or path was truncated.
			throw std::exception();
		}

		WCHAR* lastSlash = wcsrchr(path, L'\\');
		if (lastSlash)
		{
			*(lastSlash + 1) = L'\0';
		}

		switch (assetType)
		{
		case AssetType::Shader:
			return std::wstring(path) + L"shaders\\HLSL\\";
			break;
		default:
			return std::wstring(path);
			break;
		}
	}
	inline std::wstring GetAssetFullPath(LPCWSTR assetName, AssetType assetType)
	{
		// Root assets path.
		std::wstring wideAssetsPath = GetAssetsPath(assetType);
		return wideAssetsPath + assetName;
	}

	static bool ParseCommandLineArgs(WCHAR* argv[], int argc) noexcept(!IS_DEBUG);
}