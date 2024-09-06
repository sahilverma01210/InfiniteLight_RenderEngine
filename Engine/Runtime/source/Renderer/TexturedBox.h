#pragma once
#include "ILObject.h"

namespace Renderer
{
	class TexturedBox : public ILObject<TexturedBox>
	{
	public:
		TexturedBox(D3D12RHI& gfx, std::mt19937& rng,
			std::uniform_real_distribution<float>& adist,
			std::uniform_real_distribution<float>& ddist,
			std::uniform_real_distribution<float>& odist,
			std::uniform_real_distribution<float>& rdist);
	private:
		struct PSMaterialConstant
		{
			alignas(16) XMFLOAT3 color;
			float specularIntensity = 0.6f;
			float specularPower = 30.0f;
			float padding[2];
		};

		PSMaterialConstant colorConst;
		
		// model transform
		XMFLOAT3X3 mt;
	};
}