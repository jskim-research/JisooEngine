#include "Editor/UI/EditorUI.h"

#include "Editor/UI/ViewportPanel.h"
#include "Editor/UI/WorldOutlinerPanel.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Input/InputTypes.h"
#include "Runtime/Render/Renderer.h"

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cfloat>

namespace
{
ImGuiKey TranslateInputKey(EInputKey Key)
{
    if (Key >= EInputKey::Digit0 && Key <= EInputKey::Digit9)
    {
        return static_cast<ImGuiKey>(
            ImGuiKey_0 + static_cast<int>(Key) - static_cast<int>(EInputKey::Digit0));
    }
    if (Key >= EInputKey::A && Key <= EInputKey::Z)
    {
        return static_cast<ImGuiKey>(
            ImGuiKey_A + static_cast<int>(Key) - static_cast<int>(EInputKey::A));
    }
    if (Key >= EInputKey::F1 && Key <= EInputKey::F12)
    {
        return static_cast<ImGuiKey>(
            ImGuiKey_F1 + static_cast<int>(Key) - static_cast<int>(EInputKey::F1));
    }

    switch (Key)
    {
    case EInputKey::Backspace: return ImGuiKey_Backspace;
    case EInputKey::Tab: return ImGuiKey_Tab;
    case EInputKey::Enter: return ImGuiKey_Enter;
    case EInputKey::Shift: return ImGuiKey_LeftShift;
    case EInputKey::Control: return ImGuiKey_LeftCtrl;
    case EInputKey::Alt: return ImGuiKey_LeftAlt;
    case EInputKey::CapsLock: return ImGuiKey_CapsLock;
    case EInputKey::Escape: return ImGuiKey_Escape;
    case EInputKey::Space: return ImGuiKey_Space;
    case EInputKey::PageUp: return ImGuiKey_PageUp;
    case EInputKey::PageDown: return ImGuiKey_PageDown;
    case EInputKey::End: return ImGuiKey_End;
    case EInputKey::Home: return ImGuiKey_Home;
    case EInputKey::Left: return ImGuiKey_LeftArrow;
    case EInputKey::Up: return ImGuiKey_UpArrow;
    case EInputKey::Right: return ImGuiKey_RightArrow;
    case EInputKey::Down: return ImGuiKey_DownArrow;
    case EInputKey::Insert: return ImGuiKey_Insert;
    case EInputKey::Delete: return ImGuiKey_Delete;
    case EInputKey::Semicolon: return ImGuiKey_Semicolon;
    case EInputKey::Equal: return ImGuiKey_Equal;
    case EInputKey::Comma: return ImGuiKey_Comma;
    case EInputKey::Minus: return ImGuiKey_Minus;
    case EInputKey::Period: return ImGuiKey_Period;
    case EInputKey::Slash: return ImGuiKey_Slash;
    case EInputKey::Backtick: return ImGuiKey_GraveAccent;
    case EInputKey::LeftBracket: return ImGuiKey_LeftBracket;
    case EInputKey::Backslash: return ImGuiKey_Backslash;
    case EInputKey::RightBracket: return ImGuiKey_RightBracket;
    case EInputKey::Quote: return ImGuiKey_Apostrophe;
    default: return ImGuiKey_None;
    }
}

void AllocateImGuiDescriptor(
    ImGui_ImplDX12_InitInfo* InitInfo,
    D3D12_CPU_DESCRIPTOR_HANDLE* OutCpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE* OutGpuHandle)
{
    auto* Renderer = static_cast<FRenderer*>(InitInfo->UserData);
    if (Renderer == nullptr || !Renderer->AllocateShaderResourceDescriptor(
            *OutCpuHandle,
            *OutGpuHandle))
    {
        *OutCpuHandle = {};
        *OutGpuHandle = {};
    }
}

void FreeImGuiDescriptor(
    ImGui_ImplDX12_InitInfo* InitInfo,
    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle)
{
    if (auto* Renderer = static_cast<FRenderer*>(InitInfo->UserData))
    {
        Renderer->FreeShaderResourceDescriptor(CpuHandle, GpuHandle);
    }
}

struct FImGuiRenderContext
{
    ImDrawData* DrawData = nullptr;
    ID3D12DescriptorHeap* ShaderResourceViewHeap = nullptr;
};

void RecordImGuiDrawData(ID3D12GraphicsCommandList* CommandList, void* UserData)
{
    const auto& Context = *static_cast<FImGuiRenderContext*>(UserData);
    ID3D12DescriptorHeap* DescriptorHeaps[] = {Context.ShaderResourceViewHeap};
    CommandList->SetDescriptorHeaps(1, DescriptorHeaps);
    ImGui_ImplDX12_RenderDrawData(Context.DrawData, CommandList);
}
}

FEditorUI::FEditorUI() = default;

FEditorUI::~FEditorUI()
{
    Shutdown();
}

bool FEditorUI::Initialize(
    FRenderer& InRenderer,
    const FScene* Scene,
    UWorld* InWorld,
    std::uint32_t DisplayWidth,
    std::uint32_t DisplayHeight)
{
    if (bInitialized)
    {
        return true;
    }
    if (Scene == nullptr || InWorld == nullptr || DisplayWidth == 0 || DisplayHeight == 0)
    {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& Io = ImGui::GetIO();
    Io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    Io.IniFilename = nullptr;
    Io.DisplaySize = ImVec2(
        static_cast<float>(DisplayWidth),
        static_cast<float>(DisplayHeight));
    Io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    Io.BackendPlatformName = "JisooEngine_ImGuiPlatform";
    ImGui::StyleColorsDark();

    ImGui_ImplDX12_InitInfo InitInfo;
    InitInfo.Device = InRenderer.GetD3D12Device();
    InitInfo.CommandQueue = InRenderer.GetD3D12CommandQueue();
    InitInfo.NumFramesInFlight = 2;
    InitInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    InitInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
    InitInfo.UserData = &InRenderer;
    InitInfo.SrvDescriptorHeap = InRenderer.GetShaderResourceViewHeap();
    InitInfo.SrvDescriptorAllocFn = &AllocateImGuiDescriptor;
    InitInfo.SrvDescriptorFreeFn = &FreeImGuiDescriptor;
    if (!ImGui_ImplDX12_Init(&InitInfo))
    {
        ImGui::DestroyContext();
        return false;
    }

    auto NewViewportPanel = std::make_unique<FViewportPanel>();
    if (!NewViewportPanel->Initialize(InRenderer, Scene))
    {
        ImGui_ImplDX12_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    Renderer = &InRenderer;
    World = InWorld;
    ViewportPanel = std::move(NewViewportPanel);
    WorldOutlinerPanel = std::make_unique<FWorldOutlinerPanel>();
    bInitialized = true;
    return true;
}

void FEditorUI::Shutdown()
{
    if (!bInitialized)
    {
        return;
    }

    if (Renderer != nullptr)
    {
        Renderer->WaitForGpu();
    }
    ViewportPanel.reset();
    WorldOutlinerPanel.reset();
    ImGui_ImplDX12_Shutdown();
    ImGui::DestroyContext();

    ViewportInputRegions.clear();
    Renderer = nullptr;
    World = nullptr;
    bWantsKeyboardCapture = false;
    bFrameBuilt = false;
    bInitialized = false;
}

void FEditorUI::BuildFrame(const FInputFrame& InputFrame, float DeltaSeconds)
{
    if (!bInitialized || ViewportPanel == nullptr || WorldOutlinerPanel == nullptr ||
        World == nullptr)
    {
        return;
    }

    ViewportInputRegions.clear();
    ImGuiIO& Io = ImGui::GetIO();
    Io.DeltaTime = std::max(DeltaSeconds, 0.0001f);
    FeedInput(InputFrame);
    ImGui_ImplDX12_NewFrame();
    ImGui::NewFrame();

    BuildMainMenu();
    BuildDockSpace();
    ViewportPanel->Draw(ViewportInputRegions);
    WorldOutlinerPanel->Draw(*World);

    bWantsKeyboardCapture = Io.WantCaptureKeyboard || Io.WantTextInput;
    ImGui::Render();
    bFrameBuilt = true;
}

bool FEditorUI::Render(FRenderer& InRenderer)
{
    if (!bFrameBuilt || ViewportPanel == nullptr)
    {
        return false;
    }

    for (FViewport* Viewport : ViewportPanel->GetVisibleViewports())
    {
        if (Viewport != nullptr)
        {
            InRenderer.PrepareRenderTargetForSampling(Viewport->GetOutput().Target);
        }
    }

    ImDrawData* DrawData = ImGui::GetDrawData();
    FImGuiRenderContext RenderContext{
        DrawData,
        InRenderer.GetShaderResourceViewHeap()
    };
    const bool bRecorded = DrawData != nullptr && InRenderer.RecordExternalRenderPass(
        InRenderer.GetMainRenderTargetHandle(),
        &RecordImGuiDrawData,
        &RenderContext);
    bFrameBuilt = false;
    return bRecorded;
}

std::span<const FViewportInputRegion> FEditorUI::GetViewportInputRegions() const noexcept
{
    return ViewportInputRegions;
}

std::span<FViewport* const> FEditorUI::GetVisibleViewports() const noexcept
{
    if (ViewportPanel == nullptr)
    {
        return {};
    }
    return ViewportPanel->GetVisibleViewports();
}

FViewport* FEditorUI::GetActiveViewport() const noexcept
{
    return ViewportPanel != nullptr ? ViewportPanel->GetActiveViewport() : nullptr;
}

bool FEditorUI::WantsKeyboardCapture() const noexcept
{
    return bWantsKeyboardCapture;
}

void FEditorUI::FeedInput(const FInputFrame& InputFrame)
{
    ImGuiIO& Io = ImGui::GetIO();
    if (bLastWindowFocused != InputFrame.IsWindowFocused())
    {
        Io.AddFocusEvent(InputFrame.IsWindowFocused());
        bLastWindowFocused = InputFrame.IsWindowFocused();
    }

    if (InputFrame.HasPointerPosition())
    {
        Io.AddMousePosEvent(
            static_cast<float>(InputFrame.GetPointerX()),
            static_cast<float>(InputFrame.GetPointerY()));
    }
    else
    {
        Io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }

    constexpr EInputKey MouseKeys[] = {
        EInputKey::MouseLeft,
        EInputKey::MouseRight,
        EInputKey::MouseMiddle,
        EInputKey::MouseX1,
        EInputKey::MouseX2
    };
    for (int ButtonIndex = 0; ButtonIndex < static_cast<int>(std::size(MouseKeys)); ++ButtonIndex)
    {
        const EInputKey Key = MouseKeys[ButtonIndex];
        if (InputFrame.WasPressed(Key))
        {
            Io.AddMouseButtonEvent(ButtonIndex, true);
        }
        if (InputFrame.WasReleased(Key))
        {
            Io.AddMouseButtonEvent(ButtonIndex, false);
        }
    }
    if (InputFrame.GetWheelDelta() != 0.0f)
    {
        Io.AddMouseWheelEvent(0.0f, InputFrame.GetWheelDelta());
    }

    for (std::size_t Index = 0; Index < InputKeyCount; ++Index)
    {
        const EInputKey Key = static_cast<EInputKey>(Index);
        const ImGuiKey ImGuiInputKey = TranslateInputKey(Key);
        if (ImGuiInputKey == ImGuiKey_None)
        {
            continue;
        }
        if (InputFrame.WasPressed(Key))
        {
            Io.AddKeyEvent(ImGuiInputKey, true);
        }
        if (InputFrame.WasReleased(Key))
        {
            Io.AddKeyEvent(ImGuiInputKey, false);
        }
    }

    if (InputFrame.WasPressed(EInputKey::Control) ||
        InputFrame.WasReleased(EInputKey::Control))
    {
        Io.AddKeyEvent(ImGuiMod_Ctrl, InputFrame.IsDown(EInputKey::Control));
    }
    if (InputFrame.WasPressed(EInputKey::Shift) ||
        InputFrame.WasReleased(EInputKey::Shift))
    {
        Io.AddKeyEvent(ImGuiMod_Shift, InputFrame.IsDown(EInputKey::Shift));
    }
    if (InputFrame.WasPressed(EInputKey::Alt) ||
        InputFrame.WasReleased(EInputKey::Alt))
    {
        Io.AddKeyEvent(ImGuiMod_Alt, InputFrame.IsDown(EInputKey::Alt));
    }

    for (char16_t Character : InputFrame.GetTextInput())
    {
        Io.AddInputCharacterUTF16(static_cast<ImWchar16>(Character));
    }
}

void FEditorUI::BuildMainMenu()
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }

    if (ImGui::BeginMenu("Window"))
    {
        bool bViewportOpen = ViewportPanel->IsOpen();
        if (ImGui::MenuItem("Viewport", nullptr, bViewportOpen))
        {
            ViewportPanel->SetOpen(!bViewportOpen);
        }

        bool bOutlinerOpen = WorldOutlinerPanel->IsOpen();
        if (ImGui::MenuItem("World Outliner", nullptr, bOutlinerOpen))
        {
            WorldOutlinerPanel->SetOpen(!bOutlinerOpen);
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void FEditorUI::BuildDockSpace()
{
    const ImGuiID DockSpaceId = ImGui::GetID("EditorDockSpace");
    const ImGuiViewport* MainViewport = ImGui::GetMainViewport();
    if (ImGui::DockBuilderGetNode(DockSpaceId) == nullptr)
    {
        ImGui::DockBuilderAddNode(DockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(DockSpaceId, MainViewport->WorkSize);

        ImGuiID MainDockId = DockSpaceId;
        ImGuiID OutlinerDockId = 0;
        ImGui::DockBuilderSplitNode(
            MainDockId,
            ImGuiDir_Right,
            0.25f,
            &OutlinerDockId,
            &MainDockId);
        ImGui::DockBuilderDockWindow("Viewport", MainDockId);
        ImGui::DockBuilderDockWindow("World Outliner", OutlinerDockId);
        ImGui::DockBuilderFinish(DockSpaceId);
    }

    ImGui::DockSpaceOverViewport(DockSpaceId, MainViewport);
}
