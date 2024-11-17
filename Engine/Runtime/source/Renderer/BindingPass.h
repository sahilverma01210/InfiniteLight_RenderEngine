#pragma once
#include "Pass.h"
#include "Sink.h"

namespace Renderer
{
	class Bindable;

	class BindingPass : public Pass
	{
	protected:
		BindingPass(std::string name, std::vector<std::shared_ptr<Bindable>> binds = {});
		void AddBind(std::shared_ptr<Bindable> bind) noexcept;
		void BindAll(D3D12RHI& gfx) const noexcept;
		void Finalize() override;
		UINT GetBindsSize()
		{
			return binds.size();
		}
	protected:
		template<class T>
		void AddBindSink(std::string name)
		{
			const auto index = binds.size();
			binds.emplace_back();
			RegisterSink(std::make_unique<ContainerBindableSink<T>>(std::move(name), binds, index));
		}
		std::vector<std::shared_ptr<RenderTarget>> renderTargetVector;
		std::shared_ptr<DepthStencil> depthStencil;
	private:
		void BindBufferResources(D3D12RHI& gfx) const noexcept;
	private:
		std::vector<std::shared_ptr<Bindable>> binds;
	};
}