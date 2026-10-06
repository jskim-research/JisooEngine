#include "Framework/TestRunner.h"

#include "Editor/Engine/EditorViewportInputRouting.h"
#include "Editor/UI/ViewportInputRegion.h"
#include "Editor/Viewport/EditorViewportClient.h"
#include "Editor/Viewport/EditorViewportLayout.h"
#include "Runtime/Engine/Viewport/SceneView.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Engine/Viewport/ViewportClient.h"
#include "Runtime/Input/InputSystem.h"
#include "Runtime/Render/Renderer.h"
#include "Runtime/Render/Scene/Scene.h"

#include <cmath>
#include <array>
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

    Runner.Add(
        "Viewport.InputRoutingMaintainsActiveHoverAndCapturePolicy",
        [](FTestContext& Test)
        {
            FViewport ViewportA({1, 1}, 800, 600);
            FViewport ViewportB({2, 1}, 400, 300);
            FDrawTrackingViewportClient ClientA;
            FDrawTrackingViewportClient ClientB;
            ViewportA.SetClient(&ClientA);
            ViewportB.SetClient(&ClientB);

            std::array<FViewportInputRegion, 2> Regions{{
                {&ViewportA, 0.0f, 0.0f, 400.0f, 300.0f, true},
                {&ViewportB, 400.0f, 0.0f, 800.0f, 300.0f, false}
            }};
            FInputSystem InputSystem;
            FInputEvent FocusEvent;
            FocusEvent.Type = EInputEventType::FocusGained;
            InputSystem.EnqueueInputEvent(FocusEvent);
            EnqueueKeyEvent(InputSystem, EInputEventType::KeyDown, EInputKey::MouseRight);
            InputSystem.AdvanceFrame();

            FEditorViewportInputRouting Routing;
            FEditorViewportInputRoutingResult Result = Routing.Route(
                InputSystem.GetCurrentFrame(),
                Regions,
                &ViewportB,
                false);
            Test.Expect(Result.ActivatedViewport == &ViewportA, "Hover된 A를 누르면 A가 활성 대상으로 보고되어야 한다.");
            Test.Expect(Result.Context.PointerTarget == &ClientA, "Pointer Capture는 누르기 시작한 A를 대상으로 해야 한다.");
            Test.Expect(Result.Context.KeyboardTarget == &ClientA, "활성화된 A가 같은 프레임 Keyboard 대상이어야 한다.");
            Test.Expect(IsNearlyEqual(Result.Context.PointerScaleX, 2.0f), "표시 폭과 A의 RenderTarget 폭 비율을 Pointer X 배율로 사용해야 한다.");

            Regions[0].bHovered = false;
            Regions[1].bHovered = true;
            InputSystem.AdvanceFrame();
            Result = Routing.Route(
                InputSystem.GetCurrentFrame(),
                Regions,
                &ViewportA,
                false);
            Test.Expect(Result.Context.PointerTarget == &ClientA, "누른 채 B로 이동해도 Pointer는 Capture된 A에 남아야 한다.");
            Test.Expect(Result.Context.KeyboardTarget == &ClientA, "Hover 이동은 지속 Active Viewport를 바꾸면 안 된다.");

            EnqueueKeyEvent(InputSystem, EInputEventType::KeyUp, EInputKey::MouseRight);
            InputSystem.AdvanceFrame();
            Result = Routing.Route(
                InputSystem.GetCurrentFrame(),
                Regions,
                &ViewportA,
                false);
            Test.Expect(Result.Context.PointerTarget == &ClientA, "Button Release까지 Capture된 A에 전달되어야 한다.");

            InputSystem.AdvanceFrame();
            Result = Routing.Route(
                InputSystem.GetCurrentFrame(),
                Regions,
                &ViewportA,
                true);
            Test.Expect(Result.Context.PointerTarget == &ClientB, "Capture 해제 다음 프레임부터 Hover된 B가 Pointer 대상이어야 한다.");
            Test.Expect(Result.Context.KeyboardTarget == nullptr, "ImGui가 Keyboard를 점유하면 Active Viewport 전달을 막아야 한다.");
        });

    Runner.Add(
        "Viewport.InputRoutingDropsCaptureWhenViewportDisappears",
        [](FTestContext& Test)
        {
            FViewport ViewportA({1, 1}, 800, 600);
            FViewport ViewportB({2, 1}, 800, 600);
            FDrawTrackingViewportClient ClientA;
            FDrawTrackingViewportClient ClientB;
            ViewportA.SetClient(&ClientA);
            ViewportB.SetClient(&ClientB);

            FInputSystem InputSystem;
            FInputEvent FocusEvent;
            FocusEvent.Type = EInputEventType::FocusGained;
            InputSystem.EnqueueInputEvent(FocusEvent);
            EnqueueKeyEvent(InputSystem, EInputEventType::KeyDown, EInputKey::MouseLeft);
            InputSystem.AdvanceFrame();
            FEditorViewportInputRouting Routing;
            const std::array<FViewportInputRegion, 1> RegionA{{
                {&ViewportA, 0.0f, 0.0f, 800.0f, 600.0f, true}
            }};
            const FEditorViewportInputRoutingResult InitialResult = Routing.Route(
                InputSystem.GetCurrentFrame(),
                RegionA,
                &ViewportA,
                false);
            Test.Expect(InitialResult.Context.PointerTarget == &ClientA, "A를 누른 프레임에 Pointer Capture를 시작해야 한다.");

            InputSystem.AdvanceFrame();
            const std::array<FViewportInputRegion, 1> RegionB{{
                {&ViewportB, 0.0f, 0.0f, 800.0f, 600.0f, true}
            }};
            const FEditorViewportInputRoutingResult Result = Routing.Route(
                InputSystem.GetCurrentFrame(),
                RegionB,
                &ViewportB,
                false);
            Test.Expect(Result.Context.PointerTarget == &ClientB, "Capture된 A가 사라지면 만료된 대상을 버리고 Hover된 B를 선택해야 한다.");
        });
}
