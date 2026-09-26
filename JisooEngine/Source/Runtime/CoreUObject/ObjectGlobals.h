#pragma once

#include "Runtime/CoreUObject/Object.h"

#include <string>
#include <type_traits>
#include <utility>

class UClass;

/** UClass 메타데이터에 따라 UObject 메모리와 전역 슬롯을 구성한다. */
[[nodiscard]] UObject* StaticConstructObject(
    UClass* Class,
    UObject* Outer = nullptr,
    std::string Name = {});

template<typename TObject>
[[nodiscard]] TObject* NewObject(UObject* Outer = nullptr, std::string Name = {})
{
    static_assert(std::is_base_of_v<UObject, TObject>);
    return static_cast<TObject*>(
        StaticConstructObject(TObject::StaticClass(), Outer, std::move(Name)));
}

/** Alive 객체에 파괴를 요청하며 실제 메모리 해제는 FlushPendingDestroyObjects에서 수행한다. */
void DestroyObject(UObject* Object);

/** 파괴 준비가 끝난 객체의 FinishDestroy, 소멸자와 슬롯 반환을 처리한다. */
void FlushPendingDestroyObjects();

[[nodiscard]] bool IsValid(const UObject* Object);
[[nodiscard]] bool IsValid(FObjectHandle Handle);
[[nodiscard]] UObject* ResolveObject(FObjectHandle Handle);

template<typename TObject>
[[nodiscard]] TObject* Cast(UObject* Object)
{
    static_assert(std::is_base_of_v<UObject, TObject>);
    return Object != nullptr && Object->IsA(TObject::StaticClass())
        ? static_cast<TObject*>(Object)
        : nullptr;
}

template<typename TObject>
[[nodiscard]] const TObject* Cast(const UObject* Object)
{
    static_assert(std::is_base_of_v<UObject, TObject>);
    return Object != nullptr && Object->IsA(TObject::StaticClass())
        ? static_cast<const TObject*>(Object)
        : nullptr;
}
