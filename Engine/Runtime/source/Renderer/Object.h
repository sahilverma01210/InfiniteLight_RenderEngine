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
		void AddNumIndices(UINT numIndices) noexcept
		{
			m_numIndices = numIndices;
		}
	private:
		const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept override
		{
			return staticBinds;
		}
		const UINT GetNumIndices() const noexcept override
		{
			return m_numIndices;
		}
	private:
		static UINT m_numIndices;
		static std::vector<std::unique_ptr<Bindable>> staticBinds;
	};

	template<class T>
	UINT Object<T>::m_numIndices;

	template<class T>
	std::vector<std::unique_ptr<Bindable>> Object<T>::staticBinds;
}