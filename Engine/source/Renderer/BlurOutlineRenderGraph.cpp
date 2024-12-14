#include "BlurOutlineRenderGraph.h"

namespace Renderer
{
	BlurOutlineRenderGraph::BlurOutlineRenderGraph(D3D12RHI& gfx)
		:
		RenderGraph(gfx)
	{
		{
			auto pass = std::make_unique<BufferBucketClearPass>("clearRT");
			pass->SetSinkLinkage("buffer", "$.backbuffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<BufferClearPass>("clearDS");
			pass->SetSinkLinkage("buffer", "$.masterDepth");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<ShadowMappingPass>(gfx, "shadowMap");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<LambertianPass>(gfx, "lambertian");
			pass->SetSinkLinkage("shadowMap", "shadowMap.map");
			pass->SetSinkLinkage("renderTarget", "clearRT.buffer");
			pass->SetSinkLinkage("depthStencil", "clearDS.buffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<SkyboxPass>(gfx, "skybox");
			pass->SetSinkLinkage("renderTarget", "lambertian.renderTarget");
			pass->SetSinkLinkage("depthStencil", "lambertian.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<OutlineMaskGenerationPass>(gfx, "outlineMask");
			pass->SetSinkLinkage("depthStencil", "skybox.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<BlurOutlineDrawingPass>(gfx, "outlineDraw", gfx.GetWidth(), gfx.GetHeight());
			AppendPass(std::move(pass));
		}

		{
			auto pass = std::make_unique<HorizontalBlurPass>("horizontal", gfx, gfx.GetWidth(), gfx.GetHeight());
		
			// setup blur constant buffers
			{
				RawLayout l;
				l.Add<Integer>("nTaps");
				l.Add<Array>("coefficients");
				l["coefficients"].Set<Float>(m_maxRadius * 2 + 1);
				Buffer buf{ std::move(l) };
				m_blurKernel = std::make_shared<ConstantBuffer>(gfx, 0, buf);
				SetKernelGauss(m_radius, m_sigma);
				AddGlobalSource(DirectBindableSource<ConstantBuffer>::Make("blurKernel", m_blurKernel));
			}
			{
				RawLayout l;
				l.Add<Bool>("isHorizontal");
				Buffer buf{ std::move(l) };
				m_blurHorizontal = std::make_shared<ConstantBuffer>(gfx, 1, buf);
				AddGlobalSource(DirectBindableSource<ConstantBuffer>::Make("blurHorizontal", m_blurHorizontal));
			}
		
			pass->SetSinkLinkage("scratchIn", "outlineDraw.scratchOut");
			pass->SetSinkLinkage("kernel", "$.blurKernel");
			pass->SetSinkLinkage("direction", "$.blurHorizontal");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<VerticalBlurPass>("vertical", gfx);

			// setup blur constant buffers
			{
				RawLayout l;
				l.Add<Bool>("isHorizontal");
				Buffer buf{ std::move(l) };
				m_blurVertical = std::make_shared<ConstantBuffer>(gfx, 1, buf);
				AddGlobalSource(DirectBindableSource<ConstantBuffer>::Make("blurVertical", m_blurVertical));
			}

			pass->SetSinkLinkage("renderTarget", "skybox.renderTarget");
			pass->SetSinkLinkage("depthStencil", "outlineMask.depthStencil");
			pass->SetSinkLinkage("scratchIn", "horizontal.scratchOut");
			pass->SetSinkLinkage("kernel", "$.blurKernel");
			pass->SetSinkLinkage("direction", "$.blurVertical");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<WireframePass>(gfx, "wireframe");
			pass->SetSinkLinkage("renderTarget", "vertical.renderTarget");
			pass->SetSinkLinkage("depthStencil", "vertical.depthStencil");
			AppendPass(std::move(pass));
		}
		SetSinkTarget("backbuffer", "wireframe.renderTarget");

		Finalize();
	}

	void BlurOutlineRenderGraph::RenderWidgets(D3D12RHI& gfx)
	{
		if (ImGui::Begin("Kernel"))
		{
			bool filterChanged = false;
			{
				const char* items[] = { "Gauss","Box" };
				static const char* curItem = items[0];
				if (ImGui::BeginCombo("Filter Type", curItem))
				{
					for (int n = 0; n < std::size(items); n++)
					{
						const bool isSelected = (curItem == items[n]);
						if (ImGui::Selectable(items[n], isSelected))
						{
							filterChanged = true;
							curItem = items[n];
							if (curItem == items[0])
							{
								m_kernelType = KernelType::Gauss;
							}
							else if (curItem == items[1])
							{
								m_kernelType = KernelType::Box;
							}
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}
			bool radChange = ImGui::SliderInt("Radius", &m_radius, 0, m_maxRadius);
			bool sigChange = ImGui::SliderFloat("Sigma", &m_sigma, 0.1f, 10.0f);
			if (radChange || sigChange || filterChanged)
			{
				if (m_kernelType == KernelType::Gauss)
				{
					SetKernelGauss(m_radius, m_sigma);
				}
				else if (m_kernelType == KernelType::Box)
				{
					SetKernelBox(m_radius);
				}
			}
		}
		ImGui::End();
	}

	void BlurOutlineRenderGraph::BindMainCamera(Camera& cam)
	{
		dynamic_cast<LambertianPass&>(FindPassByName("lambertian")).BindMainCamera(cam);
		dynamic_cast<SkyboxPass&>(FindPassByName("skybox")).BindMainCamera(cam);
	}

	void BlurOutlineRenderGraph::BindShadowCamera(Camera& cam)
	{
		dynamic_cast<ShadowMappingPass&>(FindPassByName("shadowMap")).BindShadowCamera(cam);
	}

	void BlurOutlineRenderGraph::SetKernelBox(int radius) noexcept(!IS_DEBUG)
	{
		assert(radius <= m_maxRadius);
		auto k = m_blurKernel->GetBuffer();
		const int nTaps = radius * 2 + 1;
		k["nTaps"] = nTaps;
		const float c = 1.0f / nTaps;
		for (int i = 0; i < nTaps; i++)
		{
			k["coefficients"][i] = c;
		}
		m_blurKernel->SetBuffer(k);
	}

	void BlurOutlineRenderGraph::SetKernelGauss(int radius, float sigma) noexcept(!IS_DEBUG)
	{
		assert(radius <= m_maxRadius);
		auto k = m_blurKernel->GetBuffer();
		const int nTaps = radius * 2 + 1;
		k["nTaps"] = nTaps;
		float sum = 0.0f;
		for (int i = 0; i < nTaps; i++)
		{
			const auto x = float(i - radius);
			const auto g = gauss(x, sigma);
			sum += g;
			k["coefficients"][i] = g;
		}
		for (int i = 0; i < nTaps; i++)
		{
			k["coefficients"][i] = (float)k["coefficients"][i] / sum;
		}
		m_blurKernel->SetBuffer(k);
	}
}