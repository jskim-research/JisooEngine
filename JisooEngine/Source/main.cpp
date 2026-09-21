#include <Windows.h>

#include "Runtime/Launch/EngineLoop.h"

#if JISOO_WITH_EDITOR
#include "Editor/Engine/EditorEngine.h"
#else
#include "Runtime/Engine/GameEngine.h"
#endif

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
#if JISOO_WITH_EDITOR
    FEditorEngine Engine;
#else
    FGameEngine Engine;
#endif

    FEngineLoop EngineLoop;
    return EngineLoop.Run(Engine);
}
