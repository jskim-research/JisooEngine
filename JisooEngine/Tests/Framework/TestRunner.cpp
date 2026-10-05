#include "Framework/TestRunner.h"

#include "Runtime/CoreUObject/ObjectArray.h"
#include "Runtime/CoreUObject/ObjectGlobals.h"

#include <exception>
#include <iostream>
#include <string>

FTestContext::FTestContext(std::string_view InTestName)
    : TestName(InTestName)
{
}

void FTestContext::Expect(bool bCondition, std::string_view Message)
{
    if (bCondition)
    {
        return;
    }

    ++FailureCount;
    std::cerr << "  " << TestName << ": " << Message << '\n';
}

std::size_t FTestContext::GetFailureCount() const
{
    return FailureCount;
}

void FTestRunner::Add(std::string_view Name, FTestFunction Function)
{
    TestCases.push_back({Name, Function});
}

int FTestRunner::Run(std::string_view Suite) const
{
    std::size_t ExecutedCount = 0;
    std::size_t FailedCount = 0;

    const std::string SuitePrefix = Suite.empty() ? std::string{} : std::string(Suite) + ".";
    for (const FTestCase& TestCase : TestCases)
    {
        if (!SuitePrefix.empty() && !TestCase.Name.starts_with(SuitePrefix))
        {
            continue;
        }

        ++ExecutedCount;
        FlushPendingDestroyObjects();
        const std::size_t ObjectCountBefore = GUObjectArray.GetObjectCount();
        FTestContext Context(TestCase.Name);

        try
        {
            TestCase.Function(Context);
        }
        catch (const std::exception& Exception)
        {
            Context.Expect(false, Exception.what());
        }
        catch (...)
        {
            Context.Expect(false, "알 수 없는 예외가 발생했다.");
        }

        FlushPendingDestroyObjects();
        const std::size_t ObjectCountAfter = GUObjectArray.GetObjectCount();
        if (ObjectCountAfter != ObjectCountBefore)
        {
            const std::string Message = "테스트 전후 UObject 수가 다르다: "
                + std::to_string(ObjectCountBefore) + " -> "
                + std::to_string(ObjectCountAfter);
            Context.Expect(false, Message);
        }

        if (Context.GetFailureCount() == 0)
        {
            std::cout << "[PASS] " << TestCase.Name << '\n';
        }
        else
        {
            ++FailedCount;
            std::cerr << "[FAIL] " << TestCase.Name << " ("
                      << Context.GetFailureCount() << ")\n";
        }

        // 누수된 UObject는 다음 테스트의 전역 상태를 오염시키므로 같은 프로세스의 실행을 중단한다.
        if (ObjectCountAfter != ObjectCountBefore)
        {
            break;
        }
    }

    if (ExecutedCount == 0)
    {
        std::cerr << "선택한 Suite에 등록된 테스트가 없다: " << Suite << '\n';
        return 1;
    }

    if (FailedCount != 0)
    {
        std::cerr << FailedCount << "개 테스트가 실패했다.\n";
        return 1;
    }

    std::cout << ExecutedCount << "개 테스트가 통과했다.\n";
    return 0;
}
