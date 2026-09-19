#include "Runtime/EngineLoop.h"

#include <Windows.h>

#include <iostream>
#include <string>

namespace
{
    void InitializeConsole()
    {
        if (GetConsoleWindow() != nullptr)
        {
            return;
        }

        if (!AttachConsole(ATTACH_PARENT_PROCESS) && !AllocConsole())
        {
            return;
        }

        FILE* ConsoleOutput = nullptr;
        freopen_s(&ConsoleOutput, "CONOUT$", "w", stdout);
    }

    void PrintLine(std::wstring_view Message)
    {
        std::wstring Output{Message};
        Output.push_back(L'\n');

        OutputDebugStringW(Output.c_str());
        std::wcout << Output;
    }
}

int FEngineLoop::Run(std::wstring_view InApplicationName)
{
    InitializeConsole();

    if (!Initialize(InApplicationName))
    {
        return -1;
    }

    Tick();
    Shutdown();
    return 0;
}

bool FEngineLoop::Initialize(std::wstring_view InApplicationName)
{
    ApplicationName = InApplicationName;
    PrintLine(L"[" + std::wstring{ApplicationName} + L"] FEngineLoop::Initialize");
    return true;
}

void FEngineLoop::Tick()
{
    PrintLine(L"[" + std::wstring{ApplicationName} + L"] FEngineLoop::Tick");
}

void FEngineLoop::Shutdown()
{
    PrintLine(L"[" + std::wstring{ApplicationName} + L"] FEngineLoop::Shutdown");
}
