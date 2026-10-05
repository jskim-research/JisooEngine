#include "Fixtures/TestObjectTypes.h"
#include "Framework/TestRunner.h"

#include "Runtime/CoreUObject/ObjectGlobals.h"
#include "Runtime/Engine/Engine.h"
#include "Runtime/Engine/World.h"

void RegisterEngineLifecycleTests(FTestRunner& Runner)
{
    Runner.Add(
        "EngineLifecycle.ActiveWorldPropagatesBeginPlay",
        [](FTestContext& Test)
        {
            ULifecycleTestActor::ResetCounters();
            ULifecycleTestComponent::ResetCounters();
            FEngine Engine;
            UWorld* World = Engine.CreateWorld("BeginPlayWorld");
            World->BeginPlay();

            ULifecycleTestActor* Actor =
                World->SpawnActor<ULifecycleTestActor>("Actor");
            ULifecycleTestComponent* Component =
                Actor->AddComponent<ULifecycleTestComponent>("Component");

            Test.Expect(Actor->GetWorld() == World, "Actor가 생성된 World를 반환해야 한다.");
            Test.Expect(Component->GetOwner() == Actor, "Component가 등록된 Actor를 반환해야 한다.");
            Test.Expect(Component->GetWorld() == World, "Component가 등록된 World를 반환해야 한다.");
            Test.Expect(Component->IsRegistered(), "Actor에 추가된 Component가 등록되어야 한다.");
            Test.Expect(ULifecycleTestActor::GetBeginPlayCount() == 1, "활성 World에 생성된 Actor는 즉시 BeginPlay해야 한다.");
            Test.Expect(ULifecycleTestComponent::GetBeginPlayCount() == 1, "시작된 Actor에 추가된 Component는 즉시 BeginPlay해야 한다.");

            Engine.Shutdown();
        });

    Runner.Add(
        "EngineLifecycle.EngineTickPropagatesToActorAndComponent",
        [](FTestContext& Test)
        {
            ULifecycleTestActor::ResetCounters();
            ULifecycleTestComponent::ResetCounters();
            FEngine Engine;
            UWorld* World = Engine.CreateWorld("TickWorld");
            World->BeginPlay();
            ULifecycleTestActor* Actor =
                World->SpawnActor<ULifecycleTestActor>("Actor");
            [[maybe_unused]] ULifecycleTestComponent* Component =
                Actor->AddComponent<ULifecycleTestComponent>("Component");

            Engine.Tick(0.25f);

            Test.Expect(ULifecycleTestActor::GetTickCount() == 1, "Engine Tick이 Actor까지 전달되어야 한다.");
            Test.Expect(ULifecycleTestComponent::GetTickCount() == 1, "Actor Tick이 Component까지 전달되어야 한다.");
            Engine.Shutdown();
        });

    Runner.Add(
        "EngineLifecycle.SpawnDuringTickStartsNextTick",
        [](FTestContext& Test)
        {
            ULifecycleTestActor::ResetCounters();
            FEngine Engine;
            UWorld* World = Engine.CreateWorld("SpawnDuringTickWorld");
            World->BeginPlay();
            [[maybe_unused]] ULifecycleTestActor* Original =
                World->SpawnActor<ULifecycleTestActor>("Original");

            ULifecycleTestActor::RequestSpawnOnNextTick();
            Engine.Tick(0.25f);
            Test.Expect(World->GetActors().size() == 2, "Tick 중 생성된 Actor가 World 컨테이너에 추가되어야 한다.");
            Test.Expect(ULifecycleTestActor::GetTickCount() == 1, "Tick 중 생성된 Actor는 같은 Tick에 실행되면 안 된다.");

            Engine.Tick(0.25f);
            Test.Expect(ULifecycleTestActor::GetTickCount() == 3, "새 Actor는 다음 Tick부터 기존 Actor와 함께 실행되어야 한다.");
            Engine.Shutdown();
        });

    Runner.Add(
        "EngineLifecycle.DestroyDuringTickLeavesTraversalAndFlushesHierarchy",
        [](FTestContext& Test)
        {
            ULifecycleTestActor::ResetCounters();
            FEngine Engine;
            UWorld* World = Engine.CreateWorld("DestroyDuringTickWorld");
            World->BeginPlay();
            ULifecycleTestActor* Actor =
                World->SpawnActor<ULifecycleTestActor>("Actor");
            ULifecycleTestComponent* Component =
                Actor->AddComponent<ULifecycleTestComponent>("Component");
            const FObjectHandle ActorHandle = Actor->GetHandle();
            const FObjectHandle ComponentHandle = Component->GetHandle();

            ULifecycleTestActor::RequestDestroySelfOnNextTick();
            Engine.Tick(0.25f);

            Test.Expect(World->GetActors().empty(), "파괴 요청된 Actor는 즉시 World 순회 대상에서 제외되어야 한다.");
            Test.Expect(!IsObjectAllocated(ActorHandle), "프레임 말 Flush가 Actor 메모리를 해제해야 한다.");
            Test.Expect(!IsObjectAllocated(ComponentHandle), "Actor의 Component 계층도 함께 해제되어야 한다.");
            Engine.Shutdown();
        });

    Runner.Add(
        "EngineLifecycle.DestroyComponentInvalidatesBeforeFlushAndFreesAfterFlush",
        [](FTestContext& Test)
        {
            FEngine Engine;
            UWorld* World = Engine.CreateWorld("DestroyComponentWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            UActorComponent* Component =
                Actor->AddComponent<UActorComponent>("Component");
            const FObjectHandle ComponentHandle = Component->GetHandle();

            Test.Expect(Actor->DestroyComponent(Component), "Actor가 소유한 Component 파괴 요청을 받아들여야 한다.");
            Test.Expect(!IsValid(ComponentHandle), "파괴 요청된 Component는 Flush 전에도 공개 API에서 무효여야 한다.");
            Test.Expect(IsObjectAllocated(ComponentHandle), "Flush 전에는 Component 메모리가 유지되어야 한다.");
            Engine.Tick(0.0f);
            Test.Expect(!IsObjectAllocated(ComponentHandle), "Engine Tick의 Flush가 Component 메모리를 해제해야 한다.");
            Engine.Shutdown();
        });

    Runner.Add(
        "EngineLifecycle.UnregisterKeepsWorldWhileOwnerIsPendingDestroy",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "OwnerDestroyWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            [[maybe_unused]] UWorldTrackingComponent* TrackingComponent =
                Actor->AddComponent<UWorldTrackingComponent>("TrackingComponent");
            UWorldTrackingComponent::ResetUnregisterObservation(World);

            DestroyObject(Actor);
            FlushPendingDestroyObjects();

            Test.Expect(UWorldTrackingComponent::GetUnregisterCount() == 1, "Owner 파괴 중 OnUnregister가 한 번 호출되어야 한다.");
            Test.Expect(UWorldTrackingComponent::ObservedExpectedWorld(), "Owner가 파괴 대기 상태여도 OnUnregister에서 등록 World를 반환해야 한다.");
            DestroyObject(World);
            FlushPendingDestroyObjects();
        });

    Runner.Add(
        "EngineLifecycle.UnregisterKeepsWorldWhileWorldIsPendingDestroy",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "WorldDestroyWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            [[maybe_unused]] UWorldTrackingComponent* TrackingComponent =
                Actor->AddComponent<UWorldTrackingComponent>("TrackingComponent");
            UWorldTrackingComponent::ResetUnregisterObservation(World);

            DestroyObject(World);
            FlushPendingDestroyObjects();

            Test.Expect(UWorldTrackingComponent::GetUnregisterCount() == 1, "World 연쇄 파괴 중 OnUnregister가 한 번 호출되어야 한다.");
            Test.Expect(UWorldTrackingComponent::ObservedExpectedWorld(), "World가 파괴 대기 상태여도 OnUnregister에서 등록 World를 반환해야 한다.");
        });

    Runner.Add(
        "EngineLifecycle.CascadeDestroyWaitsForChildren",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "CascadeWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            UActorComponent* Component =
                Actor->AddComponent<UActorComponent>("Component");
            const FObjectHandle WorldHandle = World->GetHandle();
            const FObjectHandle ActorHandle = Actor->GetHandle();
            const FObjectHandle ComponentHandle = Component->GetHandle();

            DestroyObject(World);
            FlushPendingDestroyObjects();

            Test.Expect(!IsObjectAllocated(ComponentHandle), "Component가 부모보다 먼저 정리되어야 한다.");
            Test.Expect(!IsObjectAllocated(ActorHandle), "자식 Component 정리 뒤 Actor가 해제되어야 한다.");
            Test.Expect(!IsObjectAllocated(WorldHandle), "전체 자식 계층 정리 뒤 World가 해제되어야 한다.");
        });
}
