#pragma once

#include <string_view>

class FEngineLoop
{
public:
    int Run(std::wstring_view ApplicationName);

private:
    bool Initialize(std::wstring_view ApplicationName);
    void Tick();
    void Shutdown();

private:
    std::wstring_view ApplicationName;
};
