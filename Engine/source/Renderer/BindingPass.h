#pragma once
#include "Pass.h"

namespace Renderer
{
	class BindingPass : public Pass
	{
	protected:
		BindingPass(std::string name, std::vector<std::shared_ptr<Bindable>> binds = {});
		void AddBind(std::shared_ptr<Bindable> bind) noexcept(!IS_DEBUG);
		void BindAll(D3D12RHI& gfx) const noexcept(!IS_DEBUG);
		void Finalize() override;
		UINT GetBindsSize()
		{
			return binds.size();
		}
		template<class T>
		void AddBindSink(std::string name)
		{
			const auto index = binds.size();
			binds.emplace_back();
			RegisterSink(std::make_unique<ContainerBindableSink<T>>(std::move(name), binds, index));
		}
	private:
		void BindBufferResources(D3D12RHI& gfx) const noexcept(!IS_DEBUG);

	protected:
		std::vector<std::shared_ptr<RenderTarget>> renderTargetVector;
		std::shared_ptr<DepthStencil> depthStencil;
	private:
		std::vector<std::shared_ptr<Bindable>> binds;
	};
}