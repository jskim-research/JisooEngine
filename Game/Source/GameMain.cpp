#include <Windows.h>

#include "Runtime/EngineLoop.h"

int WINAPI wWinMain(
    HINSTANCE Instance,
    HINSTANCE PreviousInstance,
    PWSTR CommandLine,
    int ShowCommand)
{
    FEngineLoop EngineLoop;
    return EngineLoop.Run();
}
