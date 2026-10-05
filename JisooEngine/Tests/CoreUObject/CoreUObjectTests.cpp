#include "Fixtures/TestObjectTypes.h"
#include "Framework/TestRunner.h"

#include "Runtime/CoreUObject/Class.h"
#include "Runtime/CoreUObject/ObjectGlobals.h"

void RegisterCoreUObjectTests(FTestRunner& Runner)
{
    Runner.Add(
        "CoreUObject.ObjectCarriesRuntimeIdentity",
        [](FTestContext& Test)
        {
            UObject* BaseObject = NewObject<UObject>(nullptr, "Base");
            ULifecycleTestObject* DerivedObject =
                NewObject<ULifecycleTestObject>(BaseObject, "Derived");

            Test.Expect(BaseObject->GetClass() == UObject::StaticClass(), "UObject의 런타임 Class가 일치해야 한다.");
            Test.Expect(
                DerivedObject->GetClass() == ULifecycleTestObject::StaticClass(),
                "파생 객체의 런타임 Class가 일치해야 한다.");
            Test.Expect(DerivedObject->GetOuter() == BaseObject, "생성 시 지정한 Outer가 보존되어야 한다.");
            Test.Expect(DerivedObject->GetPathName() == "Base.Derived", "객체 경로가 Outer 체인을 따라야 한다.");

            DestroyObject(DerivedObject);
            DestroyObject(BaseObject);
            FlushPendingDestroyObjects();
        });

    Runner.Add(
        "CoreUObject.CastFollowsClassHierarchy",
        [](FTestContext& Test)
        {
            UObject* BaseObject = NewObject<UObject>(nullptr, "Base");
            ULifecycleTestObject* DerivedObject =
                NewObject<ULifecycleTestObject>(nullptr, "Derived");

            Test.Expect(BaseObject->IsA(UObject::StaticClass()), "UObject는 자기 Class에 속해야 한다.");
            Test.Expect(!BaseObject->IsA(ULifecycleTestObject::StaticClass()), "기반 객체를 파생 Class로 판정하면 안 된다.");
            Test.Expect(DerivedObject->IsA(UObject::StaticClass()), "파생 객체는 기반 Class에 속해야 한다.");
            Test.Expect(Cast<ULifecycleTestObject>(BaseObject) == nullptr, "잘못된 Cast는 nullptr을 반환해야 한다.");
            Test.Expect(Cast<ULifecycleTestObject>(DerivedObject) == DerivedObject, "유효한 Cast는 같은 객체를 반환해야 한다.");

            DestroyObject(DerivedObject);
            DestroyObject(BaseObject);
            FlushPendingDestroyObjects();
        });

    Runner.Add(
        "CoreUObject.PendingDestroyStopsPublicResolve",
        [](FTestContext& Test)
        {
            ULifecycleTestObject* Object =
                NewObject<ULifecycleTestObject>(nullptr, "PendingDestroy");
            const FObjectHandle Handle = Object->GetHandle();

            Test.Expect(IsValid(Object), "새 객체는 공개 API에서 유효해야 한다.");
            Test.Expect(ResolveObject(Handle) == Object, "살아 있는 Handle은 객체로 해석되어야 한다.");
            DestroyObject(Object);
            Test.Expect(!IsValid(Object), "파괴 대기 객체는 즉시 유효하지 않아야 한다.");
            Test.Expect(ResolveObject(Handle) == nullptr, "파괴 대기 Handle은 공개 API에서 해석되지 않아야 한다.");

            FlushPendingDestroyObjects();
        });

    Runner.Add(
        "CoreUObject.DestroyHooksRunExactlyOnce",
        [](FTestContext& Test)
        {
            ULifecycleTestObject::ResetCounters();
            ULifecycleTestObject* Object =
                NewObject<ULifecycleTestObject>(nullptr, "DestroyHooks");

            DestroyObject(Object);
            DestroyObject(Object);
            FlushPendingDestroyObjects();

            Test.Expect(ULifecycleTestObject::GetBeginDestroyCount() == 1, "BeginDestroy는 한 번만 호출되어야 한다.");
            Test.Expect(ULifecycleTestObject::GetFinishDestroyCount() == 1, "FinishDestroy는 한 번만 호출되어야 한다.");
            Test.Expect(ULifecycleTestObject::GetDestructorCount() == 1, "소멸자는 한 번만 호출되어야 한다.");
        });

    Runner.Add(
        "CoreUObject.StaleHandleCannotResolveReusedSlot",
        [](FTestContext& Test)
        {
            ULifecycleTestObject* Original =
                NewObject<ULifecycleTestObject>(nullptr, "Original");
            const FObjectHandle OriginalHandle = Original->GetHandle();
            DestroyObject(Original);
            FlushPendingDestroyObjects();

            ULifecycleTestObject* Replacement =
                NewObject<ULifecycleTestObject>(nullptr, "Replacement");
            const FObjectHandle ReplacementHandle = Replacement->GetHandle();

            Test.Expect(ReplacementHandle.Index == OriginalHandle.Index, "해제된 슬롯을 재사용해야 한다.");
            Test.Expect(
                ReplacementHandle.SerialNumber != OriginalHandle.SerialNumber,
                "재사용된 슬롯은 새로운 Serial을 받아야 한다.");
            Test.Expect(ResolveObject(OriginalHandle) == nullptr, "이전 Serial로 새 객체를 해석하면 안 된다.");

            DestroyObject(Replacement);
            FlushPendingDestroyObjects();
        });
}
