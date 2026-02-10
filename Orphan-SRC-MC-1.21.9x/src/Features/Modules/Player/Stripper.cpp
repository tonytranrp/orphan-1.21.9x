#include "Stripper.hpp"
#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Inventory/PlayerInventory.hpp>
#include <SDK/Minecraft/Actor/GameMode.hpp>
#include <SDK/Minecraft/World/BlockLegacy.hpp>
#include <SDK/Minecraft/World/Level.hpp>
#include <SDK/Minecraft/World/HitResult.hpp>
#include <Utils/MiscUtils/BlockUtils.hpp>
#include <SDK/Minecraft/Network/LoopbackPacketSender.hpp>
#include <SDK/Minecraft/Network/MinecraftPackets.hpp>

void Stripper::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &Stripper::onBaseTickEvent>(this);
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;
    mLastSlot = player->getSupplies()->mSelectedSlot;
    mBlocksStripped = 0;
}

void Stripper::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &Stripper::onBaseTickEvent>(this);
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    if (mLastSlot != -1)
    {
        player->getSupplies()->mSelectedSlot = mLastSlot;
    }
    mLastSlot = -1;
    mBlocksStripped = 0;
}

void Stripper::onBaseTickEvent(BaseTickEvent& event)
{
    auto player = event.mActor;
    if (!player) return;

    // Check if player has an axe in hotbar
    int bestAxe = ItemUtils::getBestItem(SItemType::Axe, true); // Force hotbar only
    if (bestAxe == -1) return; // Return if no axe found in hotbar

    int maxBlocks = mBlocksPerTick.as<int>();
    mBlocksStripped = 0;

    while (mBlocksStripped < maxBlocks)
    {
        if (!tickStrip(event)) break;
        mBlocksStripped++;
    }
}

void Stripper::stripBlock(const glm::ivec3& blockPos, int side)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    glm::vec3 vec = glm::vec3(blockPos);
    if (side != -1) vec += glm::vec3(BlockUtils::blockFaceOffsets[side]) * 0.5f;

    HitResult* res = player->getLevel()->getHitResult();
    if (!res) return;

    res->mBlockPos = vec;
    res->mFacing = side;
    res->mType = HitType::BLOCK;
    res->mIndirectHit = false;
    res->mRayDir = vec;
    res->mPos = blockPos;

    bool oldSwinging = player->isSwinging();
    int oldSwingProgress = player->getSwingProgress();

    player->getGameMode()->buildBlock(blockPos, side, true);
    
    player->setSwinging(oldSwinging);
    player->setSwingProgress(oldSwingProgress);

    vec += glm::vec3(BlockUtils::blockFaceOffsets[side]) * 0.5f;

    res->mBlockPos = vec;
    res->mFacing = side;
    res->mType = HitType::BLOCK;
    res->mIndirectHit = false;
    res->mRayDir = vec;
    res->mPos = blockPos;
}

bool Stripper::tickStrip(BaseTickEvent& event)
{
    auto player = event.mActor;
    if (!player) return false;

    auto blockSource = ClientInstance::get()->getBlockSource();
    if (!blockSource) return false;

    glm::vec3 playerPos = *player->getPos();
    float range = mRange.as<float>();

    std::vector<std::pair<glm::ivec3, int>> strippableBlocks;
    
    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            for (int z = -range; z <= range; z++)
            {
                glm::ivec3 blockPos = glm::ivec3(
                    static_cast<int>(std::floor(playerPos.x)) + x,
                    static_cast<int>(std::floor(playerPos.y)) + y - 1,
                    static_cast<int>(std::floor(playerPos.z)) + z
                );
                
                Block* block = blockSource->getBlock(blockPos);
                if (!block) continue;

                std::string blockName = block->getmLegacy()->getmName();
                if (blockName.find("_log") == std::string::npos && 
                    blockName.find("_wood") == std::string::npos) continue;

                if (blockName.find("stripped_") != std::string::npos) continue;

                if (glm::distance(playerPos, glm::vec3(blockPos)) > range) continue;

                int face = BlockUtils::getExposedFace(blockPos);
                if (face == -1) continue;

                strippableBlocks.push_back(std::make_pair(blockPos, face));
            }
        }
    }

    if (strippableBlocks.empty()) return false;

    int bestAxe = ItemUtils::getBestItem(SItemType::Axe, mHotbarOnly.mValue);
    if (bestAxe == -1) return false;

    if (mSwitchMode.mValue != SwitchMode::None) {
        if (mLastSlot == -1) mLastSlot = player->getSupplies()->mSelectedSlot;
        
        if (mSwitchMode.mValue == SwitchMode::Full) {
            player->getSupplies()->mSelectedSlot = bestAxe;
        }
        else if (mSwitchMode.mValue == SwitchMode::Spoof) {
            PacketUtils::spoofSlot(bestAxe, false);
        }
    }

    int blocksProcessed = 0;
    for (auto& blockInfo : strippableBlocks)
    {
        BlockUtils::stripBlock(
            blockInfo.first, 
            blockInfo.second, 
            true,  
            mSwitchMode.mValue == SwitchMode::Spoof ? bestAxe : -1
        );
        
        mBlocksStripped++;
        blocksProcessed++;
        
        if (mBlocksStripped >= mBlocksPerTick.as<int>())
            break;
    }

    if (mSwitchMode.mValue == SwitchMode::Spoof && mLastSlot != -1) {
        PacketUtils::spoofSlot(mLastSlot, false);
    }
    
    return blocksProcessed > 0;
} 