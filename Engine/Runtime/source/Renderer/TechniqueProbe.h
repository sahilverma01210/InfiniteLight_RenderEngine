#pragma once
#include "../_External/framework.h"

namespace Renderer
{
	class Buffer;
	class Technique;
	class Step;

	class TechniqueProbe
	{
	public:
		void SetTechnique(Technique* pTech_in)
		{
			pTech = pTech_in;
			techIdx++;
			OnSetTechnique();
		}
		void SetStep(Step* pStep_in)
		{
			pStep = pStep_in;
			stepIdx++;
			OnSetStep();
		}
		bool VisitBuffer(Buffer& buf)
		{
			bufIdx++;
			return OnVisitBuffer(buf);
		}
	protected:
		virtual void OnSetTechnique() {}
		virtual void OnSetStep() {}
		virtual bool OnVisitBuffer(Buffer&)
		{
			return false;
		}
	protected:
		Technique* pTech = nullptr;
		Step* pStep = nullptr;
		size_t techIdx;
		size_t stepIdx;
		size_t bufIdx;
		//size_t techIdx = std::numeric_limits<size_t>::max();
		//size_t stepIdx = std::numeric_limits<size_t>::max();
		//size_t bufIdx = std::numeric_limits<size_t>::max();
	};
}