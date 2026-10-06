#include "Editor/UI/WorldOutlinerPanel.h"

#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/World.h"

#include "imgui.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace
{
bool ContainsCaseInsensitive(std::string_view Text, std::string_view Search)
{
    if (Search.empty())
    {
        return true;
    }

    return std::search(
        Text.begin(),
        Text.end(),
        Search.begin(),
        Search.end(),
        [](char Left, char Right)
        {
            return std::tolower(static_cast<unsigned char>(Left)) ==
                std::tolower(static_cast<unsigned char>(Right));
        }) != Text.end();
}
}

void FWorldOutlinerPanel::Draw(const UWorld& World)
{
    if (!bOpen)
    {
        return;
    }

    const bool bDrawContents = ImGui::Begin("World Outliner", &bOpen);
    if (bDrawContents && bOpen)
    {
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint(
            "##ActorSearch",
            "Search actors",
            SearchText.data(),
            SearchText.size());
        ImGui::Separator();

        const std::string_view Search(SearchText.data());
        for (AActor* Actor : World.GetActors())
        {
            if (Actor == nullptr || !ContainsCaseInsensitive(Actor->GetName(), Search))
            {
                continue;
            }

            const FObjectHandle ActorHandle = Actor->GetHandle();
            ImGui::PushID(static_cast<int>(ActorHandle.Index));
            ImGui::PushID(static_cast<int>(ActorHandle.SerialNumber));
            const char* Label = Actor->GetName().empty()
                ? "<Unnamed Actor>"
                : Actor->GetName().c_str();
            if (ImGui::Selectable(Label, SelectedActor == ActorHandle))
            {
                SelectedActor = ActorHandle;
            }
            ImGui::PopID();
            ImGui::PopID();
        }
    }
    ImGui::End();

    if (!bOpen)
    {
        ResetState();
    }
}

void FWorldOutlinerPanel::SetOpen(bool bInOpen)
{
    if (bOpen == bInOpen)
    {
        return;
    }
    bOpen = bInOpen;
    ResetState();
}

bool FWorldOutlinerPanel::IsOpen() const noexcept
{
    return bOpen;
}

void FWorldOutlinerPanel::ResetState()
{
    SearchText.fill('\0');
    SelectedActor = {};
}
