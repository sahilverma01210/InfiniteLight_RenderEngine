#pragma once

// error exception helper macro
#define ILWND_EXCEPT( hr ) Runtime::Window::Exception( __LINE__,__FILE__,hr )
#define ILWND_LAST_EXCEPT() Runtime::Window::Exception( __LINE__,__FILE__,GetLastError() )