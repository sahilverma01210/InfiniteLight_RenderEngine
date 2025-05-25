/*
* Includes DirectX 12 Library.
*/

#pragma once

#include "d3dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib") // DirectX Shader Compiler
#pragma comment(lib, "d3dcompiler.lib") // Legacy DirectX Compiler

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12shader.h>
#include <dxcapi.h>  // DirectX Shader Compiler
#include <dxgidebug.h>
#include <D3Dcompiler.h> // Legacy DirectX Compiler
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <SimpleMath/SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;