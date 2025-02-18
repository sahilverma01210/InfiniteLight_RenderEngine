#pragma once
#include "../Common/ImGUI_Includes.h"

#include "ILMaterial.h"

namespace Renderer
{
	class PostProcessMaterial : public ILMaterial
	{
	private:
		enum class KernelType
		{
			Gauss,
			Box,
		};

		struct alignas(16) Coefficient
		{
			float value;
		};

		__declspec(align(256u)) struct Kernel
		{


			alignas(16) UINT numTaps;
			Coefficient coefficients[15];
		};

		__declspec(align(256u)) struct Direction
		{
			bool horizontal;
		};

	public:
		PostProcessMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			Technique postProcess{ "post_process", Channel::main, true};
			{
				Step horizontalBlur("horizontal");
				{
					// Add Resources & Samplers
					{
						DescriptorTable::TableParams params;
						params.resourceParameterIndex = 0;
						params.samplerParameterIndex = 1;
						params.numCbvSrvUavDescriptors = 3;
						params.numSamplerDescriptors = 1;

						std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_shared<DescriptorTable>(gfx, params));

						// Add Constants
						{
							SetKernelGauss(m_radius, m_sigma);

							std::shared_ptr<ConstantBuffer> kernelConstBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(m_kernel), static_cast<const void*>(&m_kernel));
							descriptorTable->AddConstantBufferView(gfx, kernelConstBuffer->GetBuffer());
							horizontalBlur.AddBindable(std::move(kernelConstBuffer));
							m_direction.horizontal = true;

							std::shared_ptr<ConstantBuffer> directionConstBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(m_direction), static_cast<const void*>(&m_direction));
							descriptorTable->AddConstantBufferView(gfx, directionConstBuffer->GetBuffer());
							horizontalBlur.AddBindable(std::move(directionConstBuffer));
						}

						// Add Textures
						{
							descriptorTable->AddShaderResourceView(gfx, 5);
						}

						// Add Samplers
						{
							D3D12_SAMPLER_DESC sampler{};
							sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
							sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							descriptorTable->AddSampler(gfx, &sampler);
						}

						horizontalBlur.AddBindable(std::move(descriptorTable));
					}
				}
				postProcess.AddStep(horizontalBlur);

				Step verticalBlur("vertical");
				{
					// Add Resources & Samplers
					{
						DescriptorTable::TableParams params;
						params.resourceParameterIndex = 0;
						params.samplerParameterIndex = 1;
						params.numCbvSrvUavDescriptors = 3;
						params.numSamplerDescriptors = 1;

						std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_shared<DescriptorTable>(gfx, params));

						// Add Constants
						{
							SetKernelGauss(m_radius, m_sigma);

							std::shared_ptr<ConstantBuffer> kernelConstBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(m_kernel), static_cast<const void*>(&m_kernel));
							descriptorTable->AddConstantBufferView(gfx, kernelConstBuffer->GetBuffer());
							verticalBlur.AddBindable(std::move(kernelConstBuffer));
							m_direction.horizontal = false;

							std::shared_ptr<ConstantBuffer> directionConstBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(m_direction), static_cast<const void*>(&m_direction));
							descriptorTable->AddConstantBufferView(gfx, directionConstBuffer->GetBuffer());
							verticalBlur.AddBindable(std::move(directionConstBuffer));
						}

						// Add Textures
						{
							descriptorTable->AddShaderResourceView(gfx, 6);
						}

						// Add Samplers
						{
							D3D12_SAMPLER_DESC sampler{};
							sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
							sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
							descriptorTable->AddSampler(gfx, &sampler);
						}

						verticalBlur.AddBindable(std::move(descriptorTable));
					}
				}
				postProcess.AddStep(verticalBlur);
			}
			m_techniques.push_back(std::move(postProcess));
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

	private:
		static constexpr int m_maxRadius = 7;
		int m_radius = 5;
		float m_sigma = 4.0f;
		KernelType m_kernelType = KernelType::Gauss;
		Kernel m_kernel = {};
		Direction m_direction = { true };
	};
}