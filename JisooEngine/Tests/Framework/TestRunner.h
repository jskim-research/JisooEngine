#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

class FTestContext
{
public:
    explicit FTestContext(std::string_view InTestName);

    void Expect(bool bCondition, std::string_view Message);
    [[nodiscard]] std::size_t GetFailureCount() const;

private:
    std::string_view TestName;
    std::size_t FailureCount = 0;
};

using FTestFunction = void (*)(FTestContext&);

/** 이름이 고정된 테스트를 순서대로 실행하고 각 테스트의 UObject 누수를 격리해 보고한다. */
class FTestRunner
{
public:
    void Add(std::string_view Name, FTestFunction Function);

    /** 비어 있지 않은 Suite 이름은 `Suite.` 접두사가 일치하는 테스트만 실행한다. */
    int Run(std::string_view Suite = {}) const;

private:
    struct FTestCase
    {
        std::string_view Name;
        FTestFunction Function = nullptr;
    };

    std::vector<FTestCase> TestCases;
};
