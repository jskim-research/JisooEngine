# 임시 구현 및 재검토 목록

전체 엔진 아키텍처를 완성하기 전에 최소 동작 검증을 위해 단순화했거나 임시로 구현한 항목을 추적한다. 이 문서의 항목은 확정된 설계 결정이 아니며, 관련 시스템을 구현할 때 다시 검토한다.

## UObject 참조와 생명주기

- [ ] 컨테이너와 객체 관계에 직접 저장한 `FObjectHandle`을 타입 안전한 UObject 포인터 래퍼로 교체한다.
  - World → Actor, Actor → Component처럼 생존을 보장하는 관계는 `TObjectPtr<T>`에 해당하는 강한 참조가 필요하다.
  - 외부 관찰자처럼 생존을 보장하지 않는 관계는 `TWeakObjectPtr<T>`에 해당하는 약한 참조가 필요하다.
  - `std::shared_ptr`이 아니라 `GUObjectArray`, GC와 Property 참조 추적을 이해하는 전용 타입으로 구현한다.
- [ ] `Component → Owner`와 `SceneComponent → AttachParent/Children` 관계가 강한 참조인지 약한 참조인지 GC·직렬화·복제 요구와 함께 확정한다.
- [ ] `AActor::GetWorld()`가 `PendingDestroy` 또는 `BeginDestroyed` World를 언제부터 `nullptr`로 취급할지 확정한다.
  - EndPlay·OnUnregister 중 World 시스템 등록 해제에 필요한 접근 범위를 함께 검토한다.
  - 필요하면 내부 정리용 `GetWorldEvenIfPendingDestroy()`를 분리한다.
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

- [ ] Component 등록을 현재 bool 상태에서 World 시스템 등록 과정으로 확장한다.
  - FScene, Physics, Navigation 등이 생기면 OnRegister·OnUnregister 순서와 실패 처리를 확정한다.
- [ ] SceneComponent 부착 규칙에 KeepRelative·KeepWorld, Socket과 절대 Transform 옵션을 추가할 시점을 결정한다.
- [ ] 최소 Math 타입의 Quaternion 연산, 역행렬, SIMD와 수치 오차 정책을 확정한다.
- [ ] Euler 회전 합성 순서를 확정한 뒤 외부 Rotator API를 추가한다.
- [ ] PrimitiveComponent의 Visibility·Cast Shadow·Bounds를 FScene의 값 기반 Add·Update·Remove와 연결한다.
- [ ] Collision·Physics State는 FScene 연결과 분리된 Component 등록 단계로 설계한다.

## Object Reflection과 데이터

- [ ] Property·Function Reflection과 자동 참조 추적을 추가한다.
- [ ] CDO·Archetype과 Default Subobject 생성 규칙을 추가할 때 `FObjectInitializer` 책임을 확장한다.
- [ ] 이름 기반 Class Registry는 저장·로드가 클래스 이름으로 객체를 생성해야 할 때 도입한다.
- [ ] 현재 Python Header Generator의 최상위 단일 상속 제한을 확장할지 Clang 기반 분석기로 전환할지 결정한다.
- [ ] 직렬화, Package·Asset과 논리 콘텐츠 경로 규칙을 구현한다.

## Render Scene 이후

- [ ] `FScene`, `FPrimitiveSceneProxy`와 안정적인 Scene Handle 표현을 확정한다.
- [ ] Component와 SceneProxy 사이의 description·update payload 단위를 확정한다.
- [ ] Geometry·Material render reference와 Renderer 자원 수명·Fence 연동을 구현한다.
- [ ] Render Thread와 비동기 update command queue는 Single Thread 구조가 실제 병목이 된 뒤 검토한다.
