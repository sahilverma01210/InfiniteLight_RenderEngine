#pragma once
#include "../_External/framework.h"

namespace Renderer
{
	class RHI {
	public:
		virtual void OnInit() = 0;
		virtual void OnDestroy() = 0;
	};
}