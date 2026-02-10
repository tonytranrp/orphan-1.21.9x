//
// Created by ssi on 8/17/2024.
// Edited by player5 (1/24/2025)
//

#pragma once
#include <Features/Modules/Module.hpp>
#include <Features/Modules/Setting.hpp>

class Spider : public ModuleBase<Spider> {
public:
    enum class Mode {
        Clip,
        Flareon
    };
    DividerSetting dMode = DividerSetting("- Mode -", "Settings for mode");
    EnumSettingT<Mode> mMode = EnumSettingT<Mode>("Mode", "The climbing mode to use", Mode::Clip, "Clip", "Flareon");
    BoolSetting mOnGroundOnly = BoolSetting("OnGround only", "Uses spider only if on the ground", false);

    DividerSetting dSpeed = DividerSetting("- Speed -", "Settings for speed");
    NumberSetting mSpeed = NumberSetting("Speed", "Adjust the climbing speed", 2.50, 1, 5, 0.01);


    Spider() : ModuleBase("Spider", "Allows you to climb up walls", ModuleCategory::Movement, 0, false) {
        addSetting(&dMode);
        addSetting(&mMode);
        addSetting(&mOnGroundOnly);

        addSetting(&dSpeed);
        addSetting(&mSpeed);

        mNames = {
            {Lowercase, "spider"},
            {LowercaseSpaced, "spider"},
            {Normal, "Spider"},
            {NormalSpaced, "Spider"},
        };
    }

    float mPosY = 0.f;
    bool mWasCollided = false;

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
};