#include "Renderer.h"
#include "Mesh.h"
#include "DynamicConstant.h"
#include "ModelProbe.h"
#include "Node.h"
#include "RenderMath.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Renderer
{
	Graphics::Graphics(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
		:
		pRHI(std::make_unique<D3D12RHI>(width, height, hWnd)),
		fc(*pRHI)
	{
		//TestScaleMatrixTranslation();
		//TestDynamicConstant();
		//pRHI = std::make_unique<D3D12RHI>(width, height);

		camera = new Camera(*pRHI);

		//TestMaterialSystemLoading(*pRHI);

		//goblin = std::make_unique<Model>(*pRHI, "models\\gobber\\GoblinX.obj", 4.0f);
		//goblin->SetRootTransform(XMMatrixTranslation(0.0f, 0.0f, -4.0f));
		//nano = std::make_unique<Model>(*pRHI, "models\\nano_textured\\nanosuit.obj", 2.0f);
		//nano->SetRootTransform(XMMatrixTranslation(0.0f, -7.0f, 6.0f));
		
		sponza = std::make_unique<Model>(*pRHI, "models\\sponza\\sponza.obj", 1.0f / 20.0f);
		sponza->SetRootTransform(XMMatrixTranslation(0.0f, -7.0f, 6.0f));
		//cube = std::make_unique<TestCube>(*pRHI, 4.0f);
		//cube1 = std::make_unique<TestCube>(*pRHI, 4.0f);
		//cube->SetPos({ 4.0f,0.0f,0.0f });
		//cube1->SetPos({ 0.0f,4.0f,0.0f });

		light = new PointLight(*pRHI);
		uiManager.InitUI(*pRHI);
	}

	void Graphics::StartFrame()
	{
		pRHI->StartFrame();
		uiManager.StartUIFrame(*pRHI);
	}

	void Graphics::Update()
	{
		camera->Update(*pRHI);
		light->Bind(*pRHI, camera->GetMatrix());

		// pRHI->RenderImGUI(); => Renders inside IMGUI Window when called before Object Draw Calls.

		light->Submit(fc);

		// Draw Model.
		//nano->Submit(fc);
		//cube->Submit(fc);
		//cube1->Submit(fc);
		//goblin->Submit(fc);
		sponza->Submit(fc);
		fc.Execute(*pRHI);
		// Update ImGUI.
		{
			// Mesh techniques window
			class TP : public TechniqueProbe
			{
			public:
				void OnSetTechnique() override
				{
					using namespace std::string_literals;
					ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, pTech->GetName().c_str());
					bool active = pTech->IsActive();
					ImGui::Checkbox(("Tech Active##"s + std::to_string(techIdx)).c_str(), &active);
					pTech->SetActiveState(active);
				}
				bool OnVisitBuffer(Buffer& buf) override
				{
					namespace dx = DirectX;
					float dirty = false;
					const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
					auto tag = [tagScratch = std::string{}, tagString = "##" + std::to_string(bufIdx)]
					(const char* label) mutable
						{
							tagScratch = label + tagString;
							return tagScratch.c_str();
						};
					if (auto v = buf["scale"]; v.Exists())
					{
						dcheck(ImGui::SliderFloat(tag("Scale"), &v, 1.0f, 2.0f, "%.3f", 3.5f));
					}
					if (auto v = buf["offset"]; v.Exists())
					{
						dcheck(ImGui::SliderFloat(tag("offset"), &v, 0.0f, 1.0f, "%.3f", 2.5f));
					}
					if (auto v = buf["materialColor"]; v.Exists())
					{
						dcheck(ImGui::ColorPicker3(tag("Color"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
					}
					if (auto v = buf["specularColor"]; v.Exists())
					{
						dcheck(ImGui::ColorPicker3(tag("Spec. Color"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
					}
					if (auto v = buf["specularGloss"]; v.Exists())
					{
						dcheck(ImGui::SliderFloat(tag("Glossiness"), &v, 1.0f, 100.0f, "%.1f", 1.5f));
					}
					if (auto v = buf["specularWeight"]; v.Exists())
					{
						dcheck(ImGui::SliderFloat(tag("Spec. Weight"), &v, 0.0f, 2.0f));
					}
					if (auto v = buf["useSpecularMap"]; v.Exists())
					{
						dcheck(ImGui::Checkbox(tag("Spec. Map Enable"), &v));
					}
					if (auto v = buf["useNormalMap"]; v.Exists())
					{
						dcheck(ImGui::Checkbox(tag("Normal Map Enable"), &v));
					}
					if (auto v = buf["normalMapWeight"]; v.Exists())
					{
						dcheck(ImGui::SliderFloat(tag("Normal Map Weight"), &v, 0.0f, 2.0f));
					}
					return dirty;
				}
			};

			class MP : ModelProbe
			{
			public:
				void SpawnWindow(Model& model)
				{
					ImGui::Begin("Model");
					ImGui::Columns(2, nullptr, true);
					model.Accept(*this);
					ImGui::NextColumn();
					if (pSelectedNode != nullptr)
					{
						bool dirty = false;
						const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
						auto& tf = ResolveTransform();
						ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, "Translation");
						dcheck(ImGui::SliderFloat("X", &tf.x, -60.f, 60.f));
						dcheck(ImGui::SliderFloat("Y", &tf.y, -60.f, 60.f));
						dcheck(ImGui::SliderFloat("Z", &tf.z, -60.f, 60.f));
						ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, "Orientation");
						dcheck(ImGui::SliderAngle("X-rotation", &tf.xRot, -180.0f, 180.0f));
						dcheck(ImGui::SliderAngle("Y-rotation", &tf.yRot, -180.0f, 180.0f));
						dcheck(ImGui::SliderAngle("Z-rotation", &tf.zRot, -180.0f, 180.0f));
						if (dirty)
						{
							pSelectedNode->SetAppliedTransform(
								dx::XMMatrixRotationX(tf.xRot) *
								dx::XMMatrixRotationY(tf.yRot) *
								dx::XMMatrixRotationZ(tf.zRot) *
								dx::XMMatrixTranslation(tf.x, tf.y, tf.z)
							);
						}
					}
					if (pSelectedNode != nullptr)
					{
						TP probe;
						pSelectedNode->Accept(probe);
					}
					ImGui::End();
				}
			protected:
				bool PushNode(Node& node) override
				{
					// if there is no selected node, set selectedId to an impossible value
					const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();
					// build up flags for current node
					const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
						| ((node.GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
						| (node.HasChildren() ? 0 : ImGuiTreeNodeFlags_Leaf);
					// render this node
					const auto expanded = ImGui::TreeNodeEx(
						(void*)(intptr_t)node.GetId(),
						node_flags, node.GetName().c_str()
					);
					// processing for selecting node
					if (ImGui::IsItemClicked())
					{
						// used to change the highlighted node on selection change
						struct Probe : public TechniqueProbe
						{
							virtual void OnSetTechnique()
							{
								if (pTech->GetName() == "Outline")
								{
									pTech->SetActiveState(highlighted);
								}
							}
							bool highlighted = false;
						} probe;
						// remove highlight on prev-selected node
						if (pSelectedNode != nullptr)
						{
							pSelectedNode->Accept(probe);
						}
						// add highlight to newly-selected node
						probe.highlighted = true;
						node.Accept(probe);
						pSelectedNode = &node;
					}
					// signal if children should also be recursed
					return expanded;
				}
				void PopNode(Node& node) override
				{
					ImGui::TreePop();
				}
			private:
				Node* pSelectedNode = nullptr;
				struct TransformParameters
				{
					float xRot = 0.0f;
					float yRot = 0.0f;
					float zRot = 0.0f;
					float x = 0.0f;
					float y = 0.0f;
					float z = 0.0f;
				};
				std::unordered_map<int, TransformParameters> transformParams;
			private:
				TransformParameters& ResolveTransform() noexcept
				{
					const auto id = pSelectedNode->GetId();
					auto i = transformParams.find(id);
					if (i == transformParams.end())
					{
						return LoadTransform(id);
					}
					return i->second;
				}
				TransformParameters& LoadTransform(int id) noexcept
				{
					const auto& applied = pSelectedNode->GetAppliedTransform();
					const auto angles = ExtractEulerAngles(applied);
					const auto translation = ExtractTranslation(applied);
					TransformParameters tp;
					tp.zRot = angles.z;
					tp.xRot = angles.x;
					tp.yRot = angles.y;
					tp.x = translation.x;
					tp.y = translation.y;
					tp.z = translation.z;
					return transformParams.insert({ id,{ tp } }).first->second;
				}
			};
			static MP modelProbe;

			modelProbe.SpawnWindow(*sponza);
			if (camera->m_imGUIwndOpen) camera->SpawnControlWindow(*pRHI);
			if (light->m_imGUIwndOpen) light->SpawnControlWindow();
			//cube->SpawnControlWindow(*pRHI, "Cube");
			//cube1->SpawnControlWindow(*pRHI, "Cube1");
			//goblin->ShowWindow();
			//nano->ShowWindow();
		}
	}

	void Graphics::EndFrame()
	{
		uiManager.EndUIFrame(*pRHI);
		pRHI->EndFrame();
		fc.Reset();
	}

	void Graphics::Destroy()
	{
		uiManager.DestroyUI(*pRHI);
		pRHI->OnDestroy();
	}

	void Graphics::Rotate(float dx, float dy)
	{
		camera->Rotate(dx, dy);
	}

	void Graphics::Translate(XMFLOAT3 translation)
	{
		camera->Translate(translation);
	}

	void Graphics::ToggleImguiDemoWindow()
	{
		showDemoWindow = !showDemoWindow;
	}

	void Graphics::ShowImguiDemoWindow()
	{
		if (showDemoWindow)
		{
			ImGui::ShowDemoWindow(&showDemoWindow);
		}
	}
}