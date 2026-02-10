//
// Created by vastrakai on 7/12/2024.
//

#include "Friends.hpp"

#include <Orphan.hpp>
#include <Features/Modules/Player/Teams.hpp>
#include <Features/Modules/Misc/HackerAlert.hpp>

void Friends::onInit()
{
    mFriends = Orphan::Prefs->mFriends;
}

bool Friends::isFriend(const std::string& name)
{
    auto lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    for (const auto& friendName : mFriends)
    {
        auto lowerFriendName = friendName;
        std::transform(lowerFriendName.begin(), lowerFriendName.end(), lowerFriendName.begin(), ::tolower);
        if (lowerName == lowerFriendName)
        {
            return true;
        }
    }
    return false;
}

bool Friends::isEnemy(const std::string& name)
{
    return StringUtils::containsAnyIgnoreCase(name, mEnemies);
}

bool Friends::isTrueEnemy(const std::string& name)
{
    for (const auto& hackerName : gFriendManager->mEnemies) {

        std::string result;
        std::string trimmedName = StringUtils::toLower(name);
        for (size_t i = 0; i < trimmedName.length(); i++) {
            if (trimmedName[i] == '§' && i + 1 < trimmedName.length()) {
                i++; // Skip both '§' and the next character
            }
            else {
                result += trimmedName[i];
            }
        }

        if (HackerAlert::isHackerName(result, StringUtils::toLower(hackerName))) {
            return true;
        }
    }
    return false;
}

bool Friends::isFriend(Actor* actor)
{
    return isFriend(actor->getRawName()) || Teams::instance->isOnTeam(actor);
}

bool Friends::isEnemy(Actor* actor)
{
    return isEnemy(actor->getRawName());
}

bool Friends::isTrueEnemy(Actor* actor)
{
    return isTrueEnemy(actor->getRawName());
}

void Friends::addFriend(const std::string& name)
{
    mFriends.push_back(name);
    Orphan::Prefs->mFriends = mFriends;
    PreferenceManager::save(Orphan::Prefs);
}

void Friends::addEnemy(const std::string& name)
{
    mEnemies.push_back(name);
    Orphan::Prefs->mEnemies = mEnemies;
    PreferenceManager::save(Orphan::Prefs);
}

void Friends::addFriend(Actor* actor)
{
    addFriend(actor->getRawName());
}

void Friends::addEnemy(Actor* actor)
{
    addEnemy(actor->getRawName());
}

void Friends::removeFriend(const std::string& name)
{
    std::erase(mFriends, name);
    Orphan::Prefs->mFriends = mFriends;
    PreferenceManager::save(Orphan::Prefs);
}

void Friends::removeEnemy(const std::string& name)
{
    for (const auto& hackerName : gFriendManager->mEnemies) {

        std::string result;
        std::string trimmedName = StringUtils::toLower(name);
        for (size_t i = 0; i < trimmedName.length(); i++) {
            if (trimmedName[i] == '§' && i + 1 < trimmedName.length()) {
                i++; // Skip both '§' and the next character
            }
            else {
                result += trimmedName[i];
            }
        }

        if (HackerAlert::isHackerName(result, StringUtils::toLower(hackerName))) {
            std::erase(mEnemies, hackerName);
            std::erase(mEnemies, name);
        }
    }
    Orphan::Prefs->mEnemies = mEnemies;
    PreferenceManager::save(Orphan::Prefs);
}

void Friends::removeFriend(Actor* actor)
{
    removeFriend(actor->getRawName());
}

void Friends::removeEnemy(Actor* actor)
{
    removeEnemy(actor->getRawName());
}

void Friends::clearFriends()
{
    mFriends.clear();
    Orphan::Prefs->mFriends = mFriends;
    PreferenceManager::save(Orphan::Prefs);
}

void Friends::clearEnemies()
{
    mEnemies.clear();
    Orphan::Prefs->mEnemies = mEnemies;
    PreferenceManager::save(Orphan::Prefs);
}

double calculateMatchPercentage(const std::string& str1, const std::string& str2) {
    size_t maxLength = std::max(str1.length(), str2.length());
    size_t matchCount = 0;

    for (size_t i = 0; i < std::min(str1.length(), str2.length()); ++i) {
        if (str1[i] == str2[i]) {
            ++matchCount;
        }
    }

    return (static_cast<double>(matchCount) / maxLength) * 100.0;
}
