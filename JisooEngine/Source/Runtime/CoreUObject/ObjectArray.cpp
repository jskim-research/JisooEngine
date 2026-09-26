#include "Runtime/CoreUObject/ObjectArray.h"

#include "Runtime/CoreUObject/Object.h"

#include <cassert>

FUObjectArray GUObjectArray;

FObjectHandle FUObjectArray::AllocateObjectIndex(UObject* ObjectMemory)
{
    assert(ObjectMemory != nullptr);

    std::uint32_t Index = 0;
    if (!FreeIndices.empty())
    {
        Index = FreeIndices.back();
        FreeIndices.pop_back();
    }
    else
    {
        assert(Objects.size() < FObjectHandle::InvalidIndex);
        Index = static_cast<std::uint32_t>(Objects.size());
        Objects.emplace_back();
    }

    FUObjectItem& Item = Objects[Index];
    assert(Item.Object == nullptr);
    Item.Object = ObjectMemory;
    ++ObjectCount;
    return FObjectHandle{Index, Item.SerialNumber};
}

void FUObjectArray::FreeObjectIndex(FObjectHandle Handle, const UObject* ExpectedObject)
{
    if (Handle.Index >= Objects.size())
    {
        assert(false);
        return;
    }

    FUObjectItem& Item = Objects[Handle.Index];
    if (Item.SerialNumber != Handle.SerialNumber || Item.Object != ExpectedObject)
    {
        assert(false);
        return;
    }

    Item.Object = nullptr;
    ++Item.SerialNumber;
    if (Item.SerialNumber == 0)
    {
        ++Item.SerialNumber;
    }
    FreeIndices.push_back(Handle.Index);
    assert(ObjectCount > 0);
    --ObjectCount;
}

UObject* FUObjectArray::Resolve(FObjectHandle Handle) const
{
    if (!Handle.IsSet() || Handle.Index >= Objects.size())
    {
        return nullptr;
    }

    const FUObjectItem& Item = Objects[Handle.Index];
    if (Item.SerialNumber != Handle.SerialNumber)
    {
        return nullptr;
    }

    return Item.Object;
}

bool FUObjectArray::IsValid(FObjectHandle Handle) const
{
    const UObject* Object = Resolve(Handle);
    return Object != nullptr && Object->GetState() == EObjectState::Alive;
}

std::size_t FUObjectArray::GetObjectCount() const
{
    return ObjectCount;
}
