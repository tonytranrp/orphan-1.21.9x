#pragma once

#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Network/Packets/SetActorMotionPacket.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include "Utils/GameUtils/ChatUtils.hpp"
#include <Features/Events/RenderEvent.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>


class DamageEffect : public ModuleBase<DamageEffect> {
public:
    enum class Stile {
        theme,
        blood

    };
    NumberSetting mSize = NumberSetting("Size", "Size of particles", 5, 1, 20, 1);
    NumberSetting mparticlesamount = NumberSetting("amount", "Amount of particles", 20, 10, 100, 1);
    NumberSetting mLifeTime = NumberSetting("LifeTime", "LifeTime of particles", 500, 10, 1000, 1);

    DamageEffect() : ModuleBase<DamageEffect>("DamageEffect", "Visual String", ModuleCategory::Visual, 0, false) {
        addSetting(&mSize);
        addSetting(&mparticlesamount);
        addSetting(&mLifeTime);
        mNames = {
            {Lowercase, "damageeffect"},
            {LowercaseSpaced, "damage effect"},
            {Normal, "DamageEffect"},
            {NormalSpaced, "Damage Effect"}
        };
    }

    struct PlayerInfo { 
        std::string playerName;
        glm::vec3 HurtPosition;
        std::vector<glm::vec3> hurtPositions;
        PlayerInfo() : HurtPosition(0.0f) {}
    };

    std::unordered_map<Actor*, PlayerInfo> playerMap;
    float LifeTime = 0;

    void onEnable() override;
    void onDisable() override;
    glm::vec3 generateRandomDirection();
    void initializeParticles(Actor* actor, const glm::vec3& basePos, int particlesamount);
    void updateParticles(float deltaTime);
    void createDamageEffect(Actor* actor, int particlesamount);
    void onPacketInEvent(PacketInEvent& event);
    void onRenderEvent(RenderEvent& event); 
    //void drawTextAtPosition(ImDrawList* drawList, const glm::vec3& worldPos, const std::string& text, float fontSize, ImColor color);

};
