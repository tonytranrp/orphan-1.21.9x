//
// Created by vastrakai on 7/2/2024.
//

#include "ConfigManager.hpp"

#include <fstream>
#include <Features/FeatureManager.hpp>
#include <Utils/FileUtils.hpp>
#include <nlohmann/json.hpp>
#include <Utils/MiscUtils/NotifyUtils.hpp>

#include "spdlog/spdlog.h"

std::string ConfigManager::getConfigPath()
{
    return FileUtils::getOrphanDir() + "Configs\\";
}

bool ConfigManager::configExists(const std::string& name)
{
    return FileUtils::fileExists(getConfigPath() + name + ".json");
}

void ConfigManager::loadConfig(const std::string& name)
{
    std::ifstream file(getConfigPath() + name + ".json");
    nlohmann::json j;
    file >> j;
    file.close();

    gFeatureManager->mModuleManager->deserialize(j);

    LastLoadedConfig = name;

    spdlog::info("Loaded config " + name + " successfully.");
    NotifyUtils::notify("Loaded config " + name + "!", 3.f, Notification::Type::Info);
}

void ConfigManager::saveConfig(const std::string& name)
{
    std::string path = getConfigPath() + name + ".json";
    nlohmann::json j;
    try {
        j = gFeatureManager->mModuleManager->serialize();
    } catch (const std::exception& e) {
        spdlog::error("Failed to serialize config: {}", e.what());
        return;
    } catch (...) {
        spdlog::error("Failed to serialize config: unknown error");
        return;
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open config file for writing: {}", path);
        return;
    }
    try {
        file << j.dump(4);
        file.close();
    } catch (const std::exception& e) {
        spdlog::error("Failed to write config file: {} ({})", path, e.what());
        return;
    } catch (...) {
        spdlog::error("Failed to write config file: {} (unknown error)", path);
        return;
    }

    LastLoadedConfig = name;

    spdlog::info("Config saved successfully to {}", path);
    NotifyUtils::notify("Saved config as " + name + ".", 3.f, Notification::Type::Info);
}
