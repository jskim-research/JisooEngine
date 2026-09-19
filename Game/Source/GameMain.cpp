#include <Windows.h>

#include "Runtime/EngineLoop.h"

int WINAPI wWinMain(
    HINSTANCE Instance,
    HINSTANCE PreviousInstance,
    PWSTR CommandLine,
    int ShowCommand)
{
    UNREFERENCED_PARAMETER(Instance);
    UNREFERENCED_PARAMETER(PreviousInstance);
    UNREFERENCED_PARAMETER(CommandLine);
    UNREFERENCED_PARAMETER(ShowCommand);

    FEngineLoop EngineLoop;
    return EngineLoop.Run(L"JisooGame");
}
