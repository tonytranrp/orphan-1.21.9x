//
// Created by vastrakai on 8/3/2024.
// Edited by player5 (2/9/2025)
//

#include "Step.hpp"

#include "Speed.hpp"
#include "LongJump.hpp"
#include <SDK/Minecraft/MinecraftSim.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Actor/Components/StateVectorComponent.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerAuthInputPacket.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <SDK/Minecraft/World/BlockLegacy.hpp>

void Step::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &Step::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketOutEvent, &Step::onPacketOutEvent, nes::event_priority::VERY_LAST>(this);
}

void Step::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &Step::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketOutEvent, &Step::onPacketOutEvent>(this);
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    player->getMaxAutoStepComponent()->mMaxStepHeight = 0.5625f;
}

bool Step::canFallDown() {
    auto pos = *ClientInstance::get()->getLocalPlayer()->getPos();

    for (int checkOffsetY = 1; checkOffsetY <= (mMaxFallDistance.mValue + 1); ++checkOffsetY) {
        glm::vec3 blockPos = { pos.x, pos.y - (float)checkOffsetY, pos.z };
        if (!BlockUtils::isAirBlock(blockPos)) {
            return true;
        }
    }

    return false;
}

bool Step::isVoid() {
    auto pos = *ClientInstance::get()->getLocalPlayer()->getPos();

    for (int checkOffsetY = 1; checkOffsetY <= 255; ++checkOffsetY) {
        glm::vec3 blockPos = { pos.x, pos.y - (float)checkOffsetY, pos.z };
        Block* block = ClientInstance::get()->getBlockSource()->getBlock(blockPos);
        if (block->mLegacy->mMaterial->mIsBlockingMotion) {
            return false;
        }
    }

    return true;
}

bool mShouldClimb = false;
bool mShouldStop = false;
bool mStepEnabled = true;

void Step::onBaseTickEvent(BaseTickEvent& event)
{
    auto player = event.mActor;
    if (!player) return;

    auto handleFlareonV2Step = [this, &player]()
        {
            if (player->isCollidingHorizontal() && player->getMoveInputComponent()->mForward)
            {
                player->getStateVectorComponent()->mVelocity.y = 0.0f;

                {
                    auto& lowerPos = player->getAABBShapeComponent()->mMin;
                    auto& upperPos = player->getAABBShapeComponent()->mMax;

                    lowerPos.y += mStepHeight.mValue / 10;
                    upperPos.y += mStepHeight.mValue / 10;
                }
            }
        };

    if (mStepEnabled) {
        switch ((int)mMode.mValue) {
        case 0:
            player->getMaxAutoStepComponent()->mMaxStepHeight = mStepHeight.mValue;
            break;
        }
    } else {
        player->getMaxAutoStepComponent()->mMaxStepHeight = 0.5625f;
    }

    auto speed = gFeatureManager->mModuleManager->getModule<Speed>();
    auto longJump = gFeatureManager->mModuleManager->getModule<LongJump>();

    bool isPressed = player->getMoveInputComponent()->mForward || player->getMoveInputComponent()->mBackward || player->getMoveInputComponent()->mLeft || player->getMoveInputComponent()->mRight;

    if (mReverseStep.mValue)
    {
        if (!canFallDown()) return;
        if (mVoidCheck.mValue && isVoid()) return;
        if (mDontUseIfSpeed.mValue && speed->mEnabled) return;
        if (Keyboard::isUsingMoveKeys(true) && player->getMoveInputComponent()->mIsJumping || !player->wasOnGround() || mJumped)return;

        if (!player->isOnGround())
            player->getStateVectorComponent()->mVelocity.y -= 2;
    }
}
void Step::onPacketOutEvent(PacketOutEvent& event)
{
    if (event.mPacket->getId() == PacketID::PlayerAuthInput)
    {
        auto paip = event.getPacket<PlayerAuthInputPacket>();
        mJumped = paip->hasInputData(AuthInputAction::START_JUMPING);
    }
}
