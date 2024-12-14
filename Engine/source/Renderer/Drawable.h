#pragma once
#include "Material.h"

struct aiMesh;

namespace Renderer
{
	class Material;

	class Drawable
	{
		friend class PointLight;

	public:
		Drawable() = default;
		Drawable(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale = 1.0f) noexcept(!IS_DEBUG);
		Drawable(const Drawable&) = delete;
		virtual XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) = 0;
		void AddTechnique(Technique tech_in) noexcept(!IS_DEBUG);
		void Submit(size_t channels) const noexcept(!IS_DEBUG);
		void Bind(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG);
		void BindLighting(D3D12RHI& gfx) const noexcept(!IS_DEBUG);
		void Accept(TechniqueProbe& probe);
		void LinkTechniques(RenderGraph&);
		UINT GetIndexCount() const noexcept(!IS_DEBUG);
		virtual ~Drawable();

	protected:
		bool m_enableLighting;
		UINT m_numIndices;
		std::shared_ptr<Topology> m_topologyBindable;
		std::shared_ptr<VertexBuffer> m_vertexBufferBindable;
		std::shared_ptr<IndexBuffer> m_indexBufferBindable;
		std::unordered_map<std::string, std::unique_ptr<RootSignature>> m_rootSignBindables;
		std::unordered_map<std::string, std::unique_ptr<PipelineState>> m_psoBindables;
		std::vector<Technique> m_techniques;
	private:
		static std::shared_ptr<Bindable> m_lightBindable;
		static std::shared_ptr<Bindable> m_lightShadowBindable;
	};
}