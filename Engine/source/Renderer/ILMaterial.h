#pragma once
				
namespace Renderer
{
	struct Technique
	{
		std::string name;
		bool active = false;
		std::vector<std::string> passNames;
	};

	class ILMaterial
	{
	public:
		ResourceHandle GetMaterialHandle()
		{
			return m_materialHandle;
		}
		std::vector<Technique> GetTechniques() noexcept(!IS_DEBUG)
		{
			return m_techniques;
		}
		virtual UINT getID() const = 0;
	protected:
		template <typename T>
		static UINT getTypeID() {
			static UINT typeID = m_materialTypeID++;
			return typeID;
		}

	protected:
		ResourceHandle m_materialHandle = 1;
		std::vector<Technique> m_techniques;
	private:
		static inline UINT m_materialTypeID = 0;
	};
}