#include "Editor/UI/ViewportPanel.h"

#include "Editor/Viewport/EditorViewportLayout.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Render/Renderer.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>

FViewportPanel::FViewportPanel() = default;

FViewportPanel::~FViewportPanel()
{
    Shutdown();
}

bool FViewportPanel::Initialize(FRenderer& InRenderer, const FScene* InScene)
{
    if (Renderer != nullptr)
    {
        return true;
    }
    if (InScene == nullptr)
    {
        return false;
    }

    Renderer = &InRenderer;
    Scene = InScene;
    bOpen = true;
    return true;
}

void FViewportPanel::Shutdown()
{
    ReleaseLayout();
    Renderer = nullptr;
    Scene = nullptr;
    bOpen = false;
}

void FViewportPanel::Draw(std::vector<FViewportInputRegion>& OutRegions)
{
    VisibleViewports.clear();
    if (!bOpen)
    {
        ReleaseLayout();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    const bool bDrawContents = ImGui::Begin("Viewport", &bOpen);
    if (bDrawContents && bOpen && Renderer != nullptr)
    {
        const ImVec2 AvailableSize = ImGui::GetContentRegionAvail();
        if (AvailableSize.x >= 1.0f && AvailableSize.y >= 1.0f)
        {
            const auto Width = static_cast<std::uint32_t>(std::floor(AvailableSize.x));
            const auto Height = static_cast<std::uint32_t>(std::floor(AvailableSize.y));
            if (EnsureLayout(Width, Height))
            {
                FViewport* Viewport = Layout->GetVisibleViewports().front();
                const FViewportOutput& Output = Viewport->GetOutput();
                if ((Output.Width != Width || Output.Height != Height) &&
                    Renderer->ResizeRenderTarget(RenderTarget, Width, Height))
                {
                    Viewport->Resize(Width, Height);
                }

                const D3D12_GPU_DESCRIPTOR_HANDLE Texture =
                    Renderer->GetRenderTargetShaderResourceView(RenderTarget);
                if (Texture.ptr != 0)
                {
                    ImGui::Image(
                        ImTextureRef(static_cast<ImTextureID>(Texture.ptr)),
                        AvailableSize);

                    const ImVec2 ItemMin = ImGui::GetItemRectMin();
                    const ImVec2 ItemMax = ImGui::GetItemRectMax();
                    const bool bHovered = ImGui::IsItemHovered();
                    if (bHovered)
                    {
                        for (int ButtonIndex = 0;
                             ButtonIndex < ImGuiMouseButton_COUNT;
                             ++ButtonIndex)
                        {
                            if (ImGui::IsMouseClicked(ButtonIndex))
                            {
                                Layout->SetActiveViewport(Viewport);
                                break;
                            }
                        }
                    }
                    OutRegions.push_back({
                        Viewport,
                        ItemMin.x,
                        ItemMin.y,
                        ItemMax.x,
                        ItemMax.y,
                        bHovered
                    });
                    VisibleViewports.push_back(Viewport);
                }
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();

    if (!bOpen)
    {
        VisibleViewports.clear();
        ReleaseLayout();
    }
}

void FViewportPanel::SetOpen(bool bInOpen)
{
    bOpen = bInOpen;
    if (!bOpen)
    {
        ReleaseLayout();
    }
}

bool FViewportPanel::IsOpen() const noexcept
{
    return bOpen;
}

FViewport* FViewportPanel::GetActiveViewport() const noexcept
{
    return Layout != nullptr ? Layout->GetActiveViewport() : nullptr;
}

const std::vector<FViewport*>& FViewportPanel::GetVisibleViewports() const noexcept
{
    return VisibleViewports;
}

bool FViewportPanel::EnsureLayout(std::uint32_t Width, std::uint32_t Height)
{
    if (Layout != nullptr)
    {
        return true;
    }
    if (Renderer == nullptr || Scene == nullptr || Width == 0 || Height == 0)
    {
        return false;
    }

    RenderTarget = Renderer->CreateTextureRenderTarget(Width, Height);
    if (!RenderTarget.IsSet())
    {
        return false;
    }

    auto NewLayout = std::make_unique<FEditorViewportLayout>();
    if (!NewLayout->Initialize(Scene, RenderTarget, Width, Height))
    {
        Renderer->ReleaseRenderTarget(RenderTarget);
        RenderTarget = {};
        return false;
    }

    Layout = std::move(NewLayout);
    return true;
}

void FViewportPanel::ReleaseLayout()
{
    VisibleViewports.clear();
    Layout.reset();
    if (Renderer != nullptr && RenderTarget.IsSet())
    {
        Renderer->ReleaseRenderTarget(RenderTarget);
    }
    RenderTarget = {};
}
