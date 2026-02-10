#pragma once
#include <Features/Modules/Module.hpp>

class Nuker : public ModuleBase<Nuker> {
public:
    enum class BlockType {
        All,
        Specified,
        Bed
    };

    DividerSetting dMining = DividerSetting("- Mining -", "Settings for mining blocks");
    BoolSetting mSwing = BoolSetting("Swing", "Swings when destroying blocks", true);
    BoolSetting mHotbarOnly = BoolSetting("Hotbar Only", "Only switch to tools in the hotbar", true);
    EnumSettingT<BlockType> mBlockType = EnumSettingT<BlockType>("BlockType", "The type of block to destroy", BlockType::All, "All", "Specified", "Bed");
    NumberSetting mRange = NumberSetting("Range", "The max range for destroying blocks", 5, 0, 10, 0.25);
    NumberSetting mDestroySpeed = NumberSetting("Destroy Speed", "The destroy speed for Nuker", 1, 0.05, 1, 0.05);
    BoolSetting mInstaBreak = BoolSetting("Instant Break", "Executes StartDestroy and DestroyBlock in the same tick", false);
    BoolSetting mInfiniteDurability = BoolSetting("Infinite Durability", "Infinite durability for tools (may cause issues!)", false);
    BoolSetting mTest = BoolSetting("Test", "test", false);

    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for visuals");
    BoolSetting mRenderBlock = BoolSetting("Render Block", "Renders the block you are currently breaking", true);

    Nuker() : ModuleBase("Nuker", "Automatically breaks blocks", ModuleCategory::Player, 0, false) {
        addSettings
        (
            &dMining,
            &mSwing,
            &mHotbarOnly,
            &mBlockType,
            &mRange,
            &mDestroySpeed,
            &mInstaBreak,
            &mInfiniteDurability,
            //&mTest,

            &dVisual,
            &mRenderBlock
        );

        mNames = {
            {Lowercase, "nuker"},
            {LowercaseSpaced, "nuker"},
            {Normal, "Nuker"},
            {NormalSpaced, "Nuker"}
        };
    }

    glm::ivec3 mCurrentBlockPos = { 0, 0, 0 };
    int mCurrentBlockFace = -1;
    float mBreakingProgress = 0.f;
    bool mIsMiningBlock = false;
    bool mShouldRotate = false;
    bool mShouldSpoofSlot = false;
    bool mShouldSetbackSlot = false;
    int mPreviousSlot = -1;
    int mToolSlot = -1;

    uint64_t mLastBlockPlace = 0;
    int mLastPlacedBlockSlot = 0;

    std::string specifiedBlockID = "minecraft:stone";

    std::vector<glm::ivec3> offsetList = {
        glm::ivec3(0, -1, 0),
        glm::ivec3(0, 1, 0),
        glm::ivec3(0, 0, -1),
        glm::ivec3(0, 0, 1),
        glm::ivec3(-1, 0, 0),
        glm::ivec3(1, 0, 0),
    };

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onRenderEvent(class RenderEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
    void reset();
    void queueBlock(glm::ivec3 blockPos);
    bool isValidBlock(glm::ivec3 blockPos);

    std::string getSettingDisplay() override {
        return mBlockType.mValues[mBlockType.as<int>()];
    }

};