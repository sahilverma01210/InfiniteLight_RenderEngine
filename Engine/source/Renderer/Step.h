#pragma once
#include "Bindable.h"
#include "CommonBindables.h"
#include "D3D12RHI.h"

namespace Renderer
{
	class TechniqueProbe;
	class Drawable;
	class RenderQueuePass;
	class RenderGraph;

	class Step
	{
	public:
		Step(std::string targetPassName);
		Step(Step&&) = default;
		Step(const Step& src) noexcept;
		Step& operator=(const Step&) = delete;
		Step& operator=(Step&&) = delete;
		void AddBindable(std::shared_ptr<Bindable> bind_in) noexcept;
		void Submit(const Drawable& drawable) const;
		void Bind(D3D12RHI& gfx) const noexcept;
		void InitializeParentReferences(const Drawable& parent) noexcept;
		void Accept(TechniqueProbe& probe);
		void Link(RenderGraph& rg);
		std::vector<std::shared_ptr<Bindable>> GetBindables() noexcept;
		std::string GetTargetPass() const;

	private:
		std::vector<std::shared_ptr<Bindable>> bindables;
		RenderQueuePass* pTargetPass = nullptr;
		std::string targetPassName;
	};
}