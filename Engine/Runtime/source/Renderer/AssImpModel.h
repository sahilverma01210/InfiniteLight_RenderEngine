#pragma once
#include "Object.h"

namespace Renderer
{
	class AssImpModel : public Object<AssImpModel>
	{
	public:
		AssImpModel(D3D12RHI& gfx);
		void Update(float dt) noexcept override;
		XMMATRIX GetTransformXM() const noexcept override;
	private:
		XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
		struct PSMaterialConstant
		{
			alignas(16) XMFLOAT3 color;
			float specularIntensity = 0.6f;
			float specularPower = 30.0f;
			float padding[2];
		};

		PSMaterialConstant colorConst;
	};
}