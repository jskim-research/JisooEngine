#include "Runtime/Launch/EngineLoop.h"

#include "Runtime/Engine/Engine.h"
#include "Runtime/Platform/Windows/WindowsWindow.h"

#include <chrono>
#include <thread>

int FEngineLoop::Run(FEngine& Engine)
{
    FWindowsWindow Window;
    if (!Window.Initialize(L"JisooEngine", 1280, 720))
    {
        return -1;
    }

    if (!Engine.Initialize())
    {
        return -1;
    }

    while (Window.ProcessMessages())
    {
        Engine.Tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Engine.Shutdown();
    return 0;
}
