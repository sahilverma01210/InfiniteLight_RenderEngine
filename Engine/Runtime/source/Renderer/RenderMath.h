#pragma once
#include "../_External/framework.h"

namespace Renderer
{
	XMFLOAT3 ExtractEulerAngles(const XMFLOAT4X4& matrix);
	XMFLOAT3 ExtractTranslation(const XMFLOAT4X4& matrix);
}