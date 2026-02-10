//
// Created by vastrakai on 8/10/2024.
// Edited by rekitrelt (2/2/2025)
// Edited by player5 (2/3/2025)
//

#include "Nametags.hpp"

#include <Features/Events/NametagRenderEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Modules/Misc/Friends.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/MinecraftSim.hpp>
#include <SDK/Minecraft/Options.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Actor/Components/FlagComponent.hpp>
#include <SDK/Minecraft/Rendering/GuiData.hpp>

void Nametags::onEnable()
{
    gFeatureManager->mDispatcher->listen<RenderEvent, &Nametags::onRenderEvent>(this);
    gFeatureManager->mDispatcher->listen<NametagRenderEvent, &Nametags::onNametagRenderEvent>(this);
}
void Nametags::onDisable()
{
    gFeatureManager->mDispatcher->deafen<RenderEvent, &Nametags::onRenderEvent>(this);
    gFeatureManager->mDispatcher->deafen<NametagRenderEvent, &Nametags::onNametagRenderEvent>(this);

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;
}

void Nametags::onRenderEvent(RenderEvent& event)
{
    auto ci = ClientInstance::get();
    if (!ci->getLevelRenderer()) return;

    auto actors = ActorUtils::getActorList(true, true);
    std::ranges::sort(actors, [&](Actor* a, Actor* b) {
        auto aPosComp = a->getRenderPositionComponent();
        auto bPosComp = b->getRenderPositionComponent();
        if (!aPosComp || !bPosComp) return false;
        auto aPos = aPosComp->mPosition;
        auto bPos = bPosComp->mPosition;
        auto origin = RenderUtils::transform.mOrigin;
        return glm::distance(origin, aPos) > glm::distance(origin, bPos);
    });

    auto drawList = ImGui::GetBackgroundDrawList();

    auto localPlayer = ci->getLocalPlayer();

    for (auto actor : actors)
    {
        if (!actor->isPlayer()) continue;
        if (actor == localPlayer && ci->getOptions()->mThirdPerson->value == 0 && !localPlayer->getFlag<RenderCameraComponent>()) continue;
        if (actor == localPlayer && !mRenderLocal.mValue) continue;
        auto shape = actor->getAABBShapeComponent();
        if (!shape) continue;
        auto posComp = actor->getRenderPositionComponent();
        if (!posComp) continue;

        std::string Ab = fmt::format("{:.1f}", (mAccurate.mValue ? actor->getAbsorption()/2 : actor->getAbsorption()));

        auto themeColor = ImColor(1.f, 1.f, 1.f, 1.f); //ColorUtils::getThemedColor(0);

        bool isfriend = gFriendManager->isFriend(actor);
        if (gFriendManager->isFriend(actor) && mShowFriends.mValue) {
            themeColor = ImColor(0.0f, 1.0f, 0.0f);
        }
        bool isenemy = gFriendManager->isTrueEnemy(actor);
        if (isenemy && mAlwaysShowEnemies.mValue) {
            themeColor = ImColor(1.0f, 0.0f, 0.0f);
        }

        bool TeamOverridesEnemy = (isenemy && !mAlwaysShowEnemies.mValue);
        if (mShowTeams.mValue && (TeamOverridesEnemy || !isenemy))
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



        glm::vec3 renderPos = posComp->mPosition;
        if (actor == localPlayer) renderPos = RenderUtils::transform.mPlayerPos;
        renderPos.y += 0.5f;

        glm::vec3 origin = RenderUtils::transform.mOrigin;
        glm::vec2 screen = glm::vec2(0, 0);

        if (!RenderUtils::transform.mMatrix.OWorldToScreen(origin, renderPos, screen, ci->getFov(), ci->getGuiData()->mResolution)) continue;
        if (std::isnan(screen.x) || std::isnan(screen.y)) continue;
        if (screen.x < 0 || screen.y < 0 || screen.x > ci->getGuiData()->mResolution.x * 2 || screen.y > ci->getGuiData()->mResolution.y * 2) continue;


        float fontSize = mFontSize.mValue;
        float padding = 5.f;

        if (mDistanceScaledFont.mValue)
        {
            // use distance to origin, not actor
            float distance = glm::distance(origin, renderPos) + 2.5f;
            if (distance < 0) distance = 0;
            fontSize = 1.0f / distance * 100.0f * mScalingMultiplier.mValue;
            if (fontSize < 1.0f) fontSize = 1.0f;
            if (fontSize < mMinScale.mValue) fontSize = mMinScale.mValue;
            padding = fontSize / 4;
        }

        FontHelper::pushPrefFont(true);

        std::string name = actor->getRawName();

        if (actor == localPlayer)
        {
            name = actor->getNameTag();
            // Remove everything after the first newline
            name = name.substr(0, name.find('\n'));
            name = ColorUtils::removeColorCodes(name);
        }

        if (mShowHP.mValue)
        {
            name += " [" + Ab + "]";
        }

        ImVec2 imFontSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0, name.c_str());
        ImVec2 pos = ImVec2(screen.x - imFontSize.x / 2, screen.y - imFontSize.y - 5);

        ImVec2 rectMin = ImVec2(pos.x - padding, pos.y - padding);
        ImVec2 rectMax = ImVec2(pos.x + imFontSize.x + padding, pos.y + imFontSize.y + padding);

        drawList->AddRectFilled(rectMin, rectMax, ImColor(0.0f, 0.0f, 0.0f, 0.5f), 10.f);

        drawList->AddText(ImGui::GetFont(), fontSize, pos, themeColor, name.c_str());

        FontHelper::popPrefFont();
    }
}

void Nametags::onNametagRenderEvent(NametagRenderEvent& event)
{
    auto actor = event.mActor;
    auto localPlayer = ClientInstance::get()->getLocalPlayer();
    auto ci = ClientInstance::get();

    if (ActorUtils::isBot(actor)) return;
    if (!actor->isPlayer()) return;
    if (actor == localPlayer && ci->getOptions()->mThirdPerson->value == 0 && !localPlayer->getFlag<RenderCameraComponent>()) return;
    if (actor == localPlayer && !mRenderLocal.mValue) return;
    auto shape = actor->getAABBShapeComponent();
    if (!shape) return;
    auto posComp = actor->getRenderPositionComponent();
    if (!posComp) return;

    glm::vec3 renderPos = posComp->mPosition;
    if (actor == localPlayer) renderPos = RenderUtils::transform.mPlayerPos;
    renderPos.y += 0.5f;

    glm::vec3 origin = RenderUtils::transform.mOrigin;
    glm::vec2 screen = glm::vec2(0, 0);

    if (!RenderUtils::transform.mMatrix.OWorldToScreen(origin, renderPos, screen, ci->getFov(), ci->getGuiData()->mResolution)) return;
    if (std::isnan(screen.x) || std::isnan(screen.y)) return;
    if (screen.x < 0 || screen.y < 0 || screen.x > ci->getGuiData()->mResolution.x * 2 || screen.y > ci->getGuiData()->mResolution.y * 2) return;

    event.cancel();
}

void Nametags::onPacketInEvent(PacketInEvent& event)
{
    if (event.mPacket->getId() == PacketID::ChangeDimension) {
        mLastHealTime = NOW;
        mHealths.clear();
    }
}