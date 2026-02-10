#pragma once
//
// Created by vastrakai on 10/4/2024.
// Edited by player5 (1/24/2025)
//

class Tracers : public ModuleBase<Tracers> {
public:
    enum class CenterPoint {
        Top,
        Center,
        Bottom
    };

    EnumSettingT<CenterPoint> mCenterPoint = EnumSettingT<CenterPoint>("Center Point", "The point to center the line on", CenterPoint::Center, "Top", "Center", "Bottom");
    BoolSetting mRenderFilled = BoolSetting("Render Filled", "Render the ESP filled", true);
    BoolSetting mRenderLocal = BoolSetting("Render Local", "Render the ESP on yourself", false);
    BoolSetting mShowFriends = BoolSetting("Show Friends", "Render the ESP on friends", true);

    Tracers() : ModuleBase("Tracers", "Draws a line to every player", ModuleCategory::Visual, 0, false) {
        addSetting(&mCenterPoint);
        addSetting(&mRenderFilled);
        addSetting(&mRenderLocal);
        addSetting(&mShowFriends);

        mNames = {
            {Lowercase, "tracers"},
            {LowercaseSpaced, "tracers"},
            {Normal, "Tracers"},
            {NormalSpaced, "Tracers"}
        };
    }

    void onEnable() override;
    void onDisable() override;
    void onRenderEvent(class RenderEvent& event);
};