#pragma once

// error exception helper macro
#define ILWND_EXCEPT( hr ) Window::Exception( __LINE__,__FILE__,hr )
#define ILWND_LAST_EXCEPT() Window::Exception( __LINE__,__FILE__,GetLastError() )