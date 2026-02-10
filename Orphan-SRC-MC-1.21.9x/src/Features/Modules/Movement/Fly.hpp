#pragma once
//
// Created by vastrakai on 6/30/2024.
// Edited by player5 (1/24/2025)
//

#include <Features/Modules/Setting.hpp>
#include <Features/Auth/Authorization.hpp>
//bool pat = Auth::isPrivateUser();
class Fly : public ModuleBase<Fly> {
public:
    enum class Mode {
        Motion,
        Elytra,
        Jump,
        Flareon
    };

    DividerSetting dFly = DividerSetting("- Flying -", "Settings for flying");
   // EnumSettingT<Mode> mMode = EnumSettingT("Mode", "The mode of the fly", Mode::Motion, (Auth::isPrivateUser() ? ("Motion", "Elytra", "Jump", "Flareon") : ("Motion", "Elytra", "Jump")));
    EnumSettingT<Mode> mMode = (Auth::isPrivateUser() ? EnumSettingT("Mode", "The mode of the fly", Mode::Motion, "Motion", "Elytra", "Jump", "Flareon") : EnumSettingT("Mode", "The mode of the fly", Mode::Motion, "Motion", "Elytra", "Jump"));
    NumberSetting mHeightLoss = NumberSetting("Height Loss", "The amount of height to lose per airjump", 0.5f, 0.f, 2.f, 0.1f);
    NumberSetting mJumpDelay = NumberSetting("Jump Delay", "The amount of time to wait before jumping (in seconds)", 0.2f, 0.f, 1.f, 0.1f);
    BoolSetting mResetOnGround = BoolSetting("Reset On Ground", "Reset the airjump height when on ground", true);
    BoolSetting mResetOnDisable = BoolSetting("Reset On Disable", "Resets velocity when you disable", true);

    DividerSetting dSpeed = DividerSetting("- Speed -", "Settings for speed");
    NumberSetting mSpeed = NumberSetting("Speed", "The speed of the fly", 5.6f, 0.f, 20.f, 0.1f);
    BoolSetting mTimerBoost = BoolSetting("Timer Boost", "Boost the timer", false);
    NumberSetting mTimerBoostValue = NumberSetting("Timer Boost Value", "The new timer value", 1.f, 0.1f, 60.f, 0.1f);
    BoolSetting mSpeedFriction = BoolSetting("Speed Friction", "Applies friction to speed", true);
    NumberSetting mFriction = NumberSetting("Friction", "The amount of friction to apply", 0.975f, 0.f, 1.f, 0.1f);

    DividerSetting dBypass = DividerSetting("- Bypass -", "Settings for bypassing anticheat");
    BoolSetting mApplyGlideFlags = BoolSetting("Apply Glide Flags", "Applies glide flags to the player", true);
    BoolSetting mDamageOnly = BoolSetting("Damage Only", "Only fly when you take damage", false);
    NumberSetting mFlyTime = NumberSetting("Fly Time", "The amount of time to fly after taking damage (in seconds)", 1.f, 0.f, 10.f, 0.1f);

    DividerSetting dOther = DividerSetting("- Other -", "Other settings");
    NumberSetting mClipDistance = NumberSetting("Clip Distance", "The distance required to clip", 3, 0, 10, 1);
    NumberSetting mClipDownAmount = NumberSetting("Clip Down Amount", "How far down to clip", -3, -10, 0, 1);
    NumberSetting mClipUpAmount = NumberSetting("Clip Up Amount", "How far up to clip after", 3, 0, 10, 1);
    BoolSetting mKeepMotion = BoolSetting("Keep Motion", "Keeps the same motion you had when enabling", false);
    BoolSetting mDebug = BoolSetting("Debug", "Displays debug messages", false);

    Fly() : ModuleBase("Fly", "Allows you to fly", ModuleCategory::Movement, 0, false) {
        addSettings(
            &dFly,
            &mMode,
            &mHeightLoss,
            &mJumpDelay,
            &mResetOnGround,
            &mResetOnDisable
        );
        addSettings(
            &dSpeed,
            &mSpeed,
            &mTimerBoost,
            &mTimerBoostValue,
            &mSpeedFriction,
            &mFriction
        );
        addSettings(
            &dBypass,
            &mApplyGlideFlags,
            &mDamageOnly,
            &mFlyTime
        );
        if (Auth::isPrivateUser()) {
            addSettings(
                &dOther,
                &mClipDistance,
                &mClipDownAmount,
                &mClipUpAmount,
                &mKeepMotion,
                &mDebug
            );
        }

        VISIBILITY_CONDITION(dFly, mMode.mValue != Mode::Flareon);
        VISIBILITY_CONDITION(mHeightLoss, mMode.mValue == Mode::Jump);
        VISIBILITY_CONDITION(mJumpDelay, mMode.mValue == Mode::Jump);
        VISIBILITY_CONDITION(mResetOnGround, mMode.mValue == Mode::Jump);
        VISIBILITY_CONDITION(mResetOnDisable, mMode.mValue == Mode::Jump);

        VISIBILITY_CONDITION(dSpeed, mMode.mValue != Mode::Flareon);
        VISIBILITY_CONDITION(mSpeed, (mMode.mValue == Mode::Motion || mMode.mValue == Mode::Jump || mMode.mValue == Mode::Elytra || mMode.mValue == Mode::Flareon));
        VISIBILITY_CONDITION(mTimerBoost, mMode.mValue != Mode::Flareon);
        VISIBILITY_CONDITION(mTimerBoostValue, mTimerBoost.mValue && mMode.mValue != Mode::Flareon);
        VISIBILITY_CONDITION(mSpeedFriction, mMode.mValue == Mode::Jump);
        VISIBILITY_CONDITION(mFriction, mMode.mValue == Mode::Jump && mSpeedFriction.mValue);

        VISIBILITY_CONDITION(dBypass, mMode.mValue != Mode::Flareon);
        VISIBILITY_CONDITION(mApplyGlideFlags, mMode.mValue == Mode::Motion);
        VISIBILITY_CONDITION(mDamageOnly, mMode.mValue == Mode::Jump);
        VISIBILITY_CONDITION(mFlyTime, mMode.mValue == Mode::Jump && mDamageOnly.mValue);

        VISIBILITY_CONDITION(mClipDistance, mMode.mValue == Mode::Flareon);
        VISIBILITY_CONDITION(mClipDownAmount, mMode.mValue == Mode::Flareon);
        VISIBILITY_CONDITION(mClipUpAmount, mMode.mValue == Mode::Flareon);

        VISIBILITY_CONDITION(mDebug, true);

        VISIBILITY_CONDITION(mKeepMotion, mMode.mValue == Mode::Flareon);

        mNames = {
            {Lowercase, "fly"},
            {LowercaseSpaced, "fly"},
            {Normal, "Fly"},
            {NormalSpaced, "Fly"}
        };

        gFeatureManager->mDispatcher->listen<PacketInEvent, &Fly::onPacketInEvent>(this);
    }

    float mCurrentY = 0.f;
    uint64_t mLastJump = 0;
    float mCurrentFriction = 1.f;
    uint64_t mLastDamage = 0;

    // Flareon mode tracking
    float mDistanceTraveled = 0.f;
    glm::vec3 mLastPos = glm::vec3(0.f);
    bool mNetSkipEnabled = true;
    int mCurrentStage = 1;
    float mOriginalY = 0.f;
    glm::vec3 mInitialMotion = glm::vec3(0.f);
    uint32_t mPacketAllowTicks = 0;
    std::vector<glm::vec3> mDebugPositions;

    void onEnable() override;
    void onDisable() override;
    void displayDebug(const std::string& message) const;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onRenderEvent(class RenderEvent& event);
    bool tickJump(Actor* player);
    void jump();
    void onPacketOutEvent(class PacketOutEvent& event);
    void onPacketInEvent(class PacketInEvent& event);

    std::string getSettingDisplay() override
    {
        return mMode.mValues[mMode.as<int>()];
    }
};