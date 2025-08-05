#pragma once
#include "ILMesh.h"

struct aiMesh;

namespace Renderer
{
	class Mesh : public ILMesh
	{
	public:
		Mesh(D3D12RHI& gfx, const aiMesh& mesh, float scale = 1.0f) noexcept(!IS_DEBUG);
		std::vector<VertexStruct> MakeVertices(const aiMesh& mesh, float scale = 1.0f) const noexcept(!IS_DEBUG);
		std::vector<USHORT> MakeIndices(const aiMesh& mesh) const noexcept(!IS_DEBUG);
		void Update(FXMMATRIX accumulatedTranform) const noexcept(!IS_DEBUG);
		std::unordered_map<std::string, bool>& GetRenderEffects() noexcept { return m_renderEffects; }
		void SetRenderEffect(const std::string& effectName, bool enabled) noexcept { m_renderEffects[effectName] = enabled; }

	private:
		UINT m_meshIdx;
		mutable XMFLOAT4X4 m_transform;
		std::unordered_map<std::string, bool> m_renderEffects;
	};
}