#include "Runtime/Launch/EngineLoop.h"

#include "Runtime/Engine/Engine.h"
#include "Runtime/Platform/Windows/WindowsWindow.h"

#include <chrono>
#include <thread>

int FEngineLoop::Run(FEngine& Engine)
{
    constexpr std::uint32_t ClientWidth = 1280;
    constexpr std::uint32_t ClientHeight = 720;

    FWindowsWindow Window;
    if (!Window.Initialize(L"JisooEngine", ClientWidth, ClientHeight))
    {
        return -1;
    }

    FEngineInitParams InitParams{};
    InitParams.NativeWindowHandle = Window.GetNativeHandle();
    InitParams.ClientWidth = ClientWidth;
    InitParams.ClientHeight = ClientHeight;

    if (!Engine.Initialize(InitParams))
    {
        Engine.Shutdown();
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
