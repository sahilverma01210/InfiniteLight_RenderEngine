/*
* Includes Common Windows SDK and Standard Libraries.
*/

#pragma once

// target Windows 7 or later
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>
// The following #defines disable a bunch of unused windows stuff. If you 
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define STRICT

#include <windows.h>
#include <wrl.h>
#include <tchar.h>
#include <shellapi.h>
#include <initguid.h> 

using namespace Microsoft::WRL;

// Standard C++ Libraries.

#include <string>
#include <optional>
#include <random>
#include <algorithm>
#include <iterator>
#include <memory>
#include <exception>
#include <sstream>
#include <aclapi.h>
#include <sddl.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <cmath> 
#include <numbers> 
#include <ranges>
#include <limits>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <bitset>
#include <array>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <filesystem>
#include <utility>
#include <stdexcept>
#include <cctype>
#include <cassert>
#include <typeinfo>
#include <fstream>

// Nlohmann JSON
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// STD & C++ Built-In Types
using Uint8 = std::uint8_t;
using Uint16 = std::uint16_t;
using Uint32 = std::uint32_t;
using Uint64 = std::uint64_t;
using Int8 = std::int8_t;
using Int16 = std::int16_t;
using Int32 = std::int32_t;
using Int64 = std::int64_t;
using Bool32 = std::uint32_t;
using Bool = bool;
using Char = char;
using Uchar = unsigned char;
using Short = short;
using Ushort = unsigned short;
using Int = int;
using Uint = unsigned int;
using Wchar = wchar_t;
using Float = float;
using Float64 = double;
using Uintptr = std::uintptr_t;
using Intptr = std::intptr_t;