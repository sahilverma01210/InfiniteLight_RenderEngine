#pragma once
#include "Drawable.h"

namespace Renderer
{
	template<class T>
	class Object : public Drawable
	{
		friend class PointLight;
	public:
		bool IsStaticInitialized() const noexcept
		{
			return !staticBinds.empty();
		}
		void AddStaticBind(std::unique_ptr<Bindable> bind) noexcept
		{
			staticBinds.push_back(std::move(bind));
		}
	private:
		const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept override
		{
			return staticBinds;
		}
	private:
		static std::vector<std::unique_ptr<Bindable>> staticBinds;
	};

	template<class T>
	std::vector<std::unique_ptr<Bindable>> Object<T>::staticBinds;
}