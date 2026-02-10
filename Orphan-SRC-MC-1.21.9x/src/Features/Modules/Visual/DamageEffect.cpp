#include "DamageEffect.hpp"

#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Network/Packets/SetActorMotionPacket.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Utils/GameUtils/ChatUtils.hpp>
#include <Features/FeatureManager.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <random>

// Structure for storing particle information
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;// Particle lifetime
};
// Map to store particle information for each player
std::unordered_map<Actor*, std::vector<Particle>> particleMap;

// Function to generate random directions
glm::vec3 DamageEffect::generateRandomDirection() {
    static std::default_random_engine engine;
    static std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    return glm::normalize(glm::vec3(distribution(engine), distribution(engine), distribution(engine)));
}

// Function to initialize particles
void DamageEffect::initializeParticles(Actor* actor, const glm::vec3& basePos ,int particlesamount) {
    std::vector<Particle>& particles = particleMap[actor];
  
    for (int i = 0; i < particlesamount; ++i) {
        float xoffset = MathUtils::randomFloat(-0.5, 0.5);
        float zoffset = MathUtils::randomFloat(-0.5, 0.5);
        float yoffset = MathUtils::randomFloat(-1, 1);
        Particle particle;
        particle.position = basePos + glm::vec3(xoffset, yoffset, zoffset);
        particle.velocity = generateRandomDirection() * 0.5f; // Random speed
        particle.life = LifeTime; 
        particles.push_back(particle);
    }
}

// Function to update particles
void DamageEffect::updateParticles(float deltaTime) {
    for (auto& [actor, particles] : particleMap) {
        for (auto& particle : particles) {
            particle.position += (particle.velocity * (particle.life/2000)) * deltaTime;
            particle.position.y -= 0.5f * deltaTime; // gravity
            particle.life -= deltaTime*10 ; // Reduce life time
        }
        // Delete particles whose Life Time has run out.
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p) { return p.life <= 0.0f; }), particles.end());
    }
}

// Function to generate particles
void DamageEffect::createDamageEffect(Actor* actor,int particlesamount) {
    if (actor && actor->getPos()) {
        initializeParticles(actor, *actor->getPos(), particlesamount);
        //ChatUtils::displayClientMessage("Damage");
    }
}

void DamageEffect::onPacketInEvent(PacketInEvent& event) {
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    if (event.mPacket->getId() == PacketID::SetActorMotion) {
        auto sem = event.getPacket<SetActorMotionPacket>();
        if (sem->mRuntimeID == player->getRuntimeID()) {
            // playDamageSound();
        }
    }
}

void DamageEffect::onEnable() {
    gFeatureManager->mDispatcher->listen<PacketInEvent, &DamageEffect::onPacketInEvent, nes::event_priority::LAST>(this);
    gFeatureManager->mDispatcher->listen<RenderEvent, &DamageEffect::onRenderEvent>(this);
}

void DamageEffect::onDisable() {
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &DamageEffect::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->deafen<RenderEvent, &DamageEffect::onRenderEvent>(this);
}

void DamageEffect::onRenderEvent(RenderEvent& event) {
    auto drawList = ImGui::GetBackgroundDrawList();
    LifeTime = mLifeTime.mValue;
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;
    if (!ClientInstance::get()->getLevelRenderer()) return;

    glm::vec3 playerPos = *player->getPos();
    float maxDistance = 10.0f;//Maximum distance to draw particles

    auto actors = ActorUtils::getActorList(true, true);
    std::unordered_set<Actor*> currentActors(actors.begin(), actors.end());

    // Delete old player information
    for (auto it = playerMap.begin(); it != playerMap.end();) {
        if (currentActors.find(it->first) == currentActors.end()) {
            it = playerMap.erase(it);
        }
        else {
            ++it;
        }
    }
    updateParticles(ImRenderUtils::getDeltaTime()); // Particle update
    

    for (auto actor : actors) {
        auto mobHurtTimeComp = actor->getMobHurtTimeComponent();
        if (!mobHurtTimeComp) continue;

        float mHurtTime = static_cast<float>(mobHurtTimeComp->mHurtTime);

        if (playerMap.find(actor) == playerMap.end()) {
            playerMap[actor] = PlayerInfo();
            playerMap[actor].playerName = actor->getRawName();
            if (mHurtTime == 0) playerMap[actor].HurtPosition = *actor->getPos();
        }

        auto& playerInfo = playerMap[actor];
        glm::vec3 currentPosition = *actor->getPos();

        if (mHurtTime == 10) {
            playerInfo.HurtPosition = currentPosition;
            int particlesamount = mparticlesamount.mValue;
            createDamageEffect(actor, particlesamount); // Damage effect generation
        }

        //if (mHurtTime > 0.0f) {
            //  Particle Drawing
        if (particleMap.count(actor)) {
            for (const auto& particle : particleMap[actor]) {
                //if (particle.life == 0) continue;
                float distanceToPlayer = glm::distance(playerPos, particle.position);
                if (distanceToPlayer <= maxDistance) {
                    ImVec2 screenPos;
                    
                    if (RenderUtils::worldToScreen(particle.position, screenPos)) {

                        float distance = glm::distance(currentPosition, particle.position);
                        float radius = mSize.mValue;

                        float halfWidth = radius;
                        float halfHeight = radius;
                        ImVec2 topLeft(screenPos.x - halfWidth, screenPos.y - halfHeight);
                        ImVec2 bottomRight(screenPos.x + halfWidth, screenPos.y + halfHeight);
                        drawList->AddRectFilled(topLeft, bottomRight, ImColor(138, 3, 3, 255));


                    }
                }
            }
        }
        //}
    }
}

