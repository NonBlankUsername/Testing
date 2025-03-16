#include "Visuals.hpp"
#include "Entities/Entities.hpp"
std::mutex DrawM;

ImColor HealthColor(float HealthPercentage) {
    float r = (1.0f - HealthPercentage) * 255.0f;
    float g = HealthPercentage * 255.0f;
    return ImColor(static_cast<int>(r), static_cast<int>(g), 0);
}


void DrawEntity(const CEntities::Entity& Entity, ImDrawList* DrawList) {
    auto ViewMatrix = reinterpret_cast<float(*)[4][4]>(CEntities::Client + Offsets::ViewMatrix);

    const auto LocalPawn = *reinterpret_cast<uintptr_t*>(CEntities::Client + Offsets::LocalPlayer);
    Vector3 LocalPosition = *reinterpret_cast<Vector3*>(LocalPawn + Offsets::OldOrigin);
    float dx = Entity.FeetPos.X - LocalPosition.X;
    float dy = Entity.FeetPos.Y - LocalPosition.Y;
    float dz = Entity.FeetPos.Z - LocalPosition.Z;
    float Distance = sqrt(dx * dx + dy * dy + dz * dz);
    float ScaledDistance = Distance / 10.0f;
    std::string DistanceText = "Distance: " + std::to_string(static_cast<int>(ScaledDistance)) + "m";

    Vector2 Feet, Head;
    if (Entity.FeetPos.WorldToScreen(Feet, ViewMatrix) && Entity.HeadPos.WorldToScreen(Head, ViewMatrix)) {
        float Height = Feet.Y - Head.Y;
        float Width = Height * 0.5f;

        float HealthWidth = 4.0f;
        float HealthPadding = 4.0f;
        float HealthHeight = Height * (Entity.Health / 100.0f);
        float HealthPercentage = Entity.Health / 100.0f;

        ImColor Bar;
        if (Entity.Health > 85) {
            Bar = ImColor(0, 255, 0);
        }
        else if (Entity.Health > 35) {
            Bar = ImColor(255, 165, 0);
        }
        else {
            Bar = ImColor(255, 0, 0);
        }

        ImVec2 MousePos = ImGui::GetMousePos();
        ImVec2 BoxMin = { Feet.X - Width / 2, Head.Y };
        ImVec2 BoxMax = { Feet.X + Width / 2, Head.Y + Height };

        // Use the customizable filled box color here (from Globals::ESP)
        if (Globals::ESP::FilledBox) {
            DrawList->AddRectFilled(BoxMin, BoxMax, 0.7f);  // Changed to use FilledBoxColor
        }

        ImColor Border = ImColor(Globals::ESP::BoxColor);
        ImColor NameColor = ImColor(Globals::ESP::NameColor);
        ImColor HealthTextColor = ImColor(Globals::ESP::HealthTextColor);
        ImColor HealthTextColor = ImColor(Globals::ESP::FilledBoxColor);

        if (MousePos.x >= BoxMin.x && MousePos.x <= BoxMax.x && MousePos.y >= BoxMin.y && MousePos.y <= BoxMax.y) {
            Border = ImColor(234, 93, 96);
            NameColor = ImColor(234, 93, 96);
            HealthTextColor = ImColor(234, 93, 96);
        }

        if (Globals::ESP::Box) {
            DrawList->AddRect(BoxMin, BoxMax, Border, 0.7f);
        }

        if (Globals::ESP::Name) {
            ImVec2 NamePos = { BoxMin.x, BoxMin.y - 15.0f };
            DrawList->AddText(NamePos, NameColor, Entity.Name.c_str());
        }

        if (Globals::ESP::Health) {
            float NewHealthWidth = 3.0f;
            float CornerRadius = 4.0f;

            DrawList->AddRectFilled(
                { Feet.X - Width / 2 - NewHealthWidth - HealthPadding, Head.Y },
                { Feet.X - Width / 2 - HealthPadding, Head.Y + Height },
                ImColor(0, 0, 0, 76), CornerRadius);

            DrawList->AddRectFilled(
                { Feet.X - Width / 2 - NewHealthWidth - HealthPadding, Head.Y + Height - HealthHeight },
                { Feet.X - Width / 2 - HealthPadding, Head.Y + Height },
                Bar, CornerRadius);
        }

        if (Globals::ESP::HealthText) {
            ImVec2 HealthTextPos = { BoxMax.x, BoxMin.y - 15.0f };
            DrawList->AddText(HealthTextPos, HealthTextColor, std::to_string(static_cast<int>(Entity.Health)).c_str());
        }

        if (Globals::ESP::Distance) {
            ImVec2 DistancePos = { BoxMax.x + 5.0f, BoxMin.y };
            DrawList->AddText(DistancePos, ImColor(255, 255, 255), DistanceText.c_str());
        }
    }
}

void Visuals::Players() {
    Entities->Update();

    ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

    for (const auto& Entity : Entities->GetEntities())
        DrawEntity(Entity, DrawList);
}

