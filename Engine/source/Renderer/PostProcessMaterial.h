#pragma once
#include "ILMaterial.h"
#include "UIManager.h"

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

	public:
		PostProcessMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			Technique postProcess{ "post_process", Channel::main, true};
			{
				Step horizontalBlur("horizontal");
				{
					// Add Pipeline State Obejct
					{
						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = layout.GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						ID3DBlob* pixelShader;
						ID3DBlob* vertexShader;

						D3DCompileFromFile(GetAssetFullPath(L"BlurOutline_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath(L"Fullscreen_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);

						CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];

						// define static sampler 
						CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
						staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
						staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
						staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
						staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
						staticSampler.MipLODBias = 0.0f;
						staticSampler.MinLOD = 0.0f;
						staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

						samplers[0] = staticSampler;

						PipelineDescription pipelineDesc{};
						pipelineDesc.numConstantBufferViews = 2;
						pipelineDesc.numShaderResourceViews = 1;
						pipelineDesc.numSamplers = 1;
						pipelineDesc.samplers = samplers;
						pipelineDesc.backFaceCulling = true;
						pipelineDesc.numElements = vec.size();
						pipelineDesc.inputElementDescs = inputElementDescs;
						pipelineDesc.pixelShader = pixelShader;
						pipelineDesc.vertexShader = vertexShader;

						m_pipelineDesc["horizontal"] = pipelineDesc;
					}

					// setup blur constant buffers
					{
						RawLayout l;
						l.Add<Integer>("nTaps");
						l.Add<Array>("coefficients");
						l["coefficients"].Set<Float>(m_maxRadius * 2 + 1);
						Buffer buf{ std::move(l) };
						m_blurKernel = std::make_shared<ConstantBuffer>(gfx, 0, buf);
						horizontalBlur.AddBindable(m_blurKernel);
						SetKernelGauss(m_radius, m_sigma);
					}
					{
						RawLayout l;
						l.Add<Bool>("isHorizontal");
						Buffer buf{ std::move(l) };
						buf["isHorizontal"] = true;
						m_horizontalFilter = std::make_shared<ConstantBuffer>(gfx, 1, buf);
						horizontalBlur.AddBindable(m_horizontalFilter);
					}

					std::shared_ptr<ShaderResourceView> srvBindablePtr = std::move(std::make_shared<ShaderResourceView>(gfx, 2, 1, 2));
					horizontalBlur.AddBindable(srvBindablePtr);
				}
				postProcess.AddStep(horizontalBlur);

				Step verticalBlur("vertical");
				{
					// Add Pipeline State Obejct
					{
						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = layout.GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						ID3DBlob* pixelShader;
						ID3DBlob* vertexShader;

						D3DCompileFromFile(GetAssetFullPath(L"BlurOutline_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath(L"Fullscreen_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);

						CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];

						// define static sampler 
						CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
						staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
						staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
						staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
						staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
						staticSampler.MipLODBias = 0.0f;
						staticSampler.MinLOD = 0.0f;
						staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

						samplers[0] = staticSampler;

						PipelineDescription pipelineDesc{};
						pipelineDesc.numConstantBufferViews = 2;
						pipelineDesc.numShaderResourceViews = 1;
						pipelineDesc.numSamplers = 1;
						pipelineDesc.samplers = samplers;
						pipelineDesc.backFaceCulling = true;
						pipelineDesc.numElements = vec.size();
						pipelineDesc.inputElementDescs = inputElementDescs;
						pipelineDesc.pixelShader = pixelShader;
						pipelineDesc.vertexShader = vertexShader;
						pipelineDesc.blending = true;
						pipelineDesc.depthStencilMode = Mode::Mask;

						m_pipelineDesc["vertical"] = pipelineDesc;
					}

					// setup blur constant buffers
					{
						RawLayout l;
						l.Add<Integer>("nTaps");
						l.Add<Array>("coefficients");
						l["coefficients"].Set<Float>(m_maxRadius * 2 + 1);
						Buffer buf{ std::move(l) };
						m_blurKernel = std::make_shared<ConstantBuffer>(gfx, 0, buf);
						SetKernelGauss(m_radius, m_sigma);
						verticalBlur.AddBindable(m_blurKernel);
					}
					{
						RawLayout l;
						l.Add<Bool>("isHorizontal");
						Buffer buf{ std::move(l) };
						buf["isHorizontal"] = false;
						m_verticalFilter = std::make_shared<ConstantBuffer>(gfx, 1, buf);
						verticalBlur.AddBindable(m_verticalFilter);
					}

					std::shared_ptr<ShaderResourceView> srvBindablePtr = std::move(std::make_unique<ShaderResourceView>(gfx, 2, 1, 2));
					verticalBlur.AddBindable(srvBindablePtr);
				}
				postProcess.AddStep(verticalBlur);
			}
			m_techniques.push_back(std::move(postProcess));
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
		void SetKernelBox(int radius) noexcept(!IS_DEBUG)
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
		void SetKernelGauss(int radius, float sigma) noexcept(!IS_DEBUG)
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

	private:
		KernelType m_kernelType = KernelType::Gauss;
		static constexpr int m_maxRadius = 7;
		int m_radius = 5;
		float m_sigma = 4.0f;
		std::shared_ptr<ConstantBuffer> m_blurKernel;
		std::shared_ptr<ConstantBuffer> m_horizontalFilter;
		std::shared_ptr<ConstantBuffer> m_verticalFilter;
		std::shared_ptr<ShaderResourceView> m_srvBindable;
	};
}