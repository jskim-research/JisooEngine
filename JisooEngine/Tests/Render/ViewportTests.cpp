#include "Framework/TestRunner.h"

#include "Editor/Viewport/EditorViewportLayout.h"
#include "Runtime/Engine/Viewport/SceneView.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Engine/Viewport/ViewportClient.h"
#include "Runtime/Render/Renderer.h"
#include "Runtime/Render/Scene/Scene.h"

#include <vector>

namespace
{
    class FDrawTrackingViewportClient final : public FViewportClient
    {
    public:
        void Draw(FViewport& Viewport, FRenderer&) override
        {
            ++DrawCount;
            LastViewport = &Viewport;
        }

        int DrawCount = 0;
        FViewport* LastViewport = nullptr;
    };
}

void RegisterViewportTests(FTestRunner& Runner)
{
    Runner.Add(
        "Viewport.DrawForwardsToAssociatedClient",
        [](FTestContext& Test)
        {
            constexpr FRenderTargetHandle Target{0, 1};
            FViewport Viewport(Target, 1280, 720);
            FDrawTrackingViewportClient Client;
            FRenderer Renderer;
            Viewport.SetClient(&Client);

            Viewport.Draw(Renderer);

            Test.Expect(Client.DrawCount == 1, "Viewport Draw가 연결된 Client를 한 번 호출해야 한다.");
            Test.Expect(Client.LastViewport == &Viewport, "Client가 Draw를 요청한 Viewport를 받아야 한다.");
        });

    Runner.Add(
        "Viewport.SceneViewFamilyOwnsFrameViews",
        [](FTestContext& Test)
        {
            constexpr FRenderTargetHandle Target{2, 3};
            FScene Scene;
            FSceneViewFamily Family;
            Family.Scene = &Scene;
            Family.Output = {Target, 1280, 720};
            Family.Views.push_back({
                FMatrix::Identity(),
                FMatrix::Identity(),
                {0, 0, 1280, 720},
                {}});

            Test.Expect(Family.Views.size() == 1, "Family가 프레임에 사용할 SceneView 목록을 보관해야 한다.");
            Test.Expect(Family.Views.front().ViewRect.Width == 1280, "SceneView가 자신의 pixel 출력 영역을 보관해야 한다.");
            Test.Expect(Family.Scene == &Scene, "Family가 렌더할 Scene을 명시해야 한다.");
            Test.Expect(Family.Output.Target == Target, "Family가 Renderer에서 해석할 Target Handle을 값으로 보관해야 한다.");
        });

    Runner.Add(
        "Viewport.RejectsMissingRenderTarget",
        [](FTestContext& Test)
        {
            FViewport Viewport({}, 1280, 720);
            FDrawTrackingViewportClient Client;
            FRenderer Renderer;
            Viewport.SetClient(&Client);

            Viewport.Draw(Renderer);

            Test.Expect(!Viewport.IsRenderable(), "Target Handle이 없는 Viewport는 렌더 가능한 출력으로 취급하면 안 된다.");
            Test.Expect(Client.DrawCount == 0, "유효한 Target이 없으면 Client에 Draw를 전달하면 안 된다.");
        });

    Runner.Add(
        "Viewport.EditorLayoutStartsWithSingleVisibleSlot",
        [](FTestContext& Test)
        {
            constexpr FRenderTargetHandle Target{0, 1};
            FScene Scene;
            FEditorViewportLayout Layout;

            Test.Expect(Layout.Initialize(&Scene, Target, 1280, 720), "유효한 Scene, Target과 크기로 Editor Layout을 초기화해야 한다.");
            Test.Expect(Layout.GetCreatedSlotCount() == 1, "초기 구현은 최대 네 Slot 중 Slot 0만 생성해야 한다.");
            const std::vector<FViewport*> VisibleViewports = Layout.GetVisibleViewports();
            Test.Expect(VisibleViewports.size() == 1, "Single Layout은 Viewport 하나만 표시해야 한다.");
            Test.Expect(VisibleViewports.front()->GetOutput().Target == Target, "Layout이 Renderer가 발급한 Target Handle을 Slot Viewport에 보존해야 한다.");
            Test.Expect(Layout.GetLayoutMode() == EEditorViewportLayoutMode::Single, "초기 LayoutMode는 Single이어야 한다.");
        });
}
