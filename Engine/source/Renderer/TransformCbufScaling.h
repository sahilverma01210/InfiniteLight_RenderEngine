//#pragma once
//#include "TransformBuffer.h"
//#include "DynamicConstant.h"
//
//namespace Renderer
//{
//	class TransformCbufScaling : public TransformBuffer
//	{
//	public:
//		TransformCbufScaling(D3D12RHI& gfx, UINT rootParameterIndex, float scale);
//		void Accept(TechniqueProbe& probe) override;
//		void Bind(D3D12RHI& gfx) noexcept override;
//		std::unique_ptr<CloningBindable> Clone() const noexcept override;
//	private:
//		static RawLayout MakeLayout();
//	private:
//		Buffer buf;
//	};
//}