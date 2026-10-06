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
    Window.SetInputEventSink(&Engine.GetInputEventSink());
    if (!Window.Initialize(L"JisooEngine", ClientWidth, ClientHeight))
    {
        Window.SetInputEventSink(nullptr);
        return -1;
    }

    FEngineInitParams InitParams{};
    InitParams.NativeWindowHandle = Window.GetNativeHandle();
    InitParams.ClientWidth = ClientWidth;
    InitParams.ClientHeight = ClientHeight;

    if (!Engine.Initialize(InitParams))
    {
        Window.SetInputEventSink(nullptr);
        Engine.Shutdown();
        return -1;
    }

    auto PreviousFrameTime = std::chrono::steady_clock::now();
    while (Window.ProcessMessages())
    {
        FWindowResizeEvent ResizeEvent;
        if (Window.ConsumeResizeEvent(ResizeEvent))
        {
            if (!Engine.HandleWindowResize(ResizeEvent))
            {
                Window.SetInputEventSink(nullptr);
                Engine.Shutdown();
                return -1;
            }

            // GPU 대기가 포함될 수 있는 Resize 시간을 다음 Simulation Delta에 누적하지 않는다.
            PreviousFrameTime = std::chrono::steady_clock::now();
        }

        const auto CurrentFrameTime = std::chrono::steady_clock::now();
        const float DeltaSeconds = std::chrono::duration<float>(
            CurrentFrameTime - PreviousFrameTime).count();
        PreviousFrameTime = CurrentFrameTime;

        Engine.Tick(DeltaSeconds);

        // 현재는 Frame Pacing을 구현하지 않았으므로 무제한 busy loop만 방지한다.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Window.SetInputEventSink(nullptr);
    Engine.Shutdown();
    return 0;
}
