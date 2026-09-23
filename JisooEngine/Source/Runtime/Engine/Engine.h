#pragma once

#include <cstdint>
#include <memory>

class FRenderer;

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
 * 런타임 엔진의 생애주기를 제공하고 Renderer를 소유한다.
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

    /** 초기화된 엔진의 한 프레임을 갱신한다. */
    virtual void Tick();

    /** 소유한 런타임 시스템을 해제하며, 반복 호출해도 안전하다. */
    virtual void Shutdown();

private:
    std::unique_ptr<FRenderer> Renderer;
};
