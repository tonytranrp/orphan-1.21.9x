//
// Edited by player5 (1/24/2025)
//

#pragma once

#include <Features/Modules/Module.hpp>

class DestroyProgress : public ModuleBase<DestroyProgress>
{
public:
    enum class ColorMode {
        Default,
        Theme
    };
    enum class Mode {
        In,
        Orphan,
    };

    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for visuals");
    EnumSettingT<ColorMode> mColorMode = EnumSettingT<ColorMode>("Color", "The color mode", ColorMode::Default, "Default", "Theme");
    EnumSettingT<Mode> mMode = EnumSettingT<Mode>("Mode", "Animation style", Mode::Orphan, "In", "Orphan");
    BoolSetting mFilled = BoolSetting("Filled", "Fill box", true);
    NumberSetting mOpacity = NumberSetting("Opacity", "Opacity of the box", 0.40f, 0.f, 1.f, 0.1f);

    DividerSetting dText = DividerSetting("- Text -", "Settings for text");
    NumberSetting mFontSize = NumberSetting("Font Size", "The size of the font", 20, 1, 40, 0.1);;
    BoolSetting mBold = BoolSetting("Bold Text", "Use bold text", false);
    BoolSetting mDistanceScaledFont = BoolSetting("Distance Scaled Font", "Scale the font based on distance", true);
    NumberSetting mScalingMultiplier = NumberSetting("Scaling Multiplier", "The multiplier to use for scaling the font", 1.25f, 0.f, 5.f, 0.25f);





    DestroyProgress() : ModuleBase("DestroyProgress", "Render Destroy Progress", ModuleCategory::Visual, 0, false) {
        addSettings
        (
            &dVisual,
            &mColorMode,
            &mMode,
            &mFilled,
            &mOpacity,

            &dText,
            &mFontSize,
            &mBold,
            &mDistanceScaledFont,
            &mScalingMultiplier
        );
        VISIBILITY_CONDITION(mFontSize, !mDistanceScaledFont.mValue);
        VISIBILITY_CONDITION(mScalingMultiplier, mDistanceScaledFont.mValue);


        mNames = {
            {Lowercase, "destroyprogress"},
            {LowercaseSpaced, "destroy progress"},
            {Normal, "DestroyProgress"},
            {NormalSpaced, "Destroy Progress"}
        };
    }

    void onEnable() override;
    void onDisable() override;
    void onRenderEvent(class RenderEvent& event);
};