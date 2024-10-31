#pragma once
#include "D3D12RHI.h"
#include "Bindable.h"
#include "BindableCodex.h"
#include "CommonBindables.h"
#include "Technique.h"

struct aiMesh;

namespace Renderer
{
	class TechniqueProbe;
	class Material;

	class Drawable
	{
		friend class PointLight;
	public:
		Drawable() = default;
		Drawable(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale = 1.0f) noexcept;
		Drawable(const Drawable&) = delete;
		virtual XMMATRIX GetTransformXM() const noexcept = 0;
		void AddRootSignatureObject(std::unique_ptr<RootSignature> bindable) noexcept
		{
			rootSignBindables.push_back(std::move(bindable));
		}
		void AddPipelineStateObject(std::unique_ptr<PipelineState> bindable) noexcept
		{
			psoBindables.push_back(std::move(bindable));
		}
		void AddTechnique(Technique tech_in) noexcept;
		void Submit(class FrameCommander& frame) const noexcept;
		void Bind(D3D12RHI& gfx, size_t targetPass) const noexcept;
		void BindLighting(D3D12RHI& gfx) const noexcept;
		void Accept(TechniqueProbe& probe);
		UINT GetIndexCount() const noexcept;
		virtual ~Drawable();
	protected:
		UINT m_numIndices;
		std::shared_ptr<Topology> topologyBindable;
		std::shared_ptr<VertexBuffer> vertexBufferBindable;
		std::shared_ptr<IndexBuffer> indexBufferBindable;
		std::vector<std::unique_ptr<RootSignature>> rootSignBindables;
		std::vector<std::unique_ptr<PipelineState>> psoBindables;
		std::vector<Technique> techniques;
	private:
		static std::shared_ptr<Bindable> lightBindable;
	};
}