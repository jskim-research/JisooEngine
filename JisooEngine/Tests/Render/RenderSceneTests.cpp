#include "Fixtures/TestObjectTypes.h"
#include "Framework/TestRunner.h"

#include "Runtime/CoreUObject/ObjectGlobals.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/Components/PrimitiveComponent.h"
#include "Runtime/Engine/Components/SceneComponent.h"
#include "Runtime/Engine/World.h"
#include "Runtime/Render/Scene/PrimitiveSceneProxy.h"
#include "Runtime/Render/Scene/Scene.h"

#include <cmath>
#include <memory>
#include <type_traits>
#include <utility>

static_assert(std::is_same_v<
    decltype(std::declval<const FScene&>().ResolvePrimitive(
        std::declval<FPrimitiveSceneHandle>())),
    const FPrimitiveSceneProxy*>);

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

    std::unique_ptr<FPrimitiveSceneProxy> MakeProxy()
    {
        return std::make_unique<FPrimitiveSceneProxy>(FPrimitiveSceneDescription{});
    }
}

void RegisterRenderSceneTests(FTestRunner& Runner)
{
    Runner.Add(
        "RenderScene.RegistersPrimitiveProxy",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "RegisterProxyWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            UPrimitiveComponent* Primitive =
                Actor->AddComponent<UPrimitiveComponent>("Primitive");
            const FPrimitiveSceneHandle Handle = Primitive->GetPrimitiveSceneHandle();

            Test.Expect(Handle.IsSet(), "등록된 Primitive가 Scene Handle을 보관해야 한다.");
            Test.Expect(World->GetScene()->ResolvePrimitive(Handle) != nullptr, "Scene Handle이 등록된 Proxy를 해석해야 한다.");
            Test.Expect(World->GetScene()->GetPrimitiveCount() == 1, "World Scene이 Proxy 하나를 소유해야 한다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "RenderScene.ExposesReadOnlyProxyAccess",
        [](FTestContext& Test)
        {
            FScene Scene;
            const FPrimitiveSceneHandle Handle = Scene.AddPrimitive(MakeProxy());
            const FPrimitiveSceneProxy* ResolvedProxy = Scene.ResolvePrimitive(Handle);
            std::size_t VisitCount = 0;
            const FPrimitiveSceneProxy* VisitedProxy = nullptr;

            Scene.ForEachPrimitive(
                [&VisitCount, &VisitedProxy](const FPrimitiveSceneProxy& Proxy)
                {
                    ++VisitCount;
                    VisitedProxy = &Proxy;
                });

            Test.Expect(VisitCount == 1, "등록된 Proxy만 한 번 순회해야 한다.");
            Test.Expect(VisitedProxy == ResolvedProxy, "순회가 Resolve와 같은 읽기 전용 Proxy를 제공해야 한다.");
            Scene.RemovePrimitive(Handle);
        });

    Runner.Add(
        "RenderScene.PropagatesFlags",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "FlagsWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            UPrimitiveComponent* Primitive =
                Actor->AddComponent<UPrimitiveComponent>("Primitive");
            const FPrimitiveSceneProxy* Proxy = World->GetScene()->ResolvePrimitive(
                Primitive->GetPrimitiveSceneHandle());

            Primitive->SetVisibility(false);
            Primitive->SetCastShadow(false);

            Test.Expect(Proxy != nullptr && !Proxy->IsVisible(), "Visibility 변경이 Proxy에 동기 반영되어야 한다.");
            Test.Expect(Proxy != nullptr && !Proxy->CastsShadow(), "Cast Shadow 변경이 Proxy에 동기 반영되어야 한다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "RenderScene.PropagatesTransformAndBounds",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "TransformProxyWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            USceneComponent* Root = Actor->AddComponent<USceneComponent>("Root");
            UPrimitiveComponent* Primitive =
                Actor->AddComponent<UPrimitiveComponent>("Primitive");
            Primitive->AttachToComponent(Root);
            const FPrimitiveSceneProxy* Proxy = World->GetScene()->ResolvePrimitive(
                Primitive->GetPrimitiveSceneHandle());

            FTransform RootTransform;
            RootTransform.Translation = {10.0f, 0.0f, 0.0f};
            Root->SetRelativeTransform(RootTransform);
            FTransform PrimitiveTransform;
            PrimitiveTransform.Translation = {5.0f, 0.0f, 0.0f};
            Primitive->SetRelativeTransform(PrimitiveTransform);
            Primitive->SetLocalBounds({{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}});

            Test.Expect(Proxy != nullptr && NearlyEqual(Proxy->GetLocalToWorld().M[3][0], 15.0f), "합성된 World Transform이 Proxy에 반영되어야 한다.");
            Test.Expect(Proxy != nullptr && NearlyEqual(Proxy->GetWorldBounds().Min.X, 14.0f), "World Bounds 최솟값이 Proxy에 반영되어야 한다.");
            Test.Expect(Proxy != nullptr && NearlyEqual(Proxy->GetWorldBounds().Max.X, 16.0f), "World Bounds 최댓값이 Proxy에 반영되어야 한다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "RenderScene.RemoveInvalidatesHandle",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "RemoveProxyWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            UPrimitiveComponent* Primitive =
                Actor->AddComponent<UPrimitiveComponent>("Primitive");
            const FPrimitiveSceneHandle Handle = Primitive->GetPrimitiveSceneHandle();

            Test.Expect(Actor->DestroyComponent(Primitive), "Primitive 파괴 요청이 성공해야 한다.");
            Test.Expect(World->GetScene()->ResolvePrimitive(Handle) == nullptr, "제거된 Handle은 즉시 무효가 되어야 한다.");
            Test.Expect(World->GetScene()->GetPrimitiveCount() == 0, "Scene이 제거된 Proxy를 소유하면 안 된다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "RenderScene.ReusedSlotChangesGeneration",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "ReuseProxyWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            UPrimitiveComponent* Original =
                Actor->AddComponent<UPrimitiveComponent>("Original");
            const FPrimitiveSceneHandle OriginalHandle = Original->GetPrimitiveSceneHandle();
            Actor->DestroyComponent(Original);
            UPrimitiveComponent* Replacement =
                Actor->AddComponent<UPrimitiveComponent>("Replacement");
            const FPrimitiveSceneHandle ReplacementHandle = Replacement->GetPrimitiveSceneHandle();

            Test.Expect(ReplacementHandle.Index == OriginalHandle.Index, "해제된 Scene 슬롯을 재사용해야 한다.");
            Test.Expect(ReplacementHandle.Generation != OriginalHandle.Generation, "재사용된 Scene 슬롯은 새로운 Generation을 받아야 한다.");
            Test.Expect(World->GetScene()->ResolvePrimitive(OriginalHandle) == nullptr, "이전 Generation으로 새 Proxy를 해석하면 안 된다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "RenderScene.InvalidHandleCannotUpdateOrRemove",
        [](FTestContext& Test)
        {
            FScene Scene;
            const FPrimitiveSceneHandle Handle = Scene.AddPrimitive(MakeProxy());
            Scene.RemovePrimitive(Handle);

            Test.Expect(!Scene.UpdatePrimitiveTransform(Handle, {}), "제거된 Handle로 Transform을 갱신하면 안 된다.");
            Test.Expect(!Scene.UpdatePrimitiveBounds(Handle, {}), "제거된 Handle로 Bounds를 갱신하면 안 된다.");
            Test.Expect(!Scene.UpdatePrimitiveFlags(Handle, {}), "제거된 Handle로 Flags를 갱신하면 안 된다.");
            Test.Expect(!Scene.RemovePrimitive(Handle), "제거된 Handle을 다시 제거하면 안 된다.");
        });

    Runner.Add(
        "RenderScene.ActorDestroyRemovesProxy",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "ActorDestroyProxyWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            [[maybe_unused]] UPrimitiveComponent* Primitive =
                Actor->AddComponent<UPrimitiveComponent>("Primitive");
            Test.Expect(World->GetScene()->GetPrimitiveCount() == 1, "Actor 파괴 전에 Proxy가 등록되어 있어야 한다.");

            DestroyObject(Actor);
            FlushPendingDestroyObjects();

            Test.Expect(World->GetScene()->GetPrimitiveCount() == 0, "Actor 직접 파괴가 Component Proxy를 제거해야 한다.");
            DestroyTestWorld(World);
        });

    Runner.Add(
        "RenderScene.WorldDestroyRemovesProxy",
        [](FTestContext& Test)
        {
            UWorld* World = NewObject<UWorld>(nullptr, "WorldDestroyProxyWorld");
            AActor* Actor = World->SpawnActor<AActor>("Actor");
            [[maybe_unused]] UProxyTrackingPrimitiveComponent* Primitive =
                Actor->AddComponent<UProxyTrackingPrimitiveComponent>("Primitive");
            UProxyTrackingPrimitiveComponent::ResetUnregisterObservation(World);

            DestroyObject(World);
            FlushPendingDestroyObjects();

            Test.Expect(UProxyTrackingPrimitiveComponent::ObservedExpectedWorld(), "World 파괴 중에도 Proxy 제거에 등록 World를 사용해야 한다.");
            Test.Expect(UProxyTrackingPrimitiveComponent::GetPrimitiveCountBeforeUnregister() == 1, "OnUnregister 진입 전 Proxy가 Scene에 있어야 한다.");
            Test.Expect(UProxyTrackingPrimitiveComponent::GetPrimitiveCountAfterUnregister() == 0, "OnUnregister가 World 해제 전에 Proxy를 제거해야 한다.");
        });
}
