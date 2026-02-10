#pragma once
//
// Created by vastrakai on 7/8/2024.
//

#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>


class Aura : public ModuleBase<Aura> {
public:
    enum class Mode {
        Single,
        Multi,
        Switch
    };

    enum class AttackMode {
        Earliest,
        Synced
    };

    enum class RotateMode {
        None,
        Normal,
        Flick
    };

    enum class SwitchMode {
        None,
        Full,
        Spoof
    };

    enum class BypassMode {
        None,
        FlareonV2,
        Raycast
    };

    DividerSetting dAttack = DividerSetting("- Attack -", "Settings for attacking");
    EnumSettingT<Mode> mMode = EnumSettingT("Mode", "The mode of the aura", Mode::Switch, "Single", "Multi", "Switch");
    EnumSettingT<AttackMode> mAttackMode = EnumSettingT("Attack Mode", "The mode of attack", AttackMode::Synced, "Earliest", "Synced");
    BoolSetting mRandomizeAPS = BoolSetting("Randomize APS", "Whether or not to randomize the APS", false);
    NumberSetting mAPS = NumberSetting("APS", "Attacks per second", 8, 0, 20, 0.1);
    NumberSetting mAPSMin = NumberSetting("APS Min", "The minimum APS to randomize", 8, 0, 20, 0.1);
    NumberSetting mAPSMax = NumberSetting("APS Max", "The maximum APS to randomize", 12, 0, 20, 0.1);
    BoolSetting mFistFriends = BoolSetting("Fist Friends", "Whether or not to fist friends", false);
    BoolSetting mAttackThroughWalls = BoolSetting("Attack through walls", "Whether or not to attack through walls", true);
    BoolSetting mSwing = BoolSetting("Swing", "Whether or not to swing the arm", true);
    BoolSetting mSwingDelay = BoolSetting("Swing Delay", "Whether or not to delay the swing", false);
    NumberSetting mSwingDelayValue = NumberSetting("Swing Delay Value", "The delay between swings (in seconds)", 4.5f, 0.f, 10.f, 0.1f);
    EnumSettingT<BypassMode> mBypassMode = EnumSettingT("Bypass Mode", "The type of bypass", BypassMode::Raycast, "None", "FlareonV2", "Raycast");

    DividerSetting dRotate = DividerSetting("- Rotation -", "Settings for rotation");
    EnumSettingT<RotateMode> mRotateMode = EnumSettingT("Rotate Mode", "The mode of rotation", RotateMode::Flick, "None", "Normal", "Flick");
    BoolSetting mStrafe = BoolSetting("Strafe", "Whether or not to strafe around the target", true);

    DividerSetting dSwitch = DividerSetting("- Switching -", "Settings for switching");
    EnumSettingT<SwitchMode> mSwitchMode = EnumSettingT("Switch Mode", "The mode of switching", SwitchMode::Spoof, "None", "Full", "Spoof");
    BoolSetting mAutoFireSword = BoolSetting("Auto Fire Sword", "Whether or not to automatically use the fire sword", false);
    BoolSetting mFireSwordSpoof = BoolSetting("Fire Sword Spoof", "Whether or not to spoof the fire sword", false);
    BoolSetting mThrowProjectiles = BoolSetting("Throw Projectiles", "Whether or not to throw projectiles at the target", false);
    NumberSetting mThrowDelay = NumberSetting("Throw Delay", "The delay between throwing projectiles (in ticks)", 1, 0, 20, 0.1);
    BoolSetting mAutoBow = BoolSetting("Auto Bow", "Whether or not to automatically shoot the bow", false);
    BoolSetting mHotbarOnly = BoolSetting("Hotbar Only", "Whether or not to only attack with items in the hotbar", false);

    DividerSetting dRange = DividerSetting("- Range -", "Settings for range");
    NumberSetting mRange = NumberSetting("Range", "The range at which to attack enemies", 5, 0, 10, 0.1);
    BoolSetting mDynamicRange = BoolSetting("Dynamic Range", "Sets the range to the specified value when not moving", false);
    NumberSetting mDynamicRangeValue = NumberSetting("Dynamic Value", "The value for the dynamic range", 3, 0, 10, 0.1);
    BoolSetting mLimitAngle = BoolSetting("FOV/Angles", "Limit the attack frustum relative to the camera", false);
    NumberSetting mHorizontalMax = NumberSetting("Horizontal Angle", "Horizontal aura range", 90, 45, 180, 1);
    NumberSetting mVerticalMax = NumberSetting("Vertical Angle", "Vertical aura range", 180, 45, 180, 1);



    DividerSetting dOther = DividerSetting("- Other -", "Other settings");
    BoolSetting mThirdPerson = BoolSetting("Third Person", "Whether or not switch to third-person camera view on enable", false);
    BoolSetting mThirdPersonOnlyOnAttack = BoolSetting("Only On Attack", "Switch to third-person view only when attacking", false);
    BoolSetting mDisableOnDimensionChange = BoolSetting("Auto Disable", "Whether or not to disable the aura on dimension change", true);
    BoolSetting mDebug = BoolSetting("Debug", "Whether or not to display debug information", false);
    BoolSetting mInteract = BoolSetting("Interact", "Whether or not to interact with the target", false);

    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for the visuals");
    BoolSetting mVisuals = BoolSetting("Visuals", "Whether or not to render visuals around the target", true);
    NumberSetting mUpDownSpeed = NumberSetting("Up-Down Speed", "Speed of spheres rotate", 1, 0, 20, 1);
    NumberSetting mSpheresAmount = NumberSetting("Spheres Amount", "Amount of spheres to draw", 8, 0, 20, 1);
    NumberSetting mSpheresSizeMultiplier = NumberSetting("Spheres Size multiplier", "Multiplied size of spheres", 1, 0, 3, 0.1);
    NumberSetting mSpheresSize = NumberSetting("Spheres Size", "Size of spheres", 4, 0, 20, 1);
    NumberSetting mSpheresMinSize = NumberSetting("Spheres Min Size", "Min size of spheres", 0, 0, 20, 1);
    NumberSetting mSpheresRadius = NumberSetting("Spheres radius", "Distance from target", 1, 0, 2, 0.1);




    Aura() : ModuleBase("Aura", "Automatically attacks nearby enemies", ModuleCategory::Combat, 0, false) {
        addSettings(
            &dAttack,
            &mMode,
            &mAttackMode,
            &mRandomizeAPS,
            &mAPS,
            &mAPSMin,
            &mAPSMax,
            &mFistFriends,
            &mAttackThroughWalls,
            &mSwing,
            &mSwingDelay,
            &mSwingDelayValue,
            &mBypassMode,

            &dRotate,
            &mRotateMode,
            &mStrafe,

            &dSwitch,
            &mSwitchMode,
            &mAutoFireSword,
            &mFireSwordSpoof,
            &mThrowProjectiles,
            &mThrowDelay,
            &mAutoBow,
            &mHotbarOnly,

            &dRange,
            &mRange,
            &mDynamicRange,
            &mDynamicRangeValue,
            &mLimitAngle,
            &mHorizontalMax,
            &mVerticalMax,

            &dOther,
            &mThirdPerson,
            &mThirdPersonOnlyOnAttack,
            &mDisableOnDimensionChange,
            &mDebug,

            &dVisual,
            &mVisuals,
			&mUpDownSpeed,
            &mSpheresAmount,
			&mSpheresSizeMultiplier,
			&mSpheresSize,
			&mSpheresMinSize,
			&mSpheresRadius
            //&mInteract, // TODO: Implement this (more like delete)
        );

        VISIBILITY_CONDITION(mAutoFireSword, mSwitchMode.mValue != SwitchMode::None);
        VISIBILITY_CONDITION(mFireSwordSpoof, mAutoFireSword.mValue);
        VISIBILITY_CONDITION(mAPS, !mRandomizeAPS.mValue);
        VISIBILITY_CONDITION(mAPSMin, mRandomizeAPS.mValue);
        VISIBILITY_CONDITION(mAPSMax, mRandomizeAPS.mValue);
        VISIBILITY_CONDITION(mThrowDelay, mThrowProjectiles.mValue);
        VISIBILITY_CONDITION(mDynamicRangeValue, mDynamicRange.mValue);
        VISIBILITY_CONDITION(mHorizontalMax, mLimitAngle.mValue);
        VISIBILITY_CONDITION(mVerticalMax, mLimitAngle.mValue);

        VISIBILITY_CONDITION(mSwingDelay, mSwing.mValue);
        VISIBILITY_CONDITION(mSwingDelayValue, mSwingDelay.mValue && mSwing.mValue);

        VISIBILITY_CONDITION(mThirdPersonOnlyOnAttack, mThirdPerson.mValue);

        // vis conditions for visuals
        VISIBILITY_CONDITION(mUpDownSpeed, mVisuals.mValue);
        VISIBILITY_CONDITION(mSpheresAmount, mVisuals.mValue);
        VISIBILITY_CONDITION(mSpheresSizeMultiplier, mVisuals.mValue);
        VISIBILITY_CONDITION(mSpheresSize, mVisuals.mValue);
        VISIBILITY_CONDITION(mSpheresMinSize, mVisuals.mValue);
        VISIBILITY_CONDITION(mSpheresRadius, mVisuals.mValue);



        mNames = {
            {Lowercase, "aura"},
            {LowercaseSpaced, "aura"},
            {Normal, "Aura"},
            {NormalSpaced, "Aura"}
        };
    }

    AABB mTargetedAABB = AABB();
    bool mRotating = false;
    static inline bool sHasTarget = false;
    static inline Actor* sTarget = nullptr;
    static inline int64_t sTargetRuntimeID = 0;
    int64_t mLastSwing = 0;
    int64_t mLastTransaction = 0;
    int mLastSlot = 0;
    bool mIsThirdPerson = false;

    int getSword(Actor* target);
    bool shouldUseFireSword(Actor* target);
    void onEnable() override;
    void onDisable() override;
    void rotate(Actor* target);
    void shootBow(Actor* target);
    void throwProjectiles(Actor* target);
    void onRenderEvent(class RenderEvent& event);
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
    void onPacketInEvent(class PacketInEvent& event);
    void onBobHurtEvent(class BobHurtEvent& event);
    void onBoneRenderEvent(class BoneRenderEvent& event);
    Actor* findObstructingActor(Actor* player, Actor* target);

    std::string getSettingDisplay() override {
        return mMode.mValues[mMode.as<int>()];
    }
};
