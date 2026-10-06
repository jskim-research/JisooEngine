#include "Editor/Viewport/EditorViewportLayout.h"

#include "Editor/Viewport/EditorViewportClient.h"
#include "Runtime/Engine/Viewport/Viewport.h"

FEditorViewportLayout::FEditorViewportLayout() = default;

FEditorViewportLayout::~FEditorViewportLayout()
{
    Shutdown();
}

bool FEditorViewportLayout::Initialize(
    const FScene* Scene,
    FRenderTargetHandle Target,
    std::uint32_t Width,
    std::uint32_t Height)
{
    if (Slots[0] != nullptr)
    {
        return true;
    }

    if (Scene == nullptr || !Target.IsSet() || Width == 0 || Height == 0)
    {
        return false;
    }

    auto Slot = std::make_unique<FEditorViewportSlot>();
    Slot->Viewport = std::make_unique<FViewport>(
        Target,
        Width,
        Height);
    Slot->Client = std::make_unique<FEditorViewportClient>();
    Slot->Client->SetScene(Scene);
    Slot->Client->SetViewMode(EViewMode::Lit);
    Slot->Viewport->SetClient(Slot->Client.get());
    Slots[0] = std::move(Slot);
    ActiveViewport = Slots[0]->Viewport.get();
    LayoutMode = EEditorViewportLayoutMode::Single;
    return true;
}

void FEditorViewportLayout::Shutdown()
{
    ActiveViewport = nullptr;
    for (std::unique_ptr<FEditorViewportSlot>& Slot : Slots)
    {
        Slot.reset();
    }
    LayoutMode = EEditorViewportLayoutMode::Single;
}

std::vector<FViewport*> FEditorViewportLayout::GetVisibleViewports() const
{
    std::vector<FViewport*> Result;
    if (Slots[0] != nullptr && Slots[0]->Viewport->IsRenderable())
    {
        Result.push_back(Slots[0]->Viewport.get());
    }
    return Result;
}

bool FEditorViewportLayout::SetActiveViewport(FViewport* Viewport) noexcept
{
    for (const std::unique_ptr<FEditorViewportSlot>& Slot : Slots)
    {
        if (Slot != nullptr && Slot->Viewport.get() == Viewport)
        {
            ActiveViewport = Viewport;
            return true;
        }
    }
    return false;
}

FViewport* FEditorViewportLayout::GetActiveViewport() const noexcept
{
    return ActiveViewport;
}

FEditorViewportClient* FEditorViewportLayout::GetActiveViewportClient() const noexcept
{
    for (const std::unique_ptr<FEditorViewportSlot>& Slot : Slots)
    {
        if (Slot != nullptr && Slot->Viewport.get() == ActiveViewport)
        {
            return Slot->Client.get();
        }
    }
    return nullptr;
}

std::size_t FEditorViewportLayout::GetCreatedSlotCount() const noexcept
{
    std::size_t Count = 0;
    for (const std::unique_ptr<FEditorViewportSlot>& Slot : Slots)
    {
        Count += Slot != nullptr ? 1u : 0u;
    }
    return Count;
}

EEditorViewportLayoutMode FEditorViewportLayout::GetLayoutMode() const noexcept
{
    return LayoutMode;
}
