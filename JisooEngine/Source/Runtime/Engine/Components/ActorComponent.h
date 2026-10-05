#pragma once

#include "Runtime/CoreUObject/Object.h"

#include "Runtime/Engine/Components/ActorComponent.generated.h"

class AActor;
class UWorld;

UCLASS()
/**
 * Actor에 부착되는 비공간 기능의 생명주기와 Tick 진입점을 제공한다.
 * Owner는 이 Component보다 오래 살아야 하며 실제 소유 관계는 Actor의 Component 컨테이너가 관리한다.
 */
class UActorComponent : public UObject
{
    GENERATED_BODY()

public:
    [[nodiscard]] AActor* GetOwner() const;

    /** 등록 중에는 실제 등록 World를 반환하고, 미등록 상태에서는 Owner의 World를 반환한다. */
    [[nodiscard]] UWorld* GetWorld() const;
    [[nodiscard]] bool IsRegistered() const;
    [[nodiscard]] bool HasBegunPlay() const;
    [[nodiscard]] bool IsComponentTickEnabled() const;
    void SetComponentTickEnabled(bool bEnabled);

    virtual void TickComponent(float DeltaSeconds);
    virtual void EndPlay();

    void BeginDestroy() override;

protected:
    explicit UActorComponent(const FObjectInitializer& ObjectInitializer);
    ~UActorComponent() override;

    /** DispatchBeginPlay가 등록 상태와 중복 호출을 확인한 뒤 호출하는 사용자 확장 지점이다. */
    virtual void BeginPlay();

    virtual void OnRegister();

    /** 등록 World가 유지된 상태에서 호출되며 반환한 뒤 Component의 등록 상태가 해제된다. */
    virtual void OnUnregister();

private:
    void RegisterComponent(AActor* InOwner);
    void UnregisterComponent();
    void DispatchBeginPlay();
    void DispatchTick(float DeltaSeconds);
    void DispatchEndPlay();

    FObjectHandle OwnerHandle;
    UWorld* RegisteredWorld = nullptr;
    bool bRegistered = false;
    bool bHasBegunPlay = false;
    bool bTickEnabled = true;

    friend class AActor;
};
