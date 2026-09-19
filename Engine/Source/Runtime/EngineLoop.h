#pragma once

class FEngineLoop
{
public:
    int Run();

private:
    bool Initialize();
    void Tick();
    void Shutdown();
};
