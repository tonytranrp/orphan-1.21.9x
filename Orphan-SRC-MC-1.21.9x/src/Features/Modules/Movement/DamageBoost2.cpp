//
// Created by ssi on 10/26/2024.
//

#include "DamageBoost2.hpp"

#include <Features/Events/PacketInEvent.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Network/Packets/Packet.hpp>
#include <SDK/Minecraft/Network/Packets/SetActorMotionPacket.hpp>

void DamageBoost2::onEnable()
{
    gFeatureManager->mDispatcher->listen<PacketInEvent, &DamageBoost2::onPacketInEvent>(this);
}

void DamageBoost2::onDisable()
{
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &DamageBoost2::onPacketInEvent>(this);
}

void DamageBoost2::onPacketInEvent(PacketInEvent& event)
{
    if (event.mPacket->getId() == PacketID::SetActorMotion) 
    {
        auto motionPacket = event.getPacket<SetActorMotionPacket>();
        auto player = ClientInstance::get()->getLocalPlayer();
        auto packet = std::reinterpret_pointer_cast<SetActorMotionPacket>(event.mPacket);
        glm::vec3 pMotion = packet->mMotion;
        float vecMag = glm::length(pMotion);
        if (!player || motionPacket->mRuntimeID != player->getRuntimeID()) return;
        float boostSpeed = mCustomSpeed.mValue ? (player->isOnGround() ? mGroundSpeed.mValue : mOffGroundSpeed.mValue) : 100.f;
        glm::vec2 motion = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, (boostSpeed) / 10);
        StateVectorComponent* stateVector = player->getStateVectorComponent();
        if (vecMag > mKBReq.mValue) 
        {

        if (packet->mRuntimeID == ClientInstance::get()->getLocalPlayer()->getRuntimeID()) 
            {
                pMotion.x = motion.x;
                pMotion.z = motion.y;
                float oldY = pMotion.y;
                float newMagnitude = glm::length(pMotion);
                float maxAllowedMagnitude = mMaxMagnitude.mValue * vecMag;
                if (newMagnitude > maxAllowedMagnitude) 
                {
                    pMotion = (pMotion / newMagnitude) * maxAllowedMagnitude;
                }
                pMotion.y = oldY * mYVel.mValue;
                packet->mMotion = pMotion;
            }
        }
    }
}