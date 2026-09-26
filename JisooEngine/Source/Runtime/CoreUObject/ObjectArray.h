#pragma once

#include "Runtime/CoreUObject/ObjectHandle.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class UObject;

struct FUObjectItem
{
    UObject* Object = nullptr;
    std::uint32_t SerialNumber = 1;
};

/**
 * 생성된 UObject의 주소와 Serial을 전역 슬롯으로 추적한다.
 * 초기 구현은 Game Thread에서만 생성·조회·해제하는 것을 전제로 한다.
 */
class FUObjectArray final
{
public:
    /** 생성 전 확보한 메모리를 PendingConstruction 슬롯에 등록한다. */
    [[nodiscard]] FObjectHandle AllocateObjectIndex(UObject* ObjectMemory);

    /** Handle과 주소가 모두 일치하는 슬롯을 반환하고 Serial을 갱신한다. */
    void FreeObjectIndex(FObjectHandle Handle, const UObject* ExpectedObject);

    /** 생애주기 상태와 무관하게 현재 슬롯 주소를 반환한다. */
    [[nodiscard]] UObject* Resolve(FObjectHandle Handle) const;

    /** Handle이 현재 Alive UObject를 가리키는지 확인한다. */
    [[nodiscard]] bool IsValid(FObjectHandle Handle) const;

    [[nodiscard]] std::size_t GetObjectCount() const;

private:
    std::vector<FUObjectItem> Objects;
    std::vector<std::uint32_t> FreeIndices;
    std::size_t ObjectCount = 0;
};

extern FUObjectArray GUObjectArray;
