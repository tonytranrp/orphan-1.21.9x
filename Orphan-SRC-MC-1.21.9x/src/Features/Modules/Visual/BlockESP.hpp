#pragma once
//
// Created by vastrakai on 7/7/2024.
//

#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/World/Chunk/ChunkSource.hpp>
#include <chrono>


class BlockESP : public ModuleBase<BlockESP>
{
public:
    enum class BlockRenderMode {
        Filled,
        Outline,
        Both
    };

    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for visuals");
    EnumSettingT<BlockRenderMode> mRenderMode = EnumSettingT("Render Mode", "The mode to render block", BlockRenderMode::Outline, "Filled", "Outline", "Both");
    NumberSetting mOpacity = NumberSetting("Opacity", "How opaque the box is", 0.25f, 0.1f, 0.5f, 0.05f);
    BoolSetting mEnableFade = BoolSetting("Enable Fade", "Enable fade effect for blocks", false);
    NumberSetting mFadeRange = NumberSetting("Fade Range", "The range at which blocks start fading", 20.f, 1.f, 100.f, 1.f);
    NumberSetting mFullRenderDistance = NumberSetting("Full Render Distance", "The distance within which blocks are fully visible", 10.f, 1.f, 100.f, 1.f);
    BoolSetting mRenderCurrentChunk = BoolSetting("Render Current Chunk", "Renders the current chunk", false);
    BoolSetting mOnlyExposedOres = BoolSetting("Only Exposed", "Show only air-exposed ores", false);

    DividerSetting dScanning = DividerSetting("- Scanning -", "Settings for block detection");
    NumberSetting mRadius = NumberSetting("Radius", "The radius of the block esp", 20.f, 1.f, 100.f, 1.f);
    NumberSetting mChunkRadius = NumberSetting("Chunk Radius", "Maximum chunk radius for block search", 4.f, 1.f, 32.f, 1.f);
    NumberSetting mBottomY = NumberSetting("Bottom Y Limit", "Lower Y-axis detection limit", 100.f, -3.f, 100.f, 1.f);
    NumberSetting mUpdateFrequency = NumberSetting("Update Frequency", "Block update frequency in ticks", 1.f, 1.f, 40.f, 0.01f);
    NumberSetting mChunkUpdatesPerTick = NumberSetting("Updates Per Tick", "Subchunks processed per tick", 5.f, 1.f, 24.f, 1.f);

    DividerSetting dOres = DividerSetting("- Ores -", "Settings for ore detection");
    BoolSetting mChests = BoolSetting("Chests", "Show chests", true);
    BoolSetting mCoal = BoolSetting("Coal", "Show coal ore", false);
    BoolSetting mDiamond = BoolSetting("Diamond", "Show diamond ore", true);
    BoolSetting mEmerald = BoolSetting("Emerald", "Show emerald ore", true);
    BoolSetting mGold = BoolSetting("Gold", "Show gold ore", false);
    BoolSetting mIron = BoolSetting("Iron", "Show iron ore", false);
    BoolSetting mLapis = BoolSetting("Lapis", "Show lapis ore", false);
    BoolSetting mPortal = BoolSetting("Portal", "Show portal blocks", false);
    BoolSetting mRedstone = BoolSetting("Redstone", "Show redstone ore", true);

    BlockESP() : ModuleBase("BlockESP", "Draws a box around selected ores", ModuleCategory::Visual, 0, false) {

        addSettings(
            &dVisual,
            &mRenderMode,
            &mOpacity,
            &mEnableFade,
            &mFadeRange,
            &mFullRenderDistance,
            &mRenderCurrentChunk,
            &mOnlyExposedOres,

            &dScanning,
            &mRadius,
            &mChunkRadius,
            &mBottomY,
            &mUpdateFrequency,
            &mChunkUpdatesPerTick,

            &dOres,
            &mCoal,
            &mDiamond,
            &mEmerald,
            &mGold,
            &mIron,
            &mLapis,
            &mPortal,
            &mRedstone
        );
        VISIBILITY_CONDITION(mOpacity, mRenderMode.mValue != BlockRenderMode::Outline);
        VISIBILITY_CONDITION(mFadeRange, mEnableFade.mValue);
        VISIBILITY_CONDITION(mFullRenderDistance, mEnableFade.mValue);


        mNames = {
            {Lowercase, "oreesp"},
            {LowercaseSpaced, "ore esp"},
            {Normal, "OreESP"},
            {NormalSpaced, "Ore ESP"}
        };
    }

    ChunkPos mSearchCenter;
    ChunkPos mCurrentChunkPos;
    int mSubChunkIndex = 0;
    int mDirectionIndex = 0;
    int mSteps = 1;
    int mStepsCount = 0;
    int64_t mSearchStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    bool mChestESP;

    struct FoundBlock
    {
        const Block* block;
        AABB aabb;
        ImColor color;
        uint64_t discoveryTime;  // When the block was discovered
        glm::vec3 playerPosAtDiscovery; // Player position when block was discovered
    };

    std::unordered_map<BlockPos, FoundBlock> mFoundBlocks = {};

    void moveToNext();
    void tryProcessSub(bool& processed, ChunkPos currentChunkPos, int subChunkIndex);
    bool processSub(ChunkPos processChunk, int subChunk);
    static void reset();

    void onEnable() override;
    void onDisable() override;
    std::vector<int> getEnabledBlocks();
    void onBlockChangedEvent(class BlockChangedEvent& event);
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketInEvent(class PacketInEvent& event);
    void onRenderEvent(class RenderEvent& event);
};