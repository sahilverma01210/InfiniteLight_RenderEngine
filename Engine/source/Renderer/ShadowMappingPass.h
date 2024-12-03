#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Source.h"
#include "RenderTarget.h"
#include "Camera.h"
#include "TextureBuffer.h"
#include "Viewport.h"
#include "../Common/ILMath.h"

namespace Renderer
{
	class D3D12RHI;

	class ShadowMappingPass : public RenderQueuePass
	{
	public:
		void BindShadowCamera(const Camera& cam) noexcept
		{
			pShadowCamera = &cam;
		}
		ShadowMappingPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))
		{
			pDepthCube = std::make_shared<DepthCubeMapTextureBuffer>(gfx, size);
			gfx.SetDepthBuffer(pDepthCube->GetBuffer());
			RegisterSource(DirectBindableSource<DepthCubeMapTextureBuffer>::Make("map", pDepthCube));

			DirectX::XMStoreFloat4x4(
				&projection,
				DirectX::XMMatrixPerspectiveFovLH(PI / 2.0f, 1.0f, 0.5f, 100.0f)
			);
			// +x
			DirectX::XMStoreFloat3(&cameraDirections[0], DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
			DirectX::XMStoreFloat3(&cameraUps[0], DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// -x
			DirectX::XMStoreFloat3(&cameraDirections[1], DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
			DirectX::XMStoreFloat3(&cameraUps[1], DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// +y
			DirectX::XMStoreFloat3(&cameraDirections[2], DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			DirectX::XMStoreFloat3(&cameraUps[2], DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			// -y
			DirectX::XMStoreFloat3(&cameraDirections[3], DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));
			DirectX::XMStoreFloat3(&cameraUps[3], DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			// +z
			DirectX::XMStoreFloat3(&cameraDirections[4], DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			DirectX::XMStoreFloat3(&cameraUps[4], DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// -z
			DirectX::XMStoreFloat3(&cameraDirections[5], DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			DirectX::XMStoreFloat3(&cameraUps[5], DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			SetDepthBuffer(pDepthCube->GetDepthBuffer(0));
		}
		void Execute(D3D12RHI& gfx) const noexcept override
		{
			const auto shadowPos = pShadowCamera->GetPos();
			const auto pos = XMLoadFloat3(&shadowPos);

			UINT initWidth = gfx.GetWidth();
			UINT initHeight = gfx.GetHeight();

			gfx.ResizeFrame(size, size);
			gfx.SetProjection(XMLoadFloat4x4(&projection));

			for (size_t i = 0; i < 6; i++)
			{
				auto d = pDepthCube->GetDepthBuffer(i);
				d->Clear(gfx);
				SetDepthBuffer(std::move(d));
				const auto lookAt = pos + XMLoadFloat3(&cameraDirections[i]);
				gfx.SetCamera(XMMatrixLookAtLH(pos, lookAt, XMLoadFloat3(&cameraUps[i])));
				RenderQueuePass::Execute(gfx);
			}
			gfx.ResizeFrame(initWidth, initHeight);
		}
		void DumpShadowMap(D3D12RHI& gfx, const std::string& path) const
		{
		}
	private:
		void SetDepthBuffer(std::shared_ptr<DepthStencil> ds) const
		{
			const_cast<ShadowMappingPass*>(this)->depthStencil = std::move(ds);
		}
		static constexpr UINT size = 1000;
		const Camera* pShadowCamera = nullptr;
		std::shared_ptr<DepthCubeMapTextureBuffer> pDepthCube;
		XMFLOAT4X4 projection;
		std::vector<XMFLOAT3> cameraDirections{ 6 };
		std::vector<XMFLOAT3> cameraUps{ 6 };
	};
}