#include "BufferClearPass.h"

namespace Renderer
{
	BufferClearPass::BufferClearPass(std::string name)
		:
		Pass(std::move(name))
	{
		RegisterSink(DirectBufferSink<BufferResource>::Make("buffer", m_buffer));
		RegisterSource(DirectBufferSource<BufferResource>::Make("buffer", m_buffer));
	}

	void BufferClearPass::Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		m_buffer->Clear(gfx);
	}

	BufferBucketClearPass::BufferBucketClearPass(std::string name)
		:
		Pass(std::move(name))
	{
		RegisterSink(DirectBufferBucketSink<BufferResource>::Make("buffer", m_bufferVector));
		RegisterSource(DirectBufferBucketSource<BufferResource>::Make("buffer", m_bufferVector));
	}

	void BufferBucketClearPass::Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		m_bufferVector[gfx.GetCurrentBackBufferIndex()]->Clear(gfx);
	}
}