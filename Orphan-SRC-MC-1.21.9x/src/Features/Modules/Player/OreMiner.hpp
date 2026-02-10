#pragma once
#include <Features/Modules/Module.hpp>
#include <chrono>
#include <iostream>

class OreMiner : public ModuleBase<OreMiner> {
public:
public:

    enum class Mode {
        Hive,
    };
    enum class CalcMode {
        Minecraft
    };
    enum class OrePriority {
        High,
        Medium,
        Low
    };
    enum class UncoverMode {
        Normal
    };
    enum class OreSelectionMode {
        Normal,
        Closest
    };
    enum class ProgressBarStyle
    {
        Old,
        New
    };

    EnumSettingT<Mode> mMode = EnumSettingT<Mode>("Mode", "The mining mode", Mode::Hive, "Hive");
    EnumSettingT<CalcMode> mCalcMode = EnumSettingT<CalcMode>("Calc Mode", "The calculation mode destroy speed", CalcMode::Minecraft, "Minecraft");

    DividerSetting dOres = DividerSetting("- Ores -", "Select ores to be mined");
    BoolSetting mCoal = BoolSetting("Coal", "Destroy coal ore", false);
    BoolSetting mDiamond = BoolSetting("Diamond", "Destroy diamond ore", false);
    BoolSetting mEmerald = BoolSetting("Emerald", "Destroy emerald ore", false);
    BoolSetting mGold = BoolSetting("Gold", "Destroy gold ore", false);
    BoolSetting mIron = BoolSetting("Iron", "Destroy iron ore", false);
    BoolSetting mLapis = BoolSetting("Lapis", "Destroy lapis ore", false);
    BoolSetting mRedstone = BoolSetting("Redstone", "Destroy redstone ore", false);

    DividerSetting dPriority = DividerSetting("- Priority -", "Settings for ore priority");
    EnumSettingT<OrePriority> mCoalPriority = EnumSettingT<OrePriority>("Coal Priority", "The priority for coal ore", OrePriority::Medium, "High", "Medium", "Low");
    EnumSettingT<OrePriority> mDiamondPriority = EnumSettingT<OrePriority>("Diamond Priority", "The priority for diamond ore", OrePriority::Medium, "High", "Medium", "Low");
    EnumSettingT<OrePriority> mEmeraldPriority = EnumSettingT<OrePriority>("Emerald Priority", "The priority for emerald ore", OrePriority::Medium, "High", "Medium", "Low");
    EnumSettingT<OrePriority> mGoldPriority = EnumSettingT<OrePriority>("Gold Priority", "The priority for gold ore", OrePriority::Medium, "High", "Medium", "Low");
    EnumSettingT<OrePriority> mIronPriority = EnumSettingT<OrePriority>("Iron Priority", "The priority for iron ore", OrePriority::Medium, "High", "Medium", "Low");
    EnumSettingT<OrePriority> mLapisPriority = EnumSettingT<OrePriority>("Lapis Priority", "The priority for lapis ore", OrePriority::Medium, "High", "Medium", "Low");
    EnumSettingT<OrePriority> mRedstonePriority = EnumSettingT<OrePriority>("Redstone Priority", "The priority for redstone ore", OrePriority::Medium, "High", "Medium", "Low");

    DividerSetting dMining = DividerSetting("- Mining -", "Settings for mining blocks");
    NumberSetting mDestroySpeed = NumberSetting("Destroy Time", "The time it takes to destroy for OreMiner", 1, 0.01, 1, 0.01);
    NumberSetting mOtherDestroySpeed = NumberSetting("Other Destroy Speed", "The other destroy speed for OreMiner", 1, 0.01, 1, 0.01);
    NumberSetting mRange = NumberSetting("Range", "The max range for destroying blocks", 5, 0, 10, 0.01);
    NumberSetting mUncoverRange = NumberSetting("Uncover Range", "The max range for uncovering blocks", 3, 1, 8, 1);
    BoolSetting mDynamicDestroySpeed = BoolSetting("Dynamic Destroy Speed", "use faster destroy speed to specified block", false);
    BoolSetting mSwing = BoolSetting("Swing", "Swings when destroying blocks", true);
    BoolSetting mHotbarOnly = BoolSetting("Hotbar Only", "Only switch to tools in the hotbar", true);
    BoolSetting mInfiniteDurability = BoolSetting("Infinite Durability", "Infinite durability for tools (may cause issues!)", true);
    BoolSetting mUncover = BoolSetting("Uncover", "Uncover ores if nothing around you is already exposed", true);
    BoolSetting mOnGroundOnly = BoolSetting("OnGround Only", "use dynamic destroy speed only on ground", false);
    BoolSetting mNuke = BoolSetting("Nuke", "destroy block instantly", false);
    BoolSetting mAvoidEnemyOre = BoolSetting("Avoid Enemy Ore", "Avoid ore that enemy is targetting", false);
    BoolSetting mAlwaysMine = BoolSetting("Always mine", "Keep mining ore", true);

    DividerSetting dVisual = DividerSetting("- Visual -", "Visual settings");
    BoolSetting mRenderProgressBar = BoolSetting("Render Progress Bar", "Renders the progress bar", true);
    EnumSettingT<ProgressBarStyle> mProgressBarStyle = EnumSettingT<ProgressBarStyle>("Progress Bar Style", "The render progress mode", ProgressBarStyle::New, "Old", "New");
    NumberSetting mOffset = NumberSetting("Offset From Center", "render pos offset from center", 20, 0, 200, 0.1);
    BoolSetting mRenderBlock = BoolSetting("Render Block", "Renders the block you are currently breaking", true);

    OreMiner() : ModuleBase("OreMiner", "Automatically breaks ores", ModuleCategory::Player, 0, false) {

        VISIBILITY_CONDITION(dPriority, mEmerald.mValue || mDiamond.mValue || mGold.mValue || mIron.mValue || mCoal.mValue || mRedstone.mValue || mLapis.mValue)
        VISIBILITY_CONDITION(mCoalPriority, mCoal.mValue);
        VISIBILITY_CONDITION(mDiamondPriority, mDiamond.mValue);
        VISIBILITY_CONDITION(mEmeraldPriority, mEmerald.mValue);
        VISIBILITY_CONDITION(mGoldPriority, mGold.mValue);
        VISIBILITY_CONDITION(mIronPriority, mIron.mValue);
        VISIBILITY_CONDITION(mLapisPriority, mLapis.mValue);
        VISIBILITY_CONDITION(mRedstonePriority, mRedstone.mValue);

        addSettings(
            &dOres,
            &mCoal,
            &mDiamond,
            &mEmerald,
            &mGold,
            &mIron,
            &mLapis,
            &mRedstone,

            &dPriority,
            &mCoalPriority,
            &mDiamondPriority,
            &mEmeraldPriority,
            &mGoldPriority,
            &mIronPriority,
            &mLapisPriority,
            &mRedstonePriority,

            &dMining,
            &mDestroySpeed,
            &mOtherDestroySpeed,
            &mRange,
            &mUncoverRange,
            &mSwing,
            &mHotbarOnly,
            &mInfiniteDurability,
            &mUncover,
            &mDynamicDestroySpeed,
            &mOnGroundOnly,
            &mNuke,
            &mAvoidEnemyOre,

            &dVisual,
            &mRenderProgressBar,
            &mProgressBarStyle,
            &mOffset,
            &mRenderBlock
        );

        VISIBILITY_CONDITION(mOnGroundOnly, mDynamicDestroySpeed.mValue);
        VISIBILITY_CONDITION(mNuke, mDynamicDestroySpeed.mValue && mOnGroundOnly.mValue);

        VISIBILITY_CONDITION(mOffset, mProgressBarStyle.mValue == ProgressBarStyle::New);
        mNames = {
            {Lowercase, "oreminer"},
            {LowercaseSpaced, "ore miner"},
            {Normal, "OreMiner"},
            {NormalSpaced, "Ore Miner"}
        };

        gFeatureManager->mDispatcher->listen<RenderEvent, &OreMiner::onRenderEvent, nes::event_priority::LAST>(this);
    }

    bool stealEnabled = false;

    uint64_t lastStoleTime = 0;
    bool antiStealerEnabled = false;
    bool stealerDetected = false;
    int amountOfStolenBlocks = 0;
    uint64_t stealerDetectionStartTime = 0;
    bool startedStealerDetection = false;
    uint64_t uncoverDisabledTime = 0;
    bool uncoverEnabled = true;
    uint64_t lastStealerDetected = 0;

    struct PathFindingResult {
        glm::ivec3 blockPos;
        float time;
    };

    static inline glm::ivec3 mCurrentBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    glm::ivec3 mTargettingBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    glm::ivec3 mLastTargettingBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    glm::ivec3 mEnemyTargettingBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    glm::ivec3 mLastEnemyLayerBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    bool mCanSteal = false;
    bool mIsStealing = false;
    bool mCurrentUncover = false;
    int mCurrentBlockFace = -1;
    float mBreakingProgress = 0.f;
    float mCurrentDestroySpeed = 1.f;
    static inline bool mIsMiningBlock = false;
    static inline bool mWasMiningBlock = false;
    bool mIsUncovering = false;
    bool mWasUncovering = false;
    float mLastTargettingBlockPosDestroySpeed = 1.f;
    int mLastToolSlot = 0;
    bool mIsConfuserActivated = false;
    glm::ivec3 mLastConfusedPos = { INT_MAX, INT_MAX, INT_MAX };
    bool mOffGround = false;

    bool mShouldRotate = false;
    bool mShouldRotateToPlacePos = false;
    bool mShouldSpoofSlot = false;
    bool mShouldSetbackSlot = false;
    glm::ivec3 mBlackListedOrePos = { INT_MAX, INT_MAX, INT_MAX };
    int mPreviousSlot = -1;
    int mToolSlot = -1;

    std::vector<glm::ivec3> miningRedstones;
    glm::ivec3 mCurrentPlacePos = { INT_MAX, INT_MAX, INT_MAX };

    uint64_t mLastBlockPlace = 0;
    uint64_t mLastStealerUpdate = 0;
    uint64_t mLastStealerDetected = 0;
    uint64_t mLastConfuse = 0;
    uint64_t mLastUncoverDetected = 0;
    uint64_t mLastReplaced = 0;
    int mLastPlacedBlockSlot = 0;

    // Replacer
    bool mCanReplace = false;
    int mStartDestroyCount = 0;
    glm::ivec3 mLastReplacedPos = { INT_MAX, INT_MAX, INT_MAX };

    uint64_t mPing = 100;
    uint64_t mEventDelay = 0;

    std::vector<glm::ivec3> mFakePositions;
    //std::vector<glm::ivec3> mLastUpdatedBlockPositions;
    std::vector<glm::ivec3> mLastBrokenOrePos;
    std::vector<glm::ivec3> mLastBrokenCoveringBlockPos;
    std::vector<glm::ivec3> mOreBlackList;

    std::vector<glm::ivec3> mOffsetList = {
        glm::ivec3(0, -1, 0),
        glm::ivec3(0, 1, 0),
        glm::ivec3(0, 0, -1),
        glm::ivec3(0, 0, 1),
        glm::ivec3(-1, 0, 0),
        glm::ivec3(1, 0, 0),
    };

    std::chrono::time_point<std::chrono::steady_clock> mGlitchTime = std::chrono::steady_clock::now();

    std::vector<int> mEmeraldIds = {
        129,
        662,
    };

    std::vector<int> mDiamondIds = {
        56,
        660,
    };

    std::vector<int> mGoldIds = {
        14,
        657,
    };

    std::vector<int> mIronIds = {
        15,
        656,
    };

    std::vector<int> mCoalIds = {
        16,
        661,
    };

    std::vector<int> mRedstoneIds = {
        73,
        74,
        658,
        659,
    };

    std::vector<int> mLapisIds = {
        21,
        655,
    };

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onRenderEvent(class RenderEvent& event);
    void renderProgressBar();
    void renderNewProgressBar();
    void renderBlock();
    void renderFakeOres();
    void reset();
    void onPacketOutEvent(class PacketOutEvent& event);
    void onPacketInEvent(class PacketInEvent& event);
    void onSendImmediateEvent(class SendImmediateEvent& event);
    void onPingUpdateEvent(class PingUpdateEvent& event);
    void initializeOreMiner();
    void resetSyncSpeed();
    void queueBlock(glm::ivec3 blockPos);
    bool isValidBlock(glm::ivec3 blockPos, bool oreOnly, bool exposedOnly, bool isStealing = false, bool usePriority = false, OrePriority priority = OrePriority::Medium);
    bool isValidRedstone(glm::ivec3 blockPos);
    bool isOre(std::vector<int> Ids, int id);
    PathFindingResult getBestPathToBlock(glm::ivec3 blockPos);

    std::string getSettingDisplay() override {
        return mMode.mValues[mMode.as<int>()];
    }
};
