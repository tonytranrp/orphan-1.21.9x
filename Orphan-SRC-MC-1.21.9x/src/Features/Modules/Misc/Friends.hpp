#pragma once
//
// Created by vastrakai on 7/12/2024.
// "Enemies" Created by rekitrelt
//

#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>

inline class Friends* gFriendManager = nullptr;

class Friends : public ModuleBase<Friends> {
public:
    Friends() : ModuleBase<Friends>("Friends", "Add yo homies with .friend!", ModuleCategory::Misc, 0, false)
    {
        mNames = {
            {Lowercase, "friends"},
            {LowercaseSpaced, "friends"},
            {Normal, "Friends"},
            {NormalSpaced, "Friends"},
        };

        gFriendManager = this;
    }

    std::vector<std::string> mFriends;
    std::vector<std::string> mEnemies;

    void onInit() override;

    bool isFriend(const std::string& name);
    bool isFriend(Actor* actor);
    void addFriend(const std::string& name);
    void addFriend(Actor* actor);
    void removeFriend(const std::string& name);
    void removeFriend(Actor* actor);
    void clearFriends();

    bool isEnemy(const std::string& name);
    bool isEnemy(Actor* actor);
    bool isTrueEnemy(const std::string& name);
    bool isTrueEnemy(Actor* actor);
    void addEnemy(const std::string& name);
    void addEnemy(Actor* actor);
    void removeEnemy(const std::string& name);
    void removeEnemy(Actor* actor);
    void clearEnemies();
};