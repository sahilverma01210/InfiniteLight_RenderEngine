#pragma once
#include "D3D12RTAccelerationStructure.h"
#include "Model.h"

namespace Renderer
{
	class AccelerationStructure
	{

	public:
		explicit AccelerationStructure(D3D12RHI& gfx);
		void AddInstance(Model& model);
		Int32 Build();
		void Clear();
		Int32 GetTLASIndex() const;
	private:
		void BuildBottomLevels();
		void BuildTopLevel();

	private:
		D3D12RHI& m_gfx;
		ResourceHandle m_tlasIdx;
		std::vector<D3D12RTInstance> m_rtInstances;
		std::vector<D3D12RTGeometry> m_rtGeometries;
		std::unique_ptr<D3D12RTTopLevelAS> m_tlas;
		std::vector<std::unique_ptr<D3D12RTBottomLevelAS>> m_blases;
	};
}