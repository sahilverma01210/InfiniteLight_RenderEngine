#include "Drawable.h"

namespace Renderer
{
	void Drawable::Bind(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		m_vertexBufferBindable->Bind(gfx);
		m_indexBufferBindable->Bind(gfx);

		SetTransform(gfx);
	}

	void Drawable::Draw(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		gfx.DrawIndexed(m_numIndices);
	}
}