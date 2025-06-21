#pragma once
#include "D3D12Buffer.h"
#include "D3D12Texture.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

namespace Renderer
{
	class Drawable
	{
	public:
		virtual void SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG) = 0;
		void Draw(D3D12RHI& gfx) const noexcept(!IS_DEBUG);

	protected:
		UINT m_numIndices;
		UINT m_vertexSizeInBytes;
		UINT m_indexSizeInBytes;
		UINT m_vertexStrideInBytes;
		std::shared_ptr<D3D12Buffer> m_vertexBuffer;
		std::shared_ptr<D3D12Buffer> m_indexBuffer;
	};
}