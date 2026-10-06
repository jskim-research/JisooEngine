#include "Runtime/Input/InputRouter.h"

void FInputRouter::Route(
    const FInputFrame& InputFrame,
    const FInputRouteContext& Context,
    float DeltaSeconds) const
{
    if (Context.KeyboardTarget != nullptr &&
        Context.KeyboardTarget == Context.PointerTarget)
    {
        Context.KeyboardTarget->ProcessInput(InputFrame, DeltaSeconds);
        return;
    }

    if (Context.KeyboardTarget != nullptr)
    {
        FInputFrame KeyboardFrame = InputFrame;
        KeyboardFrame.ClearPointerInput();
        Context.KeyboardTarget->ProcessInput(KeyboardFrame, DeltaSeconds);
    }

    if (Context.PointerTarget != nullptr)
    {
        FInputFrame PointerFrame = InputFrame;
        PointerFrame.ClearKeyboardInput();
        Context.PointerTarget->ProcessInput(PointerFrame, DeltaSeconds);
    }
}
