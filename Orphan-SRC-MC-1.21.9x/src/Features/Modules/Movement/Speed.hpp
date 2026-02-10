#pragma once
//
// Created by vastrakai on 7/10/2024.
//
#include <Features/FeatureManager.hpp>
#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Network/Packets/MobEffectPacket.hpp>

enum class JumpType {
    Vanilla,
    Motion
};


enum class FastfallMode {
    None,
    Predict,
    SetVel,
    Timer,
    Multiply
};

enum StrafeMode {
    None,
    Hive,
    Ground,
    Custom
};


struct FrictionPreset
{
    float speed = 3.39;
    bool strafe = true;
    float strafeSpeed = 2.5;
    float friction = 1;
    FastfallMode fastFall = FastfallMode::None;
    int fallTicks = 5;
    float fallSpeed = 1.00;
    bool fastFall2 = false;
    int fallTicks2 = 5;
    float fallSpeed2 = 1.00;
    JumpType jumpType = JumpType::Motion;
    float jumpHeight = 0.42f;

    FrictionPreset() = default;
    FrictionPreset(float speed, bool strafe, bool useStrafeSpeed, float strafeSpeed, float friction, bool timerBoost, float timerSpeed, FastfallMode fastFall, int fallTicks, float fallSpeed, bool fastFall2, int fallTicks2, float fallSpeed2, JumpType jumpType, float jumpHeight)
        : speed(speed), strafe(strafe), strafeSpeed(strafeSpeed), friction(friction), fastFall(fastFall), fallTicks(fallTicks), fallSpeed(fallSpeed), fastFall2(fastFall2), fallTicks2(fallTicks2), fallSpeed2(fallSpeed2), jumpType(jumpType), jumpHeight(jumpHeight)
    {}
};

class Speed : public ModuleBase<Speed> {
public:
    enum class Mode {
        Friction,
        Legit,
        HiveSafe
    };

    EnumSettingT<Mode> mMode = EnumSettingT("Mode", "The mode of speed", Mode::Friction, "Friction", "Legit", "HiveSafe");

    DividerSetting dSpeed = DividerSetting("- Speed -", "Options for speed");
    NumberSetting mFriction = NumberSetting("Friction", "The friction to apply", 0.975, 0, 1, 0.01);
    NumberSetting mSpeed = NumberSetting("Speed", "The speed to move at", 3, 0, 10, 0.01);
    EnumSetting mStrafeMode = EnumSetting("Strafe Mode", "How you strafe", StrafeMode::Hive, "None", "Hive", "Ground", "Custom");
    NumberSetting mStrafeSpeed = NumberSetting("Strafe Speed", "The speed to strafe at", 2.5, 0, 10, 0.01);
    //BoolSetting mStrafe = BoolSetting("Strafe", "Strafe bypass for hive. \n Not even a bypass just slows downs speed.", true);
    //BoolSetting mGroundStrafe = BoolSetting("Ground Strafe", "Whether or not to allow strafing", false);

    //DividerSetting dSwiftness = DividerSetting("- Swiftness -", "Options for swiftness");
    //BoolSetting mSwiftness = BoolSetting("Swiftness", "Whether or not to apply swiftness when space is pressed (will not be applied when scaffold is enabled)", false);
    //BoolSetting mSwiftnessHotbar = BoolSetting("Swiftness Hotbar", "Only uses swiftness from hotbar", false);
    //NumberSetting mSwiftnessSpeed = NumberSetting("Swiftness Speed", "The speed to apply when swiftness is active", 0.55, 0, 1, 0.01);
    //NumberSetting mSwiftnessFriction = NumberSetting("Swiftness Friction", "The friction to apply when swiftness is active", 0.975, 0, 1, 0.01);
    //BoolSetting mHoldSpace = BoolSetting("Hold Space", "Only applies swiftness effect while holding space", false);

    DividerSetting dFastFall = DividerSetting("- Fast Fall -", "Options for fast fall");
    EnumSettingT<FastfallMode> mFastFall = EnumSettingT("Fast Fall", "The mode of fast fall", FastfallMode::None, "None", "Predict", "Set Vel", "Timer", "Multiply");
    NumberSetting mFallTicks = NumberSetting("Fall Ticks", "The tick to apply down motion at", 5, 0, 20, 1);
    NumberSetting mFallSpeed = NumberSetting("Fall Speed", "The speed to fall down at", 1.00, 0, 10, 0.01);

    NumberSetting mTimerFallSpeed = NumberSetting("Timer Fall Speed", "The speed to fall down at", 20, 0, 30, 1);
    NumberSetting mMultiplyFallValue = NumberSetting("Multiply Fall Value", "The speed to fall down at", 10, 0, 20, 1);

    BoolSetting mFastFall2 = BoolSetting("Fast Fall 2", "Whether or not to fast fall again", false);
    NumberSetting mFallTicks2 = NumberSetting("Fall Ticks 2", "The tick to apply down motion at", 5, 0, 20, 1);
    NumberSetting mFallSpeed2 = NumberSetting("Fall Speed 2", "The speed to fall down at", 1.00, 0, 10, 0.01);
    NumberSetting mTimerFallSpeed2 = NumberSetting("Timer Fall Speed 2", "The speed to fall down at", 20, 0, 30, 1);
    NumberSetting mMultiplyFallValue2 = NumberSetting("Multiply Fall Value 2", "The speed to fall down at", 10, 0, 20, 1);

    DividerSetting dJump = DividerSetting("- Jump -", "Settings for jumping");
    EnumSettingT<JumpType> mJumpType = EnumSettingT("Jump Type", "The type of jump to use", JumpType::Vanilla, "Vanilla", "Motion", "None");
    NumberSetting mJumpHeight = NumberSetting("Jump Height", "The height to jump at", 0.42f, 0, 1, 0.01);

    Speed() : ModuleBase("Speed", "Lets you move faster", ModuleCategory::Movement, 0, false) {
        addSettings(
            &mMode,

            &dSpeed,
            &mFriction,
            &mSpeed,
            &mStrafeMode,
            &mStrafeSpeed,
            //&mStrafe,
            //&mGroundStrafe,

			/*&dSwiftness,
            &mSwiftness,
            &mSwiftnessHotbar,
            &mSwiftnessSpeed,
            &mSwiftnessFriction,
            &mHoldSpace,*/

            &dFastFall,
            &mFastFall,
            &mFallTicks,
            &mFallSpeed,

            &mTimerFallSpeed,
            &mMultiplyFallValue,

            &mFastFall2,
            &mFallTicks2,
            &mFallSpeed2,
            &mTimerFallSpeed2,
            &mMultiplyFallValue2,

            &dJump,
            &mJumpType,
            &mJumpHeight
        );

        VISIBILITY_CONDITION(dSpeed, mMode.mValue != Mode::HiveSafe);
        //VISIBILITY_CONDITION(dSwiftness, mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(dFastFall, mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(dJump, mMode.mValue != Mode::HiveSafe);

        VISIBILITY_CONDITION(mStrafeMode, mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mStrafeSpeed, (mStrafeMode.mValue == StrafeMode::Ground || mStrafeMode.mValue == StrafeMode::Custom) && mMode.mValue != Mode::HiveSafe);

        /*VISIBILITY_CONDITION(mSwiftness, mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mSwiftnessHotbar, mSwiftness.mValue && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mSwiftnessSpeed, mSwiftness.mValue && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mSwiftnessFriction, mSwiftness.mValue && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mHoldSpace, mSwiftness.mValue && mMode.mValue != Mode::HiveSafe);*/

        VISIBILITY_CONDITION(mSpeed, (mMode.mValue == Mode::Friction || mMode.mValue == Mode::Legit) && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mFriction, (mMode.mValue == Mode::Friction || mMode.mValue == Mode::Legit) && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mFastFall, (mMode.mValue == Mode::Friction || mMode.mValue == Mode::Legit) && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mFallTicks, mFastFall.mValue != FastfallMode::None && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mFallSpeed, mFastFall.mValue != FastfallMode::None && mFastFall.mValue != FastfallMode::Timer && mFastFall.mValue != FastfallMode::Multiply && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mFastFall2, mFastFall.mValue != FastfallMode::None && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mFallTicks2, mFastFall.mValue != FastfallMode::None && mFastFall2.mValue && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mFallSpeed2, mFastFall.mValue != FastfallMode::None && mFastFall.mValue != FastfallMode::Timer && mFastFall.mValue != FastfallMode::Multiply && mFastFall2.mValue && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mJumpType, (mMode.mValue == Mode::Friction || mMode.mValue == Mode::Legit || mMode.mValue == Mode::HiveSafe));
        VISIBILITY_CONDITION(mTimerFallSpeed, mFastFall.mValue == FastfallMode::Timer && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mTimerFallSpeed2, mFastFall.mValue == FastfallMode::Timer && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mMultiplyFallValue, mFastFall.mValue == FastfallMode::Multiply && mMode.mValue != Mode::HiveSafe);
        VISIBILITY_CONDITION(mMultiplyFallValue2, mFastFall.mValue == FastfallMode::Multiply && mFastFall2.mValue && mMode.mValue != Mode::HiveSafe);

        mNames = {
            {Lowercase, "speed"},
            {LowercaseSpaced, "speed"},
            {Normal, "Speed"},
            {NormalSpaced, "Speed"}
        };

        gFeatureManager->mDispatcher->listen<PacketInEvent, &Speed::onPacketInEvent>(this);
    }
    std::map<EffectType, uint64_t> mEffectTimers = {};
    float mDamageBoostVal = 1.f;
    bool mDamageTimerApplied = false;
    bool mClip = false;
    uint64_t mLastAvoidCheck = 0;
    glm::vec2 lastGroundMotion = {0, 0};

    void onEnable() override;
    void onDisable() override;
    void onRunUpdateCycleEvent(class RunUpdateCycleEvent& event);
    //bool tickSwiftness();
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketInEvent(class PacketInEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
    void tickLegit(Actor* player);
    void tickFriction(Actor* player);
    void tickFrictionPreset(FrictionPreset& preset);

    std::string getSettingDisplay() override
    {
        return mMode.mValues[mMode.as<int>()];
    }
};
