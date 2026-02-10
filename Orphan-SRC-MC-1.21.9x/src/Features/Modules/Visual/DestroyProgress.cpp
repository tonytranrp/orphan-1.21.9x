#include "DestroyProgress.hpp"

#include <Features/FeatureManager.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Actor/GameMode.hpp>
#include <SDK/Minecraft/World/Level.hpp>
#include <SDK/Minecraft/World/HitResult.hpp>
#include <SDK/Minecraft/Rendering/GuiData.hpp> // Ensure GuiData is included

void DestroyProgress::onEnable()
{
    gFeatureManager->mDispatcher->listen<RenderEvent, &DestroyProgress::onRenderEvent>(this);
}

void DestroyProgress::onDisable()
{
    gFeatureManager->mDispatcher->deafen<RenderEvent, &DestroyProgress::onRenderEvent>(this);
}

void DestroyProgress::onRenderEvent(RenderEvent& event)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player || ClientInstance::get()->getMouseGrabbed()) return;

    if (!player->getGameMode()) return;

    float cProgress = player->getGameMode()->mBreakProgress;
    static float lastProgress = 0.f;
    static glm::ivec3 lastMiningPos = glm::ivec3(0);
    static float transitionProgress = 0.f;

    if (0 >= cProgress || player->getLevel()->getHitResult()->mType != HitType::BLOCK) return;

    glm::ivec3 cMiningPos = player->getLevel()->getHitResult()->mBlockPos;
    glm::vec3 targetBlockPos = glm::vec3(cMiningPos) + glm::vec3(0.5f, 0.5f, 0.5f);

    if (cMiningPos != lastMiningPos) {
        lastProgress = 0.f;  
        transitionProgress = 0.f;
        lastMiningPos = cMiningPos;
    }

    float cAnimProgress = cProgress;

    // Smooth transition for mining progress
    if (player->getGameMode()->mBreakProgress) {
        transitionProgress = MathUtils::lerp(transitionProgress, 1.f, ImGui::GetIO().DeltaTime * 10.f);
        cAnimProgress = MathUtils::lerp(lastProgress, cAnimProgress, transitionProgress);
    }
    else {
        transitionProgress = 0.f;
    }

    lastProgress = cAnimProgress;
    cAnimProgress = MathUtils::clamp(cAnimProgress, 0.f, 1.f);

    float animSize = MathUtils::lerp(0.f, 1.f, cAnimProgress);
    if (mMode.mValue == Mode::In) {
        animSize = 1.0f - animSize;
    }
    auto boxSize = glm::vec3(animSize, animSize, animSize);

    glm::vec3 blockPos = targetBlockPos - (boxSize / 2.f);
    auto boxAABB = AABB(blockPos, boxSize);

    ImColor cColor = ColorUtils::getThemedColor(0);
    if (mColorMode.mValue == ColorMode::Default)
        cColor = ImColor((int)(cAnimProgress * 255), (int)((1 - cAnimProgress) * 255), 0);

    cColor.Value.w = static_cast<int>(mOpacity.mValue * 255);

    std::vector<ImVec2> imPoints = MathUtils::getImBoxPoints(boxAABB);
    auto drawList = ImGui::GetBackgroundDrawList();

    if (mFilled.mValue)
        drawList->AddConvexPolyFilled(imPoints.data(), imPoints.size(), ImColor(cColor.Value.x, cColor.Value.y, cColor.Value.z, mOpacity.mValue));

    drawList->AddPolyline(imPoints.data(), imPoints.size(), cColor, 0, 2.0f);

    if (mMode.mValue == Mode::Orphan) {
        glm::vec3 origin = RenderUtils::transform.mOrigin;
        glm::vec2 screenPos = glm::vec2(0, 0);

        if (RenderUtils::transform.mMatrix.OWorldToScreen(origin, targetBlockPos, screenPos, ClientInstance::get()->getFov(), ClientInstance::get()->getGuiData()->mResolution)) {
            std::string progressText = std::to_string(static_cast<int>(cAnimProgress * 100)) + "%";

            // Font scaling based on distance
            float fontSize = mFontSize.mValue;
            if (mDistanceScaledFont.mValue) {
                float distance = glm::distance(origin, targetBlockPos) + 2.5f;
                if (distance < 0) distance = 0;
                fontSize = 1.0f / distance * 100.0f * mScalingMultiplier.mValue;
                if (fontSize < 1.0f) fontSize = 1.0f;
            }//wow

            ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0, progressText.c_str());
            ImVec2 textPos = ImVec2(screenPos.x - textSize.x / 2, screenPos.y - textSize.y / 2);

            if (mBold.mValue) {
                ImFont* font = ImGui::GetFont();
                drawList->AddText(font, fontSize, textPos, ImColor(255, 255, 255, 255), progressText.c_str());
                drawList->AddText(font, fontSize, ImVec2(textPos.x + 1, textPos.y), ImColor(255, 255, 255, 255), progressText.c_str());
                drawList->AddText(font, fontSize, ImVec2(textPos.x - 1, textPos.y), ImColor(255, 255, 255, 255), progressText.c_str());
                drawList->AddText(font, fontSize, ImVec2(textPos.x, textPos.y + 1), ImColor(255, 255, 255, 255), progressText.c_str());
                drawList->AddText(font, fontSize, ImVec2(textPos.x, textPos.y - 1), ImColor(255, 255, 255, 255), progressText.c_str());
            }
            else {
                drawList->AddText(ImGui::GetFont(), fontSize, textPos, ImColor(255, 255, 255, 255), progressText.c_str());
            }
        }
    }
}

