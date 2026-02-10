#pragma once
//
// Created by vastrakai on 8/3/2024.
// Edited by player5 (2/9/2025)
//


class Step : public ModuleBase<Step>
{
public:
    enum class Mode {
        Vanilla
    };

    EnumSettingT<Mode> mMode = EnumSettingT<Mode>("Mode", "The style of which step height is applied.", Mode::Vanilla, "Vanilla");

    DividerSetting dHeight = DividerSetting("- Height -", "Settings for height");
    NumberSetting mStepHeight = NumberSetting("Step Height", "The max height to step up blocks", 1.f, 0.0f, 5.f, 0.1f);

    DividerSetting dReverseStep = DividerSetting("- Reverse Step -", "Settings for reverse step");
    BoolSetting mReverseStep = BoolSetting("Reverse Step", "Step down blocks", false);
    NumberSetting mMaxFallDistance = NumberSetting("Max Fall Distance", "Max fall distance to step", 20, 0, 30, 1);
    BoolSetting mDontUseIfSpeed = BoolSetting("Disable With Speed", "Avoid using reverse step while the speed module is enabled", true);
    BoolSetting mVoidCheck = BoolSetting("Void Check", "Avoid stepping into the void", true);

    Step() : ModuleBase<Step>("Step", "Automatically steps up/down blocks", ModuleCategory::Movement, 0, false) {

        addSetting(&dHeight);
        addSetting(&mStepHeight);

        addSetting(&dReverseStep);
        addSetting(&mReverseStep);
        addSetting(&mMaxFallDistance);
        addSetting(&mDontUseIfSpeed);
        addSetting(&mVoidCheck);
        
        VISIBILITY_CONDITION(mMaxFallDistance, mReverseStep.mValue);
        VISIBILITY_CONDITION(mDontUseIfSpeed, mReverseStep.mValue);
        VISIBILITY_CONDITION(mVoidCheck, mReverseStep.mValue);

        mNames = {
            {Lowercase, "step"},
            {LowercaseSpaced, "step"},
            {Normal, "Step"},
            {NormalSpaced, "Step"}
        };
    }

    bool mJumped = false;
    bool mStepEnabled = true;
    bool canFallDown();
    bool isVoid();


    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
};