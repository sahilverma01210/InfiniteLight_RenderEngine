#pragma once
#include "Bindable.h"
#include "CommonBindables.h"
#include "D3D12RHI.h"
#include "TechniqueProbe.h"

namespace Renderer
{
	class Step
	{
	public:
		Step(size_t targetPass_in)
			:
			targetPass{ targetPass_in }
		{}
		Step(Step&&) = default;
		Step(const Step& src) noexcept
			:
			targetPass(src.targetPass)
		{
			bindables.reserve(src.bindables.size());

			for (auto& pb : src.bindables)
			{
				if (auto* pCloning = dynamic_cast<const CloningBindable*>(pb.get()))
				{
					bindables.push_back(pCloning->Clone());
				}
				else
				{
					bindables.push_back(pb);
				}
			}
		}
		Step& operator=(const Step&) = delete;
		Step& operator=(Step&&) = delete;
		void AddBindable(std::shared_ptr<Bindable> bind_in) noexcept
		{
			bindables.push_back(std::move(bind_in));
		}
		std::vector<std::shared_ptr<Bindable>> GetBindables() noexcept
		{
			return bindables;
		}
		void Submit(class FrameCommander& frame, const class Drawable& drawable) const;
		void Bind(D3D12RHI& gfx) const
		{
			for (const auto& b : bindables)
			{
				b->Bind(gfx);
			}
		}
		size_t GetTargetPass() const
		{
			return targetPass;
		}
		void InitializeParentReferences(const class Drawable& parent) noexcept;
		void Accept(TechniqueProbe& probe)
		{
			probe.SetStep(this);

			for (auto& pb : bindables)
			{
				pb->Accept(probe);
			}
		}
	private:
		size_t targetPass;
		std::vector<std::shared_ptr<Bindable>> bindables;
	};
}