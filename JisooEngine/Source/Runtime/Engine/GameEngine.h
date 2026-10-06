#pragma once

#include "Runtime/Engine/Engine.h"

#include <memory>

class FGameViewportClient;
class FViewport;

/** 에디터 기능 없이 독립 게임 실행에 사용하는 구체 엔진 타입이다. */
class FGameEngine : public FEngine
{
public:
    FGameEngine();
    ~FGameEngine() override;

    bool Initialize(const FEngineInitParams& InitParams) override;
    void Shutdown() override;

protected:
    [[nodiscard]] FInputRouteContext BuildInputRouteContext(
        const FInputFrame& InputFrame,
        float DeltaSeconds) override;
    void RenderFrame() override;
    void OnWindowResized(std::uint32_t Width, std::uint32_t Height) override;

private:
    std::unique_ptr<FViewport> Viewport;
    std::unique_ptr<FGameViewportClient> ViewportClient;
};
