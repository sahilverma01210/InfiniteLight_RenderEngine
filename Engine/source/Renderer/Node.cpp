#include "Node.h"
#include "Model.h"

namespace Renderer
{
	Node::Node(int id, const std::string& name, std::vector<std::shared_ptr<Mesh>> meshPtrs, const XMMATRIX& transform_in) noexcept(!IS_DEBUG)
		:
		m_id(id),
		m_meshPtrs(std::move(meshPtrs)),
		m_name(name)
	{
		XMStoreFloat4x4(&m_transform, transform_in);
		XMStoreFloat4x4(&m_appliedTransform, XMMatrixIdentity());
	}

	void Node::SpawnWindow()
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
			SetAppliedTransform(
				XMMatrixRotationX(tf.xRot) *
				XMMatrixRotationY(tf.yRot) *
				XMMatrixRotationZ(tf.zRot) *
				XMMatrixTranslation(tf.x, tf.y, tf.z)
			);
		}

		int index = 0;

		for (auto& renderEffect : m_meshPtrs[0]->GetRenderEffects())
		{
			using namespace std::string_literals;
			ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, renderEffect.first.c_str());
			ImGui::Checkbox(("Effect Active##"s + std::to_string(index)).c_str(), &renderEffect.second);

			index++;
		}

		for (int i = 0; i < m_meshPtrs.size(); i++)
		{
			for (auto& renderEffect : m_meshPtrs[i]->GetRenderEffects())
			{
				m_meshPtrs[i]->SetRenderEffect(renderEffect.first, m_meshPtrs[0]->GetRenderEffects()[renderEffect.first]);
			}
		}
	}

	void Node::OnSelect() noexcept(!IS_DEBUG)
	{
		for (auto& mp : m_meshPtrs) mp->SetRenderEffect("object_flat", true);
	}

	void Node::OnDeselect() noexcept(!IS_DEBUG)
	{
		for (auto& mp : m_meshPtrs) mp->SetRenderEffect("object_flat", false);
	}

	void Node::Update(FXMMATRIX accumulatedTransform) const noexcept(!IS_DEBUG)
	{
		const auto built =
			XMLoadFloat4x4(&m_appliedTransform) *
			XMLoadFloat4x4(&m_transform) *
			accumulatedTransform;
		for (const auto& pm : m_meshPtrs)
		{
			pm->Update(built);
		}
		for (const auto& pc : m_childPtrs)
		{
			pc->Update(built);
		}
	}

	void Node::AddChild(std::unique_ptr<Node> pChild) noexcept(!IS_DEBUG)
	{
		assert(pChild);
		m_childPtrs.push_back(std::move(pChild));
	}

	void Node::SetAppliedTransform(FXMMATRIX transform) noexcept(!IS_DEBUG)
	{
		XMStoreFloat4x4(&m_appliedTransform, transform);
	}

	const XMFLOAT4X4& Node::GetAppliedTransform() const noexcept(!IS_DEBUG)
	{
		return m_appliedTransform;
	}

	int Node::GetId() const noexcept(!IS_DEBUG)
	{
		return m_id;
	}

	void Node::Accept(Model& model)
	{
		if (model.PushNode(*this))
		{
			for (auto& cp : m_childPtrs)
			{
				cp->Accept(model);
			}
			model.PopNode(*this);
		}
	}
	
	TransformParameters& Node::ResolveTransform() noexcept(!IS_DEBUG)
	{
		const auto id = GetId();
		auto i = m_transformParams.find(id);
		if (i == m_transformParams.end())
		{
			return LoadTransform(id);
		}
		return i->second;
	}

	TransformParameters& Node::LoadTransform(int id) noexcept(!IS_DEBUG)
	{
		const auto& applied = GetAppliedTransform();
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
}