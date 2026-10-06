#include "Runtime/Engine/Viewport/Viewport.h"

#include "Runtime/Engine/Viewport/ViewportClient.h"

FViewport::FViewport(
    FRenderTargetHandle Target,
    std::uint32_t Width,
    std::uint32_t Height)
    : Output{Target, Width, Height}
{
}

void FViewport::SetClient(FViewportClient* InClient) noexcept
{
    Client = InClient;
}

FViewportClient* FViewport::GetClient() const noexcept
{
    return Client;
}

void FViewport::Draw(FRenderer& Renderer)
{
    if (Client != nullptr && IsRenderable())
    {
        Client->Draw(*this, Renderer);
    }
}

void FViewport::Resize(std::uint32_t Width, std::uint32_t Height) noexcept
{
    Output.Width = Width;
    Output.Height = Height;
}

const FViewportOutput& FViewport::GetOutput() const noexcept
{
    return Output;
}

bool FViewport::IsRenderable() const noexcept
{
    return Output.Target.IsSet() && Output.Width > 0 && Output.Height > 0;
}
