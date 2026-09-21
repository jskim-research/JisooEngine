#pragma once

class FEngine
{
public:
    virtual ~FEngine() = default;

    virtual bool Initialize();
    virtual void Tick();
    virtual void Shutdown();
};
