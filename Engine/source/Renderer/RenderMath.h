#pragma once
#include "../_External/dx12/directX12.h"
#include "../_External/common.h"

namespace Renderer
{
	XMFLOAT3 ExtractEulerAngles(const XMFLOAT4X4& matrix);
	XMFLOAT3 ExtractTranslation(const XMFLOAT4X4& matrix);
	XMMATRIX ScaleTranslation(XMMATRIX matrix, float scale);
}