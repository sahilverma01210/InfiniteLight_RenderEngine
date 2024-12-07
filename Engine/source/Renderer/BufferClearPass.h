#pragma once
#include "Pass.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "Sink.h"
#include "Source.h"

namespace Renderer
{
	class BufferClearPass : public Pass
	{
	public:
		BufferClearPass(std::string name);
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	private:
		std::shared_ptr<BufferResource> buffer;
	};

	class BufferBucketClearPass : public Pass
	{
	public:
		BufferBucketClearPass(std::string name);
		void Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	private:
		std::vector<std::shared_ptr<BufferResource>> bufferVector;
	};
}