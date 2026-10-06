#pragma once

#include "Editor/Engine/EditorViewportInputRouting.h"
#include "Runtime/Engine/Engine.h"

#include <memory>

class FEditorUI;
class FInputFrame;
class FViewport;

/** 공통 Editor 기능을 실행하기 위한 구체 엔진 타입이다. */
class FEditorEngine : public FEngine
{
public:
    FEditorEngine();
    ~FEditorEngine() override;

    bool Initialize(const FEngineInitParams& InitParams) override;
    void Shutdown() override;

protected:
    [[nodiscard]] FInputRouteContext BuildInputRouteContext(
        const FInputFrame& InputFrame,
        float DeltaSeconds) override;
    void RenderFrame() override;
    void OnWindowResized(std::uint32_t Width, std::uint32_t Height) override;

private:
    std::unique_ptr<FEditorUI> EditorUI;
    FEditorViewportInputRouting ViewportInputRouting;
};
