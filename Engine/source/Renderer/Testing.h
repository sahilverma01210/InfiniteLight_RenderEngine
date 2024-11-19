#pragma once
#include "D3D12RHI.h"

void TestDynamicConstant();

namespace Renderer
{
	void TestDynamicMeshLoading();

	void TestMaterialSystemLoading(D3D12RHI& gfx);

	void TestScaleMatrixTranslation();
}