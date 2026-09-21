#include <Windows.h>

#include "Runtime/Launch/EngineLoop.h"

int WINAPI wWinMain(
    HINSTANCE Instance,
    HINSTANCE PreviousInstance,
    PWSTR CommandLine,
    int ShowCommand)
{
    FEngineLoop EngineLoop;
    return EngineLoop.Run();
}
