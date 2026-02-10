//
// Created by alteik on 02/10/2024.
//

#include "SessionInfo.hpp"
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Network/PacketID.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <SDK/Minecraft/Network/Packets/TextPacket.hpp>
#include <Hook/Hooks/RenderHooks/D3DHook.hpp>
#include "NameProtect.hpp"
#include <Utils/MiscUtils/ImRenderUtils.hpp>
#include <Utils/FontHelper.hpp>
#include <Utils/MiscUtils/ColorUtils.hpp>
#include <Utils/SysUtils/HttpRequest.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

std::vector<std::string> gamemodesToGetStatsFor = { "sky", "sky-duos", "sky-squads", "sky-mega", "ctf", "bed", "bed-duos", "bed-squads", "sg", "sg-duos" };

// Move static variables to be instance-specific to avoid crashes
static std::unordered_map<SessionInfo*, int> mTotalKills;
static std::unordered_map<SessionInfo*, int> mTotalDeaths;
static std::unordered_map<SessionInfo*, int> mTotalPlayed;
static std::unordered_map<SessionInfo*, int> mCompletedRequests;
static std::unordered_map<SessionInfo*, bool> mShouldUpdate;

void SessionInfo::resetStatistics() {
    mTotalKills[this] = 0;
    mTotalDeaths[this] = 0;
    mTotalPlayed[this] = 0;
    mCompletedRequests[this] = 0;
    mShouldUpdate[this] = false; // Initialize this too
}

void SessionInfo::makeRequestsForAllGamemodes(const std::string &gamertag) {
    if (gamertag.empty()) {
        return;
    }
    
    resetStatistics();

    for (const auto& gamemode : gamemodesToGetStatsFor) {
        try {
            auto request = std::make_unique<HttpRequest>(
                    HttpMethod::GET,
                    "https://api.playhive.com/v0/game/all/" + gamemode + "/" + gamertag,
                    "",
                    "",
                    &SessionInfo::onHttpResponse,
                    this
            );
            int64_t now = NOW;
            mRequests.emplace_back(now, std::move(request));
        } catch (const std::exception& e) {
            // Silently handle request creation errors
        }
    }
}

void SessionInfo::onHttpResponse(HttpResponseEvent event) {
    auto sender = static_cast<SessionInfo *>(event.mSender);
    if (!sender) return;

    try {
        if (event.mStatusCode == 200) {
            nlohmann::json json = nlohmann::json::parse(event.mResponse);

            int kills = 0, deaths = 0, played = 0;

            if (json.contains("kills")) {
                kills = json["kills"];
            }
            if (json.contains("deaths")) {
                deaths = json["deaths"];
            }
            if (json.contains("played")) {
                played = json["played"];
            }

            mTotalKills[sender] += kills;
            mTotalDeaths[sender] += deaths;
            mTotalPlayed[sender] += played;

        } else if (event.mStatusCode == 404) {
            spdlog::info("[SessionInfo] Stats not found for this gamemode!");
        } else {
            spdlog::error("[SessionInfo] Request failed with status code: {}", event.mStatusCode);
        }

        mCompletedRequests[sender]++;

        if (mCompletedRequests[sender] == gamemodesToGetStatsFor.size()) {
            mShouldUpdate[sender] = false;
            //ChatUtils::displayClientMessage("[SessionInfo] All stats updated!");
        }
    } catch (const std::exception& e) {
        spdlog::error("[SessionInfo] Error processing HTTP response: {}", e.what());
    }
}

void SessionInfo::onEnable() {
    resetStatistics();
    
    // Ensure HudElement is registered
    if (mElement && HudEditor::gInstance) {
        HudEditor::gInstance->registerElement(mElement.get());
    }
    
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &SessionInfo::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<RenderEvent, &SessionInfo::onRenderEvent>(this);
    if (mElement) {
        mElement->mVisible = true;
    }
}

void SessionInfo::onDisable() {
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &SessionInfo::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<RenderEvent, &SessionInfo::onRenderEvent>(this);
    if (mElement) {
        mElement->mVisible = false;
    }
    
    // Clean up static variables for this instance
    mTotalKills.erase(this);
    mTotalDeaths.erase(this);
    mTotalPlayed.erase(this);
    mCompletedRequests.erase(this);
    mShouldUpdate.erase(this);
    
    // Clear requests
    mRequests.clear();
}

void SessionInfo::onRenderEvent(RenderEvent &event) {
    if (!ClientInstance::get()) return;
    if (!ClientInstance::get()->getLocalPlayer()) return;
    if (!ClientInstance::get()->getLevelRenderer()) return;
    if (!mElement) return;
    
    // Check if ImGui is properly initialized
    if (!ImGui::GetCurrentContext()) return;

    // Get current stats for this instance with safe defaults
    int kills = mTotalKills.count(this) ? mTotalKills[this] : 0;
    int deaths = mTotalDeaths.count(this) ? mTotalDeaths[this] : 0;
    int played = mTotalPlayed.count(this) ? mTotalPlayed[this] : 0;

    std::string
    mKillsStr = "Kills: " + std::to_string(kills),
    mDeathsStr = "Deaths: " + std::to_string(deaths),
    mGamesPlayedStr = "Games Played: " + std::to_string(played);

    auto pos = mElement->getPos();

    auto drawList = ImGui::GetForegroundDrawList();
    if (!drawList) return;

    ImVec2 size = ImVec2(200, 105);

    ImVec4 area = ImVec4(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
    ImRenderUtils::addBlur(area, 4.f, 4.f, drawList, false);
    drawList->AddRectFilled(pos, ImVec2(area.z, area.w), ImColor(0.f, 0.f, 0.f, 0.5), 10.0f);
    drawList->AddShadowRect(ImVec2(pos.x - 3, pos.y - 3), ImVec2(area.z + 3, area.w + 3), ImColor(0.f, 0.f, 0.f, 1.f), 50.f, ImVec2(0,0));
    
    ImVec2 titlePos = pos;
    titlePos.x += 64;
    titlePos.y += 3.8;
    
    ImVec2 lineStart = ImVec2(pos.x, pos.y + 30);
    ImVec2 lineEnd = ImVec2(pos.x + 200, pos.y + 33);

    int lineLength = 20;
    int lengthPerLine = 200 / lineLength;
    for (int i = 0; i < lineLength; i++) {
        ImVec2 point = ImVec2(lineStart.x + (lineEnd.x - lineStart.x) * i / lineLength, lineStart.y);
        drawList->AddRectFilled(point, ImVec2(point.x + lengthPerLine, lineEnd.y), ColorUtils::getThemedColor(i));
    }

    int startPadding = 38;

    FontHelper::pushPrefFont(true);

    ImRenderUtils::drawShadowText(drawList, "Statistics", titlePos, ImColor(255, 255, 255, 255), 22.f, false);
    ImRenderUtils::drawShadowText(drawList, mKillsStr, ImVec2(pos.x + 10, pos.y + startPadding), ImColor(255, 255, 255, 255), 20, false);
    ImRenderUtils::drawShadowText(drawList, mDeathsStr, ImVec2(pos.x + 10, pos.y + startPadding + 20), ImColor(255, 255, 255, 255), 20, false);
    ImRenderUtils::drawShadowText(drawList, mGamesPlayedStr, ImVec2(pos.x + 10, pos.y + startPadding + 40), ImColor(255, 255, 255, 255), 20, false);

    FontHelper::popPrefFont();
}

void SessionInfo::onBaseTickEvent(BaseTickEvent& event) {
    auto player = event.mActor;
    if (!player) return;
    
    // Check if the actor is valid before accessing its methods
    try {
        if (!player->isValid()) {
            return;
        }
    } catch (...) {
        return;
    }

    // Safe access to static maps
    bool shouldUpdate = mShouldUpdate.count(this) ? mShouldUpdate[this] : false;
    
    if(NOW < lastUpdate + 15000 && !shouldUpdate) {
        mShouldUpdate[this] = false;
        return;
    }
    else if (!shouldUpdate && NOW > lastUpdate + 15000) {
        mShouldUpdate[this] = true;
        
        // Use getRawName() instead of getLocalName() to avoid offset issues
        std::string playerName;
        try {
            playerName = player->getRawName();
        } catch (...) {
            playerName = "Unknown";
        }
        
        makeRequestsForAllGamemodes(playerName);
        lastUpdate = NOW;
    }

    // Store player name safely
    try {
        mPlayerName = player->getRawName();
    } catch (...) {
        mPlayerName = "Unknown";
    }

    for (auto it = mRequests.begin(); it != mRequests.end();) {
        if (it->second && it->second->isDone()) {
            it = mRequests.erase(it);
        } else {
            ++it;
        }
    }

    if (!mRequests.empty()) {
        auto &request = mRequests.front();
        if (request.second && !request.second->mRequestSent) {
            request.second->sendAsync();
        }
    }
}
