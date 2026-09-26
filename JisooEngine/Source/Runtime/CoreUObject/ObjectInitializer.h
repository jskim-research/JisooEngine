#pragma once

#include "Runtime/CoreUObject/ObjectHandle.h"

#include <string>

class UClass;
class UObject;

/** NewObject가 UObject 생성자에 전달하는 타입, 소속, 이름과 객체 슬롯 정보다. */
class FObjectInitializer
{
public:
    FObjectInitializer(
        UClass* InClass,
        UObject* InOuter,
        std::string InName,
        FObjectHandle InHandle);

    [[nodiscard]] UClass* GetClass() const;
    [[nodiscard]] UObject* GetOuter() const;
    [[nodiscard]] const std::string& GetName() const;
    [[nodiscard]] FObjectHandle GetHandle() const;

private:
    UClass* Class = nullptr;
    UObject* Outer = nullptr;
    std::string Name;
    FObjectHandle Handle;
};
