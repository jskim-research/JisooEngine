#include "Runtime/Render/Scene/Scene.h"

#include <cassert>
#include <utility>

FPrimitiveSceneHandle FScene::AddPrimitive(std::unique_ptr<FPrimitiveSceneProxy> Proxy)
{
    if (Proxy == nullptr)
    {
        return {};
    }

    std::uint32_t SlotIndex = 0;
    if (!FreePrimitiveSlots.empty())
    {
        SlotIndex = FreePrimitiveSlots.back();
        FreePrimitiveSlots.pop_back();
    }
    else
    {
        assert(PrimitiveSlots.size() < FPrimitiveSceneHandle::InvalidIndex);
        SlotIndex = static_cast<std::uint32_t>(PrimitiveSlots.size());
        PrimitiveSlots.emplace_back();
    }

    FPrimitiveSceneSlot& Slot = PrimitiveSlots[SlotIndex];
    assert(Slot.Proxy == nullptr);
    ++Slot.Generation;
    if (Slot.Generation == 0)
    {
        ++Slot.Generation;
    }
    Slot.Proxy = std::move(Proxy);
    ++PrimitiveCount;
    return {SlotIndex, Slot.Generation};
}

bool FScene::UpdatePrimitiveTransform(
    FPrimitiveSceneHandle Handle,
    const FPrimitiveTransformUpdate& Update)
{
    FPrimitiveSceneProxy* Proxy = ResolvePrimitiveMutable(Handle);
    if (Proxy == nullptr)
    {
        return false;
    }

    Proxy->ApplyTransform(Update);
    return true;
}

bool FScene::UpdatePrimitiveBounds(
    FPrimitiveSceneHandle Handle,
    const FPrimitiveBoundsUpdate& Update)
{
    FPrimitiveSceneProxy* Proxy = ResolvePrimitiveMutable(Handle);
    if (Proxy == nullptr)
    {
        return false;
    }

    Proxy->ApplyBounds(Update);
    return true;
}

bool FScene::UpdatePrimitiveFlags(
    FPrimitiveSceneHandle Handle,
    const FPrimitiveFlagsUpdate& Update)
{
    FPrimitiveSceneProxy* Proxy = ResolvePrimitiveMutable(Handle);
    if (Proxy == nullptr)
    {
        return false;
    }

    Proxy->ApplyFlags(Update);
    return true;
}

bool FScene::RemovePrimitive(FPrimitiveSceneHandle Handle)
{
    FPrimitiveSceneProxy* Proxy = ResolvePrimitiveMutable(Handle);
    if (Proxy == nullptr)
    {
        return false;
    }

    FPrimitiveSceneSlot& Slot = PrimitiveSlots[Handle.Index];
    Slot.Proxy.reset();
    FreePrimitiveSlots.push_back(Handle.Index);
    assert(PrimitiveCount > 0);
    --PrimitiveCount;
    return true;
}

const FPrimitiveSceneProxy* FScene::ResolvePrimitive(FPrimitiveSceneHandle Handle) const
{
    if (!Handle.IsSet() || Handle.Index >= PrimitiveSlots.size())
    {
        return nullptr;
    }

    const FPrimitiveSceneSlot& Slot = PrimitiveSlots[Handle.Index];
    return Slot.Generation == Handle.Generation ? Slot.Proxy.get() : nullptr;
}

std::size_t FScene::GetPrimitiveCount() const
{
    return PrimitiveCount;
}

FPrimitiveSceneProxy* FScene::ResolvePrimitiveMutable(FPrimitiveSceneHandle Handle)
{
    return const_cast<FPrimitiveSceneProxy*>(
        static_cast<const FScene*>(this)->ResolvePrimitive(Handle));
}
