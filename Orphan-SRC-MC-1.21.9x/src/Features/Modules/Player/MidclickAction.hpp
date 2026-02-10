#pragma once
//
// Created by vastrakai on 7/12/2024.
// Edited by player5 (1/24/2025)
//

#include <Features/Modules/Module.hpp>

class MidclickAction : public ModuleBase<MidclickAction> {
public:
    BoolSetting mThrowPearls = BoolSetting("Throw Pearls", "Throw an ender pearl when you middle click", false);
    BoolSetting mHotbarOnly = BoolSetting("Hotbar Only", "Only throw pearls from the hotbar", false);
    BoolSetting mAddFriend = BoolSetting("Add Friend", "Add the player you middle clicked as a friend", false);
    BoolSetting mSetNukerBlock = BoolSetting("Set Nuker Block", "Specify the block mined by nuker", false);
    MidclickAction() : ModuleBase("MidclickAction", "Performs an action when you middle click", ModuleCategory::Player, 0, false)
    {
        addSetting(&mThrowPearls);
        addSetting(&mHotbarOnly);
        addSetting(&mAddFriend);
        addSetting(&mSetNukerBlock);

        VISIBILITY_CONDITION(mHotbarOnly, mThrowPearls.mValue == true);

        mNames = {
              {Lowercase, "midclickaction"},
                {LowercaseSpaced, "midclick action"},
                {Normal, "MidclickAction"},
                {NormalSpaced, "Midclick Action"}
        };
    };

    bool mThrowNextTick = false;
    bool mRotateNextTick = false;

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
};