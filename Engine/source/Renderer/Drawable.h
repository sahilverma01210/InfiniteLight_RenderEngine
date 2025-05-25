#pragma once
#include "CommonBindables.h"

namespace Renderer
{
	class Drawable
	{
	public:
		virtual void SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG) = 0;
		void Bind(D3D12RHI& gfx) const noexcept(!IS_DEBUG);
		void Draw(D3D12RHI& gfx) const noexcept(!IS_DEBUG);

	protected:
		UINT m_numIndices;
		std::shared_ptr<VertexBuffer> m_vertexBufferBindable;
		std::shared_ptr<IndexBuffer> m_indexBufferBindable;
	};
}