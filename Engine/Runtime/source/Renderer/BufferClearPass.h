#pragma once
#include "Pass.h"

namespace Renderer
{
	class BufferResource;

	class BufferClearPass : public Pass
	{
	public:
		BufferClearPass(std::string name);
		void Execute(D3D12RHI& gfx) const noexcept override;
	private:
		std::shared_ptr<BufferResource> buffer;
	};

	class BufferBucketClearPass : public Pass
	{
	public:
		BufferBucketClearPass(std::string name);
		void Execute(D3D12RHI& gfx) const noexcept override;
	private:
		std::vector<std::shared_ptr<BufferResource>> bufferVector;
	};
}