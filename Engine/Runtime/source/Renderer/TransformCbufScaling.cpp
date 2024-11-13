//#include "TransformCbufScaling.h"
//#include "TechniqueProbe.h"
//
//namespace Renderer
//{
//	TransformCbufScaling::TransformCbufScaling(D3D12RHI& gfx, UINT rootParameterIndex, float scale)
//		:
//		TransformBuffer(gfx, rootParameterIndex),
//		buf(MakeLayout())
//	{
//		buf["scale"] = scale;
//	}
//
//	void TransformCbufScaling::Accept(TechniqueProbe& probe)
//	{
//		probe.VisitBuffer(buf);
//	}
//
//	void TransformCbufScaling::Bind(D3D12RHI& gfx) noexcept
//	{
//		const float scale = buf["scale"];
//		const auto scaleMatrix = dx::XMMatrixScaling(scale, scale, scale);
//		auto xf = GetTransforms(gfx);
//		xf.modelView = xf.modelView * scaleMatrix;
//		xf.modelViewProj = xf.modelViewProj * scaleMatrix;
//		UpdateBindImpl(gfx, xf);
//	}
//
//	std::unique_ptr<CloningBindable> TransformCbufScaling::Clone() const noexcept
//	{
//		return std::make_unique<TransformCbufScaling>(*this);
//	}
//
//	RawLayout TransformCbufScaling::MakeLayout()
//	{
//		RawLayout layout;
//		layout.Add<Float>("scale");
//		return layout;
//	}
//}