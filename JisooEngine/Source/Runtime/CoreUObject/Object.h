#pragma once

#include "Runtime/CoreUObject/ObjectHandle.h"
#include "Runtime/CoreUObject/ObjectMacros.h"

#include <string>

#include "Runtime/CoreUObject/Object.generated.h"

class FObjectInitializer;
class FUObjectArray;
class UClass;

enum class EObjectState
{
    PendingConstruction,
    Alive,
    PendingDestroy,
    BeginDestroyed,
    FinishDestroyed,
};

UCLASS()
/**
 * 엔진이 추적하는 객체의 런타임 타입, 이름, Outer와 생애주기 상태를 보관한다.
 * Outer는 소유권을 만들지 않으므로 이 객체보다 오래 살아 있어야 한다.
 */
class UObject
{
    GENERATED_BODY()

public:
    [[nodiscard]] UClass* GetClass() const;
    [[nodiscard]] UObject* GetOuter() const;
    [[nodiscard]] const std::string& GetName() const;
    [[nodiscard]] std::string GetPathName() const;
    [[nodiscard]] FObjectHandle GetHandle() const;
    [[nodiscard]] EObjectState GetState() const;

    /** 이 객체의 런타임 클래스가 TargetClass와 같거나 그 파생 클래스인지 확인한다. */
    [[nodiscard]] bool IsA(const UClass* TargetClass) const;

    /** 외부 자원 해제를 시작한다. DestroyObject 처리 중 정확히 한 번 호출된다. */
    virtual void BeginDestroy();

    /** 비동기 해제가 끝나 FinishDestroy를 호출할 수 있으면 true를 반환한다. */
    [[nodiscard]] virtual bool IsReadyForFinishDestroy() const;

    /** 객체 메모리를 해제하기 직전 남은 외부 자원을 정리한다. */
    virtual void FinishDestroy();

protected:
    explicit UObject(const FObjectInitializer& ObjectInitializer);
    virtual ~UObject();

private:
    void SetState(EObjectState InState);

    UClass* ClassPrivate = nullptr;
    UObject* OuterPrivate = nullptr;
    std::string NamePrivate;
    FObjectHandle HandlePrivate;
    EObjectState State = EObjectState::PendingConstruction;

    friend class FUObjectArray;
    friend UObject* StaticConstructObject(UClass*, UObject*, std::string);
    friend void DestroyObject(UObject*);
    friend void FlushPendingDestroyObjects();
};
