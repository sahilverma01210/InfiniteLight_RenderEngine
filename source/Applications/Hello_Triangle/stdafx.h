// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#define VULKAN_API true

#include <shellapi.h>

#if VULKAN_API // Vulkan App Includes
	#define VK_USE_PLATFORM_WIN32_KHR
	#include <vulkan/vulkan.h>

	#include <iostream>
	#include <fstream>
	#include <stdexcept>
	#include <algorithm>
	#include <vector>
	#include <cstring>
	#include <cstdlib>
	#include <cstdint>
	#include <limits>
	#include <optional>
	#include <set>
#else // DirectX App Includes
	#include <d3d12.h>
	#include <dxgi1_6.h>
	#include <D3Dcompiler.h>
	#include <DirectXMath.h>
	#include "d3dx12.h"

	#include <string>
	#include <wrl.h>
#endif