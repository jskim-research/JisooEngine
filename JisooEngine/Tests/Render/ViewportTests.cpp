#include "Framework/TestRunner.h"

#include "Editor/Viewport/EditorViewportClient.h"
#include "Editor/Viewport/EditorViewportLayout.h"
#include "Runtime/Engine/Viewport/SceneView.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Engine/Viewport/ViewportClient.h"
#include "Runtime/Input/InputSystem.h"
#include "Runtime/Render/Renderer.h"
#include "Runtime/Render/Scene/Scene.h"

#include <cmath>
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

    class FViewBuildingViewportClient final : public FViewportClient
    {
    public:
        [[nodiscard]] FSceneView BuildView(const FViewport& Viewport) const
        {
            return BuildSceneView(Viewport);
        }
    };

    bool IsNearlyEqual(float Left, float Right, float Tolerance = 0.001f)
    {
        return std::abs(Left - Right) <= Tolerance;
    }

    void EnqueueKeyEvent(FInputSystem& InputSystem, EInputEventType Type, EInputKey Key)
    {
        FInputEvent Event;
        Event.Type = Type;
        Event.Key = Key;
        InputSystem.EnqueueInputEvent(Event);
    }
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
            Test.Expect(Layout.GetActiveViewport() == VisibleViewports.front(), "Single Layout의 첫 Viewport는 초기 활성 대상이어야 한다.");
            Test.Expect(Layout.GetActiveViewportClient() == VisibleViewports.front()->GetClient(), "활성 Client는 활성 Viewport에 연결된 Client여야 한다.");
            Test.Expect(Layout.GetLayoutMode() == EEditorViewportLayoutMode::Single, "초기 LayoutMode는 Single이어야 한다.");
        });

    Runner.Add(
        "Viewport.EditorCameraMovesWithHeldKeys",
        [](FTestContext& Test)
        {
            FInputSystem InputSystem;
            FInputEvent FocusEvent;
            FocusEvent.Type = EInputEventType::FocusGained;
            InputSystem.EnqueueInputEvent(FocusEvent);
            EnqueueKeyEvent(InputSystem, EInputEventType::KeyDown, EInputKey::W);
            InputSystem.AdvanceFrame();

            FEditorViewportClient Client;
            Client.ProcessInput(InputSystem.GetCurrentFrame(), 0.5f);

            const FVector& Position = Client.GetCameraPosition();
            Test.Expect(IsNearlyEqual(Position.X, 250.0f), "W 입력은 기본 +X Forward로 초당 이동 속도와 DeltaSeconds를 반영해야 한다.");
            Test.Expect(IsNearlyEqual(Position.Y, 0.0f), "Forward 이동은 기본 상태에서 Right 축 위치를 바꾸면 안 된다.");
            Test.Expect(IsNearlyEqual(Position.Z, 0.0f), "Forward 이동은 기본 상태에서 Up 축 위치를 바꾸면 안 된다.");
        });

    Runner.Add(
        "Viewport.EditorCameraRotatesWhileRightMouseHeld",
        [](FTestContext& Test)
        {
            FInputSystem InputSystem;
            FInputEvent FocusEvent;
            FocusEvent.Type = EInputEventType::FocusGained;
            InputSystem.EnqueueInputEvent(FocusEvent);
            EnqueueKeyEvent(InputSystem, EInputEventType::KeyDown, EInputKey::MouseRight);

            FInputEvent PointerEvent;
            PointerEvent.Type = EInputEventType::PointerMove;
            PointerEvent.DeltaX = 100;
            PointerEvent.DeltaY = -200;
            InputSystem.EnqueueInputEvent(PointerEvent);
            InputSystem.AdvanceFrame();

            FEditorViewportClient Client;
            Client.ProcessInput(InputSystem.GetCurrentFrame(), 0.0f);

            Test.Expect(IsNearlyEqual(Client.GetCameraYawDegrees(), 15.0f), "우클릭 중 가로 Pointer Delta가 Yaw에 감도를 적용해야 한다.");
            Test.Expect(IsNearlyEqual(Client.GetCameraPitchDegrees(), -30.0f), "우클릭 중 세로 Pointer Delta가 Pitch에 감도를 적용해야 한다.");

            EnqueueKeyEvent(InputSystem, EInputEventType::KeyUp, EInputKey::MouseRight);
            PointerEvent.DeltaX = 50;
            PointerEvent.DeltaY = 50;
            InputSystem.EnqueueInputEvent(PointerEvent);
            InputSystem.AdvanceFrame();
            Client.ProcessInput(InputSystem.GetCurrentFrame(), 0.0f);

            Test.Expect(IsNearlyEqual(Client.GetCameraYawDegrees(), 15.0f), "우클릭을 놓은 뒤 Pointer Delta는 Yaw를 바꾸면 안 된다.");
            Test.Expect(IsNearlyEqual(Client.GetCameraPitchDegrees(), -30.0f), "우클릭을 놓은 뒤 Pointer Delta는 Pitch를 바꾸면 안 된다.");
        });

    Runner.Add(
        "Viewport.CameraRotationAffectsViewMatrix",
        [](FTestContext& Test)
        {
            constexpr FRenderTargetHandle Target{0, 1};
            FViewport Viewport(Target, 1280, 720);
            FViewBuildingViewportClient Client;
            Client.SetCameraRotationDegrees(0.0f, 90.0f);

            const FSceneView View = Client.BuildView(Viewport);

            Test.Expect(IsNearlyEqual(View.ViewMatrix.M[0][2], 0.0f), "Yaw 90도에서 월드 +X는 View Forward 성분을 가지면 안 된다.");
            Test.Expect(IsNearlyEqual(View.ViewMatrix.M[1][2], 1.0f), "Yaw 90도에서 월드 +Y가 View Forward가 되어야 한다.");
            Test.Expect(IsNearlyEqual(View.ViewMatrix.M[0][0], -1.0f), "Yaw 90도에서 View Right는 월드 -X여야 한다.");
        });
}
