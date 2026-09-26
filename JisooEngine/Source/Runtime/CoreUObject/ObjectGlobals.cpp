#include "Runtime/CoreUObject/ObjectGlobals.h"

#include "Runtime/CoreUObject/Class.h"
#include "Runtime/CoreUObject/ObjectArray.h"
#include "Runtime/CoreUObject/ObjectInitializer.h"

#include <atomic>
#include <cassert>
#include <new>
#include <utility>
#include <vector>

namespace
{
    std::vector<FObjectHandle> GPendingDestroyObjects;
    std::atomic_uint64_t GGeneratedObjectName{0};

    void* AllocateObjectMemory(const UClass& Class)
    {
        if (Class.GetClassAlignment() > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
        {
            return ::operator new(
                Class.GetClassSize(),
                std::align_val_t(Class.GetClassAlignment()));
        }
        return ::operator new(Class.GetClassSize());
    }

    void FreeObjectMemory(void* Memory, const UClass& Class)
    {
        if (Class.GetClassAlignment() > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
        {
            ::operator delete(Memory, std::align_val_t(Class.GetClassAlignment()));
            return;
        }
        ::operator delete(Memory);
    }

    std::string MakeObjectName(const UClass& Class, std::string RequestedName)
    {
        if (!RequestedName.empty())
        {
            return RequestedName;
        }

        const std::uint64_t NameIndex = GGeneratedObjectName.fetch_add(1);
        return std::string(Class.GetName()) + "_" + std::to_string(NameIndex);
    }
}

UObject* StaticConstructObject(UClass* Class, UObject* Outer, std::string Name)
{
    if (Class == nullptr || !Class->IsChildOf(UObject::StaticClass()))
    {
        return nullptr;
    }

    void* Memory = AllocateObjectMemory(*Class);
    UObject* ObjectMemory = static_cast<UObject*>(Memory);
    const FObjectHandle Handle = GUObjectArray.AllocateObjectIndex(ObjectMemory);
    FObjectInitializer Initializer(
        Class,
        Outer,
        MakeObjectName(*Class, std::move(Name)),
        Handle);

    try
    {
        Class->Construct(Memory, Initializer);
    }
    catch (...)
    {
        GUObjectArray.FreeObjectIndex(Handle, ObjectMemory);
        FreeObjectMemory(Memory, *Class);
        throw;
    }

    UObject* Object = static_cast<UObject*>(Memory);
    Object->SetState(EObjectState::Alive);
    return Object;
}

void DestroyObject(UObject* Object)
{
    if (!IsValid(Object))
    {
        return;
    }

    Object->SetState(EObjectState::PendingDestroy);
    GPendingDestroyObjects.push_back(Object->GetHandle());
}

void FlushPendingDestroyObjects()
{
    while (!GPendingDestroyObjects.empty())
    {
        std::vector<FObjectHandle> CurrentBatch;
        CurrentBatch.swap(GPendingDestroyObjects);
        std::size_t DestroyedObjectCount = 0;

        for (const FObjectHandle Handle : CurrentBatch)
        {
            UObject* Object = GUObjectArray.Resolve(Handle);
            if (Object == nullptr)
            {
                continue;
            }

            if (Object->GetState() == EObjectState::PendingDestroy)
            {
                Object->BeginDestroy();
                Object->SetState(EObjectState::BeginDestroyed);
            }

            if (!Object->IsReadyForFinishDestroy())
            {
                GPendingDestroyObjects.push_back(Handle);
                continue;
            }

            Object->FinishDestroy();
            Object->SetState(EObjectState::FinishDestroyed);

            UClass* Class = Object->GetClass();
            assert(Class != nullptr);
            Class->Destroy(Object);
            GUObjectArray.FreeObjectIndex(Handle, Object);
            FreeObjectMemory(Object, *Class);
            ++DestroyedObjectCount;
        }

        // 비동기 정리를 기다리는 객체만 남았다면 다음 프레임의 Flush에서 다시 확인한다.
        if (DestroyedObjectCount == 0
            && GPendingDestroyObjects.size() == CurrentBatch.size())
        {
            break;
        }
    }
}

bool IsValid(const UObject* Object)
{
    return Object != nullptr
        && Object->GetState() == EObjectState::Alive
        && GUObjectArray.Resolve(Object->GetHandle()) == Object;
}

bool IsValid(FObjectHandle Handle)
{
    return GUObjectArray.IsValid(Handle);
}

bool IsObjectAllocated(FObjectHandle Handle)
{
    return GUObjectArray.Resolve(Handle) != nullptr;
}

UObject* ResolveObject(FObjectHandle Handle)
{
    return IsValid(Handle) ? GUObjectArray.Resolve(Handle) : nullptr;
}
