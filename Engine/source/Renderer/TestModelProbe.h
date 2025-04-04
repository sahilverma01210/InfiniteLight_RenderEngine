#pragma once
#include "../Common/ImGUI_Includes.h"

#include "TechniqueProbe.h"
#include "ModelProbe.h"
#include "DynamicConstant.h"
#include "Model.h"
#include "Node.h"
#include "RenderMath.h"

namespace Renderer
{
	class TP : public TechniqueProbe
	{
	public:
		void OnSetTechnique() override
		{
			using namespace std::string_literals;
			ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, m_pTech->name.c_str());
			bool active = m_pTech->active;
			ImGui::Checkbox(("Tech Active##"s + std::to_string(m_techIdx)).c_str(), &active);
			m_pTech->active = active;
		}
		bool OnVisitBuffer(Buffer& buf) override
		{
			float dirty = false;
			const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
			auto tag = [tagScratch = std::string{}, tagString = "##" + std::to_string(m_bufIdx)]
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
				dcheck(ImGui::ColorPicker3(tag("Color"), reinterpret_cast<float*>(&static_cast<XMFLOAT3&>(v))));
			}
			if (auto v = buf["specularColor"]; v.Exists())
			{
				dcheck(ImGui::ColorPicker3(tag("Spec. Color"), reinterpret_cast<float*>(&static_cast<XMFLOAT3&>(v))));
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
	private:
		struct TransformParameters
		{
			float xRot = 0.0f;
			float yRot = 0.0f;
			float zRot = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
		};

	public:
		MP(std::string name) : m_name(std::move(name))
		{}
		bool SpawnWindow(Model& model)
		{
			ImGui::Begin(m_name.c_str(), &m_imGUIwndOpen);
			ImGui::Columns(2, nullptr, true);
			model.Accept(*this);
			ImGui::NextColumn();
			if (m_pSelectedNode != nullptr)
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
					m_pSelectedNode->SetAppliedTransform(
						XMMatrixRotationX(tf.xRot) *
						XMMatrixRotationY(tf.yRot) *
						XMMatrixRotationZ(tf.zRot) *
						XMMatrixTranslation(tf.x, tf.y, tf.z)
					);
				}
			}
			if (m_pSelectedNode != nullptr)
			{
				TP probe;
				m_pSelectedNode->Accept(probe);
			}
			ImGui::End();

			return m_imGUIwndOpen;
		}
	protected:
		bool PushNode(Node& node) override
		{
			// if there is no selected node, set selectedId to an impossible value
			const int selectedId = (m_pSelectedNode == nullptr) ? -1 : m_pSelectedNode->GetId();
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
						if (m_pTech->name == "Outline")
						{
							m_pTech->active = highlighted;
						}
					}
					bool highlighted = false;
				} probe;
				// remove highlight on prev-selected node
				if (m_pSelectedNode != nullptr)
				{
					m_pSelectedNode->Accept(probe);
				}
				// add highlight to newly-selected node
				probe.highlighted = true;
				node.Accept(probe);
				m_pSelectedNode = &node;
			}
			// signal if children should also be recursed
			return expanded;
		}
		void PopNode(Node& node) override
		{
			ImGui::TreePop();
		}
	private:
		TransformParameters& ResolveTransform() noexcept(!IS_DEBUG)
		{
			const auto id = m_pSelectedNode->GetId();
			auto i = m_transformParams.find(id);
			if (i == m_transformParams.end())
			{
				return LoadTransform(id);
			}
			return i->second;
		}
		TransformParameters& LoadTransform(int id) noexcept(!IS_DEBUG)
		{
			const auto& applied = m_pSelectedNode->GetAppliedTransform();
			const auto angles = ExtractEulerAngles(applied);
			const auto translation = ExtractTranslation(applied);
			TransformParameters tp;
			tp.zRot = angles.z;
			tp.xRot = angles.x;
			tp.yRot = angles.y;
			tp.x = translation.x;
			tp.y = translation.y;
			tp.z = translation.z;
			return m_transformParams.insert({ id,{ tp } }).first->second;
		}

	public:
		bool m_imGUIwndOpen = true;
	private:
		std::string m_name;
		Node* m_pSelectedNode = nullptr;
		std::unordered_map<int, TransformParameters> m_transformParams;
	};
}