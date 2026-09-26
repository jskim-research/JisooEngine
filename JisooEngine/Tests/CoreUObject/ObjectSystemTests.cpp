#include "CoreUObject/ExampleObject.h"

#include "Runtime/CoreUObject/Class.h"
#include "Runtime/CoreUObject/ObjectArray.h"
#include "Runtime/CoreUObject/ObjectGlobals.h"

#include <iostream>
#include <string_view>

namespace
{
    int FailureCount = 0;

    void Expect(bool Condition, std::string_view Message)
    {
        if (Condition)
        {
            return;
        }

        ++FailureCount;
        std::cerr << "FAILED: " << Message << '\n';
    }
}

int main()
{
    UExampleObject::ResetCounters();

    UObject* BaseObject = NewObject<UObject>(nullptr, "Base");
    UExampleObject* DerivedObject = NewObject<UExampleObject>(BaseObject, "Derived");

    Expect(BaseObject->GetClass() == UObject::StaticClass(), "UObject runtime class");
    Expect(
        DerivedObject->GetClass() == UExampleObject::StaticClass(),
        "derived runtime class");
    Expect(BaseObject->IsA(UObject::StaticClass()), "UObject IsA UObject");
    Expect(!BaseObject->IsA(UExampleObject::StaticClass()), "UObject is not derived");
    Expect(DerivedObject->IsA(UObject::StaticClass()), "derived IsA UObject");
    Expect(
        DerivedObject->IsA(UExampleObject::StaticClass()),
        "derived IsA own class");
    Expect(Cast<UExampleObject>(BaseObject) == nullptr, "invalid Cast returns nullptr");
    Expect(Cast<UExampleObject>(DerivedObject) == DerivedObject, "valid Cast succeeds");
    Expect(DerivedObject->GetOuter() == BaseObject, "Outer is preserved");
    Expect(DerivedObject->GetPathName() == "Base.Derived", "path follows Outer chain");

    const FObjectHandle OldHandle = DerivedObject->GetHandle();
    Expect(IsValid(DerivedObject), "new object is valid");
    Expect(ResolveObject(OldHandle) == DerivedObject, "handle resolves live object");

    DestroyObject(DerivedObject);
    DestroyObject(DerivedObject);
    Expect(!IsValid(DerivedObject), "pending destroy object is invalid");
    Expect(ResolveObject(OldHandle) == nullptr, "pending destroy handle does not resolve");
    FlushPendingDestroyObjects();

    Expect(UExampleObject::GetBeginDestroyCount() == 1, "BeginDestroy runs once");
    Expect(UExampleObject::GetFinishDestroyCount() == 1, "FinishDestroy runs once");
    Expect(UExampleObject::GetDestructorCount() == 1, "destructor runs once");
    Expect(!IsValid(OldHandle), "destroyed handle is stale");

    UExampleObject* Replacement = NewObject<UExampleObject>(BaseObject, "Replacement");
    const FObjectHandle ReplacementHandle = Replacement->GetHandle();
    Expect(ReplacementHandle.Index == OldHandle.Index, "freed slot is reused");
    Expect(
        ReplacementHandle.SerialNumber != OldHandle.SerialNumber,
        "reused slot receives a new serial");
    Expect(ResolveObject(OldHandle) == nullptr, "old serial cannot resolve replacement");

    DestroyObject(Replacement);
    FlushPendingDestroyObjects();
    DestroyObject(BaseObject);
    FlushPendingDestroyObjects();
    Expect(GUObjectArray.GetObjectCount() == 0, "all test objects are released");

    if (FailureCount != 0)
    {
        std::cerr << FailureCount << " object system test(s) failed.\n";
        return 1;
    }

    std::cout << "Object system tests passed.\n";
    return 0;
}
