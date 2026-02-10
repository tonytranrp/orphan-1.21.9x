//
// Edited by player5 (1/24/2025)
//

#pragma once
#include <Features/Modules/Module.hpp>

class Regen : public ModuleBase<Regen>
{
public:
    enum class StealPriority {
        Mine,
        Steal
    };

    enum class ConfuserMode {
        None,
        Always,
        Auto
    };

    enum class ProgressBarStyle
    {
        Orphan,
        Aeolus
    };

    enum class ParticleShape {
        Circle,
        Triangle,
        Square,
        Mixed
    };

    struct Particle {
        ImVec2 pos;
        ImVec2 velocity;
        float life;
        float maxLife;
        ImColor color;
        float scale;
        float rotation;
        float rotationSpeed;
        ParticleShape shape;
        bool movingInward;
    };

    static constexpr int MAX_PARTICLES = 300;
    static constexpr int PARTICLES_PER_BURST = 40;
    static constexpr float PARTICLE_LIFE = 1.5f;
    static constexpr float PARTICLE_SPEED = 400.0f;
    static constexpr float PARTICLE_SPEED_VARIANCE = 150.0f;
    static constexpr float PARTICLE_SPREAD = 1.2f;
    static constexpr float PARTICLE_SPAWN_RATE = 0.02f;
    static constexpr float MAX_ROTATION_SPEED = 12.0f;
    static constexpr float PROGRESS_BAR_HEIGHT = 48.0f;
    static constexpr float SPAWN_AREA_PADDING = 5.0f;

    // Helper function to get random spawn position along the progress bar
    ImVec2 getRandomSpawnPosition(const ImVec2& barStart, const ImVec2& barSize) {
        float randomX = barStart.x + (static_cast<float>(rand()) / RAND_MAX) * barSize.x;
        float randomY = barStart.y + (static_cast<float>(rand()) / RAND_MAX) * barSize.y;
        return ImVec2(randomX, randomY);
    }

    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for visuals");
    BoolSetting mLerp = BoolSetting("Lerp Box", "Lerps rendered box from ore to ore", true);
    EnumSettingT<ProgressBarStyle> mProgressBarStyle = EnumSettingT<ProgressBarStyle>("Progress Bar Style", "The render progress mode", ProgressBarStyle::Orphan, "Orphan", "Aeolus");
    EnumSettingT<ParticleShape> mParticleShape = EnumSettingT<ParticleShape>("Particle Shape", "The shape of particles when mining completes", ParticleShape::Mixed, "Circle", "Triangle", "Square", "Mixed");
    BoolSetting mShowParticles = BoolSetting("Show Particles", "Enable or disable mining completion particles", true);
    BoolSetting mBackground = BoolSetting("Background", "Render a background for the text", true);
    BoolSetting mThemeColor = BoolSetting("Use Theme Color", "Use the Interface theme color", true);
    BoolSetting mAccurate = BoolSetting("Accurate Health", "Display 5 (hearts) instead of 10 (half-hearts)", false);
    ColorSetting mColor1 = ColorSetting("Color", "The color of the text and progress bar", 0xFFFFFFFF);

    DividerSetting dMining = DividerSetting("- Mining -", "Settings for mining blocks");
    BoolSetting mUncover = BoolSetting("Uncover", "Uncover redstone if nothing around you is already exposed", false);
    NumberSetting mPercentRot = NumberSetting("Rotation Percentage", "How broken a block needs to be for the player to rotate", 0.5, 0, 1, 0.05);
    BoolSetting mOnGround = BoolSetting("On Ground Check", "Pauses destroy progress when off ground", false);
    NumberSetting mDestroySpeed = NumberSetting("Destroy Speed", "The destroy speed for Regen (uses dynamic speed by default)", 0.67, 0.05, 1, 0.01);
    NumberSetting mUncoverAmount = NumberSetting("Uncover Amount", "The max Amount of blocks to break to uncover a ore", 2, 1, 5, 1);
    BoolSetting mQueueOres = BoolSetting("Queue Ores", "Queue ores for mining", true);
    BoolSetting mAlwaysMine = BoolSetting("Always mine", "Keep mining ore", false);
    EnumSettingT<ConfuserMode> mConfuserMode = EnumSettingT<ConfuserMode>("Confuser Mode", "The confuser mode", ConfuserMode::Auto, "None", "Always", "Auto");

    DividerSetting dSteal = DividerSetting("- Steal -", "Settings for stealing redstone");
    BoolSetting mSteal = BoolSetting("Steal", "Steal the enemy's ore", false);
    BoolSetting mAntiSteal = BoolSetting("Anti Steal", "Stop mining if enemy tried to steal ore", false);
    EnumSettingT<StealPriority> mStealPriority = EnumSettingT<StealPriority>("Steal Priority", "The steal priority mode", StealPriority::Mine, "Mine", "Steal");
    NumberSetting mStealerTimeout = NumberSetting("Steal Timeout", "The max duration for stealer", 1500, 500, 5000, 250);

    DividerSetting dDebug = DividerSetting("- Debug -", "Settings for debug");
    BoolSetting mDebug = BoolSetting("Debug", "Enable all debug notifications", false);
    BoolSetting mBlockNotify = BoolSetting("Block", "Send message in chat when you blocked ore/ore got covered", true);
    BoolSetting mStealNotify = BoolSetting("Steal", "Send message in chat when you steal ore / your ore was stolen", true);
    BoolSetting mStealerDetectorNotify = BoolSetting("StealerDetector", "Send message in chat when your ore is being stolen", true);

    Regen() : ModuleBase("Regen", "Automatically breaks redstone", ModuleCategory::Player, 0, false) {
        addSettings(
            &dVisual,
            &mLerp,
            &mProgressBarStyle,
            &mParticleShape,
            &mShowParticles,
            &mBackground,
            &mThemeColor,
            &mAccurate,
            &mColor1,

            &dMining,
            &mUncover,
            &mConfuserMode,
            &mPercentRot,
            &mDestroySpeed,
            &mUncoverAmount,
            &mOnGround,
            &mQueueOres,
            &mAlwaysMine,

            &dSteal,
            &mSteal,
            &mAntiSteal,
            &mStealPriority,
            &mStealerTimeout,

            &dDebug,
            &mDebug
        );

        VISIBILITY_CONDITION(mUncoverAmount, mUncover.mValue);
        VISIBILITY_CONDITION(mStealerTimeout, mSteal.mValue);
        VISIBILITY_CONDITION(mStealPriority, mSteal.mValue);

        VISIBILITY_CONDITION(mColor1, !mThemeColor.mValue);

        VISIBILITY_CONDITION(mAccurate, mProgressBarStyle.mValue == ProgressBarStyle::Aeolus);
        VISIBILITY_CONDITION(mThemeColor, mProgressBarStyle.mValue == ProgressBarStyle::Aeolus);
        VISIBILITY_CONDITION(mShowParticles, mProgressBarStyle.mValue == ProgressBarStyle::Aeolus);
        VISIBILITY_CONDITION(mParticleShape, mShowParticles.mValue);

        mNames = {
            {Lowercase, "regen"},
            {LowercaseSpaced, "regen"},
            {Normal, "Regen"},
            {NormalSpaced, "Regen"}
        };

        gFeatureManager->mDispatcher->listen<RenderEvent, &Regen::onRenderEvent, nes::event_priority::LAST>(this);
    }

    const float HARDCODED_RANGE = 8.25f;

    bool stealEnabled = false;
    uint64_t lastStoleTime = 0;
    bool antiStealerEnabled = false;
    bool stealerDetected = false;

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

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onRenderEvent(class RenderEvent& event);
    void renderAeolusProgressBar();
    void renderNewProgressBar();
    void renderBlock();
    void renderFakeOres();
    void onPacketOutEvent(class PacketOutEvent& event);
    void onPacketInEvent(class PacketInEvent& event);
    void onSendImmediateEvent(class SendImmediateEvent& event);
    void onPingUpdateEvent(class PingUpdateEvent& event);
    void initializeRegen();
    void resetSyncSpeed();
    void queueBlock(glm::ivec3 blockPos);
    bool isValidBlock(glm::ivec3 blockPos, bool redstoneOnly, bool exposedOnly, bool isStealing = false);
    bool isValidRedstone(glm::ivec3 blockPos);
    PathFindingResult getBestPathToBlock(glm::ivec3 blockPos);


    void DrawRotatedTriangle(ImDrawList* drawList, const ImVec2& center, float size, float rotation, const ImColor& color);
    void DrawRotatedSquare(ImDrawList* drawList, const ImVec2& center, float size, float rotation, const ImColor& color);
    void spawnParticle(const ImVec2& textPos);
    void updateParticles(float deltaTime, const ImVec2& sourcePos);

private:
    std::vector<Particle> mParticles;
    bool mLastMiningState = false;
    float mLastParticleSpawn = 0.0f;
    float anim = 0.0f;
};
