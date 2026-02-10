#pragma once
//
// 7/31/2024.
//

#include <Features/Modules/Module.hpp>



class Phase : public ModuleBase<Phase> {
public:
    enum class Mode {
        Horizontal,
        Vertical,
        Clip
    };
    EnumSettingT<Mode> mMode = EnumSettingT<Mode>("Mode", "The mode of the module", Mode::Horizontal, "Horizontal", "Vertical", "Clip");
    DividerSetting dHorizontal = DividerSetting("- Horizontal -", "Settings for horizontal phase");
    BoolSetting mBlink = BoolSetting("Blink", "Blink when phasing", false);

    DividerSetting dVertical = DividerSetting("- Vertical -", "Settings for vertical phase");
    NumberSetting mSpeed = NumberSetting("Speed", "The speed of the vertical phase", 1.f, 0.f, 10.f, 0.1f);

    DividerSetting dClip = DividerSetting("- Clip -", "Settings for clip phase");
    BoolSetting mTest = BoolSetting("Test", "test", false);
    NumberSetting mDepth = NumberSetting("Depth", "How deep to phase into the ground", 1.f, 0.f, 2.f, 0.01f);

    Phase() : ModuleBase("Phase", "Allows you to phase through blocks", ModuleCategory::Movement, 0, false) {
        addSettings
        (
            &mMode,

            &dHorizontal,
            &mBlink,

            &dVertical,
            &mSpeed,

            &dClip,
            &mTest,
            &mDepth

        );

        VISIBILITY_CONDITION(dHorizontal, mMode.mValue == Mode::Horizontal);
        VISIBILITY_CONDITION(mBlink, mMode.mValue == Mode::Horizontal);

        VISIBILITY_CONDITION(dVertical, mMode.mValue == Mode::Vertical);
        VISIBILITY_CONDITION(mSpeed, mMode.mValue == Mode::Vertical);

        VISIBILITY_CONDITION(dClip, mMode.mValue == Mode::Clip);
        VISIBILITY_CONDITION(mTest, mMode.mValue == Mode::Clip);
        VISIBILITY_CONDITION(mDepth, mMode.mValue == Mode::Clip);


        mNames = {
            {Lowercase, "phase"},
            {LowercaseSpaced, "phase"},
            {Normal, "Phase"},
            {NormalSpaced, "Phase"}
        };
    }

    bool mMoving = false;
    bool mClip = false;

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
    void onRunUpdateCycleEvent(class RunUpdateCycleEvent& event);

    std::string getSettingDisplay() override {
        return mMode.mValues[mMode.as<int>()];
    }
};