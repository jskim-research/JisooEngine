#include "Framework/TestRunner.h"

#include "Runtime/Input/InputRouter.h"
#include "Runtime/Input/InputSystem.h"

namespace
{
    class FTrackingInputReceiver final : public IInputReceiver
    {
    public:
        void ProcessInput(const FInputFrame& InputFrame, float DeltaSeconds) override
        {
            ++ProcessCount;
            LastFrame = InputFrame;
            LastDeltaSeconds = DeltaSeconds;
        }

        FInputFrame LastFrame;
        float LastDeltaSeconds = 0.0f;
        int ProcessCount = 0;
    };

    FInputEvent MakeEvent(EInputEventType Type, EInputKey Key = EInputKey::Unknown)
    {
        FInputEvent Event;
        Event.Type = Type;
        Event.Key = Key;
        return Event;
    }
}

void RegisterInputTests(FTestRunner& Runner)
{
    Runner.Add(
        "Input.FramePreservesHeldStateAndResetsTransitions",
        [](FTestContext& Test)
        {
            FInputSystem InputSystem;
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::FocusGained));
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::KeyDown, EInputKey::W));

            FInputEvent PointerMove = MakeEvent(EInputEventType::PointerMove);
            PointerMove.PositionX = 320;
            PointerMove.PositionY = 180;
            PointerMove.DeltaX = 7;
            PointerMove.DeltaY = -3;
            InputSystem.EnqueueInputEvent(PointerMove);

            FInputEvent Wheel = MakeEvent(EInputEventType::PointerWheel);
            Wheel.WheelDelta = 1.0f;
            InputSystem.EnqueueInputEvent(Wheel);
            InputSystem.AdvanceFrame();

            const FInputFrame& FirstFrame = InputSystem.GetCurrentFrame();
            Test.Expect(FirstFrame.IsDown(EInputKey::W), "KeyDown 뒤에는 키가 눌린 상태여야 한다.");
            Test.Expect(FirstFrame.WasPressed(EInputKey::W), "KeyDown 전이는 발생한 프레임에 기록되어야 한다.");
            Test.Expect(FirstFrame.GetPointerDeltaX() == 7, "프레임의 Pointer X Delta를 누적해야 한다.");
            Test.Expect(FirstFrame.GetPointerDeltaY() == -3, "프레임의 Pointer Y Delta를 누적해야 한다.");
            Test.Expect(FirstFrame.GetWheelDelta() == 1.0f, "프레임의 Wheel Delta를 누적해야 한다.");

            InputSystem.AdvanceFrame();

            const FInputFrame& SecondFrame = InputSystem.GetCurrentFrame();
            Test.Expect(SecondFrame.IsDown(EInputKey::W), "새 이벤트가 없어도 Down 상태는 유지되어야 한다.");
            Test.Expect(!SecondFrame.WasPressed(EInputKey::W), "Pressed 전이는 다음 프레임에 남으면 안 된다.");
            Test.Expect(SecondFrame.GetPointerDeltaX() == 0, "Pointer Delta는 다음 프레임에 초기화되어야 한다.");
            Test.Expect(SecondFrame.GetWheelDelta() == 0.0f, "Wheel Delta는 다음 프레임에 초기화되어야 한다.");

            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::KeyUp, EInputKey::W));
            InputSystem.AdvanceFrame();

            const FInputFrame& ThirdFrame = InputSystem.GetCurrentFrame();
            Test.Expect(!ThirdFrame.IsDown(EInputKey::W), "KeyUp 뒤에는 키가 눌리지 않은 상태여야 한다.");
            Test.Expect(ThirdFrame.WasReleased(EInputKey::W), "KeyUp 전이는 발생한 프레임에 기록되어야 한다.");
        });

    Runner.Add(
        "Input.FocusLossReleasesHeldKeys",
        [](FTestContext& Test)
        {
            FInputSystem InputSystem;
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::FocusGained));
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::KeyDown, EInputKey::A));
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::KeyDown, EInputKey::MouseRight));
            InputSystem.AdvanceFrame();

            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::FocusLost));
            InputSystem.AdvanceFrame();

            const FInputFrame& Frame = InputSystem.GetCurrentFrame();
            Test.Expect(!Frame.IsWindowFocused(), "FocusLost 뒤에는 Window Focus가 해제되어야 한다.");
            Test.Expect(!Frame.IsDown(EInputKey::A), "Focus를 잃으면 Keyboard Down 상태를 해제해야 한다.");
            Test.Expect(!Frame.IsDown(EInputKey::MouseRight), "Focus를 잃으면 Pointer Button Down 상태를 해제해야 한다.");
            Test.Expect(Frame.WasReleased(EInputKey::A), "FocusLost로 해제된 Keyboard 키에 Released를 기록해야 한다.");
            Test.Expect(Frame.WasReleased(EInputKey::MouseRight), "FocusLost로 해제된 Pointer Button에 Released를 기록해야 한다.");
        });

    Runner.Add(
        "Input.RouterSeparatesKeyboardAndPointerTargets",
        [](FTestContext& Test)
        {
            FInputSystem InputSystem;
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::FocusGained));
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::KeyDown, EInputKey::D));
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::KeyDown, EInputKey::MouseLeft));

            FInputEvent PointerMove = MakeEvent(EInputEventType::PointerMove);
            PointerMove.PositionX = 10;
            PointerMove.PositionY = 20;
            PointerMove.DeltaX = 4;
            PointerMove.DeltaY = 5;
            InputSystem.EnqueueInputEvent(PointerMove);
            InputSystem.AdvanceFrame();

            FTrackingInputReceiver KeyboardReceiver;
            FTrackingInputReceiver PointerReceiver;
            FInputRouter Router;
            Router.Route(
                InputSystem.GetCurrentFrame(),
                {&KeyboardReceiver, &PointerReceiver},
                0.25f);

            Test.Expect(KeyboardReceiver.ProcessCount == 1, "Keyboard Target은 프레임당 한 번 호출되어야 한다.");
            Test.Expect(KeyboardReceiver.LastFrame.IsDown(EInputKey::D), "Keyboard Target에는 Keyboard 상태가 전달되어야 한다.");
            Test.Expect(!KeyboardReceiver.LastFrame.IsDown(EInputKey::MouseLeft), "Keyboard Target에는 Pointer Button 상태를 노출하면 안 된다.");
            Test.Expect(PointerReceiver.ProcessCount == 1, "Pointer Target은 프레임당 한 번 호출되어야 한다.");
            Test.Expect(PointerReceiver.LastFrame.IsDown(EInputKey::MouseLeft), "Pointer Target에는 Pointer Button 상태가 전달되어야 한다.");
            Test.Expect(!PointerReceiver.LastFrame.IsDown(EInputKey::D), "Pointer Target에는 Keyboard 상태를 노출하면 안 된다.");
            Test.Expect(PointerReceiver.LastFrame.GetPointerDeltaX() == 4, "Pointer Target에는 Pointer Delta가 전달되어야 한다.");
        });

    Runner.Add(
        "Input.RouterCallsSharedTargetOnce",
        [](FTestContext& Test)
        {
            FInputSystem InputSystem;
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::FocusGained));
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::KeyDown, EInputKey::W));
            InputSystem.EnqueueInputEvent(MakeEvent(EInputEventType::KeyDown, EInputKey::MouseRight));
            InputSystem.AdvanceFrame();

            FTrackingInputReceiver Receiver;
            FInputRouter Router;
            Router.Route(InputSystem.GetCurrentFrame(), {&Receiver, &Receiver}, 0.5f);

            Test.Expect(Receiver.ProcessCount == 1, "같은 Receiver가 두 채널을 소유하면 중복 호출하면 안 된다.");
            Test.Expect(Receiver.LastFrame.IsDown(EInputKey::W), "공유 Target은 Keyboard 상태를 받아야 한다.");
            Test.Expect(Receiver.LastFrame.IsDown(EInputKey::MouseRight), "공유 Target은 Pointer 상태를 받아야 한다.");
            Test.Expect(Receiver.LastDeltaSeconds == 0.5f, "Router는 현재 DeltaSeconds를 Receiver에 전달해야 한다.");
        });
}
