#pragma once
#include "../_External/common.h"

namespace Renderer
{
	class Buffer;
	class Technique;
	class Step;

	class TechniqueProbe
	{
	public:
		virtual ~TechniqueProbe() {}
		void SetTechnique(Technique* pTech_in)
		{
			m_pTech = pTech_in;
			m_techIdx++;
			OnSetTechnique();
		}
		void SetStep(Step* pStep_in)
		{
			m_pStep = pStep_in;
			m_stepIdx++;
			OnSetStep();
		}
		bool VisitBuffer(Buffer& buf)
		{
			m_bufIdx++;
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
		Technique* m_pTech = nullptr;
		Step* m_pStep = nullptr;
		size_t m_techIdx;
		size_t m_stepIdx;
		size_t m_bufIdx;
	};
}