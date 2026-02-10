#pragma once
//
// Created by vastrakai on 8/7/2024.
// Edited by player5 (1/24/2025)
//


class ItemESP : public ModuleBase<ItemESP> {
public:
    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for visuals");
    BoolSetting mDistanceLimited = BoolSetting("Distance Limited", "Only show items within a certain distance", true);
    NumberSetting mDistance = NumberSetting("Distance", "The distance to show items within", 100.f, 0.f, 100.f, 1.f);
    BoolSetting mRenderFilled = BoolSetting("Render Filled", "Render the boxes filled", false);
    BoolSetting mThemedColor = BoolSetting("Themed Color", "Use the theme color", true);
    BoolSetting mHighlightUsefulItems = BoolSetting("Highlight Useful Items", "Toggle highlighting of useful items", true);
    BoolSetting mShowEnchant = BoolSetting("Show Enchant", "Show the enchantments of the items", true);

    DividerSetting dText = DividerSetting("- Text -", "Settings for text");
    BoolSetting mShowNames = BoolSetting("Show Names", "Show the item names", true);
    BoolSetting mDistanceScaledFont = BoolSetting("Distance Scaled Font", "Scale the font based on distance", true);
    NumberSetting mFontSize = NumberSetting("Font Size", "The size of the font", 20, 1, 40, 0.1);;
    NumberSetting mScalingMultiplier = NumberSetting("Scaling Multiplier", "The multiplier to use for scaling the font", 1.25f, 0.f, 5.f, 0.25f);



    ItemESP () : ModuleBase("ItemESP", "Draws a box around items", ModuleCategory::Visual, 0, false) {
        addSettings(
            &dVisual,
            &mDistanceLimited,
            &mDistance,
            &mRenderFilled,
            &mThemedColor,
            &mHighlightUsefulItems,
            &mShowEnchant,

            &dText,
            &mShowNames,
            &mDistanceScaledFont,
            &mFontSize,
            &mScalingMultiplier
        );

        VISIBILITY_CONDITION(mDistance, mDistanceLimited.mValue);
        VISIBILITY_CONDITION(mFontSize, mShowNames.mValue);
        VISIBILITY_CONDITION(mDistanceScaledFont, mShowNames.mValue);
        VISIBILITY_CONDITION(mFontSize, mShowNames.mValue && !mDistanceScaledFont.mValue);
        VISIBILITY_CONDITION(mScalingMultiplier, mShowNames.mValue && mDistanceScaledFont.mValue);

        mNames = {
            {Lowercase, "itemesp"},
            {LowercaseSpaced, "item esp"},
            {Normal, "ItemESP"},
            {NormalSpaced, "Item ESP"}
        };
    }

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onRenderEvent(class RenderEvent& event);
};