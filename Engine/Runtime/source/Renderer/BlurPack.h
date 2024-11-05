#pragma once
#include "UIManager.h"
#include "CommonBindables.h"
#include "../Common/ILMath.h"

using namespace Common;

namespace Renderer
{
	class BlurPack
	{
	public:
		BlurPack(D3D12RHI& gfx, int radius = 7, float sigma = 2.6f)
			:
			radius(radius),
			sigma(sigma)
		{
			SetKernelGauss(gfx, radius, sigma);
			pcb = std::move(std::make_unique<ConstantBuffer>(gfx, 0, sizeof(kernel), &kernel));
			ccb = std::move(std::make_unique<ConstantBuffer>(gfx, 1, sizeof(control), &control));
			ccb1 = std::move(std::make_unique<ConstantBuffer>(gfx, 1, sizeof(control), &control));
		}
		void SetHorizontal(D3D12RHI& gfx)
		{
			control.horizontal = TRUE;
			ccb->Update(gfx, &control);
			ccb->Bind(gfx);

			Bind(gfx);
		}
		void SetVertical(D3D12RHI& gfx)
		{
			control.horizontal = FALSE;
			ccb1->Update(gfx, &control);
			ccb1->Bind(gfx);

			Bind(gfx);
		}
		void Bind(D3D12RHI& gfx) noexcept
		{
			pcb->Bind(gfx);
		}
		void RenderWidgets(D3D12RHI& gfx)
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
								kernelType = KernelType::Gauss;
							}
							else if (curItem == items[1])
							{
								kernelType = KernelType::Box;
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

			bool radChange = ImGui::SliderInt("Radius", &radius, 0, 15);
			bool sigChange = ImGui::SliderFloat("Sigma", &sigma, 0.1f, 10.0f);
			if (radChange || sigChange || filterChanged)
			{
				if (kernelType == KernelType::Gauss)
				{
					SetKernelGauss(gfx, radius, sigma);
					pcb->Update(gfx, &kernel);
				}
				else if (kernelType == KernelType::Box)
				{
					SetKernelBox(gfx, radius);
					pcb->Update(gfx, &kernel);
				}
			}
		}
		// nTaps should be 6sigma - 1
		// for more accurate coefs, need to integrate, but meh :/
		void SetKernelGauss(D3D12RHI& gfx, int radius, float sigma) noexcept
		{
			assert(radius <= maxRadius);
			kernel.nTaps = radius * 2 + 1;
			float sum = 0.0f;
			for (int i = 0; i < kernel.nTaps; i++)
			{
				const auto x = float(i - radius);
				const auto g = gauss(x, sigma);
				sum += g;
				kernel.coefficients[i].x = g;
			}
			for (int i = 0; i < kernel.nTaps; i++)
			{
				kernel.coefficients[i].x /= sum;
			}
		}
		void SetKernelBox(D3D12RHI& gfx, int radius) noexcept
		{
			assert(radius <= maxRadius);
			kernel.nTaps = radius * 2 + 1;
			const float c = 1.0f / kernel.nTaps;
			for (int i = 0; i < kernel.nTaps; i++)
			{
				kernel.coefficients[i].x = c;
			}
		}
	private:
		enum class KernelType
		{
			Gauss,
			Box,
		};
		static constexpr int maxRadius = 15;
		int radius;
		float sigma;
		KernelType kernelType = KernelType::Gauss;
		struct Kernel
		{
			int nTaps;
			float padding[3];
			DirectX::XMFLOAT4 coefficients[maxRadius * 2 + 1];
		} kernel;
		struct Control
		{
			BOOL horizontal;
			float padding[3];
		} control;
		std::shared_ptr<ConstantBuffer> pcb;
		std::shared_ptr<ConstantBuffer> ccb;
		std::shared_ptr<ConstantBuffer> ccb1;
	};
}