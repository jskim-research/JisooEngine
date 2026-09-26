#pragma once

#include <cstddef>
#include <string_view>

class FObjectInitializer;
class UObject;

/**
 * UObject 파생 타입의 상속 관계, 메모리 배치와 생성·소멸 진입점을 보관한다.
 * UClass 자체는 초기 객체 시스템에서 UObject 생애주기에 참여하지 않는다.
 */
class UClass final
{
public:
    using FConstructObject = void (*)(void* Memory, const FObjectInitializer& ObjectInitializer);
    using FDestroyObject = void (*)(UObject* Object);

    UClass(
        std::string_view InName,
        UClass* InSuperClass,
        std::size_t InClassSize,
        std::size_t InClassAlignment,
        FConstructObject InConstructor,
        FDestroyObject InDestructor);

    [[nodiscard]] std::string_view GetName() const;
    [[nodiscard]] UClass* GetSuperClass() const;
    [[nodiscard]] std::size_t GetClassSize() const;
    [[nodiscard]] std::size_t GetClassAlignment() const;

    /** 이 클래스가 TargetClass와 같거나 그 파생 클래스인지 확인한다. */
    [[nodiscard]] bool IsChildOf(const UClass* TargetClass) const;

    /** NewObject가 확보한 Memory에 이 클래스의 실제 C++ 객체를 생성한다. */
    void Construct(void* Memory, const FObjectInitializer& ObjectInitializer) const;

    /** FinishDestroy가 끝난 객체의 실제 파생 클래스 소멸자를 호출한다. */
    void Destroy(UObject* Object) const;

private:
    std::string_view Name;
    UClass* SuperClass = nullptr;
    std::size_t ClassSize = 0;
    std::size_t ClassAlignment = 0;
    FConstructObject Constructor = nullptr;
    FDestroyObject Destructor = nullptr;
};
