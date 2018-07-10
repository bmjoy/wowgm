#pragma once

#include <csignal>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace wowgm::signal
{
    static volatile std::sig_atomic_t gSignal;

    extern "C" static void handler(int signal)
    {
        gSignal = signal;
    }

    BOOL WINAPI win_Handler(DWORD signal)
    {
        switch (signal)
        {
            case CTRL_C_EVENT:
                ExitProcess(0);
                return TRUE;
        }
        return FALSE;
    }

    static void install()
    {
        std::signal(SIGINT, handler);
        std::signal(SIGTERM, handler);
        std::signal(SIGABRT, handler);

        SetConsoleCtrlHandler(&win_Handler, true);
    }

    static bool is_signalled() {
        return gSignal != 0;
    }
}
