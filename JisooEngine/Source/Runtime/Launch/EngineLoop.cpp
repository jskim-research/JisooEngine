#include "Runtime/Launch/EngineLoop.h"

#include "Runtime/Engine/Engine.h"

int FEngineLoop::Run(FEngine& Engine)
{
    if (!Engine.Initialize())
    {
        return -1;
    }

    Engine.Tick();
    Engine.Shutdown();
    return 0;
}
