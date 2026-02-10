#pragma once
#include <Features/Modules/Module.hpp>

class Arraylist : public ModuleBase<Arraylist>
{
public:
    enum class BackgroundStyle {
        Opacity,
        Shadow,
        Both
    };

    enum class Display {
        Bar,
        Split,
        None
    };

    enum class ModuleVisibility {
        All,
        Bound,
    };

    DividerSetting dBackground = DividerSetting("- Background -", "Settings for the background");
    EnumSettingT<BackgroundStyle> mBackground = EnumSettingT("Background", "Background style", BackgroundStyle::Both, "Opacity", "Shadow", "Both");
    NumberSetting mBackgroundOpacity = NumberSetting("Opacity", "The opacity of the background", 0.5f, 0.0f, 1.f, 0.05f);
    NumberSetting mShadowOpacity = NumberSetting("Shadow Strength", "The boldness of the shadow", 1.f, 0.0f, 2.f, 0.05f);
    NumberSetting mBackgroundValue = NumberSetting("Background Value", "The value of the background", 0.0f, 0.0f, 1.f, 0.01f);

    DividerSetting dDisplay = DividerSetting("- Display -", "Settings for the display");
    EnumSettingT<Display> mDisplay = EnumSettingT("Display", "Display style", Display::Split, "Bar", "Split", "None");
    NumberSetting mThickness = NumberSetting("Bar Width", "The thickness of the bar", 5.f, 1.f, 5.f, 1.f);
    EnumSettingT<ModuleVisibility> mVisibility = EnumSettingT("Visibility", "Module visibility", ModuleVisibility::All, "All", "Bound");
    BoolSetting mRenderMode = BoolSetting("Render Mode", "Renders the module mode next to the module name", true);
    BoolSetting mGlow = BoolSetting("Glow", "Enables glow", true);
    BoolSetting mDisplayGlow = BoolSetting("No Text Glow", "Only applies glow to display styles", true);
    NumberSetting mGlowStrength = NumberSetting("Glow Strength", "The strength of the glow", 0.5f, 0.1f, 1.f, 0.1f);

    DividerSetting dFont = DividerSetting("- Font -", "Settings for the font");
    BoolSetting mSansFont = BoolSetting("Sans Font", "Use Product Sans for the Arraylist", false);
    BoolSetting mBoldText = BoolSetting("Bold Text", "Makes the text bold", true);
    NumberSetting mFontSize = NumberSetting("Font Size", "The size of the font", 25.f, 10.f, 40.f, 0.01f);

    DividerSetting dColor = DividerSetting("- Color -", "Settings for colors");
    BoolSetting mHorizontalColor = BoolSetting("Horizontal Color", "Makes the colors go horizontally instead of vertically", false);
    BoolSetting mThemeColor = BoolSetting("Use Theme Color", "Use the Interface theme color", true);
    NumberSetting mColors = NumberSetting("Colors", "The amount of colors in the interface", 3, 1, 6, 1);
    ColorSetting mColor1 = ColorSetting("Color 1", "The first color of the interface", 0xFFFF0000);
    ColorSetting mColor2 = ColorSetting("Color 2", "The second color of the interface", 0xFFFF7F00);
    ColorSetting mColor3 = ColorSetting("Color 3", "The third color of the interface", 0xFFFFD600);
    ColorSetting mColor4 = ColorSetting("Color 4", "The fourth color of the interface", 0xFF00FF00);
    ColorSetting mColor5 = ColorSetting("Color 5", "The fifth color of the interface", 0xFF0000FF);
    ColorSetting mColor6 = ColorSetting("Color 6", "The sixth color of the interface", 0xFF8B00FF);
    NumberSetting mColorSpeed = NumberSetting("Color Speed", "The speed of the color change", 10.f, 0.1f, 20.f, 0.1f);
    NumberSetting mSaturation = NumberSetting("Saturation", "The saturation of the interface", 1.f, 0.f, 1.f, 0.1f);
    BoolSetting mExcludeVisual = BoolSetting("Exclude Visual", "Exclude modules in the Visual category from the arraylist", false);

    Arraylist() : ModuleBase("Arraylist", "Displays a list of modules", ModuleCategory::Visual, 0, true) {
        addSettings(
            &dBackground,
            &mBackground,
            &mBackgroundOpacity,
            &mShadowOpacity,
            //&mBackgroundValue,

            &dDisplay,
            &mDisplay,
            &mThickness,
            &mVisibility,
            &mRenderMode,
            &mGlow,
            &mDisplayGlow,
            &mGlowStrength,

            &dFont,
            &mSansFont,
            &mBoldText,
            &mFontSize,

            &dColor,
            &mHorizontalColor,
            &mThemeColor,
            &mColors,
            &mColor1,
            &mColor2,
            &mColor3,
            &mColor4,
            &mColor5,
            &mColor6,
            &mColorSpeed,
            &mSaturation,
            &mExcludeVisual
        );


        VISIBILITY_CONDITION(mThickness, mDisplay.mValue != Display::None)

        VISIBILITY_CONDITION(mDisplayGlow, mGlow.mValue);
        VISIBILITY_CONDITION(mGlowStrength, mGlow.mValue);
        VISIBILITY_CONDITION(mShadowOpacity, mBackground.mValue != BackgroundStyle::Opacity);

        VISIBILITY_CONDITION(mThemeColor, mHorizontalColor.mValue);
		VISIBILITY_CONDITION(mColorSpeed, mHorizontalColor.mValue && !mThemeColor.mValue);
		VISIBILITY_CONDITION(mSaturation, mHorizontalColor.mValue && !mThemeColor.mValue);
        VISIBILITY_CONDITION(mColors, mHorizontalColor.mValue && !mThemeColor.mValue);
        VISIBILITY_CONDITION(mColor1, mHorizontalColor.mValue && mColors.mValue >= 1 && !mThemeColor.mValue);
        VISIBILITY_CONDITION(mColor2, mHorizontalColor.mValue && mColors.mValue >= 2 && !mThemeColor.mValue);
        VISIBILITY_CONDITION(mColor3, mHorizontalColor.mValue && mColors.mValue >= 3 && !mThemeColor.mValue);
        VISIBILITY_CONDITION(mColor4, mHorizontalColor.mValue && mColors.mValue >= 4 && !mThemeColor.mValue);
        VISIBILITY_CONDITION(mColor5, mHorizontalColor.mValue && mColors.mValue >= 5 && !mThemeColor.mValue);
        VISIBILITY_CONDITION(mColor6, mHorizontalColor.mValue && mColors.mValue >= 6 && !mThemeColor.mValue);

        mNames = {
            {Lowercase, "arraylist"},
            {LowercaseSpaced, "array list"},
            {Normal, "Arraylist"},
            {NormalSpaced, "Array List"}
        };
    }
    void onEnable() override;
    void onDisable() override;

    void onRenderEvent(class RenderEvent& event);
    std::string getSettingDisplay() override {
        return mDisplay.mValues[mDisplay.as<int>()];
    }
};//aa