#pragma once

#include "Runtime/Engine/Engine.h"

#include <memory>

class FEditorViewportLayout;

/** 공통 Editor 기능을 실행하기 위한 구체 엔진 타입이다. */
class FEditorEngine : public FEngine
{
public:
    FEditorEngine();
    ~FEditorEngine() override;

    bool Initialize(const FEngineInitParams& InitParams) override;
    void Shutdown() override;

protected:
    [[nodiscard]] FInputRouteContext BuildInputRouteContext() override;
    void RenderFrame() override;

private:
    std::unique_ptr<FEditorViewportLayout> ViewportLayout;
};
