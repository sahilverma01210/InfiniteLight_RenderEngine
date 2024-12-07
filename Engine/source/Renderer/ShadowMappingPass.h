#pragma once
#include "RenderMath.h"

#include "RenderQueuePass.h"
#include "Job.h"
#include "Source.h"
#include "RenderTarget.h"
#include "Camera.h"
#include "TextureBuffer.h"
#include "Viewport.h"

namespace Renderer
{
	class D3D12RHI;

	class ShadowMappingPass : public RenderQueuePass
	{
	public:
		ShadowMappingPass(D3D12RHI& gfx, std::string name)
			:
			RenderQueuePass(std::move(name))
		{
			pDepthCube = std::make_shared<DepthCubeMapTextureBuffer>(gfx, size);
			gfx.SetDepthBuffer(pDepthCube->GetBuffer());
			RegisterSource(DirectBindableSource<DepthCubeMapTextureBuffer>::Make("map", pDepthCube));

			XMStoreFloat4x4(
				&projection,
				XMMatrixPerspectiveFovLH(PI / 2.0f, 1.0f, 0.5f, 100.0f)
			);
			// +x
			XMStoreFloat3(&cameraDirections[0], XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
			XMStoreFloat3(&cameraUps[0], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// -x
			XMStoreFloat3(&cameraDirections[1], XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
			XMStoreFloat3(&cameraUps[1], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// +y
			XMStoreFloat3(&cameraDirections[2], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			XMStoreFloat3(&cameraUps[2], XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			// -y
			XMStoreFloat3(&cameraDirections[3], XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));
			XMStoreFloat3(&cameraUps[3], XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			// +z
			XMStoreFloat3(&cameraDirections[4], XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			XMStoreFloat3(&cameraUps[4], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			// -z
			XMStoreFloat3(&cameraDirections[5], XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			XMStoreFloat3(&cameraUps[5], XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			SetDepthBuffer(pDepthCube->GetDepthBuffer(0));
		}
		void BindShadowCamera(const Camera& cam) noexcept(!IS_DEBUG)
		{
			pShadowCamera = &cam;
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
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
	private:
		void SetDepthBuffer(std::shared_ptr<DepthStencil> ds) const
		{
			const_cast<ShadowMappingPass*>(this)->depthStencil = std::move(ds);
		}

	private:
		static constexpr UINT size = 1000;
		const Camera* pShadowCamera = nullptr;
		std::shared_ptr<DepthCubeMapTextureBuffer> pDepthCube;
		XMFLOAT4X4 projection;
		std::vector<XMFLOAT3> cameraDirections{ 6 };
		std::vector<XMFLOAT3> cameraUps{ 6 };
	};
}