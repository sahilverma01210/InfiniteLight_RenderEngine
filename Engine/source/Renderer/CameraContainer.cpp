#include "CameraContainer.h"

namespace Renderer
{
	CameraContainer::~CameraContainer()
	{
	}

	void CameraContainer::SpawnWindow(D3D12RHI& gfx)
	{
		if (ImGui::Begin("Cameras"))
		{
			if (ImGui::BeginCombo("Active Camera", GetActiveCamera().GetName().c_str()))
			{
				for (int i = 0; i < std::size(cameras); i++)
				{
					const bool isSelected = i == active;
					if (ImGui::Selectable(cameras[i]->GetName().c_str(), isSelected))
					{
						active = i;
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Controlled Camera", GetControlledCamera().GetName().c_str()))
			{
				for (int i = 0; i < std::size(cameras); i++)
				{
					const bool isSelected = i == controlled;
					if (ImGui::Selectable(cameras[i]->GetName().c_str(), isSelected))
					{
						controlled = i;
					}
				}
				ImGui::EndCombo();
			}

			GetControlledCamera().SpawnControlWidgets(gfx);
		}
		ImGui::End();
	}

	void CameraContainer::Bind(D3D12RHI& gfx)
	{
		gfx.SetCamera(GetActiveCamera().GetMatrix());
	}

	void CameraContainer::AddCamera(std::shared_ptr<Camera> pCam)
	{
		cameras.push_back(std::move(pCam));
	}

	void CameraContainer::LinkTechniques(RenderGraph& rg)
	{
		for (auto& pcam : cameras)
		{
			pcam->LinkTechniques(rg);
		}
	}

	void CameraContainer::Submit(size_t channels) const
	{
		for (size_t i = 0; i < cameras.size(); i++)
		{
			if (i != active)
			{
				cameras[i]->Submit(channels);
			}
		}
	}

	Camera& CameraContainer::GetActiveCamera()
	{
		return *cameras[active];
	}

	Camera& CameraContainer::GetControlledCamera()
	{
		return *cameras[controlled];
	}
}