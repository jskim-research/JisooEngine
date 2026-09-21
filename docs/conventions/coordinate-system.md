# 좌표계와 변환 규칙

JisooEngine의 공간, 단위, 회전 및 행렬 규칙을 정의한다. 수학 코드, 렌더링 코드와 셰이더는 이 문서를 공통 기준으로 사용한다.

## 기본 규칙

| 항목 | 규칙 |
|---|---|
| 좌표계 | 왼손 좌표계 |
| Forward | +X |
| Right | +Y |
| Up | +Z |
| 월드 거리 단위 | 센티미터(`cm`) |
| 외부 회전 각도 단위 | 도(`degree`) |

삼각 함수처럼 라디안을 요구하는 연산은 수학 계층의 경계에서 명시적으로 변환한다. 단위 변환이 필요한 API는 함수명이나 타입을 통해 단위를 드러낸다.

## 회전

| 회전 | 축 | 양의 방향 |
|---|---|---|
| Roll | +X | 양의 축 위에서 원점을 바라볼 때 시계 방향 |
| Pitch | +Y | 양의 축 위에서 원점을 바라볼 때 시계 방향 |
| Yaw | +Z | 양의 축 위에서 원점을 바라볼 때 시계 방향 |

양의 회전은 DirectXMath의 회전 부호 규칙을 따른다.

Euler 회전의 합성 순서는 회전 API를 구현할 때 명시적으로 확정한다. 순서가 확정되기 전에는 호출부에서 암묵적인 Euler 합성 순서를 가정하지 않는다.

## 벡터와 행렬

| 항목 | 규칙 |
|---|---|
| 행렬 저장 | 행 우선(`row-major`) |
| 벡터 표현 | 행벡터 |
| 벡터 변환 | `Vector * Matrix` |
| 위치 벡터의 동차 좌표 | `w = 1` |
| 방향 벡터의 동차 좌표 | `w = 0` |

변환 행렬은 다음 순서로 조합한다.

```text
Local = Scale * Rotation * Translation
World = Local * ParentWorld
ClipPosition = LocalPosition * World * View * Projection
```

## CPU와 HLSL

| 구간 | 규칙 |
|---|---|
| CPU | 행 우선 행렬과 행벡터 사용 |
| HLSL 행렬 선언 | `row_major` 사용 |
| HLSL 행렬 곱 | `mul(Vector, Matrix)` 사용 |
| CPU에서 GPU로 전달 | 관례적인 전치 행렬을 만들지 않음 |
| 상수 버퍼 | CPU 구조체와 셰이더의 메모리 배치를 일치시킴 |

```hlsl
row_major float4x4 WorldViewProjection;
float4 ClipPosition = mul(float4(LocalPosition, 1.0f), WorldViewProjection);
```

## Direct3D NDC

| 축 | 범위 |
|---|---|
| X | `[-1, 1]` |
| Y | `[-1, 1]` |
| 깊이 | `[0, 1]` |

## 추후 확정할 규칙

다음 항목은 관련 시스템을 구현할 때 별도 그래픽스 컨벤션으로 확정한다.

- Euler 회전 합성 순서
- UV 원점
- 삼각형 앞면의 winding
- 에셋 가져오기 과정의 축과 단위 변환
