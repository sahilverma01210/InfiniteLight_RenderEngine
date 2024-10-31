//#pragma once
//#include "../_External/framework.h"
//
//#include "Drawable.h"
//#include "CommonBindables.h"
//#include "CommonShapes.h"
//#include "UIManager.h"
//
//namespace Renderer
//{
//	class TestCube : public Drawable
//	{
//	public:
//		TestCube(D3D12RHI& gfx, float size);
//		void SetPos(DirectX::XMFLOAT3 pos) noexcept;
//		void SetRotation(float roll, float pitch, float yaw) noexcept;
//		DirectX::XMMATRIX GetTransformXM() const noexcept override;
//		void SpawnControlWindow(D3D12RHI& gfx, const char* name) noexcept;
//	private:
//		//std::unique_ptr<Bindable> rootSignBindableOutline;
//		//std::unique_ptr<Bindable> psoBindableOutline;
//		DirectX::XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
//		float roll = 0.0f;
//		float pitch = 0.0f;
//		float yaw = 0.0f;
//		PipelineDescription pipelineDesc{};
//	};
//}