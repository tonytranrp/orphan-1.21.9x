#pragma once
//
// Created by ssi on 10/26/2024.
//

#include <Features/Modules/Module.hpp>

class DamageBoost2 : public ModuleBase<DamageBoost2> {
public:
    DividerSetting dSpeed = DividerSetting("- Speed -", "Settings for speed");
    NumberSetting mMaxMagnitude = NumberSetting("Magnitude", "Highest multiplier relative to received knockback", 1.f, 0.f, 7.0f, 0.1f);
    NumberSetting mYVel = NumberSetting("Y Velocity", "Vertical velocity multiplier", 0.f, 0.f, 20.0f, 0.1f);
    NumberSetting mGroundSpeed = NumberSetting("Ground Velocity", "Velocity multiplier if hit on-ground", 0.f, 0.f, 100.0f, 0.1f);
    NumberSetting mOffGroundSpeed = NumberSetting("Air Velocity", "Velocity multiplier if hit in-air", 100.f, 0.f, 100.0f, 0.1f);

    DividerSetting dLimit = DividerSetting("- Limit -", "Settings for limitation");
    BoolSetting mCustomSpeed = BoolSetting("Custom Speed", "Custom ground & air velocity", false);
    NumberSetting mKBReq = NumberSetting("Required Knockback", "Minimum knockback required", 0.f, 0.f, 2.0f, 0.01f);


    DamageBoost2() : ModuleBase("DamageBoost2", "Boosts your speed when taking damage", ModuleCategory::Movement, 0, false) {

        addSettings
        (
            &dSpeed,
            &mMaxMagnitude,
            &mYVel,
            &mGroundSpeed,
            &mOffGroundSpeed
        );

        addSettings
        (
            &dLimit,
            &mCustomSpeed,
            &mKBReq
        );

        VISIBILITY_CONDITION(mGroundSpeed, mCustomSpeed.mValue);
        VISIBILITY_CONDITION(mOffGroundSpeed, mCustomSpeed.mValue);

        mNames = {
            {Lowercase, "damageboost+"},
            {LowercaseSpaced, "damage boost+"},
            {Normal, "DamageBoost+"},
            {NormalSpaced, "Damage Boost+"}
        };
    }

    void onEnable() override;
    void onDisable() override;
    void onPacketInEvent(class PacketInEvent& event);
};