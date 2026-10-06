#include "Framework/TestRunner.h"

#include "Runtime/Render/Renderer.h"

#include <Windows.h>

namespace
{
class FHiddenTestWindow
{
public:
    FHiddenTestWindow()
    {
        WindowHandle = CreateWindowExW(
            0,
            L"STATIC",
            L"JisooEngine Renderer Test",
            WS_OVERLAPPEDWINDOW,
            0,
            0,
            64,
            64,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);
    }

    ~FHiddenTestWindow()
    {
        if (WindowHandle != nullptr)
        {
            DestroyWindow(WindowHandle);
        }
    }

    [[nodiscard]] HWND GetHandle() const noexcept
    {
        return WindowHandle;
    }

private:
    HWND WindowHandle = nullptr;
};

void RecordTrackingOverlay(ID3D12GraphicsCommandList* CommandList, void* UserData)
{
    auto* bRecorded = static_cast<bool*>(UserData);
    *bRecorded = CommandList != nullptr;
}
}

void RegisterRendererTests(FTestRunner& Runner)
{
    Runner.Add(
        "Renderer.RenderTargetHandleRejectsReleasedGeneration",
        [](FTestContext& Test)
        {
            FHiddenTestWindow Window;
            Test.Expect(Window.GetHandle() != nullptr, "Renderer 통합 테스트용 Window를 생성해야 한다.");
            if (Window.GetHandle() == nullptr)
            {
                return;
            }

            FRenderer Renderer;
            Test.Expect(Renderer.Initialize(Window.GetHandle(), 64, 64), "RenderTarget 수명 검증을 위해 Renderer를 초기화해야 한다.");
            if (Renderer.GetMainRenderTargetHandle().IsSet() == false)
            {
                return;
            }

            const FRenderTargetHandle MainTarget = Renderer.GetMainRenderTargetHandle();
            Test.Expect(!Renderer.ReleaseRenderTarget(MainTarget), "Main RenderTarget은 일반 Target 해제 API로 반환할 수 없어야 한다.");

            const FRenderTargetHandle First = Renderer.CreateTextureRenderTarget(32, 32);
            Test.Expect(First.IsSet(), "첫 Texture RenderTarget Handle을 발급해야 한다.");
            Test.Expect(Renderer.GetRenderTargetShaderResourceView(First).ptr != 0, "생성된 Target은 유효한 SRV를 제공해야 한다.");
            Test.Expect(Renderer.ReleaseRenderTarget(First), "첫 Texture RenderTarget을 해제해야 한다.");
            Test.Expect(Renderer.GetRenderTargetShaderResourceView(First).ptr == 0, "해제된 Handle은 SRV를 해석하지 못해야 한다.");
            Test.Expect(!Renderer.ResizeRenderTarget(First, 48, 48), "해제된 Handle의 Resize를 거부해야 한다.");
            Test.Expect(!Renderer.ReleaseRenderTarget(First), "동일 Handle의 이중 해제를 거부해야 한다.");

            const FRenderTargetHandle Second = Renderer.CreateTextureRenderTarget(48, 48);
            Test.Expect(Second.IsSet(), "해제 Slot에 두 번째 Texture RenderTarget을 생성해야 한다.");
            Test.Expect(Second.Index == First.Index, "해제된 RenderTarget Slot을 재사용해야 한다.");
            Test.Expect(Second.Generation != First.Generation, "재사용된 Slot은 새로운 Generation을 받아야 한다.");
            Test.Expect(Renderer.GetRenderTargetShaderResourceView(First).ptr == 0, "이전 Generation은 새 Target의 SRV를 해석하지 못해야 한다.");
            Test.Expect(Renderer.GetRenderTargetShaderResourceView(Second).ptr != 0, "새 Generation Handle은 새 Target을 해석해야 한다.");
            Test.Expect(Renderer.ReleaseRenderTarget(Second), "두 번째 Texture RenderTarget을 정상 해제해야 한다.");
        });

    Runner.Add(
        "Renderer.EndFrameRecordsFinalOverlayBeforePresent",
        [](FTestContext& Test)
        {
            FHiddenTestWindow Window;
            FRenderer Renderer;
            if (Window.GetHandle() == nullptr || !Renderer.Initialize(Window.GetHandle(), 64, 64))
            {
                Test.Expect(false, "최종 Overlay 검증을 위해 Renderer를 초기화해야 한다.");
                return;
            }

            bool bOverlayRecorded = false;
            Test.Expect(Renderer.BeginFrame(), "최종 Overlay를 기록할 Renderer Frame을 시작해야 한다.");
            Test.Expect(
                Renderer.EndFrame(&RecordTrackingOverlay, &bOverlayRecorded),
                "최종 Overlay 기록 뒤 명령 제출과 Present를 완료해야 한다.");
            Test.Expect(bOverlayRecorded, "EndFrame이 Present 전에 최종 Overlay Recorder를 호출해야 한다.");
        });
}
