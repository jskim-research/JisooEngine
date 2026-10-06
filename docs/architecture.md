# 엔진 아키텍처

JisooEngine의 제품별 책임과 의존 방향을 정의한다. 새 코드를 배치하거나 참조를 추가할 때 이 문서를 기준으로 판단한다.

## 제품 구조

저장소 루트는 두 제품을 함께 개발하는 워크스페이스이다. Engine은 공통 엔진 기능과 실행 진입점 소스를 제공하고, Game은 게임 코드·콘텐츠·설정과 프로젝트 전용 실행 타깃을 소유한다.

```text
Repository/
├─ CMakeLists.txt
├─ Build/VS2022-x64/           # 통합 Visual Studio 솔루션과 CMake 캐시
├─ JisooEngine/
│  ├─ CMakeLists.txt
│  ├─ Source/
│  │  ├─ main.cpp             # 두 실행 타깃이 각각 컴파일하는 공통 진입점
│  │  ├─ Runtime/
│  │  │  ├─ CoreUObject/      # UObject 타입 정보, 생성·파괴와 전역 객체 추적
│  │  │  ├─ Launch/           # FEngineLoop
│  │  │  ├─ Platform/Windows/ # 최소 Win32 창과 메시지 처리
│  │  │  ├─ Input/            # 플랫폼 독립 입력 상태와 Viewport 라우팅
│  │  │  ├─ Engine/           # FEngine, Game Scene과 공통 Viewport 계층
│  │  │  └─ Render/           # Render Scene과 D3D12 Renderer
│  │  └─ Editor/
│  │     ├─ Engine/           # FEditorEngine
│  │     ├─ UI/               # ImGui Context, DockSpace와 Editor Panel 구성
│  │     └─ Viewport/         # Editor ViewportClient와 최대 4 Slot Layout
│  ├─ ThirdParty/ImGui/       # Editor 타깃에서만 빌드하는 Dear ImGui와 DX12 Backend
│  ├─ Content/
│  ├─ Config/
│  ├─ Shaders/
│  ├─ Intermediate/
│  └─ Binaries/
└─ JisooGame/
   ├─ CMakeLists.txt
   ├─ Source/
   │  ├─ Runtime/             # 게임 로직 구현 시 사용; 현재 비어 있음
   │  └─ Editor/              # 게임 전용 편집 기능 구현 시 사용; 현재 비어 있음
   ├─ Content/
   ├─ Config/
   ├─ Intermediate/
   └─ Binaries/
```

| 제품 | 소유하는 책임 |
|---|---|
| `JisooEngine` | 범용 런타임 기반, 공통 편집 기능, 공통 진입점 소스, 엔진 콘텐츠·설정·셰이더 |
| `JisooGame` | 게임 고유 실행 로직과 편집 기능, 게임 콘텐츠·설정, `JisooGame`·`JisooGameEditor` 실행 타깃 |

Unreal Engine의 Engine·Game 책임 분리를 참고하되, 현재는 정적 링크와 통합 CMake를 사용한다. `JisooGameEditor.exe`는 이 게임을 위한 에디터 실행 파일이며, 다른 게임의 DLL을 동적으로 로드하는 범용 에디터가 아니다.

`Runtime`, `Editor` 같은 분류 디렉터리 아래에는 `Launch`, `Engine`, `Gameplay`처럼 책임별 하위 디렉터리에 소스를 배치한다. 하위 디렉터리마다 별도 라이브러리를 만들지는 않는다. 공통 진입점은 `JisooEngine/Source/main.cpp` 하나를 사용한다.

Game Runtime·Editor 폴더는 현재 `.gitkeep`만 두어 위치를 보존한다. 실제 코드가 생길 때 필요한 정적 라이브러리와 링크 관계를 추가하며, 빈 라이브러리를 유지하기 위한 Module 클래스는 만들지 않는다.

## 통합 CMake와 타깃

루트 CMake는 공통 빌드 설정과 두 제품을 하나의 빌드 그래프로 구성한다. Engine CMake는 엔진 라이브러리를, Game CMake는 프로젝트 전용 실행 타깃을 정의한다.

현재 타깃은 다음 네 개이다.

| CMake 타깃 | 형태 | 책임 |
|---|---|---|
| `JisooEngineRuntime` | 정적 라이브러리 | 공통 런타임 기능, `FEngineLoop`, `FEngine`, `FGameEngine` |
| `JisooEngineEditor` | 정적 라이브러리 | `FEditorEngine`과 공통 편집 기능 |
| `JisooGame` | 실행 파일 | 에디터 없는 독립 게임 실행 |
| `JisooGameEditor` | 실행 파일 | JisooGame 프로젝트의 에디터 실행 |

기본 Visual Studio 시작 프로젝트는 `JisooGameEditor`이다. 두 실행 파일은 `JisooGame/Binaries/Win64/<구성>`에 생성한다. `source_group()`은 소스 디렉터리를 Visual Studio 필터로 표현한다.

## 의존 규칙

현재 링크 관계는 다음과 같다.

```text
JisooEngineEditor ──> JisooEngineRuntime

JisooGame.exe
└─ JisooEngineRuntime

JisooGameEditor.exe
├─ JisooEngineRuntime
└─ JisooEngineEditor
```

게임 코드가 추가되어도 다음 규칙을 유지한다.

- Engine Runtime은 Editor나 JisooGame의 구체 타입·규칙·콘텐츠를 직접 참조하지 않는다.
- Engine Editor는 JisooGame의 구체 타입이나 게임 전용 편집 규칙에 의존하지 않는다.
- Game Runtime은 Engine Runtime을 사용할 수 있지만 Editor 헤더를 포함하거나 Editor 라이브러리를 링크하지 않는다.
- Game Editor는 Game Runtime과 Engine Editor를 사용할 수 있으며 독립 게임 실행 파일에는 포함하지 않는다.
- 엔진은 자신이 정의한 공통 인터페이스를 통해 Game의 구현을 호출할 수 있다. 이 호출을 위해 Engine에서 Game의 구체 타입을 include하지 않는다.
- 게임 타입 등록과 초기화 연결은 실제 게임 기능을 구현할 때 추가한다. 라이브러리를 링크하는 것만으로 게임 코드가 자동 실행된다고 가정하지 않는다.

현재 의존 규칙은 구현·리뷰에서 확인한다. 소스 폴더 분리만으로 잘못된 include가 컴파일 단계에서 자동 차단되는 것은 아니다.

## CoreUObject 객체 시스템

`Runtime/CoreUObject`는 World·Actor·Component와 Asset이 공통으로 사용할 객체 정체성과 생애주기 기반을 제공한다.

```text
UClass ── 타입 이름·부모·크기·생성 및 소멸 함수
  │
  └─ UObject ── Class·Name·Outer·Handle·생애주기 상태
         │
         └─ GUObjectArray ── 객체 주소·Index·Serial
```

- `UClass`는 초기 구현에서 `UObject`를 상속하지 않는 프로세스 수명의 타입 설명자다.
- UObject는 `NewObject`로만 생성하며, 생성 전에 `GUObjectArray` 슬롯과 생성 문맥을 확보한다.
- 리플렉션 UObject는 `const FObjectInitializer&` 생성자만 사용하며, 생성 코드가 이를 명시적으로 전달한다.
- `DestroyObject`는 파괴 대기 상태만 설정하고 `FlushPendingDestroyObjects`가 `BeginDestroy`, `FinishDestroy`, 실제 소멸과 슬롯 반환을 순서대로 수행한다.
- Flush는 현재 대기 목록을 별도 batch로 분리해 처리한다. `BeginDestroy` 중 추가된 파괴 요청은 다음 batch에 보존하며, 자식 객체가 남은 World·Actor는 `IsReadyForFinishDestroy`에서 부모의 실제 해제를 지연한다.
- `FObjectHandle`은 `Index + Serial`로 슬롯 재사용 뒤 오래된 참조가 새 객체를 가리키지 않게 한다.
- `Outer`는 이름 경로와 논리적 소속만 나타내며 소유권이나 생존 참조를 만들지 않는다.
- `UCLASS`와 `GENERATED_BODY`의 타입 선언 및 생성 함수는 `Scripts/GenerateHeaders.py`가 `Intermediate/Generated`에 만든다.

현재 생성기는 최상위 단일 상속 `UCLASS`만 지원한다. 중첩·네임스페이스·템플릿·조건부 선언과 다중 상속은 생성 단계에서 거부한다. Property Reflection, GC, CDO, 직렬화, Package·Asset과 이름 기반 Class Registry는 이후 범위다.

## Game Scene 컨테이너와 생명주기

Game Scene의 현재 컨테이너 관계는 다음과 같다.

```text
FEngine
└─ UWorld Handle 목록
   └─ AActor Handle 목록
      └─ UActorComponent Handle 목록
         └─ USceneComponent 부착 계층
            └─ UPrimitiveComponent
```

- 컨테이너는 `FObjectHandle`을 보관하고 실제 할당과 해제는 CoreUObject가 담당한다. `Outer`는 World·Actor·Component의 논리적 소속과 이름 경로를 표현하지만 메모리 소유권으로 사용하지 않는다.
- World와 Actor는 Tick 시작 시점의 Handle 목록을 복사해 순회한다. Tick 중 생성된 객체는 다음 프레임부터 참여하고, 파괴 요청된 객체는 Handle 해석에 실패하므로 남은 순회에서 제외된다.
- World가 시작된 뒤 생성된 Actor와 Actor가 시작된 뒤 추가된 Component는 즉시 BeginPlay한다. EndPlay와 Unregister는 실제 객체 메모리 해제 전에 수행한다.
- `UActorComponent`는 등록된 `UWorld`를 비소유 raw pointer로 캐시한다. 이 포인터는 `OnRegister` 전에 설정되고 `OnUnregister`가 끝난 뒤 해제되며, World는 등록된 Component의 정리가 끝날 때까지 실제 메모리를 유지한다.
- 상위 컨테이너를 파괴할 때 Component, Actor, World 순서로 실제 해제한다. 직접 `DestroyObject`가 호출된 경우에도 부모는 자식 Handle의 슬롯이 반환될 때까지 FinishDestroy를 기다린다.
- `USceneComponent`는 같은 Actor 안에서만 부모·자식 관계를 만들며 `ComponentToWorld = Local * ParentWorld` 규칙을 사용한다. 회전은 Euler 합성 순서가 확정되기 전까지 `FQuat` 값으로 보관한다.
- `UPrimitiveComponent`는 Visibility, Cast Shadow, Local/World Bounds를 소유하고 등록 생애주기 동안 `FScene`의 `FPrimitiveSceneProxy`와 값 기반으로 동기화한다. Geometry·Material과 Physics 연결은 아직 포함하지 않는다.

## Game Scene과 Render Scene 경계

Game Scene과 Render Scene은 별도의 표현과 생애주기를 가진다. `UWorld`와 Actor가 `UPrimitiveComponent`를 소유하고, `FScene`은 Renderer가 소비하는 `FPrimitiveSceneProxy`를 소유한다.

```text
Game Scene                                  Render Scene
UWorld                                      FScene
└─ Actor                                    └─ FPrimitiveSceneProxy
   └─ UPrimitiveComponent                       ├─ Transform·Bounds
        ├─ 렌더 의도와 에셋 설정                 ├─ Geometry·Material render reference
        └─ FPrimitiveSceneHandle                 └─ Visibility·render flags
               │
               ├─ CreateSceneProxy(Description)
               ├─ Add(Proxy) ────────────────────>
               ├─ Update(Handle, Values) ────────>
               └─ Remove(Handle) ────────────────>
```

다음 규칙을 유지한다.

- `FPrimitiveSceneProxy`는 자신을 생성한 `UPrimitiveComponent` 포인터나 다른 Game Scene 객체 포인터를 저장하지 않는다.
- Renderer는 `UPrimitiveComponent`, Actor와 `UWorld`를 역참조하지 않고 `FScene`이 소유한 Render Scene 표현만 읽는다.
- `UPrimitiveComponent`는 Transform, Bounds, Geometry·Material 설정, Visibility와 Cast Shadow 같은 렌더 의도를 소유한다.
- Render Pass 선택, PSO와 Descriptor 구성, Draw 순서, D3D12 Command 기록과 제출은 Renderer 책임이며 Component에 넣지 않는다.
- Component 등록 시 현재 상태를 값 형태의 Scene description으로 복사한 Proxy를 전달하고 `Index + Generation` scene handle을 받는다. 이후 변경과 제거는 이 handle과 값 기반 갱신을 사용한다.
- 초기 구현은 Single Thread이므로 Add·Update·Remove를 동기적으로 적용한다. Render Thread와 비동기 update command queue는 필요해질 때 별도로 결정한다.
- Culling은 Proxy를 `FScene`에서 등록 해제하지 않는다. 등록된 Proxy에서 View별·프레임별 Visible 목록을 별도로 만든다.
- GPU resource의 실제 수명은 Component나 SceneProxy 포인터 수명에 기대지 않고 Renderer의 resource 관리와 Fence 완료 시점으로 관리한다.

SceneProxy는 원본 Component 접근을 한 단계 감싸는 전달 객체나 성능 캐시로 정의하지 않는다. 책임 경계를 위해 독립된 Render Scene 상태를 보유하며, Component 상태를 다시 읽어야 하는 경우에는 명시적인 값 갱신을 추가한다. 이전 자체엔진처럼 Proxy가 Component 포인터를 보관하고 렌더 경로에서 역참조하는 혼합형은 사용하지 않는다.

`FPrimitiveSceneProxy`는 Transform, Local/World Bounds, Visibility와 Cast Shadow를 공통으로 보관하고, 파생 Proxy가 `GatherMeshBatches`에서 Pass 독립적인 `FMeshBatch`를 제출한다. 현재 `FTriangleSceneProxy`는 CPU 정점 세 개와 단색 Opaque Material 입력을 보관한다. Renderer는 Visible Proxy의 Batch를 프레임 로컬 Collector에 한 번 모으고, `FMeshPassPipeline`은 등록된 Pass에 relevance를 질의해 Batch별 `FMeshPassMask`를 만든 뒤 등록 순서대로 Pass를 실행한다.

현재 Dynamic Geometry는 Proxy가 소유한 CPU 정점의 `std::span`으로 표현하며 Processor가 해당 FrameResource의 선형 Upload Buffer에 복사한다. Upload Buffer는 Frame Slot의 Fence 완료 뒤에만 재사용한다. Static Mesh용 영구 GPU Buffer Handle과 범용 Material render reference는 Static Mesh·Material 구현 시 별도로 확정한다.

각 `FMeshPass`는 영구 Root Signature·PSO의 초기화와 종료, relevance, Pass 시작 상태를 소유한다. `FMeshPass::Execute`는 관련 Batch만 Processor에 전달하고 Draw Command를 정렬한 뒤 Pass별 바인딩 캐시를 빈 상태로 시작해 기록한다. Pass는 RenderTarget, Viewport, Scissor와 Root Signature를 다시 설정하고 각 Draw Command는 PSO, Root Constants, Geometry와 Draw 인자를 다시 설정하므로 이전 Pass의 바인딩을 전제로 하지 않는다.

Pass Processor는 PSO와 Binding을 선택해 Draw Command를 만들지만 D3D12 CommandList를 직접 기록하지 않는다. `FRenderer`는 Device·SwapChain·FrameResource·CommandList와 Queue 제출을 소유하고, 구체 Pass 타입이나 Pass별 조건 분기를 알지 않은 채 `FMeshPassPipeline`만 실행한다. 현재 Pipeline은 Opaque Pass 하나를 순서가 있는 목록으로 소유하며 Shader Library, PSO Cache, Render Graph와 RHI는 도입하지 않는다.

선택 이유와 검토한 대안은 [Game Scene과 Render Scene 책임 분리 결정](decisions/Game%20Scene과%20Render%20Scene%20책임%20분리%20결정.md)에 기록한다.

## 입력 수집과 라우팅

```text
FWindowsWindow::WndProc
└─ IInputEventSink (비소유)
   └─ FEngine::FInputSystem
      └─ FInputFrame
         └─ FInputRouter
            ├─ FGameViewportClient
            └─ FEditorViewportClient
```

- `FEngine`은 `FInputSystem`과 `FInputRouter`를 소유하며 전역 Input Singleton을 사용하지 않는다.
- `FWindowsWindow`는 Win32 메시지를 플랫폼 독립 `FInputEvent`로 바꾸고 등록된 Sink에 전달할 뿐 입력 상태와 대상을 소유하지 않는다. `FEngineLoop`가 Window와 Engine을 연결하고 Engine 종료 전에 연결을 해제한다.
- `FInputSystem`은 Tick 사이에 도착한 이벤트를 대기 큐에 값으로 보관한다. Engine Tick 시작에 이를 순서대로 소비해 지속 `Down`, 프레임 한정 `Pressed`·`Released`, Pointer 위치·Delta, Wheel과 Focus를 `FInputFrame`으로 확정한다.
- `FInputRouter`는 입력을 해석하지 않고 구체 Engine이 만든 `FInputRouteContext`의 Keyboard·Pointer Target에 허용된 채널만 전달한다. 같은 Client가 두 채널을 소유하면 한 번만 호출한다.
- 현재 Game은 `FGameViewportClient`를 두 채널의 고정 Target으로 사용한다. Editor는 Hover·Capture된 Viewport를 Pointer Target으로, Layout의 Active Viewport를 Keyboard Target으로 선택한다. 공통 Client의 기본 `ProcessInput`은 아무 동작도 하지 않으며, Editor Client가 우클릭 Pointer Delta와 WASD·QE를 비행 카메라 회전·이동으로 해석한다.
- ImGui 의존 코드는 Editor 계층과 vendoring한 ThirdParty Backend에만 둔다. `FEditorUI`가 Runtime의 `FInputFrame`을 ImGui IO로 전달하므로 `FWindowsWindow`, `FInputSystem`과 `FInputRouter`는 ImGui 타입을 참조하지 않는다.
- Engine Tick은 입력 프레임 확정 뒤 `RouteCurrentInput()`에서 Route Context 생성과 `FInputRouter` 호출을 한 단계로 수행한다. Editor의 Context 생성은 먼저 Panel을 구성하며, `FViewportPanel`은 ImGui Image Item의 배치·Hover를 프레임 한정 `FViewportInputRegion`으로 보고한다. `FEditorViewportInputRouting`은 Region, Pointer Button과 지속 Capture를 값 기반으로 조합해 Click된 Viewport를 Active 대상으로 보고하고 Capture 또는 Hover 대상을 Pointer Target으로, Active Viewport를 Keyboard Target으로 선택한다.
- `FInputRouter`는 선택된 Region의 Window 좌표 원점과 RenderTarget 배율을 Pointer 위치·Delta에 적용해 `FViewportClient`에는 Viewport pixel 좌표를 전달한다. Text 입력은 UTF-16 code unit 목록으로 한 Tick만 보관하며 Editor UI의 `InputText`에 공급한다.

## Viewport와 View 렌더 요청

Viewport 출력 표면, 사용 정책과 프레임 로컬 렌더 데이터를 분리한다.

```text
FEditorEngine
└─ FEditorViewportLayout
   └─ Slot[0..3]
      └─ FViewport ── FEditorViewportClient
                           │ 매 프레임 생성
                           ▼
                    FSceneViewFamily
                    └─ FSceneView[1..N]
                           │
                           ▼
                       FRenderer
```

- `FViewport`는 Renderer가 발급한 `FRenderTargetHandle`과 pixel 크기, 비소유 Client 연결과 Draw 진입점을 보관한다. 실제 RenderTarget GPU 자원은 Renderer가 소유하며 Viewport는 직접 참조하지 않는다.
- `FViewportClient`는 Scene 비소유 참조, Camera 위치·Pitch·Yaw와 ViewMode를 지속 상태로 보관하고 좌표계 규칙에 맞는 ViewMatrix를 포함한 `FSceneViewFamily`와 `FSceneView`를 프레임 값으로 만들어 Renderer에 즉시 제출한다. `FViewport::Draw`는 이 Client 호출을 전달할 뿐 렌더 정책을 소유하지 않는다.
- `FSceneViewFamily`는 동일한 Scene, `FRenderTargetHandle`, 출력 크기와 ViewMode를 공유하는 렌더 요청이며 하나 이상의 `FSceneView`를 값으로 소유한다. `FSceneView`는 View·Projection, ViewRect와 CameraPosition의 프레임 스냅숏이다.
- `FEditorViewportLayout`은 최대 네 Slot의 수명과 배치·표시 정책 및 활성 Client 조회를 소유하고 Renderer를 참조하지 않는다. 현재는 Single 모드의 Slot 0만 생성하고 활성 대상으로 사용한다.
- Editor와 Game의 구체 Engine은 World Tick 이후 Renderer 프레임을 열고, 표시할 Viewport의 Draw 진입점을 호출한 뒤 프레임을 닫는다. Client는 Family 제출만 하며 BeginFrame, EndFrame과 Present를 제어하지 않는다.
- Renderer는 `Index + Generation` Target Handle을 내부 슬롯으로 해석한다. 현재 Main Target 슬롯은 프레임 시작에 DXGI가 선택한 BackBuffer와 RTV에 연결되며, 같은 Handle이라도 실제 BackBuffer는 프레임마다 달라질 수 있다.
- Renderer는 `BeginFrame -> RenderViewFamily 1..N -> EndFrame` 순서를 제공한다. `RenderViewFamily`는 Family가 지정한 Target을 최초 사용할 때 RenderTarget 상태로 전환·Clear하고 `FMeshPassPipeline`을 실행한다. `EndFrame`은 사용한 off-screen Target을 sampling 가능한 최종 상태로 전환하고 선택적인 최종 Overlay를 Main Target에 기록한 뒤, 모든 Target의 최종 상태 복원과 프레임당 한 번의 제출·Present·Fence signal을 수행한다. Overlay 뒤에는 다른 Render Pass를 기록하지 않는다.
- View와 Projection은 왼손 좌표계, +X Forward, +Y Right, +Z Up과 행벡터 규칙을 따르며 `View * Projection` 순서로 합성한다.

Editor 실행의 현재 소유권과 프레임 흐름은 다음과 같다.

```text
FEditorEngine
└─ FEditorUI
   ├─ FViewportPanel
   │  └─ FEditorViewportLayout
   │     └─ Slot[0] ─ FViewport ─ FEditorViewportClient
   └─ FWorldOutlinerPanel

InputFrame → ImGui Panel 구성·Region 수집 → Input Route → World Tick
           → off-screen Viewport 렌더 → Main Target ImGui 합성 → Present
```

- `FEditorUI`는 Editor 전용 ImGui Context와 DX12 Backend, DockSpace와 구체 Panel 구성을 소유한다. Game 실행 타깃은 ImGui 소스와 Editor UI를 링크하지 않는다.
- `FViewportPanel`은 `FEditorViewportLayout`을 소유한다. Panel이 보이는 프레임만 Layout의 Viewport를 렌더 목록에 노출하며, 닫으면 Layout·Viewport와 Renderer Target을 해제하고 다시 열 때 기본 상태로 생성한다. Camera·ViewMode·Layout 상태 복원은 아직 구현하지 않는다.
- Renderer는 off-screen Color Target의 Resource·RTV·SRV와 재사용 가능한 Target Slot을 소유한다. Panel은 Handle과 ImGui Texture ID로 사용하는 GPU Descriptor 값만 조회한다. Panel 표시 영역이 연속 변경되는 동안에는 기존 Target을 새 사각형에 스케일해 표시하고, 크기가 안정화된 뒤에만 GPU 완료를 기다려 Target과 Viewport 출력 크기를 한 번 갱신한다.
- Scene Viewport 렌더 뒤 `EndFrame`이 Color Target을 `PIXEL_SHADER_RESOURCE`로 전환하고 Main SwapChain Target에 ImGui DrawData를 최종 Overlay로 기록한 뒤 한 번만 Present한다. ImGui Overlay는 Renderer의 Shader-visible SRV Heap을 명시적으로 연결한다.
- 최초 수직 구현은 Single Viewport와 World Outliner의 Actor 검색·Panel 내부 선택까지만 포함한다. 실제 2·4분할 활성화, Depth Target, Grid·Gizmo, 공용 Selection System과 Wireframe·Depth·WorldNormal Pass는 아직 포함하지 않는다.

## 코드 배치 기준

| 코드의 역할 | 배치할 위치 |
|---|---|
| 공통 실행 진입점과 구체 엔진 선택 | `JisooEngine/Source/main.cpp` |
| 공통 실행 루프 | `JisooEngine/Source/Runtime/Launch` |
| Windows 창 생성과 메시지 처리 | `JisooEngine/Source/Runtime/Platform/Windows` |
| 범용 엔진 생명주기와 독립 게임 실행 관리 | `JisooEngine/Source/Runtime/Engine` |
| 모든 게임에서 사용할 편집 기능 | `JisooEngine/Source/Editor` |
| 보드 이동·벽면 주행 등 게임 고유 로직 | `JisooGame/Source/Runtime` |
| 주행 파라미터 패널 등 게임 전용 편집 기능 | `JisooGame/Source/Editor` |

`FGameEngine`은 특정 게임의 규칙을 구현하는 클래스가 아니라 범용 게임 실행을 관리하는 엔진이다. 게임 로직은 Game Runtime에 작성한다.

## 콘텐츠와 설정 소유권

| 경로 | 소유권 |
|---|---|
| `JisooEngine/Content` | 엔진이 제공하는 공용 콘텐츠 |
| `JisooEngine/Config` | 엔진 기본 설정 |
| `JisooEngine/Shaders` | 엔진 렌더링 셰이더 |
| `JisooGame/Content` | JisooGame 전용 콘텐츠 |
| `JisooGame/Config` | JisooGame 전용 설정 |

시작 장면, 게임 규칙 선택, 논리 에셋 경로와 설정 병합은 관련 시스템을 구현할 때 확정한다. 물리적인 저장 위치를 콘텐츠 참조 식별자로 직렬화하지 않는다.

## 실행 진입점

두 실행 타깃은 같은 `JisooEngine/Source/main.cpp`를 서로 다른 설정으로 각각 컴파일한다.

| 실행 타깃 | 진입점 컴파일 설정 | 생성하는 엔진 |
|---|---|---|
| `JisooGame` | `JISOO_WITH_EDITOR=0` | `FGameEngine` |
| `JisooGameEditor` | `JISOO_WITH_EDITOR=1` | `FEditorEngine` |

`main.cpp`는 엔진 라이브러리에 포함하지 않는다. `JISOO_WITH_EDITOR`는 각 실행 타깃의 PRIVATE 정의이며, 공통 Runtime 라이브러리의 클래스 정의를 바꾸는 데 사용하지 않는다.

진입점은 구체 엔진을 생성하고 `FEngineLoop::Run(FEngine&)`에 전달한다. 루프는 `FEngine` 인터페이스만 사용하며, 별도의 Application 래퍼나 Game Module 초기화 클래스는 두지 않는다.

```text
JisooGame.exe       -> main.cpp -> FEngineLoop.Run(FGameEngine)
JisooGameEditor.exe -> main.cpp -> FEngineLoop.Run(FEditorEngine)
```

현재 `FEngineLoop`는 최소 Win32 창을 생성하고 메시지를 처리하며 `steady_clock`으로 DeltaSeconds를 계산해, 창을 닫을 때까지 `FEngine::Tick`을 반복 호출한 뒤 종료한다. `FWindowsWindow`는 창과 네이티브 핸들을 소유하지만 범용 Application 계층은 두지 않는다. `WM_SIZE`는 최신 Client Area 크기와 최소화 여부만 담은 `FWindowResizeEvent`로 변환되고, EngineLoop가 프레임 사이에 이를 Engine에 전달한다. Engine은 SwapChain Resize 성공 뒤 Game Viewport 또는 Editor ImGui `DisplaySize`를 같은 크기로 동기화하며 최소화 중에는 렌더링을 중단한다.

`FEngine`은 Input System·Router, World Handle 목록과 D3D12 전용 `FRenderer`를 소유한다. 각 `UWorld`는 CPU-side `FScene`을 소유한다. 한 프레임은 Input Frame 확정·라우팅 → World Tick → 구체 Engine의 Viewport Render → Pending UObject Flush 순서로 처리하므로 현재 Single Thread에서는 World Tick 동안 Scene 갱신을 끝내고 Renderer 구간에는 읽기 전용으로 취급한다. 종료할 때는 Window의 입력 Sink 연결을 먼저 끊고, 구체 Engine이 Viewport·Client를 해제한 뒤 World 파괴와 Pending UObject Flush를 수행하고 Renderer를 종료한다. `FRenderer`는 RHI나 그래픽 API 다형성 계층 없이 `FD3D12Device`, `FD3D12CommandContext`, `FDXGISwapChain`, RenderTarget Handle 슬롯, 프레임별 `FFrameResource`와 `FMeshPassPipeline`을 합성한다. 현재 렌더링 범위는 Family가 지정한 Target의 상태 전환과 Clear, View별 Visible MeshBatch 수집, 순서가 있는 단일 Opaque Mesh Pass의 Draw Command 생성·기록, Present와 Fence 기반 프레임 자원 재사용까지다.

첫 수직 검증에서는 ViewportClient가 만드는 View·Projection과 `UTriangleComponent`를 사용한다. Editor Viewport는 비행 카메라를 제공하지만 Game Viewport는 아직 고정 카메라다. Frustum이 아직 없으므로 Primitive Visibility는 `IsVisible()`만 검사하고, 앞면 winding이 확정되지 않아 Opaque PSO의 Cull Mode는 None이다. 기본 World에 생성하는 `RenderValidationTriangle`은 Game 시작 Scene 연결 전까지 실제 실행 경로를 검증하기 위한 임시 콘텐츠다.

에디터 화면, 편집·플레이 월드 전환, 게임 콘텐츠 로딩·패키징은 아직 구현되지 않았다.
