#pragma once
#include "CommonBindables.h"
#include "Vertex.h"
#include "DynamicConstant.h"
				
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
		D3D12_PRIMITIVE_TOPOLOGY GetTopology() const noexcept(!IS_DEBUG)
		{
			return m_topology;
		}
		void TogglePostProcessing(bool postProcessEnabled)
		{
			m_postProcessEnabled = postProcessEnabled;
		}
		virtual UINT getID() const = 0;
	protected:
		template <typename T>
		static UINT getTypeID() {
			static UINT typeID = m_materialTypeID++;
			return typeID;
		}

	public:
		static inline ResourceHandle m_lightHandle = 0;
		static inline XMFLOAT3 m_lightPosition{};
	protected:
		ResourceHandle m_materialHandle = 1;
		D3D12_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		std::vector<Technique> m_techniques;
		bool m_postProcessEnabled = true;
		bool m_wireframeEnabled = true;
	private:
		static inline UINT m_materialTypeID = 0;
	};
}