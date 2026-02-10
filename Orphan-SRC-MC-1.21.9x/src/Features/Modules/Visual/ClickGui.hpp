#pragma once
//
// Created by vastrakai on 6/29/2024.
//


#include <Features/FeatureManager.hpp>
#include <Features/Modules/Setting.hpp>

class ClickGui : public ModuleBase<ClickGui>
{
public:
    enum class ClickGuiStyle {
        Modern,
        Simple
    };
    enum class ClickGuiAnimation {
        Zoom,
        Bounce
    };
    enum class ClickGuiSort {
        Normal,
        Length
    };
    enum class ClickGuiColorDirection {
        Horizontal,
        Vertical
    };
    enum class ModuleColorStyle {
        Full,
        Accent
    };
    enum class DropdownVisual {
        None,
        Plus,
        Dot,
        Gear
    };
    enum class SortMode {
        Normal,
        Length
    };

    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for the visuals");
    EnumSettingT<ClickGuiStyle> mStyle = EnumSettingT<ClickGuiStyle>("Style", "Style of the ClickGui", ClickGuiStyle::Simple, "Orphan", "Simple");
    EnumSettingT<ClickGuiAnimation> mAnimation = EnumSettingT<ClickGuiAnimation>("Animation", "Animation when toggling the ClickGui", ClickGuiAnimation::Zoom, "Zoom", "Bounce");
    EnumSettingT<DropdownVisual> mDropType = EnumSettingT<DropdownVisual>("Dropdown Visual", "Style of the icons at the right of each module", DropdownVisual::None, "None", "Plus", "Dot", "Gear");
    EnumSettingT<SortMode> mSortMode = EnumSettingT<SortMode>("Sorting", "How to sort the modules", SortMode::Normal, "Normal", "Length");
    BoolSetting mShadow = BoolSetting("Shadows", "Enable shadows behind the GUI", true);

    DividerSetting dSimple = DividerSetting("- Simple GUI -", "Settings for the Simple GUI");
    EnumSettingT<ClickGuiSort> mSort = EnumSettingT<ClickGuiSort>("Sort", "How to sort the modules in the GUI", ClickGuiSort::Normal, "Normal", "Length");
    EnumSettingT<ClickGuiColorDirection> mColorDirection = EnumSettingT<ClickGuiColorDirection>("Color Direction", "Direction of color gradients", ClickGuiColorDirection::Horizontal, "Horizontal", "Vertical");
    EnumSettingT<ModuleColorStyle> mModuleColorStyle = EnumSettingT<ModuleColorStyle>("Module Style", "How to display enabled modules", ModuleColorStyle::Full, "Full", "Accent");
    NumberSetting mShadowThickness = NumberSetting("Shadow Thickness", "Thickness of the shadow surrounding the ClickGUI", 75.f, 50.f, 100.f, 1.f);

    DividerSetting dOrphan = DividerSetting("- Orphan GUI -", "Settings for the Orphan GUI");
    BoolSetting miOS = BoolSetting("iOS Toggles", "Use iOS-style boolean toggles", true);
    NumberSetting mBG = NumberSetting("Background", "Opacity of the background", 0.5f, 0.f, 1.f, 0.01f);
    BoolSetting mRoundTop = BoolSetting("Round Top", "Round the top of the GUI", true);
    BoolSetting mRoundBot = BoolSetting("Round Bottom", "Round the bottom of the GUI", true);

    DividerSetting dAudit = DividerSetting("- Audit GUI -", "Settings for the Audit GUI");
    NumberSetting mScanLineSpeed = NumberSetting("Scan Line Speed", "Speed of the TV scan lines", 1.0f, 0.1f, 5.0f, 0.1f);
    NumberSetting mScanLineOpacity = NumberSetting("Scan Line Opacity", "Opacity of the TV scan lines", 0.15f, 0.0f, 1.0f, 0.01f);
    BoolSetting mGlowEffect = BoolSetting("Glow Effect", "Enable glow effect on elements", true);

    DividerSetting dOther = DividerSetting("- Other -", "Other settings");
    NumberSetting mBlurStrength = NumberSetting("Blur Strength", "The strength of the blur.", 7.f, 0.f, 20.f, 0.1f);
    NumberSetting mEaseSpeed = NumberSetting("Ease Speed", "The speed of the easing.", 18.f, 5.f, 20.f, 0.1f);
    NumberSetting mMidclickRounding = NumberSetting("Midclick Rounding", "The value to round to when middle-clicking a NumberSetting.", 1.f, 0.01f, 1.f, 0.01f);

    ClickGui() : ModuleBase("ClickGui", "A customizable GUI for toggling modules.", ModuleCategory::Visual, VK_INSERT, false) {
        // Register your features here
        gFeatureManager->mDispatcher->listen<RenderEvent, &ClickGui::onRenderEvent, nes::event_priority::LAST>(this);
        gFeatureManager->mDispatcher->listen<WindowResizeEvent, &ClickGui::onWindowResizeEvent>(this);

        addSettings(
            &dVisual,
            &mStyle,
            &mAnimation,
            &mDropType,
            &mSortMode,
            &mShadow,
            &mShadowThickness
        );

        addSettings(
            &dSimple,
            &mSort,
            &mColorDirection,
            &mModuleColorStyle
        );

        addSettings(
            &dOrphan,
            &miOS,
            &mBG,
            &mRoundTop,
            &mRoundBot
        );

        addSettings(
            &dAudit,
            &mScanLineSpeed,
            &mScanLineOpacity,
            &mGlowEffect
        );

        addSettings(
            &dOther,
            &mMidclickRounding
        );

        VISIBILITY_CONDITION(dSimple, mStyle.mValue == ClickGuiStyle::Simple);
        VISIBILITY_CONDITION(mSort, mStyle.mValue == ClickGuiStyle::Simple);
        VISIBILITY_CONDITION(mColorDirection, mStyle.mValue == ClickGuiStyle::Simple);
        VISIBILITY_CONDITION(mModuleColorStyle, mStyle.mValue == ClickGuiStyle::Simple);

        VISIBILITY_CONDITION(dOrphan, mStyle.mValue == ClickGuiStyle::Modern);
        VISIBILITY_CONDITION(miOS, mStyle.mValue == ClickGuiStyle::Modern);
        VISIBILITY_CONDITION(mBG, mStyle.mValue == ClickGuiStyle::Modern);
        VISIBILITY_CONDITION(mRoundTop, mStyle.mValue == ClickGuiStyle::Modern);
        VISIBILITY_CONDITION(mRoundBot, mStyle.mValue == ClickGuiStyle::Modern);

        mNames = {
            {Lowercase, "clickgui"},
            {LowercaseSpaced, "click gui"},
            {Normal, "ClickGui"},
            {NormalSpaced, "Click Gui"}
        };
    }

    void onEnable() override;
    void onDisable() override;

    void onWindowResizeEvent(class WindowResizeEvent& event);
    void onMouseEvent(class MouseEvent& event);
    void onKeyEvent(class KeyEvent& event);
    float getEaseAnim(EasingUtil ease, int mode);
    void onRenderEvent(class RenderEvent& event);

    std::string getSettingDisplay() override {
        return mStyle.mValues[mStyle.as<int>()];
    }
};