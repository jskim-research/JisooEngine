#include "Framework/TestRunner.h"

#include <iostream>
#include <string_view>

void RegisterCoreUObjectTests(FTestRunner& Runner);
void RegisterEngineLifecycleTests(FTestRunner& Runner);
void RegisterSceneComponentTests(FTestRunner& Runner);
void RegisterRenderSceneTests(FTestRunner& Runner);
void RegisterViewportTests(FTestRunner& Runner);

int main(int ArgumentCount, char* Arguments[])
{
    std::string_view Suite;
    if (ArgumentCount == 3 && std::string_view(Arguments[1]) == "--suite")
    {
        Suite = Arguments[2];
    }
    else if (ArgumentCount != 1)
    {
        std::cerr << "사용법: JisooEngineTests [--suite <이름>]\n";
        return 1;
    }

    FTestRunner Runner;
    RegisterCoreUObjectTests(Runner);
    RegisterEngineLifecycleTests(Runner);
    RegisterSceneComponentTests(Runner);
    RegisterRenderSceneTests(Runner);
    RegisterViewportTests(Runner);
    return Runner.Run(Suite);
}
