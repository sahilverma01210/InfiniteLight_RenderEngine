#pragma once
#include "Pass.h"

namespace Renderer
{
	class BufferClearPass : public Pass
	{
	public:
		BufferClearPass(std::string name)
			:
			Pass(std::move(name))
		{
			RegisterSink(DirectBufferSink<BufferResource>::Make("buffer", m_buffer));
			RegisterSource(DirectBufferSource<BufferResource>::Make("buffer", m_buffer));
		}
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			m_buffer->Clear(gfx);
		}

	private:
		std::shared_ptr<BufferResource> m_buffer;
	};

	class BufferBucketClearPass : public Pass
	{
	public:
		BufferBucketClearPass(std::string name)
			:
			Pass(std::move(name))
		{
			RegisterSink(DirectBufferBucketSink<BufferResource>::Make("buffer", m_bufferVector));
			RegisterSource(DirectBufferBucketSource<BufferResource>::Make("buffer", m_bufferVector));
		}

		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override
		{
			m_bufferVector[gfx.GetCurrentBackBufferIndex()]->Clear(gfx);
		}

	private:
		std::vector<std::shared_ptr<BufferResource>> m_bufferVector;
	};
}