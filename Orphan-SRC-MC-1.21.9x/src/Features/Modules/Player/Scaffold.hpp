#pragma once
//
// Created by vastrakai on 7/10/2024.
// Edited by player5 (1/24/2025)
//

#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerAuthInputPacket.hpp>

class Scaffold : public ModuleBase<Scaffold>
{
public:
    enum class RotateMode
    {
        None,
        Normal,
        Down,
        Backwards
    };

    enum class FlickMode
    {
        None,
        Combat,
        Always
    };

    enum class PlacementMode
    {
        Normal
    };

    enum class SwitchMode
    {
        None,
        Full,
        Fake,
        Spoof
    };

    enum class SwitchPriority
    {
        First,
        Highest
    };

    enum class TowerMode
    {
        Vanilla,
        Velocity,
        Clip
    };

    enum class BlockHUDStyle
    {
        None,
        Orphan
    };

    enum class BypassMode
    {
        None,
        LowPlaces
    };
    EnumSettingT<BlockHUDStyle> mBlockHUDStyle = EnumSettingT<BlockHUDStyle>("HUD Style", "The style for the block HUD", BlockHUDStyle::Orphan, "None", "Orphan");

    DividerSetting dBypass = DividerSetting("- Bypass -", "Settings for bypassing anticheat");
    EnumSettingT<BypassMode> mBypassMode = EnumSettingT<BypassMode>("Bypass Mode", "The mode of bypassing anticheat", BypassMode::None, "None", "Low Places");
    BoolSetting mAdjust = BoolSetting("Custom Height", "Use custom values for the clipping (at your own risk)", false);
    NumberSetting mPositiveHeight = NumberSetting("Height", "Where to lock player Y pos (2.62 = standing on floor)", 2.61f, 0.01f, 3.f, 0.01f);

    DividerSetting dPlacing = DividerSetting("- Placing -", "Settings for placing blocks");
    NumberSetting mPlaces = NumberSetting("Places", "The amount of blocks to place per tick", 1, 0, 2, 0.05);
    NumberSetting mRange = NumberSetting("Range", "The range at which to place blocks", 5, 0, 10, 0.25);
    NumberSetting mExtend = NumberSetting("Extend", "The distance to extend the placement forwards", 3, 0, 10, 1);
    NumberSetting mLRExtend = NumberSetting("Sideways Extend", "Override extend when moving sideways", 0, 0, 10, 1);
    BoolSetting mNoDiag = BoolSetting("No Diagonal", "Prevent diagonal placement", false);
    BoolSetting mSwing = BoolSetting("Swing", "Whether or not to swing the arm", true);
    BoolSetting mAvoidUnderplace = BoolSetting("Avoid Underplace", "Avoid placing under blocks", false);
    BoolSetting mLockY = BoolSetting("Lock Y", "Lock the Y position", false);
    BoolSetting mFastClutch = BoolSetting("Fast Clutch", "Whether or not to use fast clutch", false);
    NumberSetting mClutchFallDistance = NumberSetting("Clutch Fall Dist", "The fall distance to clutch at", 3, 0, 20, 0.25);
    NumberSetting mCluchPlaces = NumberSetting("Clutch Places", "The amount of blocks to place per tick", 1, 0, 20, 1);

    DividerSetting dTower = DividerSetting("- Towering -", "Settings for towering");
    EnumSettingT<TowerMode> mTowerMode = EnumSettingT<TowerMode>("Tower Mode", "The mode for tower placement", TowerMode::Vanilla, "Vanilla", "Velocity", "Clip");
    NumberSetting mTowerSpeed = NumberSetting("Tower Speed", "The speed for tower placement", 3.5, 0, 20, 0.25);
    BoolSetting mFallDistanceCheck = BoolSetting("Fall Distance Check", "Check fall distance before towering", false);
    BoolSetting mAllowMovement = BoolSetting("Allow Movement", "Allow movement while towering", false);

    DividerSetting dRotations = DividerSetting("- Rotation -", "Settings for player rotations");
    EnumSettingT<RotateMode> mRotateMode = EnumSettingT<RotateMode>("Rotate Mode", "The mode of rotation", RotateMode::Normal, "None", "Normal", "Down", "Backwards");
    EnumSettingT<FlickMode> mFlickMode = EnumSettingT<FlickMode>("Flick Mode", "The mode for block flicking", FlickMode::Combat, "None", "Combat", "Always");

    DividerSetting dSwitching = DividerSetting("- Switching -", "Settings for switching blocks");
    EnumSettingT<SwitchMode> mSwitchMode = EnumSettingT<SwitchMode>("Switch Mode", "The mode for block switching", SwitchMode::Full, "None", "Full", "Fake", "Spoof");
    EnumSettingT<SwitchPriority> mSwitchPriority = EnumSettingT<SwitchPriority>("Switch Prio", "The priority for block switching", SwitchPriority::First, "First", "Highest");
    BoolSetting mHotbarOnly = BoolSetting("Hotbar Only", "Only place blocks from the hotbar", false);

    Scaffold() : ModuleBase("Scaffold", "Automatically places blocks below you", ModuleCategory::Player, 0, false)
    {
        addSettings(
            &mBlockHUDStyle,

            &dBypass,
            &mBypassMode,
            &mAdjust,
            &mPositiveHeight,

            &dPlacing,
            &mPlaces,
            &mRange,
            &mExtend,
            &mLRExtend,
            &mNoDiag,
            &mSwing,
            &mAvoidUnderplace,
            &mLockY,
            &mFastClutch,
            &mClutchFallDistance,
            &mCluchPlaces,

            &dTower,
            &mTowerMode,
            &mTowerSpeed,
            &mFallDistanceCheck,
            &mAllowMovement,

            &dRotations,
            &mRotateMode,
            &mFlickMode,

            &dSwitching,
            &mSwitchMode,
            &mSwitchPriority,
            &mHotbarOnly
        );

        VISIBILITY_CONDITION(mFlickMode, mRotateMode.mValue != RotateMode::None);

        VISIBILITY_CONDITION(mSwitchPriority, mSwitchMode.mValue != SwitchMode::None);
        VISIBILITY_CONDITION(mHotbarOnly, mSwitchMode.mValue != SwitchMode::None);
        VISIBILITY_CONDITION(mTowerSpeed, mTowerMode.mValue != TowerMode::Vanilla);

        VISIBILITY_CONDITION(mClutchFallDistance, mFastClutch.mValue);
        VISIBILITY_CONDITION(mCluchPlaces, mFastClutch.mValue);

        VISIBILITY_CONDITION(mPlaces, mBypassMode.mValue == BypassMode::LowPlaces);;

        mNames =
        {
            {Lowercase, "scaffold"},
            {LowercaseSpaced, "scaffold"},
            {Normal, "Scaffold"},
            {NormalSpaced, "Scaffold"}
        };

        gFeatureManager->mDispatcher->listen<RenderEvent, &Scaffold::onRenderEvent>(this);
    }

    float mStartY = 0;
    glm::vec3 mLastBlock = { 0, 0, 0 };
    int mLastFace = -1;
    bool mShouldRotate = false;
    uint64_t mLastSwitchTime = 0;
    int mLastSlot = -1;
    bool mShouldClip = false;
    bool wasDisabledMidAir = false;

    // Tower stuff
    bool mIsTowering = false;
    uint64_t mLastPlaceTime = 0;

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    bool tickPlace(class BaseTickEvent& event);
    void onRenderEvent(class RenderEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
    glm::vec3 getRotBasedPos(float extend, float yPos);
    glm::vec3 getPlacePos(float extend);

    std::string getSettingDisplay() override {
        return mRotateMode.mValues[mRotateMode.as<int>()];
    }
};