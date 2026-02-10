#pragma once

//
// Created by alteik on 15/10/2024.
//

class Zoom : public ModuleBase<Zoom> {
public:

    DividerSetting dZoom = DividerSetting("- Zoom -", "Settings for zoom");
    BoolSetting mSmooth = BoolSetting("Smooth Zoom", "Whether or not to zoom smooth", true);
    NumberSetting mZoomValue = NumberSetting("Zoom Value", "How much to zoom in", 60.f, 10.f, 120.f, 1.f);

    DividerSetting dScroll = DividerSetting("- Scroll -", "Settings for scrolling");
    BoolSetting mScroll = BoolSetting("Scroll", "Zoom in/out by scrolling", true);
    NumberSetting mScrollIncrement = NumberSetting("Scroll Increment", "The amount to zoom in/out", 8.0f, 0.1f, 10.0f, 0.1f);


    Zoom() : ModuleBase<Zoom>("Zoom", "Decreases your field of view", ModuleCategory::Visual, 0, false) {

        addSetting(&dZoom);
        addSetting(&mSmooth);
        addSetting(&mZoomValue);

        addSetting(&dScroll);
        addSetting(&mScroll);
        addSetting(&mScrollIncrement);


        VISIBILITY_CONDITION(mScrollIncrement, mScroll.mValue);

        mNames = {
            {Lowercase, "zoom"},
            {LowercaseSpaced, "zoom"},
            {Normal, "Zoom"},
            {NormalSpaced, "Zoom"}
        };

        mEnableWhileHeld = true;
    }

    float mPastFov = 0;
    float mCurrentValue = 0;
    float mStartValue = 0;

    void onEnable() override;
    bool mZoomingIn = false; // Flag to indicate zooming in
    bool misEnabled = false;
    void onDisable() override;
    void onMouseEvent(class MouseEvent& event);
    void onRenderEvent(class RenderEvent& event);
};
