#pragma once
//
// Created by vastrakai on 7/5/2024.
//

#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/Inventory/ContainerManagerModel.hpp>

class ChestStealer : public ModuleBase<ChestStealer> {
public:
    enum class Mode
    {
        Normal,
        Silent
    };
    EnumSettingT<Mode> mMode = EnumSettingT("Mode", "The mode of the chest stealer", Mode::Normal, "Normal", "Silent");
    BoolSetting mIgnoreUseless = BoolSetting("Ignore Useless", "Whether or not to ignore useless items", true);
    DividerSetting dDelay = DividerSetting("- Delay -", "Settings for delay");
    NumberSetting mDelay = NumberSetting("Delay", "The delay between stealing items (in milliseconds)", 50, 0, 500, 1);
    BoolSetting mRandomizeDelay = BoolSetting("Randomize Delay", "Randomizes the delay between stealing items", false);
    NumberSetting mRandomizeMin = NumberSetting("Randomize Min", "The minimum delay to randomize", 50, 0, 500, 1);
    NumberSetting mRandomizeMax = NumberSetting("Randomize Max", "The maximum delay to randomize", 100, 0, 500, 1);


    DividerSetting dChestAura = DividerSetting("- Chest Aura -", "Settings for chest aura");
    BoolSetting mChestAura = BoolSetting("Chest Aura", "Opens nearby chests automatically", false);
    NumberSetting mRange = NumberSetting("Range", "Max distance for chest opening. (For best performance, use 4)", 4, 0, 15, 1);
    NumberSetting mAuraDelay = NumberSetting("Chest Aura Delay", "Time between opening chests (ms)", 100, 0, 150, 1);

    ChestStealer() : ModuleBase<ChestStealer>("ChestStealer", "Steals items from chests", ModuleCategory::Player, 0, false)
    {
        addSettings(
            &mMode,
            &mIgnoreUseless,

            &dDelay,
            &mDelay,
            &mRandomizeDelay,
            &mRandomizeMin,
            &mRandomizeMax,

            &mChestAura,
            &mRange,
            &mAuraDelay
        );

        VISIBILITY_CONDITION(mRange, mChestAura.mValue);
        VISIBILITY_CONDITION(mAuraDelay, mChestAura.mValue);
        VISIBILITY_CONDITION(mRandomizeMin, mRandomizeDelay.mValue);
        VISIBILITY_CONDITION(mRandomizeMax, mRandomizeDelay.mValue);
        VISIBILITY_CONDITION(mDelay, mRandomizeDelay.mValue == false);

        mNames = {
            {Lowercase, "cheststealer"},
            {LowercaseSpaced, "chest stealer"},
            {Normal, "ChestStealer"},
            {NormalSpaced, "Chest Stealer"}
        };
    }

    bool mIsStealing = false;
    uint64_t mLastItemTaken = 0;
    bool mIsChestOpen = false;
    std::vector<ItemStack> mItemsToTake = {};
    bool mTotalDirty = false;
    int mTotalItems = 0;
    int mRemainingItems = 0;
    glm::vec3 mLastPos = glm::vec3(0);
    glm::vec3 mHighlightedPos = glm::vec3(0);
    ContainerID mCurrentContainerId = ContainerID::None;
    uint64_t mLastOpen = 0;

    void onContainerScreenTickEvent(class ContainerScreenTickEvent& event) const;
    void reset();
    void onEnable() override;
    void onDisable() override;
    void takeItem(int slot, ItemStack& item);
    void onBaseTickEvent(class BaseTickEvent& event);
    bool doDelay();
    void onRenderEvent(class RenderEvent& event);
    void onPacketInEvent(class PacketInEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
    uint64_t getDelay() const;

    bool mIsChestOpened = false;
    uint64_t mTimeOfLastChestOpen;
    std::vector<glm::vec3> mOpenedChestPositions;

    std::string getSettingDisplay() override {
        if (mRandomizeDelay.mValue)
        {
            return std::to_string(static_cast<int>(mRandomizeMin.mValue)) + " " + std::to_string(static_cast<int>(mRandomizeMax.mValue));
        }
        else
        {
            return std::to_string(static_cast<int>(mDelay.mValue)) + "";
        }
    }
};
