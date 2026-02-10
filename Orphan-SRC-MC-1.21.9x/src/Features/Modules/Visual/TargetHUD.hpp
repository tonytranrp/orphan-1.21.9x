#pragma once
#include <SDK/Minecraft/Actor/EntityId.hpp>

#include "HudEditor.hpp"

class TargetHUD : public ModuleBase<TargetHUD> {
public:
    enum class Style {
        Orphan,
        EDest
    };

    enum class ParticleShape {
        Circle,
        Triangle,
        Square,
        Mixed
    };

    EnumSettingT<Style> mStyle = EnumSettingT("Style", "The style of the target HUD", Style::Orphan, "Orphan", "EDest");
    EnumSettingT<ParticleShape> mParticleShape = EnumSettingT("Particle Shape", "The shape of the particles", ParticleShape::Circle, "Circle", "Triangle", "Square", "Mixed");
    BoolSetting mDynamicMode = BoolSetting("Dynamic Mode", "Position the HUD next to the target in 3D space", false);
    NumberSetting mFontSize = NumberSetting("TargetHud Size", "The size of the font", 20, 1, 40, 1);
    BoolSetting mHealthCalculation = BoolSetting("Health Calculation", "Calculate health", true);
    BoolSetting mShowParticles = BoolSetting("Show Particles", "Show damage particles", true);

    TargetHUD();

    struct TargetTextureHolder {
        ID3D11ShaderResourceView* texture = nullptr;
        bool loaded = false;
        EntityId associatedEntity = EntityId();
    };

    float mHealth = 0;
    float mMaxHealth = 0;
    float mLastHealth = 0;
    float mLastMaxHealth = 0;
    float mAbsorption = 0;
    float mMaxAbsorption = 0;
    float mLastAbsorption = 0;
    float mLastMaxAbsorption = 0;
    float mLerpedHealth = 0;
    float mLerpedAbsorption = 0;
    std::string mLastPlayerName = "";
    float mLastHurtTime = 0;
    float mHurtTime = 0;
    Actor* mLastTarget = nullptr;
    std::map<Actor*, TargetTextureHolder> mTargetTextures;
    constexpr static uint64_t cHurtTimeDuration = 500;

    struct HealthInfo {
        float health = 20;
        float lastAbsorption = 0;
        float damage = 1;
    };
    std::map<std::string, HealthInfo> mHealths;
    uint64_t mLastHealTime = 0;

    std::unique_ptr<HudElement> mElement;

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void validateTextures();
    void calculateHealths();
    ID3D11ShaderResourceView* getActorSkinTex(Actor* actor);
    void onRenderEvent(class RenderEvent& event);
    void onPacketInEvent(class PacketInEvent& event);

    struct Particle {
        ImVec2 pos;
        ImVec2 velocity;
        float life;
        float maxLife;
        ImColor color;
        bool movingInward;
        float scale;
        float rotation;
        float rotationSpeed;
        ParticleShape shape;
    };

private:
    std::vector<Particle> mParticles;
    float mLastParticleSpawn = 0.0f;
    const float PARTICLE_SPAWN_RATE = 0.05f;
    const int MAX_PARTICLES = 20;
    const float PARTICLE_LIFE = 0.8f;
    const float PARTICLE_SPEED = 400.0f;
    const float PARTICLE_SPEED_VARIANCE = 150.0f;
    const int PARTICLES_PER_BURST = 4;
    const float INWARD_PARTICLE_CHANCE = 0.3f;
    const float MAX_ROTATION_SPEED = 8.0f;
    const float PARTICLE_SPREAD = 0.3f;

    float mVisibilityAnimation = 0.0f;
    ImVec2 mTargetPosition = ImVec2(0, 0);
    ImVec2 mCurrentPosition = ImVec2(0, 0);
    const float ANIMATION_SPEED = 8.0f;
    const float POSITION_LERP_SPEED = 12.0f;

    std::string mCurrentStatus = "Winning";
    std::string mTargetStatus = "Winning";
    float mStatusTransition = 1.0f;

    void updateParticles(float deltaTime, const ImVec2& sourcePos);
    void spawnParticle(const ImVec2& sourcePos);
    void updateAnimation(float deltaTime, bool visible, const ImVec2& targetPos);
};