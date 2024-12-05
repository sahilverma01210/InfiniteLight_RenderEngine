#include "BufferClearPass.h"

namespace Renderer
{
	BufferClearPass::BufferClearPass(std::string name)
		:
		Pass(std::move(name))
	{
		RegisterSink(DirectBufferSink<BufferResource>::Make("buffer", buffer));
		RegisterSource(DirectBufferSource<BufferResource>::Make("buffer", buffer));
	}

	void BufferClearPass::Execute(D3D12RHI& gfx) const noexcept
	{
		buffer->Clear(gfx);
	}

	BufferBucketClearPass::BufferBucketClearPass(std::string name)
		:
		Pass(std::move(name))
	{
		RegisterSink(DirectBufferBucketSink<BufferResource>::Make("buffer", bufferVector));
		RegisterSource(DirectBufferBucketSource<BufferResource>::Make("buffer", bufferVector));
	}

	void BufferBucketClearPass::Execute(D3D12RHI& gfx) const noexcept
	{
		bufferVector[gfx.GetCurrentBackBufferIndex()]->Clear(gfx);
	}
}