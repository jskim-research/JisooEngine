#include "Runtime/Launch/EngineLoop.h"

int FEngineLoop::Run()
{
    if (!Initialize())
    {
        return -1;
    }

    Tick();
    Shutdown();
    return 0;
}

bool FEngineLoop::Initialize()
{
    return true;
}

void FEngineLoop::Tick()
{
}

void FEngineLoop::Shutdown()
{
}
