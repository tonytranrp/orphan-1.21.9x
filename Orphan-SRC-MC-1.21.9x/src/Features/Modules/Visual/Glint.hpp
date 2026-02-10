#pragma once
//
// Created by vastrakai on 10/19/2024.
// Edited by player5 (1/24/2025)
//


class Glint : public ModuleBase<Glint>
{
public:
    NumberSetting mSaturation = NumberSetting("Saturation", "The saturation of the glint", 1, 0, 1, 0.1f);

    Glint() : ModuleBase("Glint", "Makes the glint on items more visible", ModuleCategory::Visual, 0, false) {
        addSetting(&mSaturation);

        mNames = {
            {Lowercase, "glint"},
            {LowercaseSpaced, "glint"},
            {Normal, "Glint"},
            {NormalSpaced, "Glint"}
        };

    }

    void onEnable() override;
    void onDisable() override;
    void onRenderItemInHandDescriptionEvent(class RenderItemInHandDescriptionEvent& event);
};