#pragma once
#include <Features/Modules/Module.hpp>

class Stripper : public ModuleBase<Stripper>
{
public:
    enum class SwitchMode {
        None,
        Full,
        Spoof
    };

    NumberSetting mRange = NumberSetting("Range", "The range at which to strip blocks", 5, 0, 10, 0.25);
    NumberSetting mBlocksPerTick = NumberSetting("Blocks Per Tick", "The amount of blocks to strip per tick", 1, 1, 10, 1);
    EnumSettingT<SwitchMode> mSwitchMode = EnumSettingT<SwitchMode>("Switch Mode", "The mode for switching to axes", SwitchMode::Full, "None", "Full", "Spoof");
    BoolSetting mHotbarOnly = BoolSetting("Hotbar Only", "Only use axes from the hotbar", false);

    Stripper() : ModuleBase("Stripper", "Automatically strips logs", ModuleCategory::Player, 0, false)
    {
        addSettings(
            &mRange,
            &mBlocksPerTick,
            &mSwitchMode,
            &mHotbarOnly
        );

        VISIBILITY_CONDITION(mHotbarOnly, mSwitchMode.mValue != SwitchMode::None);

        mNames = {
            {Lowercase, "stripper"},
            {LowercaseSpaced, "stripper"},
            {Normal, "Stripper"},
            {NormalSpaced, "Stripper"}
        };
    }

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void stripBlock(const glm::ivec3& blockPos, int side);
    bool tickStrip(class BaseTickEvent& event);

    int mLastSlot = -1;
    int mBlocksStripped = 0;
    int mLastPktSlot = -1;
};