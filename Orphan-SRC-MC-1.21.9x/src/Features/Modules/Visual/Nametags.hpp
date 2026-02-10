#pragma once
//
// Created by vastrakai on 8/10/2024.
// Edited by rekitrelt (2/2/2025)
// Edited by player5 (2/3/2025)
//

#include <Features/Modules/Module.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Modules/Setting.hpp>

class Nametags : public ModuleBase<Nametags>
{
public:
    enum class Style {
        Solstice
    };

    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for visuals");
    EnumSettingT<Style> mStyle = EnumSettingT<Style>("Style", "The style of the nametags", Style::Solstice, "Solstice");
    BoolSetting mRenderLocal = BoolSetting("Render Local", "Render the nametags on the local player", false);
    BoolSetting mShowHP = BoolSetting("Show Absorption", "Show the gapple hearts (absorption) of the player", true);
    BoolSetting mAccurate = BoolSetting("Accurate", "5 (hearts) instead of 10 (half-hearts)", true);

    DividerSetting dColor = DividerSetting("- Color -", "Settings for color");
    BoolSetting mShowFriends = BoolSetting("Show Friends", "Render the nametags on friends", true);
    BoolSetting mShowTeams = BoolSetting("Show Teams", "Color names based on team", true);
    BoolSetting mAlwaysShowEnemies = BoolSetting("Show Enemies", "Shows enemies", true);

    DividerSetting dText = DividerSetting("- Text -", "Settings for text");
    BoolSetting mDistanceScaledFont = BoolSetting("Distance Scaled Font", "Scale the font based on distance", true);
    NumberSetting mFontSize = NumberSetting("Font Size", "The size of the font", 23, 1, 40, 0.1);
    NumberSetting mScalingMultiplier = NumberSetting("Scaling Multiplier", "The multiplier to use for scaling the font", 0, 0.f, 5.f, 0.1f);
    NumberSetting mMinScale = NumberSetting("Minimum Scale", "The minimum scale to use for scaling the font", 20.f, 0.01f, 20.f, 0.1f);

    //NumberSetting mBlurStrength = NumberSetting("Blur Strength", "The strength of the blur.", 0.f, 0.f, 10.f, 0.1f);

    Nametags() : ModuleBase("Nametags", "Draws nametags above players", ModuleCategory::Visual, 0, false) {
        addSettings(
            &dVisual,
            &mStyle,
            &mRenderLocal,
            &mShowHP,
            &mAccurate,

            &dColor,
            &mShowFriends,
            &mShowTeams,
            &mAlwaysShowEnemies,

            &dText,
            &mDistanceScaledFont,
            &mFontSize,
            &mScalingMultiplier,
            &mMinScale

            //&mBlurStrength,
        );

        VISIBILITY_CONDITION(mAccurate, mShowHP.mValue);

        VISIBILITY_CONDITION(mFontSize, !mDistanceScaledFont.mValue);
        VISIBILITY_CONDITION(mScalingMultiplier, mDistanceScaledFont.mValue);
        VISIBILITY_CONDITION(mMinScale, mDistanceScaledFont.mValue);

        mNames = {
            {Lowercase, "nametags"},
            {LowercaseSpaced, "nametags"},
            {Normal, "Nametags"},
            {NormalSpaced, "Nametags"}
        };
    }

    struct HealthInfo {
        float health = 20;
        float lastAbsorption = 0;
        float damage = 1;
    };
    std::map<std::string, HealthInfo> mHealths;
    uint64_t mLastHealTime = 0;

    void onEnable() override;
    void onDisable() override;
    void calculateHealths();
    void onPacketInEvent(class PacketInEvent& event);
    void onBaseTickEvent(BaseTickEvent& event);
    void onRenderEvent(class RenderEvent& event);
    void onNametagRenderEvent(class NametagRenderEvent& event);
};