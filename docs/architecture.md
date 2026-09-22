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

현재 `FEngineLoop`는 최소 Win32 창을 생성하고 메시지를 처리하며, 창을 닫을 때까지 `FEngine::Tick`을 반복 호출한 뒤 종료한다. `FWindowsWindow`는 창과 네이티브 핸들을 소유하지만 범용 Application 계층은 두지 않는다. 렌더러, 에디터 화면, 편집·플레이 월드 전환, 게임 콘텐츠 로딩·패키징은 아직 구현되지 않았다.
