#pragma once

#include "Runtime/CoreUObject/Object.h"
#include "Runtime/CoreUObject/ObjectGlobals.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Runtime/Engine/World.generated.h"

class AActor;

UCLASS()
/**
 * Actor의 생성·삭제·시작·종료와 프레임 Tick을 관리하는 Game Scene 컨테이너다.
 * Tick 도중 생성된 Actor는 다음 프레임부터 Tick하며 파괴 요청된 Actor는 즉시 순회에서 제외한다.
 */
class UWorld : public UObject
{
    GENERATED_BODY()

public:
    template<typename TActor>
    [[nodiscard]] TActor* SpawnActor(std::string Name = {})
    {
        static_assert(std::is_base_of_v<AActor, TActor>);

        TActor* Actor = NewObject<TActor>(this, std::move(Name));
        AddActor(Actor);
        return Actor;
    }

    /** 소유한 Actor를 즉시 Tick 컨테이너에서 제외하고 실제 메모리 해제를 예약한다. */
    bool DestroyActor(AActor* Actor);

    [[nodiscard]] std::vector<AActor*> GetActors() const;
    [[nodiscard]] bool HasBegunPlay() const;

    void BeginPlay();
    void Tick(float DeltaSeconds);
    void EndPlay();

    void BeginDestroy() override;
    [[nodiscard]] bool IsReadyForFinishDestroy() const override;

protected:
    explicit UWorld(const FObjectInitializer& ObjectInitializer);
    ~UWorld() override;

private:
    void AddActor(AActor* Actor);
    void RouteWorldDestroyed();

    std::vector<FObjectHandle> Actors;
    std::vector<FObjectHandle> DestroyingActors;
    bool bHasBegunPlay = false;
    bool bWorldDestroyRouted = false;
};
