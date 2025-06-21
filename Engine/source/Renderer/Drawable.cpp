#include "Drawable.h"

namespace Renderer
{
	void Drawable::Draw(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		gfx.SetVertexBuffer(m_vertexBuffer->GetBuffer(), m_vertexSizeInBytes, m_vertexStrideInBytes);
		gfx.SetIndexBuffer(m_indexBuffer->GetBuffer(), m_indexSizeInBytes);

		SetTransform(gfx);

		gfx.DrawIndexed(m_numIndices);
	}
}