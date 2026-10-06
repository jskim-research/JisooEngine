#include "Runtime/Input/InputRouter.h"

#include <cmath>

void FInputRouter::TransformPointerToTarget(
    FInputFrame& InputFrame,
    const FInputRouteContext& Context)
{
    if (!Context.bTransformPointerToTarget)
    {
        return;
    }

    InputFrame.PointerX = static_cast<std::int32_t>(std::lround(
        static_cast<float>(InputFrame.PointerX - Context.PointerOriginX) *
        Context.PointerScaleX));
    InputFrame.PointerY = static_cast<std::int32_t>(std::lround(
        static_cast<float>(InputFrame.PointerY - Context.PointerOriginY) *
        Context.PointerScaleY));
    InputFrame.PointerDeltaX = static_cast<std::int32_t>(std::lround(
        static_cast<float>(InputFrame.PointerDeltaX) * Context.PointerScaleX));
    InputFrame.PointerDeltaY = static_cast<std::int32_t>(std::lround(
        static_cast<float>(InputFrame.PointerDeltaY) * Context.PointerScaleY));
}

void FInputRouter::Route(
    const FInputFrame& InputFrame,
    const FInputRouteContext& Context,
    float DeltaSeconds) const
{
    if (Context.KeyboardTarget != nullptr &&
        Context.KeyboardTarget == Context.PointerTarget)
    {
        FInputFrame RoutedFrame = InputFrame;
        TransformPointerToTarget(RoutedFrame, Context);
        Context.KeyboardTarget->ProcessInput(RoutedFrame, DeltaSeconds);
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
        TransformPointerToTarget(PointerFrame, Context);
        Context.PointerTarget->ProcessInput(PointerFrame, DeltaSeconds);
    }
}
