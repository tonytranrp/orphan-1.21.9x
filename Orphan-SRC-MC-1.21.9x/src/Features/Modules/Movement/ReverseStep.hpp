#pragma once
//
// Created by alteik on 09/10/2024.
// Edited by player5 (1/24/2025)
/*/

#include <Features/Modules/Module.hpp>

class ReverseStep : public ModuleBase<ReverseStep>
{
public:
    enum class Mode {
        Motion
    };

    EnumSettingT<Mode> mMode = EnumSettingT<Mode>("Mode", "Falling style", Mode::Motion, "Motion");
    NumberSetting mMaxFallDistance = NumberSetting("Max Fall Distance", "Max fall distance to step", 20, 0, 30, 1);
    BoolSetting mDontUseIfSpeed = BoolSetting("Disable With Speed", "Avoid using reverse step while the speed module is enabled", true);
    BoolSetting mDontUseIfLongJump = BoolSetting("Disable With Longjump", "Avoid using reverse step while the longjump module is enabled", true);
    BoolSetting mVoidCheck = BoolSetting("Void Check", "Avoid stepping into the void", true);

    ReverseStep() : ModuleBase<ReverseStep>("ReverseStep", "Automatically steps down blocks", ModuleCategory::Movement, 0, false) {
        addSetting(&mMode);
        addSetting(&mMaxFallDistance);
        addSetting(&mDontUseIfSpeed);
        addSetting(&mDontUseIfLongJump);
        addSetting(&mVoidCheck);

        mNames = {
                {Lowercase, "reversestep"},
                {LowercaseSpaced, "reverse step"},
                {Normal, "ReverseStep"},
                {NormalSpaced, "Reverse Step"}
        };
    }

    bool mJumped = false;

    bool canFallDown();
    bool isVoid();

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
};*/