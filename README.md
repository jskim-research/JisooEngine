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

## 개발 명령

| 명령 | 사용 가능한 구성 | 동작 |
|---|---|---|
| `GenerateProjectFiles.bat` | 해당 없음 | CMake를 사용하여 Visual Studio 2022 프로젝트 파일을 생성하거나 갱신합니다. |
| `OpenVisualStudio.bat` | 해당 없음 | 솔루션이 없으면 생성하고 Visual Studio에서 엽니다. |
| `RunEditor.bat [구성]` | `Debug`(기본), `Release` | 프로젝트 파일이 없으면 생성하고, 에디터를 빌드한 뒤 실행합니다. |
| `BuildEngine.bat [구성]` | `Debug`(기본), `Release` | 프로젝트 파일이 없으면 생성하고 지정한 구성으로 전체 프로젝트를 빌드합니다. |

`[구성]`은 생략할 수 있으며, 생략하면 `Debug`를 사용합니다.

## 생성 위치

| 항목 | 경로 |
|---|---|
| Visual Studio 솔루션 | `Intermediate/Build/VS2022-x64/JisooEngine.sln` |
| 실행 파일과 실행 PDB | `Binaries/Win64/<구성>` |
| 정적 라이브러리 | `Intermediate/Libraries/Win64/<구성>` |
| 컴파일 중간 심볼 | `Intermediate/Symbols/Win64/<구성>` |

경로의 `<구성>`에는 `Debug` 또는 `Release`가 들어갑니다.

## 문서

- [엔진 아키텍처](docs/architecture.md)
- [좌표계와 변환 규칙](docs/conventions/coordinate-system.md)
- [개발 계획](docs/development-plan.md)
