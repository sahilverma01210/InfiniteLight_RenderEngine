#pragma once
#include "../Common/ImGUI_Includes.h"

#include "ILMaterial.h"

namespace Renderer
{
	class PostProcessMaterial : public ILMaterial
	{
		enum class KernelType
		{
			Gauss,
			Box,
		};

		struct alignas(16) Coefficient
		{
			float value;
		};

		__declspec(align(256u)) struct PostProcessMatHandles
		{
			ResourceHandle kernelConstIdx;
			ResourceHandle frameBufferIdx1;
			ResourceHandle frameBufferIdx2;
		};

		__declspec(align(256u)) struct Kernel
		{
			alignas(16) UINT numTaps;
			Coefficient coefficients[15];
		};

	public:
		PostProcessMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			Technique postProcess{ "post_process", true};
			postProcess.passNames.push_back("horizontal");
			postProcess.passNames.push_back("vertical");
			m_techniques.push_back(std::move(postProcess));

			SetKernelGauss(m_radius, m_sigma);
			m_postProcessMatHandles.kernelConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_kernel), static_cast<const void*>(&m_kernel)), ResourceType::Constant);
			m_postProcessMatHandles.frameBufferIdx1 = RenderGraph::m_renderTargetHandles["Outline_Draw"];
			m_postProcessMatHandles.frameBufferIdx2 = RenderGraph::m_renderTargetHandles["Horizontal_Blur"];
			m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_postProcessMatHandles), static_cast<const void*>(&m_postProcessMatHandles)), ResourceType::Constant);
		}
		void SetKernelBox(int radius) noexcept(!IS_DEBUG)
		{
			assert(radius <= m_maxRadius);
			const int nTaps = radius * 2 + 1;
			m_kernel.numTaps = nTaps;
			const float c = 1.0f / nTaps;
			for (int i = 0; i < nTaps; i++)
			{
				m_kernel.coefficients[i].value = c;
			}
		}
		void SetKernelGauss(int radius, float sigma) noexcept(!IS_DEBUG)
		{
			assert(radius <= m_maxRadius);
			const int nTaps = radius * 2 + 1;
			m_kernel.numTaps = nTaps;
			float sum = 0.0f;
			for (int i = 0; i < nTaps; i++)
			{
				const auto x = float(i - radius);
				const auto g = gauss(x, sigma);
				sum += g;
				m_kernel.coefficients[i].value = g;
			}
			for (int i = 0; i < nTaps; i++)
			{
				m_kernel.coefficients[i].value = (float)m_kernel.coefficients[i].value / sum;
			}
		}
		void RenderWidgets(D3D12RHI& gfx)
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
		UINT getID() const override {
			return getTypeID<PostProcessMaterial>();
		}

	private:
		static constexpr int m_maxRadius = 7;
		int m_radius = 5;
		float m_sigma = 4.0f;
		KernelType m_kernelType = KernelType::Gauss;
		Kernel m_kernel = {};
		PostProcessMatHandles m_postProcessMatHandles{};
	};
}