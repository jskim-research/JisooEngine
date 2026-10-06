#include "Runtime/Engine/Engine.h"

#include "Runtime/CoreUObject/ObjectGlobals.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/Components/TriangleComponent.h"
#include "Runtime/Engine/World.h"
#include "Runtime/Render/Renderer.h"

#include <algorithm>
#include <utility>

FEngine::FEngine() = default;

FEngine::~FEngine()
{
    Shutdown();
}

bool FEngine::Initialize(const FEngineInitParams& InitParams)
{
    if (bInitialized)
    {
        return true;
    }

    std::unique_ptr<FRenderer> NewRenderer = std::make_unique<FRenderer>();
    if (!NewRenderer->Initialize(
            InitParams.NativeWindowHandle,
            InitParams.ClientWidth,
            InitParams.ClientHeight))
    {
        return false;
    }

    Renderer = std::move(NewRenderer);
    bInitialized = true;

    if (Worlds.empty() && CreateWorld("World") == nullptr)
    {
        Shutdown();
        return false;
    }

    for (UWorld* World : GetWorlds())
    {
        World->BeginPlay();
    }

    const std::vector<UWorld*> ActiveWorlds = GetWorlds();
    if (!ActiveWorlds.empty())
    {
        AActor* TriangleActor = ActiveWorlds.front()->SpawnActor<AActor>(
            "RenderValidationTriangle");
        if (TriangleActor == nullptr)
        {
            Shutdown();
            return false;
        }

        UTriangleComponent* TriangleComponent =
            TriangleActor->AddComponent<UTriangleComponent>("Triangle");
        if (TriangleComponent == nullptr)
        {
            Shutdown();
            return false;
        }

        FTransform TriangleTransform;
        TriangleTransform.Translation = {300.0f, 0.0f, 0.0f};
        TriangleComponent->SetRelativeTransform(TriangleTransform);
    }
    return true;
}

void FEngine::Tick(float DeltaSeconds)
{
    InputSystem.AdvanceFrame();
    RouteCurrentInput(DeltaSeconds);

    const std::vector<FObjectHandle> WorldSnapshot = Worlds;
    for (const FObjectHandle Handle : WorldSnapshot)
    {
        if (UWorld* World = Cast<UWorld>(ResolveObject(Handle)))
        {
            World->Tick(DeltaSeconds);
        }
    }

    if (!bWindowMinimized)
    {
        RenderFrame();
    }

    FlushPendingDestroyObjects();
}

void FEngine::Shutdown()
{
    InputSystem.Reset();

    const std::vector<UWorld*> WorldSnapshot = GetWorlds();
    for (UWorld* World : WorldSnapshot)
    {
        DestroyWorld(World);
    }
    FlushPendingDestroyObjects();

    if (Renderer != nullptr)
    {
        Renderer->Shutdown();
        Renderer.reset();
    }

    bInitialized = false;
    bWindowMinimized = false;
}

UWorld* FEngine::CreateWorld(std::string Name)
{
    UWorld* World = NewObject<UWorld>(nullptr, std::move(Name));
    if (World == nullptr)
    {
        return nullptr;
    }

    Worlds.push_back(World->GetHandle());
    if (bInitialized)
    {
        World->BeginPlay();
    }
    return World;
}

bool FEngine::DestroyWorld(UWorld* World)
{
    if (!IsValid(World))
    {
        return false;
    }

    const FObjectHandle WorldHandle = World->GetHandle();
    const auto WorldIterator = std::ranges::find(Worlds, WorldHandle);
    if (WorldIterator == Worlds.end())
    {
        return false;
    }

    World->EndPlay();
    Worlds.erase(WorldIterator);
    DestroyObject(World);
    return true;
}

std::vector<UWorld*> FEngine::GetWorlds() const
{
    std::vector<UWorld*> Result;
    Result.reserve(Worlds.size());
    for (const FObjectHandle Handle : Worlds)
    {
        if (UWorld* World = Cast<UWorld>(ResolveObject(Handle)))
        {
            Result.push_back(World);
        }
    }
    return Result;
}

IInputEventSink& FEngine::GetInputEventSink() noexcept
{
    return InputSystem;
}

bool FEngine::HandleWindowResize(const FWindowResizeEvent& Event)
{
    if (Event.bMinimized)
    {
        bWindowMinimized = true;
        return true;
    }
    if (Renderer == nullptr || Event.Width == 0 || Event.Height == 0 ||
        !Renderer->Resize(Event.Width, Event.Height))
    {
        return false;
    }

    bWindowMinimized = false;
    OnWindowResized(Event.Width, Event.Height);
    return true;
}

void FEngine::RouteCurrentInput(float DeltaSeconds)
{
    const FInputFrame& InputFrame = InputSystem.GetCurrentFrame();
    InputRouter.Route(
        InputFrame,
        BuildInputRouteContext(InputFrame, DeltaSeconds),
        DeltaSeconds);
}

FInputRouteContext FEngine::BuildInputRouteContext(const FInputFrame&, float)
{
    return {};
}

void FEngine::RenderFrame()
{
}

void FEngine::OnWindowResized(std::uint32_t, std::uint32_t)
{
}

FRenderer* FEngine::GetRenderer() const noexcept
{
    return Renderer.get();
}
