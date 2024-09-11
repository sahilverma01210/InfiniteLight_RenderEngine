#pragma once

#include "../Common/ILMath.h"
#include "../ImGUI/ImGUI_Manager.h"

#include "D3D12RHI.h"
#include "Camera.h"
#include "PointLight.h"
#include "AssImpModel.h"

namespace Renderer
{
	void init(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);

	void update(float angle);

	void destroy();
}