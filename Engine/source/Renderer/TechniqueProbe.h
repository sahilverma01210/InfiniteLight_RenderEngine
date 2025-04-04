#pragma once
#include "../_External/common.h"

namespace Renderer
{
	class Buffer;
	struct Technique;

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
		bool VisitBuffer(Buffer& buf)
		{
			m_bufIdx++;
			return OnVisitBuffer(buf);
		}
	protected:
		virtual void OnSetTechnique() {}
		virtual bool OnVisitBuffer(Buffer&)
		{
			return false;
		}

	protected:
		Technique* m_pTech = nullptr;
		size_t m_techIdx;
		size_t m_stepIdx;
		size_t m_bufIdx;
	};
}