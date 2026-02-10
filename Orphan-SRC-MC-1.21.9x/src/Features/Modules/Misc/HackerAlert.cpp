#include "HackerAlert.hpp"
#include <Features/Events/BaseTickEvent.hpp>
#include <SDK/Minecraft/World/Level.hpp>
#include <Utils/MiscUtils/DataStore.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Modules/Misc/Friends.hpp>
#include <curl/curl.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_set>

#pragma once

std::string trimFormattingCodes(std::string str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        // Remove formatting codes (§ and the next character)
        if (str[i] == '§' && i + 1 < str.length()) {
            i++; // Skip the next character
        }
        // Remove occurrences of "..."
        else if (i + 2 < str.length() && str[i] == '.' && str[i + 1] == '.' && str[i + 2] == '.') {
            i += 2; // Skip all three dots
        }
        else {
            result += str[i];
        }
    }
    return result;
}

std::string toLowerCase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// Function to check if a player's name matches a hacker name considering the "..." suffix
bool HackerAlert::isHackerName(const std::string& playerName, const std::string& hackerName) {
    // If the hacker name ends with '...', remove the '...' part
    std::string trimmedHackerName = toLowerCase(trimFormattingCodes(hackerName));
    std::string trimmedPlayerName = toLowerCase(trimFormattingCodes(playerName));

    size_t hackerLength = trimmedHackerName.length();
    size_t playerLength = trimmedPlayerName.length();

    // Ensure we only compare up to the length of the hacker name
    size_t minLength = std::min(hackerLength, playerLength);

    // Check the prefix for similarity, allowing a 70% match
    size_t matchLength = 0;
    for (size_t i = 0; i < minLength; ++i) {
        if (trimmedPlayerName[i] != trimmedHackerName[i]) {
            break;
        }
        ++matchLength;
    }

    // If 70% or more of the characters match
    return (float)matchLength / (float)hackerLength >= 0.7f;
}


void HackerAlert::checkForHackers(const std::vector<std::string>& playerNames) {
    if (gFriendManager->mEnemies.empty()) {
        spdlog::info("Hacker list is empty. Fetching updated list...");
        updateHackerList();
    }
    std::unordered_set<std::string> lowercaseWarned;
    for (const auto& name : warnedPlayers) {
        lowercaseWarned.insert(toLowerCase(name));
    }

    std::unordered_set<std::string> warnedNotInList;
    std::unordered_set<std::string> currentPlayersNormalized;

    // Normalize and store current player names
    for (const auto& playerName : playerNames) {
        std::string normalizedPlayer = toLowerCase(trimFormattingCodes(playerName));
        currentPlayersNormalized.insert(normalizedPlayer);
    }

    // Check warned players to see if they have left
    for (const auto& warnedPlayer : warnedPlayers) {
        if (currentPlayersNormalized.find(toLowerCase(warnedPlayer)) == currentPlayersNormalized.end()) {
            // Player no longer in list -> warn that they left
            std::string message = "§c§lHacker Left: §r" + warnedPlayer;
            ChatUtils::displayClientMessage(message);
            if (mShowNotifications.mValue) {
                NotifyUtils::notify("Hacker Left: " + warnedPlayer, 5.0f, Notification::Type::Warning);
            }
            if (mPlaySound.mValue) {
                ClientInstance::get()->playUi("random.orb", 1.0f, 1.0f);
            }

            warnedNotInList.insert(warnedPlayer);
        }
    }

    // Check for hackers in the current player list
    for (const auto& playerName : playerNames) {
        std::string normalizedPlayer = trimFormattingCodes(playerName);

        if (lowercaseWarned.find(toLowerCase(trimFormattingCodes(playerName))) != lowercaseWarned.end()) {
            continue;
        }

        for (const auto& hackerName : gFriendManager->mEnemies) {
            std::string normalizedHacker = toLowerCase(hackerName);

            // Improve name comparison logic to avoid issues with suffix numbers
            if (isHackerName(toLowerCase(normalizedPlayer), normalizedHacker)) {
                std::string message = "§c§lHacker Detected: §r" + playerName;
                ChatUtils::displayClientMessage(message);
                if (mShowNotifications.mValue) {
                    NotifyUtils::notify("Hacker Detected: " + playerName, 5.0f, Notification::Type::Warning);
                }
                if (mPlaySound.mValue) {
                    ClientInstance::get()->playUi("random.orb", 1.0f, 1.0f);
                }

                warnedPlayers.insert(normalizedPlayer);
                if (!gFriendManager->isEnemy(normalizedPlayer)) {
                    gFriendManager->addEnemy(normalizedPlayer);
                }
                break;
            }
        }
    }

    // Remove players that left
    for (const auto& leftPlayer : warnedNotInList) {
        warnedPlayers.erase(leftPlayer);
    }
}


// Callback function for CURL to write data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch the CSV data from the Google Sheet
std::string fetchGoogleSheetCSV(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Allow libcurl to follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Set up the function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            spdlog::error("Failed to fetch Google Sheet: {}", curl_easy_strerror(res));
            return "";
        }
    }
    return readBuffer;
}

std::vector<std::string> parseCSV(const std::string& csvData) {
    std::vector<std::string> hackerNames;
    std::istringstream stream(csvData);
    std::string line;

    while (std::getline(stream, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        // Split the line into columns (names in this case)
        std::istringstream lineStream(line);
        std::string name;

        while (std::getline(lineStream, name, ',')) {
            // Trim leading/trailing whitespace
            name.erase(0, name.find_first_not_of(" \t\n\r\f\v"));
            name.erase(name.find_last_not_of(" \t\n\r\f\v") + 1);

            // Add the name to the list if it's not empty
            if (!name.empty()) {
                hackerNames.push_back(name);
            }
        }
    }

    return hackerNames;
}


void HackerAlert::updateHackerList() {
    // Fetch the CSV data from the Google Sheet
    std::string csvUrl = "https://docs.google.com/spreadsheets/d/1M5fze-XYN1u7sotsua8BFuF98jO6cvlvvWp66gUTGkQ/export?format=csv";
    std::string csvData = fetchGoogleSheetCSV(csvUrl);
    if (csvData.empty()) {
        spdlog::error("Failed to fetch or parse the Google Sheet.");
        return;
    }

    // Debug: Print the raw CSV data
    spdlog::info("Raw CSV Data:\n{}", csvData);

    // Parse the CSV data and update the cached hacker list
    cachedHackerList = parseCSV(csvData);
    spdlog::info("Updated hacker list with {} entries.", cachedHackerList.size());

    for (const std::string& str : cachedHackerList) {
        std::string trimmedName = str;
        auto it = std::find(gFriendManager->mEnemies.begin(), gFriendManager->mEnemies.end(), trimmedName);
        if (it == gFriendManager->mEnemies.end()) {
            gFriendManager->mEnemies.push_back(trimmedName);
            spdlog::info("Added new enemy: {}", trimmedName);
        }
    }
}

void HackerAlert::onEnable() {
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &HackerAlert::onBaseTickEvent>(this);

    // Update the hacker list when the feature is enabled
    updateHackerList();
}

void HackerAlert::onDisable() {
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &HackerAlert::onBaseTickEvent>(this);
}

void HackerAlert::onBaseTickEvent(BaseTickEvent& event) {
    //static bool lastSaveState = mSaveToDatabase.mValue;
    //if (lastSaveState != mSaveToDatabase.mValue) {
    //    lastSaveState = mSaveToDatabase.mValue;
    //    if (mSaveToDatabase.mValue) mPlayerStore.load();
    //}

    auto player = event.mActor;
    if (!player) return;

    // Get the list of player names in the game
    std::vector<std::string> playerNames;
    std::unordered_map<mce::UUID, PlayerListEntry>* playerList = player->getLevel()->getPlayerList();
    for (auto& entry : *playerList | std::views::values) {
        if (entry.mName.length() <= 17) {
            playerNames.emplace_back(entry.mName);
        }
    }

    // Check for hackers
    checkForHackers(playerNames);
}