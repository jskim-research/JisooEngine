# JisooEngine

DirectX 12와 C++20으로 개발하는 Windows용 3D 게임 엔진입니다.
엔진 구조와 렌더링 시스템을 직접 설계하고 구현하는 것을 목표로 합니다.

## 시작하기

### 요구 사항

- Windows 10/11
- Visual Studio 2022와 v143 툴체인
- Visual Studio의 **Desktop development with C++** 워크로드
- Windows SDK
- CMake 3.24 이상
- Python 3.10 이상

## 개발 명령

| 명령 | 사용 가능한 구성 | 동작 |
|---|---|---|
| `GenerateProjectFiles.bat` | 해당 없음 | 통합 CMake 그래프로 Visual Studio 2022 솔루션을 생성하거나 갱신합니다. |
| `OpenVisualStudio.bat` | 해당 없음 | 솔루션이 없으면 생성하고 Visual Studio에서 엽니다. |
| `BuildAll.bat [구성]` | `Debug`(기본), `Release` | Engine·Game의 모든 모듈과 실행 파일을 빌드합니다. |
| `JisooEngine\BuildEngine.bat [구성]` | `Debug`(기본), `Release` | Engine Runtime과 Engine Editor 모듈만 빌드합니다. |
| `JisooGame\BuildGame.bat [구성]` | `Debug`(기본), `Release` | 필요한 Engine Runtime을 포함해 Game 실행 파일을 빌드합니다. |
| `JisooGame\RunEditor.bat [구성]` | `Debug`(기본), `Release` | 필요한 Engine Runtime·Editor와 `JisooGameEditor`를 빌드하고 실행합니다. |
| `JisooGame\RunGame.bat [구성]` | `Debug`(기본), `Release` | Game 실행 파일을 빌드하고 실행합니다. |

`[구성]`은 생략할 수 있으며, 생략하면 `Debug`를 사용합니다.

Visual Studio 기본 시작 프로젝트는 `JisooGameEditor`입니다. `JisooGame`은 에디터 없는 독립 게임 실행 파일입니다.
두 실행 타깃은 `JisooEngine/Source/main.cpp`를 각각 컴파일하며, 빌드 설정에 따라 `FEditorEngine` 또는 `FGameEngine`을 사용합니다.
현재 Game 고유 코드는 없으므로 Game Runtime·Editor 라이브러리는 만들지 않습니다. 실제 게임 기능을 구현할 때 추가합니다.

## 생성 위치

| 항목 | 경로 |
|---|---|
| Visual Studio 솔루션 | `Build/VS2022-x64/JisooEngine.sln` |
| Engine 정적 라이브러리 | `JisooEngine/Intermediate/Libraries/Win64/<구성>` |
| Editor 실행 파일 | `JisooGame/Binaries/Win64/<구성>/JisooGameEditor.exe` |
| Game 실행 파일 | `JisooGame/Binaries/Win64/<구성>/JisooGame.exe` |
| 실행 파일 PDB | 각 실행 파일과 같은 디렉터리 |
| 제품별 컴파일 중간 파일 | `JisooEngine/Intermediate`, `JisooGame/Intermediate` |
| UObject 생성 코드 | `JisooEngine/Intermediate/Generated` |

경로의 `<구성>`에는 `Debug` 또는 `Release`가 들어갑니다.

## 문서

- [엔진 아키텍처](docs/architecture.md)
- [좌표계와 변환 규칙](docs/conventions/coordinate-system.md)
- [개발 계획](docs/development-plan.md)
- [임시 구현 및 재검토 목록](docs/implementation-revisit-list.md)
