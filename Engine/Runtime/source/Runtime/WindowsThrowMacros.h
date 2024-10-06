#pragma once

// error exception helper macro
#define ILWND_EXCEPT( hr ) Runtime::Window::HrException( __LINE__,__FILE__,hr )
#define ILWND_LAST_EXCEPT() Runtime::Window::HrException( __LINE__,__FILE__,GetLastError() )
#define ILWND_NOGFX_EXCEPT() Runtime::Window::NoGfxException( __LINE__,__FILE__ )