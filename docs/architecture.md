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
│  │  │  └─ Engine/           # FEngine, FGameEngine
│  │  └─ Editor/
│  │     └─ Engine/           # FEditorEngine
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
- `FObjectHandle`은 `Index + Serial`로 슬롯 재사용 뒤 오래된 참조가 새 객체를 가리키지 않게 한다.
- `Outer`는 이름 경로와 논리적 소속만 나타내며 소유권이나 생존 참조를 만들지 않는다.
- `UCLASS`와 `GENERATED_BODY`의 타입 선언 및 생성 함수는 `Scripts/GenerateHeaders.py`가 `Intermediate/Generated`에 만든다.

현재 생성기는 최상위 단일 상속 `UCLASS`만 지원한다. 중첩·네임스페이스·템플릿·조건부 선언과 다중 상속은 생성 단계에서 거부한다. Property Reflection, GC, CDO, 직렬화, Package·Asset과 이름 기반 Class Registry는 이후 범위다.

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
               ├─ Add(Description) ──────────────>
               ├─ Update(Handle, Values) ────────>
               └─ Remove(Handle) ────────────────>
```

다음 규칙을 유지한다.

- `FPrimitiveSceneProxy`는 자신을 생성한 `UPrimitiveComponent` 포인터나 다른 Game Scene 객체 포인터를 저장하지 않는다.
- Renderer는 `UPrimitiveComponent`, Actor와 `UWorld`를 역참조하지 않고 `FScene`이 소유한 Render Scene 표현만 읽는다.
- `UPrimitiveComponent`는 Transform, Bounds, Geometry·Material 설정, Visibility와 Cast Shadow 같은 렌더 의도를 소유한다.
- Render Pass 선택, PSO와 Descriptor 구성, Draw 순서, D3D12 Command 기록과 제출은 Renderer 책임이며 Component에 넣지 않는다.
- Component 등록 시 값 형태의 Scene description을 전달하고 안정적인 scene handle을 받는다. 이후 변경과 제거는 이 handle과 값 기반 갱신을 사용한다.
- 초기 구현은 Single Thread이므로 Add·Update·Remove를 동기적으로 적용한다. Render Thread와 비동기 update command queue는 필요해질 때 별도로 결정한다.
- Culling은 Proxy를 `FScene`에서 등록 해제하지 않는다. 등록된 Proxy에서 View별·프레임별 Visible 목록을 별도로 만든다.
- GPU resource의 실제 수명은 Component나 SceneProxy 포인터 수명에 기대지 않고 Renderer의 resource 관리와 Fence 완료 시점으로 관리한다.

SceneProxy는 원본 Component 접근을 한 단계 감싸는 전달 객체나 성능 캐시로 정의하지 않는다. 책임 경계를 위해 독립된 Render Scene 상태를 보유하며, Component 상태를 다시 읽어야 하는 경우에는 명시적인 값 갱신을 추가한다. 이전 자체엔진처럼 Proxy가 Component 포인터를 보관하고 렌더 경로에서 역참조하는 혼합형은 사용하지 않는다.

SceneProxy를 타입별 계층으로 구성할지 공통 데이터 표현으로 먼저 구현할지, scene description과 update payload를 어떤 단위로 나눌지, Geometry·Material render reference를 어떤 handle로 표현할지는 관련 타입을 구현할 때 확정한다. MeshBatch, PassProcessor, Render Graph와 RHI는 이 결정의 적용 범위가 아니다.

선택 이유와 검토한 대안은 [Game Scene과 Render Scene 책임 분리 결정](decisions/Game%20Scene과%20Render%20Scene%20책임%20분리%20결정.md)에 기록한다.

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

현재 `FEngineLoop`는 최소 Win32 창을 생성하고 메시지를 처리하며, 창을 닫을 때까지 `FEngine::Tick`을 반복 호출한 뒤 종료한다. `FWindowsWindow`는 창과 네이티브 핸들을 소유하지만 범용 Application 계층은 두지 않는다.

`FEngine`은 D3D12 전용 `FRenderer`를 소유하고 초기화·프레임 렌더링·종료 수명을 관리한다. `FRenderer`는 RHI나 그래픽 API 다형성 계층 없이 `FD3D12Device`, `FD3D12CommandContext`, `FDXGISwapChain`과 프레임별 `FFrameResource`를 합성한다. 현재 렌더링 범위는 BackBuffer 상태 전환, Clear, Present와 Fence 기반 프레임 자원 재사용까지이며, Scene 렌더링과 Render Pipeline은 아직 연결하지 않는다.

에디터 화면, 편집·플레이 월드 전환, 게임 콘텐츠 로딩·패키징은 아직 구현되지 않았다.
