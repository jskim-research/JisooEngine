#pragma once

#include "Runtime/CoreUObject/ObjectHandle.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class FRenderer;
class UWorld;

/**
 * 엔진 초기화에 필요한 플랫폼 창 정보를 전달한다.
 * ClientWidth와 ClientHeight는 Window Client Area의 pixel 크기다.
 */
struct FEngineInitParams
{
    void* NativeWindowHandle = nullptr;
    std::uint32_t ClientWidth = 0;
    std::uint32_t ClientHeight = 0;
};

/**
 * 런타임 엔진의 생애주기를 제공하고 World 컨테이너와 Renderer를 소유한다.
 * 파생 엔진은 Initialize, Tick, Shutdown 순서를 유지하면서 기능을 확장한다.
 */
class FEngine
{
public:
    FEngine();
    virtual ~FEngine();

    /**
     * 플랫폼 창에 연결된 Renderer를 생성한다.
     * 초기화가 완료된 뒤 다시 호출하면 현재 상태를 유지하고 성공을 반환한다.
     */
    virtual bool Initialize(const FEngineInitParams& InitParams);

    /** 모든 World와 Renderer를 갱신한 뒤 파괴 대기 UObject를 처리한다. */
    virtual void Tick(float DeltaSeconds);

    /** 소유한 런타임 시스템을 해제하며, 반복 호출해도 안전하다. */
    virtual void Shutdown();

    /** Engine이 관리하고 Tick할 World를 생성한다. 초기화 이후 생성하면 즉시 BeginPlay한다. */
    [[nodiscard]] UWorld* CreateWorld(std::string Name = {});

    /** World를 Tick 컨테이너에서 제외하고 파괴를 예약한다. */
    bool DestroyWorld(UWorld* World);

    [[nodiscard]] std::vector<UWorld*> GetWorlds() const;

private:
    std::unique_ptr<FRenderer> Renderer;
    std::vector<FObjectHandle> Worlds;
    bool bInitialized = false;
};
