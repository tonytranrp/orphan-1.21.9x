#pragma once

#include <Features/Modules/Module.hpp>

class PacketMine : public ModuleBase<PacketMine> {
public:
    NumberSetting mDestroySpeed = NumberSetting("Destroy Speed", "The destroy speed for packet mine", 0.67, 0.05, 1, 0.01);
    NumberSetting mPercentRot = NumberSetting("Rotation Percentage", "How broken a block needs to be for the player to rotate", 0.5, 0, 1, 0.05);
    BoolSetting mAutoTool = BoolSetting("Auto Tool", "Automatically switches to the best tool", true);
    BoolSetting mSpoof = BoolSetting("Spoof", "Spoof the tool switch", true);

    PacketMine() : ModuleBase("PacketMine", "Mines blocks using packets for better performance", ModuleCategory::Misc, 0, false) {
        addSettings(
            &mDestroySpeed,
            &mPercentRot,
            &mAutoTool,
            &mSpoof
        );

        VISIBILITY_CONDITION(mSpoof, mAutoTool.mValue);

        mNames = {
            {Lowercase, "packetmine"},
            {LowercaseSpaced, "packet mine"},
            {Normal, "PacketMine"},
            {NormalSpaced, "Packet Mine"}
        };
    }

    void onEnable() override;
    void onDisable() override;
    void onPacketOutEvent(class PacketOutEvent& event);
    void onBaseTickEvent(class BaseTickEvent& event);

private:
    glm::ivec3 mBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    int mBlockFace = 0;
    bool mShouldDestroy = false;
    int mToolSlot = -1;
    uint64_t mStartTime = 0;
    bool mStartDestroying = false;
    float mBreakingProgress = 0.f;

    bool findBestTool(Block* block);
    void spoofSlot(int slot);
}; 