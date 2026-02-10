#pragma once
//
// Created by vastrakai on 7/12/2024.
//
#include <nlohmann/json.hpp>

struct Preferences
{
    std::string mDefaultConfigName;
    std::vector<std::string> mFriends;
    std::vector<std::string> mEnemies;
    bool mFallbackToD3D11 = false;
    bool mEnforceDebugging = false;
    std::string mStreamerName = "orphan";
};

class PreferenceManager {
public:
    static std::shared_ptr<Preferences> load();
    static void save(const std::shared_ptr<Preferences>& prefs);
};