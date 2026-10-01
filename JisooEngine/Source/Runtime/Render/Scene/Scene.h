#pragma once

#include "Runtime/Render/Scene/PrimitiveSceneHandle.h"
#include "Runtime/Render/Scene/PrimitiveSceneProxy.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

/**
 * 한 World의 Render Scene 상태와 Primitive Proxy 생애주기를 소유한다.
 * 현재 Single Thread에서는 갱신을 즉시 반영하며 Renderer에는 읽기 전용 Proxy를 제공한다.
 */
class FScene
{
public:
    FScene() = default;
    ~FScene() = default;

    FScene(const FScene&) = delete;
    FScene& operator=(const FScene&) = delete;

    /** Proxy 소유권을 넘겨받아 등록하고 제거·재사용을 검증할 Handle을 반환한다. */
    [[nodiscard]] FPrimitiveSceneHandle AddPrimitive(
        std::unique_ptr<FPrimitiveSceneProxy> Proxy);

    /** 유효한 Handle의 Transform과 World Bounds를 동기적으로 갱신한다. */
    bool UpdatePrimitiveTransform(
        FPrimitiveSceneHandle Handle,
        const FPrimitiveTransformUpdate& Update);

    /** 유효한 Handle의 Local·World Bounds를 동기적으로 갱신한다. */
    bool UpdatePrimitiveBounds(
        FPrimitiveSceneHandle Handle,
        const FPrimitiveBoundsUpdate& Update);

    /** 유효한 Handle의 공통 렌더 플래그를 동기적으로 갱신한다. */
    bool UpdatePrimitiveFlags(
        FPrimitiveSceneHandle Handle,
        const FPrimitiveFlagsUpdate& Update);

    /** Proxy를 즉시 Scene에서 제거하며 같은 Handle을 이후 갱신에 사용할 수 없게 한다. */
    bool RemovePrimitive(FPrimitiveSceneHandle Handle);

    /** 유효한 Handle이면 읽기 전용 Proxy를 반환하며 다음 제거 전까지만 포인터가 유효하다. */
    [[nodiscard]] const FPrimitiveSceneProxy* ResolvePrimitive(
        FPrimitiveSceneHandle Handle) const;

    /** 등록된 Proxy를 읽기 전용으로 순회하며 callback 안에서 Scene을 변경하면 안 된다. */
    template<typename TVisitor>
    void ForEachPrimitive(TVisitor&& Visitor) const
    {
        for (const FPrimitiveSceneSlot& Slot : PrimitiveSlots)
        {
            if (Slot.Proxy != nullptr)
            {
                Visitor(*Slot.Proxy);
            }
        }
    }

    [[nodiscard]] std::size_t GetPrimitiveCount() const;

private:
    struct FPrimitiveSceneSlot
    {
        std::unique_ptr<FPrimitiveSceneProxy> Proxy;
        std::uint32_t Generation = 0;
    };

    [[nodiscard]] FPrimitiveSceneProxy* ResolvePrimitiveMutable(FPrimitiveSceneHandle Handle);

    std::vector<FPrimitiveSceneSlot> PrimitiveSlots;
    std::vector<std::uint32_t> FreePrimitiveSlots;
    std::size_t PrimitiveCount = 0;
};
