#include "Framework/TestRunner.h"

#include "Runtime/CoreUObject/ObjectGlobals.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/Components/PrimitiveComponent.h"
#include "Runtime/Engine/Components/SceneComponent.h"
#include "Runtime/Engine/World.h"

#include <cmath>

namespace
{
    bool NearlyEqual(float Left, float Right)
    {
        return std::abs(Left - Right) < 0.0001f;
    }

    void DestroyTestWorld(UWorld* World)
    {
        DestroyObject(World);
        FlushPendingDestroyObjects();
    }
}

void RegisterSceneComponentTests(FTestRunner& Runner)
{
    Runner.Add(
        "SceneComponent.FirstSceneComponentBecomesRoot",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "RootWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            UActorComponent* NonSceneComponent =
                Actor->AddComponent<UActorComponent>("NonScene");
            USceneComponent* FirstSceneComponent =
                Actor->AddComponent<USceneComponent>("FirstScene");
            USceneComponent* SecondSceneComponent =
                Actor->AddComponent<USceneComponent>("SecondScene");

            Test.Expect(NonSceneComponent != nullptr, "비공간 Component도 정상 등록되어야 한다.");
            Test.Expect(Actor->GetRootComponent() == FirstSceneComponent, "처음 추가된 SceneComponent가 Root가 되어야 한다.");
            Test.Expect(Actor->GetRootComponent() != SecondSceneComponent, "후속 SceneComponent가 Root를 덮어쓰면 안 된다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "SceneComponent.RejectsCrossOwnerAttachment",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "CrossOwnerWorld");
            AActor* FirstActor = World->SpawnActor<AActor>("FirstActor");
            AActor* SecondActor = World->SpawnActor<AActor>("SecondActor");
            USceneComponent* FirstComponent =
                FirstActor->AddComponent<USceneComponent>("FirstComponent");
            USceneComponent* SecondComponent =
                SecondActor->AddComponent<USceneComponent>("SecondComponent");

            Test.Expect(!SecondComponent->AttachToComponent(FirstComponent), "서로 다른 Actor의 Component를 부착하면 안 된다.");
            Test.Expect(SecondComponent->GetAttachParent() == nullptr, "거부된 부착은 부모 Handle을 변경하면 안 된다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "SceneComponent.RejectsAttachmentCycle",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "CycleWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            USceneComponent* Parent =
                Actor->AddComponent<USceneComponent>("Parent");
            USceneComponent* Child =
                Actor->AddComponent<USceneComponent>("Child");

            Test.Expect(Child->AttachToComponent(Parent), "같은 Actor의 Component 부착은 성공해야 한다.");
            Test.Expect(!Parent->AttachToComponent(Child), "부착 계층에 순환을 만들면 안 된다.");
            Test.Expect(Child->GetAttachParent() == Parent, "거부된 순환 부착이 기존 계층을 변경하면 안 된다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "SceneComponent.PropagatesParentTransform",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "TransformWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            USceneComponent* Parent =
                Actor->AddComponent<USceneComponent>("Parent");
            USceneComponent* Child =
                Actor->AddComponent<USceneComponent>("Child");
            Child->AttachToComponent(Parent);

            FTransform ParentTransform;
            ParentTransform.Translation = {10.0f, 0.0f, 0.0f};
            Parent->SetRelativeTransform(ParentTransform);
            FTransform ChildTransform;
            ChildTransform.Translation = {5.0f, 0.0f, 0.0f};
            Child->SetRelativeTransform(ChildTransform);

            Test.Expect(
                NearlyEqual(Child->GetComponentToWorld().M[3][0], 15.0f),
                "행벡터 Local * ParentWorld 규칙으로 부모 Translation이 합성되어야 한다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "SceneComponent.UpdatesWorldBounds",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "BoundsWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            UPrimitiveComponent* Primitive =
                Actor->AddComponent<UPrimitiveComponent>("Primitive");
            FTransform Transform;
            Transform.Translation = {10.0f, 0.0f, 0.0f};
            Primitive->SetRelativeTransform(Transform);
            Primitive->SetLocalBounds({{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}});

            const FBox& WorldBounds = Primitive->GetWorldBounds();
            Test.Expect(NearlyEqual(WorldBounds.Min.X, 9.0f), "World Bounds 최솟값에 Component Transform이 반영되어야 한다.");
            Test.Expect(NearlyEqual(WorldBounds.Max.X, 11.0f), "World Bounds 최댓값에 Component Transform이 반영되어야 한다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "SceneComponent.DestroyDetachesChildren",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "DetachWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            USceneComponent* Parent =
                Actor->AddComponent<USceneComponent>("Parent");
            USceneComponent* Child =
                Actor->AddComponent<USceneComponent>("Child");
            Child->AttachToComponent(Parent);

            Test.Expect(Actor->DestroyComponent(Parent), "부모 Component 파괴 요청이 성공해야 한다.");
            Test.Expect(Child->GetAttachParent() == nullptr, "부모 파괴 요청 시 자식이 즉시 분리되어야 한다.");
            Test.Expect(Actor->GetRootComponent() == nullptr, "Root Component 파괴 시 Actor의 Root Handle이 비워져야 한다.");
            DestroyTestWorld(World);
        });
}
