# 임시 구현 및 재검토 목록

전체 엔진 아키텍처를 완성하기 전에 최소 동작 검증을 위해 단순화했거나 임시로 구현한 항목을 추적한다. 이 문서의 항목은 확정된 설계 결정이 아니며, 관련 시스템을 구현할 때 다시 검토한다.

## UObject 참조와 생명주기

- [ ] 컨테이너와 객체 관계에 직접 저장한 `FObjectHandle`을 타입 안전한 UObject 포인터 래퍼로 교체한다.
  - World → Actor, Actor → Component처럼 생존을 보장하는 관계는 `TObjectPtr<T>`에 해당하는 강한 참조가 필요하다.
  - 외부 관찰자처럼 생존을 보장하지 않는 관계는 `TWeakObjectPtr<T>`에 해당하는 약한 참조가 필요하다.
  - `std::shared_ptr`이 아니라 `GUObjectArray`, GC와 Property 참조 추적을 이해하는 전용 타입으로 구현한다.
- [ ] `Component → Owner`와 `SceneComponent → AttachParent/Children` 관계가 강한 참조인지 약한 참조인지 GC·직렬화·복제 요구와 함께 확정한다.
- [ ] `AActor::GetWorld()`가 `PendingDestroy` 또는 `BeginDestroyed` World를 언제부터 `nullptr`로 취급할지 확정한다.
  - Component의 등록 해제는 캐시된 `RegisteredWorld`를 사용하므로 이 결정과 분리한다.
  - 등록되지 않은 객체가 파괴 중 World에 접근해야 하는 사례가 생기면 별도 정리용 API를 검토한다.
- [ ] `BeginDestroyed` 상태를 `BeginDestroy()` 호출 전과 후 중 언제 설정할지 확정한다.
- [ ] World 파괴 중 `SpawnActor`, BeginPlay와 기타 상태 변경 API를 명시적으로 거부한다.
- [ ] GC 도입 시 `Outer`, 강한 참조, Root Set과 도달성 규칙을 확정하고 수동 연쇄 파괴와의 역할을 재검토한다.
- [ ] `GUObjectArray`의 슬롯 vector가 최대 사용량 이후 줄어들지 않는 현재 정책을 검토한다.
  - 현재는 해제된 Index를 `FreeIndices`에 넣어 재사용하며 주소 슬롯을 erase하거나 재배열하지 않는다.
  - 메모리 회수나 compaction이 필요해지면 Handle 안정성과 함께 설계한다.
- [ ] Pending Destroy batch vector의 프레임별 할당이 측정 가능한 비용이 되면 두 개의 재사용 queue 또는 별도 allocator를 검토한다.

## Engine Loop와 시간

- [ ] `sleep_for(1ms)`를 실제 Frame Pacing으로 교체한다.
  - 현재 구현은 고정 Tick Rate가 아니라 busy loop를 완화하는 임시 대기다.
  - VSync, Waitable Timer 또는 `sleep_until` 기반 deadline을 후보로 검토한다.
- [ ] 큰 hitch가 Gameplay에 그대로 전달되지 않도록 최대 DeltaSeconds clamp가 필요한지 검토한다.
- [ ] Gameplay·Rendering의 Variable Tick과 Physics의 Fixed Tick을 분리한다.
  - Fixed Physics가 필요하면 accumulator, 최대 substep 수와 느린 프레임 처리 정책을 함께 정한다.
- [ ] 입력 → 이동·충돌 → 카메라 → 렌더링의 프레임 순서를 실제 시스템 연결 시 검증한다.

## World와 Tick 구조

- [ ] `FEngine`의 World Handle 목록을 Editor World, PIE World와 Game World를 구분하는 World Context 구조로 확장한다.
- [ ] 현재 `UWorld → Actor` 직접 컨테이너를 저장·로드 구현 시 `UWorld → ULevel → Actor` 구조로 확장할지 결정한다.
- [ ] World → Actor → Component 직접 순회 Tick을 Tick Function·Tick Manager로 교체한다.
  - Tick 선후행 조건, Physics 전후 Tick Group과 병렬 실행이 필요해질 때 도입한다.
- [ ] Tick 중 생성된 객체를 다음 프레임부터 실행하는 현재 규칙을 Tick Group 도입 후에도 유지할지 검토한다.

## Component와 Game Scene

- [x] PrimitiveComponent 등록·해제를 FScene의 Proxy 등록·제거와 연결한다.
- [ ] Physics와 Navigation을 Component 등록 과정에 연결할 때 OnRegister·OnUnregister 순서와 실패 처리를 확정한다.
- [ ] SceneComponent 부착 규칙에 KeepRelative·KeepWorld, Socket과 절대 Transform 옵션을 추가할 시점을 결정한다.
- [ ] 최소 Math 타입의 Quaternion 연산, 역행렬, SIMD와 수치 오차 정책을 확정한다.
- [ ] Euler 회전 합성 순서를 확정한 뒤 외부 Rotator API를 추가한다.
- [x] PrimitiveComponent의 Transform·Bounds·Visibility·Cast Shadow를 FScene의 값 기반 Add·Update·Remove와 연결한다.
- [ ] Collision·Physics State는 FScene 연결과 분리된 Component 등록 단계로 설계한다.

## Object Reflection과 데이터

- [ ] Property·Function Reflection과 자동 참조 추적을 추가한다.
- [ ] CDO·Archetype과 Default Subobject 생성 규칙을 추가할 때 `FObjectInitializer` 책임을 확장한다.
- [ ] 이름 기반 Class Registry는 저장·로드가 클래스 이름으로 객체를 생성해야 할 때 도입한다.
- [ ] 현재 Python Header Generator의 최상위 단일 상속 제한을 확장할지 Clang 기반 분석기로 전환할지 결정한다.
- [ ] 직렬화, Package·Asset과 논리 콘텐츠 경로 규칙을 구현한다.

## Render Scene 이후

- [x] `FScene`, 최소 `FPrimitiveSceneProxy`와 `Index + Generation` Scene Handle을 구현한다.
- [x] Component와 SceneProxy 사이의 최초 description과 Transform·Bounds·Flags update payload를 분리한다.
- [ ] Primitive 수와 갱신량이 커지면 선형 슬롯 순회, 단건 동기 update를 SceneInfo·공간 인덱스·batch update로 확장한다.
- [ ] Geometry·Material render reference와 Renderer 자원 수명·Fence 연동을 구현한다.
- [ ] Render Thread와 비동기 update command queue는 Single Thread 구조가 실제 병목이 된 뒤 검토한다.

## Mesh 제출과 GPU 자원

- [ ] `FLinearColor`의 색 공간 변환 계약을 완성한다.
  - 입력 색상의 sRGB 여부를 리소스·API 경계에서 명시하고, 필요한 입력은 Linear로 변환한다.
  - 조명과 혼합은 Linear 공간에서 수행하고, 최종 출력 경계에서 sRGB 변환을 적용한다.
- [ ] Dynamic Upload Buffer를 고정 단일 용량에서 프레임별 페이지 기반 선형 할당기로 확장한다.
  - 각 요청은 현재 페이지에서 필요한 크기와 정렬만큼 sub-allocation한다.
  - 남은 공간이 부족하면 새 페이지를 사용하고, 기본 페이지보다 큰 단일 요청은 전용 대형 페이지로 처리한다.
  - Frame Fence 완료 전에는 페이지를 재사용하지 않으며, 할당 실패를 Draw 누락으로 숨기지 않고 진단한다.
- [ ] Opaque `FMeshDrawCommand::SortKey`를 안정적인 `PSO → Material Binding → Mesh` 순서로 구성한다.
  - 포인터 주소가 아닌 Registry ID나 명시적인 안정 ID를 사용하고, 서로 다른 Material의 여러 Draw에서 상태 변경이 줄어드는지 검증한다.
- [x] `FMeshPassPipeline`과 `FMeshPass` 계약으로 Pass 초기화·종료·relevance·순차 실행을 Renderer에서 분리한다.
- [ ] 공용 Shader Library, Root Signature Cache와 Pipeline State Cache를 한 번 초기화·종료하고 Pass가 필요한 상태를 Key로 요청하는 구조를 검토한다.
- [ ] `FOpaqueMeshPass`의 `DefaultMesh.hlsl` 직접 경로와 직접 컴파일을 Shader Library·Material Shader Map 기반 선택으로 교체한다.
  - 동적 Material 값과 Texture는 같은 Shader의 Binding으로 처리하고, Static Switch·Vertex Factory·Pass처럼 코드 구조가 달라지는 조건만 제한된 Shader Permutation으로 만든다.
  - Uber Shader의 Runtime 분기 비용과 Shader Permutation 수 증가 비용을 비교해 기능별 적용 기준을 정한다.
  - 선택된 Shader, Root Signature, Input Layout, RenderTarget Format과 Rasterizer·Blend·Depth 상태로 PSO(Pipeline State Object) Key를 구성해 Cache에서 조회한다.
- [ ] Pass 수와 Batch 수가 커지면 현재의 양방향 전체 순회를 수집 시점의 Pass별 Batch 분배로 교체한다.
- [ ] Shadow·Depth처럼 View와 Primitive 상태가 필요한 Pass를 추가할 때 `IsRelevant` 입력을 Batch 전용에서 View·Primitive relevance context로 확장한다.
- [ ] Pass마다 Root Parameter와 Descriptor Binding 구성이 달라질 때 Root Constants slot 0을 전제하는 공통 Draw 기록 계약을 확장한다.
