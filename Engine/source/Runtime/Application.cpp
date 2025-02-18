#include "Application.h"

namespace Runtime
{
	Application::Application(json appConfig) : m_window(appConfig)
	{}

	int Application::Run()
	{
        bool triggerRestart = false;
        bool exit = false;

		while (true)
		{
			// process all messages pending, but to not block for new messages
			if (const auto ecode = Window::ProcessMessages())
			{
				// if return optional has value, means we're quitting so return exit code
				return *ecode;
			}
			UpdateApp(triggerRestart, exit);

            if (triggerRestart) 
            {
                MessageBox(NULL, _T("Application will restart now!"), _T("Restart"), MB_OK);
                Restart();
            }

            if (exit)
            {
                ExitApp();
            }
		}
	}

    void Application::Restart()
    {
        TCHAR szPath[MAX_PATH];

        // Get the path of the current executable
        if (GetModuleFileName(NULL, szPath, MAX_PATH))
        {
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;

            // Create a new process with the same executable
            if (CreateProcess(szPath,   // Application path
                NULL,     // Command line (NULL = same app)
                NULL,     // Process handle not inheritable
                NULL,     // Thread handle not inheritable
                FALSE,    // Do not inherit handles
                0,        // No special flags
                NULL,     // Use parent's environment block
                NULL,     // Use parent's starting directory 
                &si,      // Pointer to STARTUPINFO structure
                &pi))     // Pointer to PROCESS_INFORMATION structure
            {
                // Close handles to avoid memory leaks
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                // Exit the current application
                ExitProcess(0);
            }
        }
    }

	void Application::UpdateApp(bool& triggerRestart, bool& exit)
	{
		m_window.UpdateWindow(triggerRestart, exit);
	}

    void Application::ExitApp()
    {
        ExitProcess(0);
    }
}