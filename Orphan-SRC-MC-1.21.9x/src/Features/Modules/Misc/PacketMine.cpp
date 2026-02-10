#include "PacketMine.hpp"
#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Inventory/PlayerInventory.hpp>
#include <SDK/Minecraft/Actor/GameMode.hpp>
#include <SDK/Minecraft/World/BlockSource.hpp>
#include <SDK/Minecraft/World/HitResult.hpp>
#include <SDK/Minecraft/World/Level.hpp>
#include <SDK/Minecraft/Network/LoopbackPacketSender.hpp>
#include <SDK/Minecraft/Network/MinecraftPackets.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerAuthInputPacket.hpp>
#include <SDK/Minecraft/World/BlockLegacy.hpp>
#include <SDK/Minecraft/Network/Packets/MobEquipmentPacket.hpp>
#include <imgui.h>

void PacketMine::onEnable() {
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &PacketMine::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketOutEvent, &PacketMine::onPacketOutEvent>(this);
}

void PacketMine::onDisable() {
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &PacketMine::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketOutEvent, &PacketMine::onPacketOutEvent>(this);

    auto player = ClientInstance::get()->getLocalPlayer();
    if (player && mShouldDestroy) {
        player->getGameMode()->stopDestroyBlock(mBlockPos);
    }

    mShouldDestroy = false;
    mStartDestroying = false;
    mBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    mToolSlot = -1;
    mBreakingProgress = 0.f;
}

void PacketMine::onPacketOutEvent(PacketOutEvent& event) {
    if (!mShouldDestroy) return;

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    if (event.mPacket->getId() == PacketID::PlayerAuthInput) {
        auto packet = event.getPacket<PlayerAuthInputPacket>();
        
        float percentDone = mBreakingProgress / mDestroySpeed.mValue;
        if (percentDone >= mPercentRot.mValue) {
            auto blockAABB = AABB(mBlockPos, glm::vec3(1, 1, 1));
            glm::vec2 rotations = MathUtils::getRots(*player->getPos(), blockAABB);
            packet->mRot = rotations;
            packet->mYHeadRot = rotations.y;
        }
    }
}

void PacketMine::onBaseTickEvent(BaseTickEvent& event) {
    auto player = event.mActor;
    if (!player || !ClientInstance::get()->getMouseGrabbed()) return;

    HitResult* hitResult = player->getLevel()->getHitResult();
    Block* block = ClientInstance::get()->getBlockSource()->getBlock(hitResult->mBlockPos);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (!block->getmLegacy()->isAir()) {
            mBlockPos = hitResult->mBlockPos;
            mBlockFace = hitResult->mFacing;
            mStartTime = NOW;
            mShouldDestroy = true;
            mBreakingProgress = 0.f;
        }
    }

    if (mStartDestroying) {
        BlockUtils::startDestroyBlock(mBlockPos, mBlockFace);
        mStartDestroying = false;
    }

    if (mShouldDestroy) {
        mStartDestroying = true;

        if (mAutoTool.mValue) {
            Block* targetBlock = ClientInstance::get()->getBlockSource()->getBlock(mBlockPos);
            if (targetBlock) {
                findBestTool(targetBlock);
                if (mSpoof.mValue && mToolSlot != -1) {
                    spoofSlot(mToolSlot);
                }
            }
        }

        Block* targetBlock = ClientInstance::get()->getBlockSource()->getBlock(mBlockPos);
        if (targetBlock) {
            float destroySpeed = ItemUtils::getDestroySpeed(mToolSlot, targetBlock);
            mBreakingProgress += destroySpeed;

            if (mBreakingProgress >= mDestroySpeed.mValue) {
                int currentSlot = player->getSupplies()->mSelectedSlot;

                BlockUtils::destroyBlock(mBlockPos, mBlockFace, false);

                if (mAutoTool.mValue && mSpoof.mValue) {
                    player->getSupplies()->mSelectedSlot = currentSlot;
                    PacketUtils::spoofSlot(currentSlot);
                }

                mShouldDestroy = false;
                mBreakingProgress = 0.f;
            }
        }
    }
}

bool PacketMine::findBestTool(Block* block) {
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return false;

    PlayerInventory* supplies = player->getSupplies();
    Container* inventory = supplies->getContainer();
    int previousSlot = supplies->mSelectedSlot;

    int bestSlot = ItemUtils::getBestBreakingTool(block, true);
    if (bestSlot != -1 && bestSlot != previousSlot) {
        mToolSlot = bestSlot;
        return true;
    }

    return false;
}

void PacketMine::spoofSlot(int slot) {
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    PacketUtils::spoofSlot(slot);
} 