#pragma once

#include "Runtime/CoreUObject/Object.h"
#include "Runtime/CoreUObject/ObjectGlobals.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Runtime/Engine/Actor.generated.h"

class UActorComponent;
class UPrimitiveComponent;
class USceneComponent;
class UWorld;

UCLASS()
/**
 * World에 배치되는 게임 객체이며 자신에게 속한 Component의 생명주기를 관리한다.
 * Component 메모리는 객체 시스템이 해제하고 Actor는 Handle 컨테이너로 논리적 소유 관계를 유지한다.
 */
class AActor : public UObject
{
    GENERATED_BODY()

public:
    [[nodiscard]] UWorld* GetWorld() const;

    template<typename TComponent>
    [[nodiscard]] TComponent* AddComponent(std::string Name = {})
    {
        static_assert(std::is_base_of_v<UActorComponent, TComponent>);

        TComponent* Component = NewObject<TComponent>(this, std::move(Name));
        if (!AddOwnedComponent(Component))
        {
            DestroyObject(Component);
            return nullptr;
        }
        return Component;
    }

    /** 소유한 Component를 즉시 컨테이너에서 제외하고 실제 메모리 해제를 예약한다. */
    bool DestroyComponent(UActorComponent* Component);

    [[nodiscard]] std::vector<UActorComponent*> GetComponents() const;

    template<typename TComponent>
    [[nodiscard]] TComponent* GetComponentByClass() const
    {
        static_assert(std::is_base_of_v<UActorComponent, TComponent>);
        for (UActorComponent* Component : GetComponents())
        {
            if (TComponent* TypedComponent = Cast<TComponent>(Component))
            {
                return TypedComponent;
            }
        }
        return nullptr;
    }

    bool SetRootComponent(USceneComponent* Component);
    [[nodiscard]] USceneComponent* GetRootComponent() const;
    [[nodiscard]] bool HasBegunPlay() const;

    virtual void Tick(float DeltaSeconds);
    virtual void EndPlay();

    void BeginDestroy() override;
    [[nodiscard]] bool IsReadyForFinishDestroy() const override;

protected:
    explicit AActor(const FObjectInitializer& ObjectInitializer);
    ~AActor() override;

    /** DispatchBeginPlay가 생명주기 상태와 Component 순서를 설정한 뒤 호출하는 사용자 확장 지점이다. */
    virtual void BeginPlay();

private:
    bool AddOwnedComponent(UActorComponent* Component);
    [[nodiscard]] bool OwnsComponent(const UActorComponent* Component) const;
    void DispatchBeginPlay();
    void DispatchTick(float DeltaSeconds);
    void DispatchEndPlay();
    void RouteActorDestroyed();

    std::vector<FObjectHandle> OwnedComponents;
    std::vector<FObjectHandle> DestroyingComponents;
    FObjectHandle RootComponentHandle;
    bool bHasBegunPlay = false;
    bool bActorDestroyRouted = false;

    friend class UWorld;
};
