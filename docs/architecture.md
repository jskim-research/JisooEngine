# 엔진 아키텍처

JisooEngine의 모듈별 책임과 의존 방향을 정의한다. 새 코드를 배치하거나 모듈 사이의 참조를 추가할 때 이 문서를 기준으로 판단한다.

## 소스 구조

모듈과 소스 디렉터리 구조는 Unreal Engine 5의 구조를 기준으로 현재 프로젝트 규모에 맞게 단순화하여 적용한다. `Runtime`, `Editor` 같은 분류 디렉터리 바로 아래에는 소스 파일을 두지 않고, `Launch`, `Core`, `Rendering`처럼 책임이 명확한 하위 디렉터리에 배치한다.

## 모듈 책임

| 모듈 | CMake 타깃 | 형태 | 책임 |
|---|---|---|---|
| Engine | `JisooEngine` | 정적 라이브러리 | 게임과 에디터에서 공통으로 사용하는 범용 런타임 기능 |
| Editor | `JisooEditor` | 실행 파일 | 엔진 데이터와 콘텐츠의 제작·검사 도구 |
| Game | `JisooGame` | 실행 파일 | 프로젝트 고유의 게임 실행 로직과 콘텐츠 규칙 |

## 의존 규칙

| 참조하는 모듈 | Engine 참조 | Editor 참조 | Game 참조 |
|---|---:|---:|---:|
| Engine | 해당 없음 | 금지 | 금지 |
| Editor | 허용 | 해당 없음 | 금지 |
| Game | 허용 | 금지 | 해당 없음 |

기본 의존 방향은 다음과 같다.

```text
Editor ──> Engine
Game   ──> Engine
```

이에 따라 다음 조건을 유지한다.

- Engine은 특정 게임의 규칙이나 콘텐츠를 알지 않는다.
- Engine은 에디터 UI와 편집 도구에 의존하지 않는다.
- Game은 Editor를 초기화하거나 링크하지 않고 독립적으로 실행할 수 있어야 한다.
- Editor와 Game의 공통 기능은 범용 런타임 기능일 때만 Engine의 공개 인터페이스로 옮긴다.

## 코드 배치 기준

| 코드의 역할 | 배치할 모듈 |
|---|---|
| 게임과 도구 양쪽에서 사용할 수 있는 범용 런타임 기능 | Engine |
| 콘텐츠 제작, 검사, 시각화 등 편집 과정에만 필요한 기능 | Editor |
| 프로젝트 고유 규칙이나 실제 게임 실행에만 필요한 기능 | Game |

Engine 코드에서 Editor 또는 Game 타입이 필요해지면 구현을 진행하기 전에 책임과 의존 방향을 다시 검토한다.

## 실행 진입점

| 실행 파일 | 진입점 | 사용하는 런타임 |
|---|---|---|
| `JisooEditor` | `Editor/Source/Launch/EditorMain.cpp` | Engine |
| `JisooGame` | `Game/Source/Launch/GameMain.cpp` | Engine |
