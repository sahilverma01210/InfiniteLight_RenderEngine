#pragma once
#include "CommonBindables.h"
#include "TechniqueProbe.h"

namespace Renderer
{
	class RenderGraph;
	class RenderPass;

	class Step
	{
	public:
		Step(std::string targetPassName);
		Step(Step&&) = default;
		Step(const Step& src) noexcept(!IS_DEBUG);
		Step& operator=(const Step&) = delete;
		Step& operator=(Step&&) = delete;
		void AddBindable(std::shared_ptr<Bindable> bind_in) noexcept(!IS_DEBUG);
		void Bind(D3D12RHI& gfx) const noexcept(!IS_DEBUG);
		std::string GetTargetPass() const;
		void Link(RenderGraph& rg);
		void Submit(const Drawable& drawable) const;
		void Accept(TechniqueProbe& probe);

	private:
		std::vector<std::shared_ptr<Bindable>> m_bindables;
		RenderPass* m_pTargetPass = nullptr;
		std::string m_targetPassName;
	};
}