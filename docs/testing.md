# 테스트 검증 목록

## CoreUObject

런타임 타입, 객체 식별, Handle 안전성과 지연 파괴 계약을 검증한다.

| 검증 항목 | 이유 |
|---|---|
| `CoreUObject.ObjectCarriesRuntimeIdentity` | 생성된 객체가 Class, Outer, 이름 경로를 일관되게 보존하는지 확인하기 위해 |
| `CoreUObject.CastFollowsClassHierarchy` | `IsA`와 `Cast`가 런타임 상속 관계 밖의 객체를 허용하지 않는지 확인하기 위해 |
| `CoreUObject.PendingDestroyStopsPublicResolve` | 파괴 요청된 객체가 공개 API에서 살아 있는 객체로 다시 해석되지 않도록 보장하기 위해 |
| `CoreUObject.DestroyHooksRunExactlyOnce` | 중복 파괴 요청이 생명주기 Hook과 소멸자를 중복 실행하지 않도록 보장하기 위해 |
| `CoreUObject.StaleHandleCannotResolveReusedSlot` | 재사용된 슬롯을 이전 Handle이 다른 객체로 오인하지 않도록 보장하기 위해 |

## EngineLifecycle

World에서 Actor와 Component로 이어지는 실행·등록·파괴 생명주기를 검증한다.

| 검증 항목 | 이유 |
|---|---|
| `EngineLifecycle.ActiveWorldPropagatesBeginPlay` | 실행 중인 World에 추가된 Actor와 Component가 즉시 시작 상태를 맞추는지 확인하기 위해 |
| `EngineLifecycle.EngineTickPropagatesToActorAndComponent` | 프레임 Tick이 World 계층을 따라 한 번씩 전달되는지 확인하기 위해 |
| `EngineLifecycle.SpawnDuringTickStartsNextTick` | 순회 중 생성된 Actor가 같은 프레임에 중복 실행되지 않도록 보장하기 위해 |
| `EngineLifecycle.DestroyDuringTickLeavesTraversalAndFlushesHierarchy` | 순회 중 파괴된 Actor가 즉시 제외되고 프레임 말에 자식까지 해제되는지 확인하기 위해 |
| `EngineLifecycle.DestroyComponentInvalidatesBeforeFlushAndFreesAfterFlush` | 논리적 제거와 실제 메모리 해제 시점이 구분되는지 확인하기 위해 |
| `EngineLifecycle.UnregisterKeepsWorldWhileOwnerIsPendingDestroy` | Owner 파괴 중에도 `OnUnregister`가 등록했던 World를 사용할 수 있도록 보장하기 위해 |
| `EngineLifecycle.UnregisterKeepsWorldWhileWorldIsPendingDestroy` | World 연쇄 파괴 중에도 Component 정리가 등록 World 문맥에서 실행되는지 확인하기 위해 |
| `EngineLifecycle.CascadeDestroyWaitsForChildren` | 부모 UObject가 자식 계층보다 먼저 해제되지 않도록 보장하기 위해 |

## SceneComponent

SceneComponent의 Root 선택, 부착 제약과 Transform·Bounds 전파를 검증한다.

| 검증 항목 | 이유 |
|---|---|
| `SceneComponent.FirstSceneComponentBecomesRoot` | Actor가 최초 공간 Component를 안정적으로 Root로 선택하는지 확인하기 위해 |
| `SceneComponent.RejectsCrossOwnerAttachment` | 서로 다른 Actor의 Component 계층이 섞이지 않도록 보장하기 위해 |
| `SceneComponent.RejectsAttachmentCycle` | 부모·자식 순환으로 Transform 갱신이 재귀하는 상황을 방지하기 위해 |
| `SceneComponent.PropagatesParentTransform` | `Local * ParentWorld` 변환 규칙이 자식 World Transform에 적용되는지 확인하기 위해 |
| `SceneComponent.UpdatesWorldBounds` | Local Bounds가 Component World Transform을 따라 갱신되는지 확인하기 위해 |
| `SceneComponent.DestroyDetachesChildren` | 부모 파괴 뒤 자식에 유효하지 않은 부착 Handle이 남지 않도록 보장하기 위해 |

## RenderScene

PrimitiveComponent와 FScene 사이의 Proxy 동기화 및 Scene Handle 수명을 검증한다.

| 검증 항목 | 이유 |
|---|---|
| `RenderScene.RegistersPrimitiveProxy` | 등록된 Primitive가 FScene 소유 Proxy와 Handle을 생성하는지 확인하기 위해 |
| `RenderScene.ExposesReadOnlyProxyAccess` | Renderer 소비 경로에서 Proxy 변경 권한이 노출되지 않도록 보장하기 위해 |
| `RenderScene.PropagatesFlags` | Visibility와 Cast Shadow 변경이 Proxy에 동기 반영되는지 확인하기 위해 |
| `RenderScene.PropagatesTransformAndBounds` | Component의 World Transform과 Bounds 변경이 Proxy에 동기 반영되는지 확인하기 위해 |
| `RenderScene.RemoveInvalidatesHandle` | Proxy 제거 직후 기존 Scene Handle이 무효가 되는지 확인하기 위해 |
| `RenderScene.ReusedSlotChangesGeneration` | 재사용된 Scene 슬롯을 이전 Handle이 새 Proxy로 해석하지 않도록 보장하기 위해 |
| `RenderScene.InvalidHandleCannotUpdateOrRemove` | 만료된 Handle을 통한 갱신과 중복 제거가 Scene 상태를 변경하지 않도록 보장하기 위해 |
| `RenderScene.ActorDestroyRemovesProxy` | Actor 직접 파괴 경로에서도 Component Proxy가 Scene에 남지 않도록 보장하기 위해 |
| `RenderScene.WorldDestroyRemovesProxy` | World 연쇄 파괴가 Scene 소멸 전에 등록된 Proxy를 해제하는지 확인하기 위해 |
| `RenderScene.MeshPassPipelineRegistersOpaquePass` | MeshPassPipeline이 현재 Opaque Pass를 실행 목록에 보유하는지 확인하기 위해 |
| `RenderScene.TriangleProxySubmitsOpaqueMeshBatch` | Triangle Proxy가 Opaque Pass에서 처리 가능한 Geometry와 Material 정보를 MeshBatch로 제출하는지 확인하기 위해 |
| `RenderScene.VisibilitySkipsBatchWithoutRemovingProxy` | 보이지 않는 Primitive가 Scene에서 제거되지 않은 채 해당 프레임의 MeshBatch 수집에서만 제외되는지 확인하기 위해 |

## HeaderGenerator

UObject 메타데이터 생성기가 지원하는 선언을 생성하고 지원하지 않는 선언을 거부하는지 검증한다.

| 검증 항목 | 이유 |
|---|---|
| `JisooHeaderGenerator` | Class 메타데이터와 생성자 전달 코드를 생성하고 templated `UCLASS`를 명시적으로 거부하는지 확인하기 위해 |

## Viewport

Viewport 출력 표면, Client 호출 경계와 Editor Layout의 초기 확장 계약을 검증한다.

| 검증 항목 | 이유 |
|---|---|
| `Viewport.DrawForwardsToAssociatedClient` | FViewport가 렌더 정책을 소유하지 않고 연결된 Client의 Draw 진입점을 정확히 호출하는지 확인하기 위해 |
| `Viewport.SceneViewFamilyOwnsFrameViews` | Renderer 요청이 Scene, RenderTarget Handle과 프레임별 SceneView 목록을 값으로 함께 보관하는지 확인하기 위해 |
| `Viewport.RejectsMissingRenderTarget` | 유효한 Target Handle이 없는 Viewport가 암묵적인 기본 BackBuffer로 렌더되지 않도록 보장하기 위해 |
| `Viewport.EditorLayoutStartsWithSingleVisibleSlot` | 초기에는 Slot 0만 생성하면서 최대 네 Slot의 Layout 구조, Single 표시 계약과 초기 Active Viewport를 함께 유지하는지 확인하기 위해 |
| `Viewport.EditorCameraMovesWithHeldKeys` | Editor 카메라 이동이 지속 키 상태, cm/s 속도와 DeltaSeconds를 함께 반영하는지 확인하기 위해 |
| `Viewport.EditorCameraRotatesWhileRightMouseHeld` | 우클릭 중 Pointer Delta만 Editor 카메라의 Yaw·Pitch로 해석하는지 확인하기 위해 |
| `Viewport.CameraRotationAffectsViewMatrix` | +X Forward, +Y Right, +Z Up 규칙에 맞게 카메라 회전이 ViewMatrix 축을 변경하는지 확인하기 위해 |

## Input

플랫폼 이벤트의 프레임 상태화와 Keyboard·Pointer 입력 대상 분리 계약을 검증한다.

| 검증 항목 | 이유 |
|---|---|
| `Input.EngineBuildsRouteContextBeforeRoutingCurrentFrame` | Engine Tick이 확정된 현재 입력으로 Route Context를 만든 뒤 같은 단계에서 즉시 라우팅해 준비·라우팅 호출 순서가 분리되지 않도록 보장하기 위해 |
| `Input.FramePreservesHeldStateAndResetsTransitions` | Down 상태는 유지하면서 Pressed·Released와 Pointer·Wheel Delta가 한 프레임에만 남는지 확인하기 위해 |
| `Input.FocusLossReleasesHeldKeys` | Window Focus를 잃었을 때 고착된 Keyboard·Pointer 입력이 남지 않도록 보장하기 위해 |
| `Input.FrameCollectsTextInputForOneTick` | WM_CHAR에서 변환한 UTF-16 입력이 순서를 유지하면서 해당 Tick에만 ImGui 같은 UI 소비자에게 제공되는지 확인하기 위해 |
| `Input.RouterSeparatesKeyboardAndPointerTargets` | UI와 다중 Viewport 확장 시 서로 다른 Keyboard·Pointer 대상에 허용된 채널만 전달되는지 확인하기 위해 |
| `Input.RouterCallsSharedTargetOnce` | 하나의 ViewportClient가 두 채널을 모두 소유할 때 같은 프레임 입력을 중복 처리하지 않도록 보장하기 위해 |
| `Input.RouterTransformsPointerIntoTargetPixels` | Window 좌표와 Delta가 선택된 Viewport Region의 원점·배율을 따라 RenderTarget pixel 좌표로 변환되는지 확인하기 위해 |
