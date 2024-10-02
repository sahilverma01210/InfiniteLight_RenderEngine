#pragma once
#include "D3D12RHI.h"
#include "Bindable.h"

namespace Renderer
{
	class Drawable
	{
		friend class PointLight;
	public:
		Drawable() = default;
		Drawable(const Drawable&) = delete;
		virtual XMMATRIX GetTransformXM() const noexcept = 0;
		void Draw(D3D12RHI& gfx, XMMATRIX transform) const;
		virtual ~Drawable() = default;
	protected:
		template<class T>
		T* QueryBindable() noexcept
		{
			for (auto& pb : bindables)
			{
				if (auto pt = dynamic_cast<T*>(pb.get()))
				{
					return pt;
				}
			}
			return nullptr;
		}
		void AddRootSignatureObject(std::unique_ptr<Bindable> bindable) noexcept;
		void AddPipelineStateObject(std::unique_ptr<Bindable> bindable) noexcept;
		void AddShaderResourceViewObject(std::unique_ptr<Bindable> bindable) noexcept;
		void AddBindable(std::shared_ptr<Bindable> bindable) noexcept;
	private:
		virtual const UINT GetNumIndices() const noexcept = 0;
		std::unique_ptr<Bindable> rootSignBindable;
		std::unique_ptr<Bindable> psoBindable;
		std::unique_ptr<Bindable> srvBindable;
		std::vector<std::shared_ptr<Bindable>> bindables;
		static std::vector<std::shared_ptr<Bindable>> staticBinds;
	};
}