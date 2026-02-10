//
// Created by vastrakai on 7/7/2024.
//

#include "ESP.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Events/ActorRenderEvent.hpp>
#include <Features/Modules/Misc/Friends.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Options.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>

void ESP::onEnable()
{
    gFeatureManager->mDispatcher->listen<RenderEvent, &ESP::onRenderEvent>(this);
}

void ESP::onDisable()
{
    gFeatureManager->mDispatcher->deafen<RenderEvent, &ESP::onRenderEvent>(this);
}

void ESP::onRenderEvent(RenderEvent& event)
{
    if (!ClientInstance::get()->getLevelRenderer()) return;

    auto actors = ActorUtils::getActorList(false, true);
    auto localPlayer = ClientInstance::get()->getLocalPlayer();

    if (mDebug.mValue)
    {
        auto botActors = ActorUtils::getActorList(false, false);
        // Remove actors that are not bots from the botActors list
        std::erase_if(botActors, [actors](Actor* actor) {
            return std::ranges::find(actors, actor) != actors.end();
        });



        // Draw a red outline around the bot actors
        auto drawList = ImGui::GetBackgroundDrawList();
        for (auto actor : botActors)
        {
            if (actor == localPlayer && ClientInstance::get()->getOptions()->mThirdPerson->value == 0 && !localPlayer->getFlag<RenderCameraComponent>()) continue;
            if (actor == localPlayer && !mRenderLocal.mValue) continue;
            auto shape = actor->getAABBShapeComponent();
            if (!shape) continue;

            auto themeColor = ImColor(1.0f, 0.0f, 0.0f);

            if (actor->isPlayer())
            {
                bool isfriend = gFriendManager->isFriend(actor);
                if (gFriendManager->isFriend(actor) && mShowFriends.mValue) {
                    themeColor = ImColor(0.0f, 1.0f, 0.0f);
                }
                bool isenemy = gFriendManager->isTrueEnemy(actor);
                if (isenemy) {
                    themeColor = ImColor(1.0f, 0.0f, 0.0f);
                }

                if (mShowTeams.mValue && !isenemy && (!isfriend && mShowFriends.mValue))
                {
                    std::string teamcolour = actor->getNameTag();
                    teamcolour = teamcolour.substr(0, teamcolour.find('\n'));
                    size_t actorTeamPos = teamcolour.find("�");
                    if (actorTeamPos != std::string::npos) {
                        teamcolour = teamcolour.substr(actorTeamPos + 1, 1);

                        static const std::unordered_map<std::string, ImColor> colorMap = {
                            {"0", ImColor(0.f, 0.f, 0.f, 1.f)},        // Black
                            {"1", ImColor(0.f, 0.f, 0.67f, 1.f)},     // Dark Blue
                            {"2", ImColor(0.f, 0.67f, 0.f, 1.f)},     // Dark Green
                            {"3", ImColor(0.f, 0.67f, 0.67f, 1.f)},   // Dark Aqua
                            {"4", ImColor(0.67f, 0.f, 0.f, 1.f)},     // Dark Red
                            {"5", ImColor(0.67f, 0.f, 0.67f, 1.f)},   // Dark Purple
                            {"6", ImColor(1.f, 0.67f, 0.f, 1.f)},     // Gold
                            {"7", ImColor(0.67f, 0.67f, 0.67f, 1.f)}, // Gray
                            {"8", ImColor(0.33f, 0.33f, 0.33f, 1.f)}, // Dark Gray
                            {"9", ImColor(0.33f, 0.33f, 1.f, 1.f)},   // Blue
                            {"a", ImColor(0.33f, 0.7f, 0.33f, 1.f)},   // Green
                            {"b", ImColor(0.33f, 1.f, 1.f, 1.f)},     // Aqua
                            {"c", ImColor(0.7f, 0.33f, 0.33f, 1.f)},   // Red
                            {"d", ImColor(1.f, 0.33f, 1.f, 1.f)},     // Light Purple
                            {"e", ImColor(1.f, 1.f, 0.33f, 1.f)},     // Yellow
                            {"f", ImColor(1.f, 1.f, 1.f, 1.f)}        // White
                        };

                        // Get the theme color based on teamcolour
                        auto it = colorMap.find(teamcolour);
                        if (it != colorMap.end()) {
                            themeColor = it->second;
                        }
                        else {
                            // Default to white if the team colour is unrecognized
                            themeColor = ImColor(1.f, 1.f, 1.f, 1.f);
                        }
                    }
                }
            }

            AABB aabb = actor->getAABB();

            std::vector<ImVec2> imPoints = MathUtils::getImBoxPoints(aabb);

            if (mRenderFilled.mValue) drawList->AddConvexPolyFilled(imPoints.data(), imPoints.size(), ImColor(themeColor.Value.x, themeColor.Value.y, themeColor.Value.z, 0.25f));
            drawList->AddPolyline(imPoints.data(), imPoints.size(), themeColor, 0, 2.0f);
        }

    }


    auto drawList = ImGui::GetBackgroundDrawList();


    for (auto actor : actors)
    {
        if (actor == localPlayer && ClientInstance::get()->getOptions()->mThirdPerson->value == 0 && !localPlayer->getFlag<RenderCameraComponent>()) continue;
        if (actor == localPlayer && !mRenderLocal.mValue) continue;
        auto shape = actor->getAABBShapeComponent();
        if (!shape) continue;

        auto themeColor = ColorUtils::getThemedColor(0);

        if (actor->isPlayer())
        {
            bool isfriend = gFriendManager->isFriend(actor);
            if (gFriendManager->isFriend(actor) && mShowFriends.mValue) {
                themeColor = ImColor(0.0f, 1.0f, 0.0f);
            }
            bool isenemy = gFriendManager->isTrueEnemy(actor);
            if (isenemy) {
                themeColor = ImColor(1.0f, 0.0f, 0.0f);
            }

            if (mShowTeams.mValue && !isenemy && (!isfriend && mShowFriends.mValue))
            {
                std::string teamcolour = actor->getNameTag();
                teamcolour = teamcolour.substr(0, teamcolour.find('\n'));
                size_t actorTeamPos = teamcolour.find("�");
                if (actorTeamPos != std::string::npos) {
                    teamcolour = teamcolour.substr(actorTeamPos + 1, 1);

                    static const std::unordered_map<std::string, ImColor> colorMap = {
                        {"0", ImColor(0.f, 0.f, 0.f, 1.f)},        // Black
                        {"1", ImColor(0.f, 0.f, 0.67f, 1.f)},     // Dark Blue
                        {"2", ImColor(0.f, 0.67f, 0.f, 1.f)},     // Dark Green
                        {"3", ImColor(0.f, 0.67f, 0.67f, 1.f)},   // Dark Aqua
                        {"4", ImColor(0.67f, 0.f, 0.f, 1.f)},     // Dark Red
                        {"5", ImColor(0.67f, 0.f, 0.67f, 1.f)},   // Dark Purple
                        {"6", ImColor(1.f, 0.67f, 0.f, 1.f)},     // Gold
                        {"7", ImColor(0.67f, 0.67f, 0.67f, 1.f)}, // Gray
                        {"8", ImColor(0.33f, 0.33f, 0.33f, 1.f)}, // Dark Gray
                        {"9", ImColor(0.33f, 0.33f, 1.f, 1.f)},   // Blue
                        {"a", ImColor(0.33f, 0.7f, 0.33f, 1.f)},   // Green
                        {"b", ImColor(0.33f, 1.f, 1.f, 1.f)},     // Aqua
                        {"c", ImColor(0.7f, 0.33f, 0.33f, 1.f)},   // Red
                        {"d", ImColor(1.f, 0.33f, 1.f, 1.f)},     // Light Purple
                        {"e", ImColor(1.f, 1.f, 0.33f, 1.f)},     // Yellow
                        {"f", ImColor(1.f, 1.f, 1.f, 1.f)}        // White
                    };

                    // Get the theme color based on teamcolour
                    auto it = colorMap.find(teamcolour);
                    if (it != colorMap.end()) {
                        themeColor = it->second;
                    }
                    else {
                        // Default to white if the team colour is unrecognized
                        themeColor = ImColor(1.f, 1.f, 1.f, 1.f);
                    }
                }
            }
        }

        AABB aabb = actor->getAABB();

        std::vector<ImVec2> imPoints = MathUtils::getImBoxPoints(aabb);

        if (mRenderFilled.mValue) drawList->AddConvexPolyFilled(imPoints.data(), imPoints.size(), ImColor(themeColor.Value.x, themeColor.Value.y, themeColor.Value.z, 0.25f));
        drawList->AddPolyline(imPoints.data(), imPoints.size(), themeColor, 0, 2.0f);
    }
}
