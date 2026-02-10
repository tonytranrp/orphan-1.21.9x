#include "Regen.hpp"
#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Events/PingUpdateEvent.hpp>
#include <Features/Events/SendImmediateEvent.hpp>
#include <Features/Modules/Visual/Interface.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Inventory/PlayerInventory.hpp>
#include <SDK/Minecraft/Actor/GameMode.hpp>
#include <SDK/Minecraft/Network/PacketID.hpp>
#include <SDK/Minecraft/Network/Packets/InventoryTransactionPacket.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerAuthInputPacket.hpp>
#include <SDK/Minecraft/Network/Packets/MobEquipmentPacket.hpp>
#include <SDK/Minecraft/Network/Packets/LevelEventPacket.hpp>
#include <SDK/Minecraft/Network/Packets/UpdateBlockPacket.hpp>
#include <SDK/Minecraft/World/BlockLegacy.hpp>
#include <SDK/Minecraft/World/Level.hpp>
#include <SDK/Minecraft/World/HitResult.hpp>
#include <Features/Modules/Player/ChestStealer.hpp>
#include <Features/Modules/Player/Scaffold.hpp>

void Regen::initializeRegen()
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    GameMode* gm = player->getGameMode();

    if (mIsMiningBlock) {
        gm->stopDestroyBlock(mCurrentBlockPos);
    }
    mCurrentBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    mTargettingBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    mCurrentBlockFace = -1;
    mBreakingProgress = 0.f;
    mShouldSpoofSlot = true;
    mIsMiningBlock = false;
    mIsUncovering = false;
    mIsStealing = false;
    mToolSlot = -1;
    mOffGround = false;
    mCanReplace = false;
    mStartDestroyCount = 0;

    mFakePositions.clear();
}

void Regen::resetSyncSpeed()
{
    mWasUncovering = false;
    mLastTargettingBlockPos = { INT_MAX, INT_MAX, INT_MAX };
    mLastTargettingBlockPosDestroySpeed = 1.f;
    mLastToolSlot = 0;
}

bool Regen::isValidBlock(glm::ivec3 blockPos, bool redstoneOnly, bool exposedOnly, bool isStealing)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    Block* block = ClientInstance::get()->getBlockSource()->getBlock(blockPos);
    if (!block) return false;

    if (block->getmLegacy()->isAir()) return false;

    AABB blockAABB = AABB(blockPos, glm::vec3(1, 1, 1));
    glm::vec3 closestPos = blockAABB.getClosestPoint(*player->getPos());
    if (HARDCODED_RANGE < glm::distance(closestPos, *player->getPos())) {
        if (mIsMiningBlock && blockPos == mCurrentBlockPos) {

            player->getGameMode()->stopDestroyBlock(mCurrentBlockPos);
            initializeRegen();
        }
        return false;
    }

    int blockId = block->getmLegacy()->getBlockId();
    bool isRedstone = blockId == 73 || blockId == 74;
    if (redstoneOnly) {
        if (!isRedstone) return false;
    }

    int exposedFace = BlockUtils::getExposedFace(blockPos);
    bool canSkipExposedCheck = mAntiSteal.mValue && mIsMiningBlock && !mIsUncovering && blockPos == mCurrentBlockPos;
    if (exposedOnly && (!isStealing || !isRedstone) && !canSkipExposedCheck) {
        if (exposedFace == -1) return false;
    }

    if (isStealing && (!mCanSteal || mTargettingBlockPos != mEnemyTargettingBlockPos) && exposedFace == -1) {
        if (mDebug.mValue && mStealNotify.mValue) ChatUtils::displayClientMessage("§esteal cancelled");
        return false;
    }

    if (mAntiSteal.mValue) {

        if (std::find(mLastBrokenOrePos.begin(), mLastBrokenOrePos.end(), blockPos) != mLastBrokenOrePos.end()) {
            return false;
        }

        if (std::find(mOreBlackList.begin(), mOreBlackList.end(), blockPos) != mOreBlackList.end()) {
            return false;
        }

        if (blockPos == mBlackListedOrePos && exposedFace == -1) {
            return false;
        }
    }

    return true;
}

bool Regen::isValidRedstone(glm::ivec3 blockPos)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return false;

    int blockID = ClientInstance::get()->getBlockSource()->getBlock(blockPos)->getmLegacy()->getBlockId();

    if (blockID != 73 && blockID != 74) return false;

    if (BlockUtils::isMiningPosition(blockPos)) return false;

    int exposedFace = BlockUtils::getExposedFace(blockPos);
    if (exposedFace != -1 && mIsMiningBlock) {
        glm::ivec3 placePos = blockPos + mOffsetList[exposedFace];
        glm::ivec3 deltaPos = mCurrentBlockPos - placePos;
        if (abs(deltaPos.x) + abs(deltaPos.y) + abs(deltaPos.z) <= 1) return false;
    }

    return true;
}

void Regen::queueBlock(glm::ivec3 blockPos)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    AABB blockAABB = AABB(blockPos, glm::vec3(1, 1, 1));
    glm::vec3 closestPos = blockAABB.getClosestPoint(*player->getPos());
    if (HARDCODED_RANGE < glm::distance(closestPos, *player->getPos())) {
        initializeRegen();
        return;
    }

    Block* block = ClientInstance::get()->getBlockSource()->getBlock(blockPos);
    mCurrentBlockPos = blockPos;
    mCurrentBlockFace = BlockUtils::getExposedFace(blockPos);
    if (mCurrentBlockFace == -1) mCurrentBlockFace = 0;
    mIsMiningBlock = true;
    mBreakingProgress = 0.f;
    int bestToolSlot = ItemUtils::getBestBreakingTool(block, true);
    float destroySpeed = ItemUtils::getDestroySpeed(bestToolSlot, block);

    mShouldRotate = true;

    PacketUtils::spoofSlot(bestToolSlot, false);
    mShouldSpoofSlot = false;
    BlockUtils::startDestroyBlock(blockPos, mCurrentBlockFace);
    mToolSlot = bestToolSlot;
    mShouldSetbackSlot = true;
    std::string blockName = block->getmLegacy()->getmName();
    for (auto& c : BlockUtils::mDynamicSpeeds) {
        if (c.blockName == blockName) {
            break;
        }
    }
}

Regen::PathFindingResult Regen::getBestPathToBlock(glm::ivec3 blockPos)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return { glm::ivec3(INT_MAX, INT_MAX, INT_MAX), 0 };

    BlockSource* source = ClientInstance::get()->getBlockSource();
    if (!source) return { glm::ivec3(INT_MAX, INT_MAX, INT_MAX), 0 };

    AABB targetBlockAABB = AABB(blockPos, glm::vec3(1, 1, 1));
    glm::vec3 targetClosestPos = targetBlockAABB.getClosestPoint(*player->getPos());
    if (HARDCODED_RANGE < glm::distance(targetClosestPos, *player->getPos())) {
        return { glm::ivec3(INT_MAX, INT_MAX, INT_MAX), 0 };
    }

    float bestBreakingTime = INT_MAX;
    glm::ivec3 bestPos = { INT_MAX, INT_MAX, INT_MAX };

    if (mUncover.mValue) {
        static std::vector<glm::ivec3> offsetList = {
            glm::ivec3(0, 1, 0),
            glm::ivec3(0, 0, -1),
            glm::ivec3(0, 0, 1),
            glm::ivec3(-1, 0, 0),
            glm::ivec3(1, 0, 0),
        };

        if (BlockUtils::getExposedFace(blockPos) != -1) {
            return { blockPos, 0 };
        }
        for (int pathLength = 1; pathLength <= mUncoverAmount.mValue; pathLength++) {
            if (pathLength != mUncoverAmount.mValue) continue;

            for (const auto& offset : offsetList) {
                glm::ivec3 pos = blockPos + offset;

                AABB pathBlockAABB = AABB(pos, glm::vec3(1, 1, 1));
                glm::vec3 pathClosestPos = pathBlockAABB.getClosestPoint(*player->getPos());
                if (HARDCODED_RANGE < glm::distance(pathClosestPos, *player->getPos())) {
                    continue;
                }

                float totalBreakTime = 0;
                if (BlockUtils::isAirBlock(pos)) continue;

                Block* block = source->getBlock(pos);
                int bestTool = ItemUtils::getBestBreakingTool(block, true);
                totalBreakTime = 1.0f / ItemUtils::getDestroySpeed(bestTool, block);

                if (BlockUtils::getExposedFace(pos) != -1 && totalBreakTime < bestBreakingTime) {
                    bestBreakingTime = totalBreakTime;
                    bestPos = pos;
                }

                if (pathLength > 1) {
                    for (const auto& offset2 : offsetList) {
                        if (offset == -offset2) continue;

                        glm::ivec3 pos2 = pos + offset2;

                        AABB path2BlockAABB = AABB(pos2, glm::vec3(1, 1, 1));
                        glm::vec3 path2ClosestPos = path2BlockAABB.getClosestPoint(*player->getPos());
                        if (HARDCODED_RANGE < glm::distance(path2ClosestPos, *player->getPos())) {
                            continue;
                        }

                        if (BlockUtils::isAirBlock(pos2)) continue;

                        Block* block2 = source->getBlock(pos2);
                        int bestTool2 = ItemUtils::getBestBreakingTool(block2, true);
                        float totalBreakTime2 = totalBreakTime + (1.0f / ItemUtils::getDestroySpeed(bestTool2, block2));

                        if (BlockUtils::getExposedFace(pos2) != -1 && totalBreakTime2 < bestBreakingTime) {
                            bestBreakingTime = totalBreakTime2;
                            bestPos = pos2;
                        }
                    }
                }
            }

            if (bestPos.x != INT_MAX) break;
        }
    }
    return { bestPos, bestBreakingTime };
}

void Regen::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &Regen::onBaseTickEvent, nes::event_priority::VERY_LAST>(this);
    gFeatureManager->mDispatcher->listen<PacketOutEvent, &Regen::onPacketOutEvent, nes::event_priority::VERY_LAST>(this);
    gFeatureManager->mDispatcher->listen<PacketInEvent, &Regen::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->listen<SendImmediateEvent, &Regen::onSendImmediateEvent, nes::event_priority::VERY_LAST>(this);
    gFeatureManager->mDispatcher->listen<PingUpdateEvent, &Regen::onPingUpdateEvent, nes::event_priority::VERY_FIRST>(this);

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    mShouldRotate = false;
    mIsMiningBlock = false;
    mEnemyTargettingBlockPos = { 0, 0, 0 };
    miningRedstones.clear();

    resetSyncSpeed();

    initializeRegen();
}

void Regen::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &Regen::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketOutEvent, &Regen::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &Regen::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->deafen<SendImmediateEvent, &Regen::onSendImmediateEvent>(this);
    gFeatureManager->mDispatcher->deafen<PingUpdateEvent, &Regen::onPingUpdateEvent>(this);

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    if (mIsMiningBlock) {
        player->getSupplies()->mSelectedSlot = mToolSlot;
        player->getGameMode()->stopDestroyBlock(mCurrentBlockPos);
        mIsMiningBlock = false;
    }
    mWasMiningBlock = false;
}

void Regen::onBaseTickEvent(BaseTickEvent& event) {
    if (!mEnabled) return;

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    mOffGround = !player->isOnGround();

    if (mOnGround.mValue && mOffGround) {
        if (mIsMiningBlock) {
            player->getGameMode()->stopDestroyBlock(mCurrentBlockPos);
            initializeRegen();
            resetSyncSpeed();
        }
        if (mShouldRotate) {
            mShouldRotate = false;
        }
        return;
    }

    mWasMiningBlock = mIsMiningBlock;

    if (!mUncover.mValue) {
        stealEnabled = false;
    }
    else if (mUncover.mValue && !stealEnabled) {
        stealEnabled = true;
    }

    mLastBrokenOrePos.clear();
    mLastBrokenCoveringBlockPos.clear();

    mCurrentUncover = mUncover.mValue;

    BlockSource* source = ClientInstance::get()->getBlockSource();
    if (!source) return;
    PlayerInventory* supplies = player->getSupplies();
    mPreviousSlot = supplies->getmSelectedSlot();

    float absorption = player->getAbsorption();
    bool maxAbsorption = 10 <= absorption;

    int pickaxeSlot = ItemUtils::getBestItem(SItemType::Pickaxe, true);
    ItemStack* stack = supplies->getContainer()->getItem(pickaxeSlot);
    bool hasPickaxe = stack->mItem && stack->getItem()->getItemType() == SItemType::Pickaxe;

    auto chestStealer = gFeatureManager->mModuleManager->getModule<ChestStealer>();
    bool mStealing = chestStealer && chestStealer->mEnabled && chestStealer->mIsStealing;

    auto scaffold = gFeatureManager->mModuleManager->getModule<Scaffold>();
    bool isScaffold = scaffold && scaffold->mEnabled;

    bool canSteal = mSteal.mValue &&
        (mCanSteal || mIsStealing || mLastStealerUpdate + 100 > NOW) &&
        isValidBlock(mEnemyTargettingBlockPos, true, false) &&
        stealEnabled;

    bool shouldSteal = false;
    if (mStealPriority.mValue == StealPriority::Mine) {
        shouldSteal = canSteal && (!mIsMiningBlock || !isValidBlock(mCurrentBlockPos, !mIsUncovering, !mIsUncovering, mIsStealing));
    }
    else if (mStealPriority.mValue == StealPriority::Steal) {
        shouldSteal = canSteal;
    }

    if ((maxAbsorption && !shouldSteal && !mAlwaysMine.mValue) ||
        player->getStatusFlag(ActorFlags::Noai) ||
        !hasPickaxe ||
        player->isDestroying() ||
        mStealing ||
        isScaffold) {
        initializeRegen();
        resetSyncSpeed();

        if (mShouldSetbackSlot) {
            PacketUtils::spoofSlot(mPreviousSlot, false);
            mShouldSetbackSlot = false;
        }
        return;
    }

    if (mAntiSteal.mValue && mIsMiningBlock) {
        int exposedFace = BlockUtils::getExposedFace(mCurrentBlockPos);
        if (exposedFace == -1) {

            if (mBreakingProgress / mCurrentDestroySpeed > 0.7f && mCurrentBlockPos == mTargettingBlockPos) {
                Block* currentBlock = source->getBlock(mCurrentBlockPos);
                int bestToolSlot = ItemUtils::getBestBreakingTool(currentBlock, true);
                mToolSlot = bestToolSlot;

                if (mShouldSpoofSlot) {
                    PacketUtils::spoofSlot(bestToolSlot, false);
                    mShouldSpoofSlot = false;
                }

                float destroySpeed = ItemUtils::getDestroySpeed(bestToolSlot, currentBlock);
                mBreakingProgress += destroySpeed;

                if (mCurrentDestroySpeed <= mBreakingProgress) {
                    mShouldRotate = true;
                    supplies->mSelectedSlot = bestToolSlot;
                    player->swing();
                    BlockUtils::destroyBlock(mCurrentBlockPos, 0, true);
                    if (mDebug.mValue) {
                        ChatUtils::displayClientMessage("§aContinued mining covered ore!");
                    }
                    supplies->mSelectedSlot = mPreviousSlot;
                    mIsMiningBlock = false;
                    return;
                }
            }
            else {

                bool shouldConfuse = mConfuserMode.mValue == ConfuserMode::Always ||
                    (mConfuserMode.mValue == ConfuserMode::Auto &&
                        mLastStealerDetected + 1500 > NOW);

                if (shouldConfuse && 0.1 <= mBreakingProgress && mIsUncovering) {
                    initializeRegen();
                    resetSyncSpeed();
                    if (mDebug.mValue) {
                        ChatUtils::displayClientMessage("§eConfusing enemies...");
                    }
                    return;
                }

                PathFindingResult result = getBestPathToBlock(mCurrentBlockPos);
                if (result.blockPos.x != INT_MAX) {
                    mIsUncovering = true;
                    queueBlock(result.blockPos);
                    return;
                }

                else if (mCurrentBlockPos == mTargettingBlockPos) {
                    Block* currentBlock = source->getBlock(mCurrentBlockPos);
                    int bestToolSlot = ItemUtils::getBestBreakingTool(currentBlock, true);
                    mToolSlot = bestToolSlot;

                    if (mShouldSpoofSlot) {
                        PacketUtils::spoofSlot(bestToolSlot, false);
                        mShouldSpoofSlot = false;
                    }

                    float destroySpeed = ItemUtils::getDestroySpeed(bestToolSlot, currentBlock);
                    mBreakingProgress += destroySpeed;

                    if (mCurrentDestroySpeed <= mBreakingProgress) {
                        mShouldRotate = true;
                        supplies->mSelectedSlot = bestToolSlot;
                        player->swing();
                        BlockUtils::destroyBlock(mCurrentBlockPos, 0, true);
                        if (mDebug.mValue) {
                            ChatUtils::displayClientMessage("§aContinued mining covered ore!");
                        }
                        supplies->mSelectedSlot = mPreviousSlot;
                        mIsMiningBlock = false;
                        return;
                    }
                }
            }
        }
    }

    if (isValidBlock(mCurrentBlockPos, !mIsUncovering, !mIsUncovering, mIsStealing) && (isValidBlock(mTargettingBlockPos, true, false) || !mQueueOres.mValue)) {
        Block* currentBlock = source->getBlock(mCurrentBlockPos);
        int exposedFace = BlockUtils::getExposedFace(mCurrentBlockPos);
        int bestToolSlot = ItemUtils::getBestBreakingTool(currentBlock, true);
        mToolSlot = bestToolSlot;

        if (mShouldSpoofSlot) {
            PacketUtils::spoofSlot(bestToolSlot, false);
            mShouldSpoofSlot = false;
            return;
        }

        bool isRedstone = currentBlock->getmLegacy()->getBlockId() == 73 || currentBlock->getmLegacy()->getBlockId() == 74;
        float destroySpeed = ItemUtils::getDestroySpeed(bestToolSlot, currentBlock);

        std::string blockName = currentBlock->getmLegacy()->getmName();
        for (auto& c : BlockUtils::mDynamicSpeeds) {
            if (c.blockName == blockName) {
                mCurrentDestroySpeed = c.destroySpeed;
                break;
            }
        }
        if (mCurrentDestroySpeed == 1.f) {
            mCurrentDestroySpeed = mDestroySpeed.mValue;
        }

        mBreakingProgress += destroySpeed;

        bool finishBreak = true;
        if (maxAbsorption && isRedstone && !mAlwaysMine.mValue && !mIsStealing) finishBreak = false;

        if ((mCurrentDestroySpeed <= mBreakingProgress && (!mIsStealing || exposedFace != -1)) && finishBreak) {
            mShouldRotate = true;
            supplies->mSelectedSlot = bestToolSlot;
            player->swing();
            BlockUtils::destroyBlock(mCurrentBlockPos, exposedFace, true);
            mCanReplace = mIsStealing || (2 <= mStartDestroyCount && !mIsUncovering);
            if (mIsStealing && mDebug.mValue) {
                ChatUtils::displayClientMessage("§astole enemy's ore");
            }
            supplies->mSelectedSlot = mPreviousSlot;
            mIsMiningBlock = false;

            mWasUncovering = mIsUncovering;
            mLastTargettingBlockPos = mTargettingBlockPos;
            mLastTargettingBlockPosDestroySpeed = destroySpeed;
            mLastToolSlot = bestToolSlot;
            return;
        }
    }
    else {
        initializeRegen();

        if (isValidBlock(mCurrentBlockPos, !mIsUncovering, !mIsUncovering, mIsStealing)) {
            return;
        }

        std::vector<BlockInfo> blockList = BlockUtils::getBlockList(*player->getPos(), HARDCODED_RANGE);
        std::vector<BlockInfo> exposedBlockList;
        std::vector<BlockInfo> unexposedBlockList;

        for (int i = 0; i < blockList.size(); i++) {
            if (isValidBlock(blockList[i].mPosition, true, false)) {
                if (BlockUtils::getExposedFace(blockList[i].mPosition) != -1) {
                    exposedBlockList.push_back(blockList[i]);
                }
                else {
                    unexposedBlockList.push_back(blockList[i]);
                }
            }
        }

        if (mCanSteal && mSteal.mValue && stealEnabled && isValidBlock(mEnemyTargettingBlockPos, true, false)) {
            queueBlock(mEnemyTargettingBlockPos);
            mTargettingBlockPos = mEnemyTargettingBlockPos;
            mIsStealing = true;
            return;
        }

        glm::vec3 playerPos = *player->getPos();
        glm::ivec3 pos = { INT_MAX, INT_MAX, INT_MAX };
        glm::ivec3 targettingPos = { INT_MAX, INT_MAX, INT_MAX };

        if (!exposedBlockList.empty()) {
            float closestDistance = INT_MAX;
            for (int i = 0; i < exposedBlockList.size(); i++) {
                glm::vec3 blockPos = exposedBlockList[i].mPosition;
                float dist = glm::distance(playerPos, blockPos);

                if (dist <= HARDCODED_RANGE && dist < closestDistance) {
                    closestDistance = dist;
                    pos = blockPos;
                    targettingPos = blockPos;
                }
            }

            if (pos.x != INT_MAX) {

                float finalDist = glm::distance(playerPos, glm::vec3(pos));
                if (finalDist <= HARDCODED_RANGE) {
                    queueBlock(pos);
                    mTargettingBlockPos = targettingPos;
                    return;
                }
            }
        }

        if (mCurrentUncover && !unexposedBlockList.empty()) {
            bool foundBlock = false;
            float fastestTime = INT_MAX;
            for (const auto& blockInfo : unexposedBlockList) {

                float dist = glm::distance(playerPos, glm::vec3(blockInfo.mPosition));
                if (dist <= HARDCODED_RANGE) {
                    PathFindingResult result = getBestPathToBlock(blockInfo.mPosition);
                    if (result.blockPos.x != INT_MAX && result.time < fastestTime) {

                        float pathDist = glm::distance(playerPos, glm::vec3(result.blockPos));
                        if (pathDist <= HARDCODED_RANGE) {
                            fastestTime = result.time;
                            pos = result.blockPos;
                            targettingPos = blockInfo.mPosition;
                            foundBlock = true;
                        }
                    }
                }
            }
            if (foundBlock) {
                queueBlock(pos);
                mIsUncovering = true;
                mTargettingBlockPos = targettingPos;
                return;
            }
        }
    }
}

void Regen::onRenderEvent(RenderEvent& event)
{

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    renderBlock();
    if (mProgressBarStyle.mValue == ProgressBarStyle::Orphan) renderNewProgressBar();
    else renderAeolusProgressBar();
}

void Regen::renderNewProgressBar()
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    static float lastProgress = 0.f;
    float percentDone = 1.f;

    percentDone = mBreakingProgress;
    percentDone /= mCurrentDestroySpeed;
    if (percentDone < lastProgress) lastProgress = percentDone;
    percentDone = MathUtils::lerp(lastProgress, percentDone, ImGui::GetIO().DeltaTime * 30.f);
    lastProgress = percentDone;

    percentDone = MathUtils::clamp(percentDone, 0.f, 1.f);

    float delta = ImGui::GetIO().DeltaTime;

    static EasingUtil inEase = EasingUtil();
    static float anim = 0.f;
    constexpr float easeSpeed = 10.f;
    (mEnabled && mWasMiningBlock || mEnabled && mIsMiningBlock) ? inEase.incrementPercentage(delta * easeSpeed / 10)
        : inEase.decrementPercentage(delta * 2 * easeSpeed / 10);
    float inScale = inEase.easeOutExpo();
    if (inEase.isPercentageMax()) inScale = 0.996;
    inScale = MathUtils::clamp(inScale, 0.0f, 0.996);
    anim = MathUtils::lerp(0, 1, inEase.easeOutExpo());
    anim = MathUtils::lerp(anim, (mEnabled && mWasMiningBlock || mEnabled && mIsMiningBlock) ? 1.f : 0.f, delta * 10.f);

    if (anim < 0.001f) return;

    auto drawList = ImGui::GetBackgroundDrawList();

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImVec2 pos = ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2.5f);
    pos.y += pos.y / 1.14;
    ImVec2 boxSize = ImVec2(216 * anim, 48 * anim);

    pos.x -= boxSize.x / 2;
    pos.y -= boxSize.y / 2;

    pos.y += 20.f;

    ImVec2 progressPos = ImVec2(pos.x, pos.y);
    ImVec2 progressSize = ImVec2(boxSize.x * percentDone, boxSize.y);

    int daPerc = percentDone * 100;

    static std::string text = "Mining 0%";

    static ImColor color = ImColor(255, 255, 0, 180);
    ImColor targetColor = ImColor(0, 255, 0, 153);

    bool isMining = mIsMiningBlock && mBreakingProgress > 0.001f;

    auto interfaceMod = gFeatureManager->mModuleManager->getModule<Interface>();
    bool isLowercase = interfaceMod->mNamingStyle.mValue == Lowercase || interfaceMod->mNamingStyle.mValue == LowercaseSpaced;

    if (mIsStealing && mEnabled)
    {
        targetColor = ImColor(153, 50, 204, 169);
        if (isLowercase) text = "stealing " + std::to_string(daPerc) + "%";
        else text = "Stealing " + std::to_string(daPerc) + "%";
    }
    else if (mIsUncovering && mEnabled)
    {
        if (isLowercase) text = "uncovering " + std::to_string(daPerc) + "%";
        else text = "Uncovering " + std::to_string(daPerc) + "%";
        targetColor = ImColor(240, 128, 128, 169);
    }
    else if (isMining && 10 <= player->getAbsorption() && mEnabled)
    {
        if (isLowercase) text = "queueing " + std::to_string(daPerc) + "%";
        else text = "Queueing " + std::to_string(daPerc) + "%";
        targetColor = ImColor(123, 104, 238, 169);
    }
    else
    {
        if (isLowercase) text = "mining " + std::to_string(daPerc) + "%";
        else text = "Mining " + std::to_string(daPerc) + "%";
        targetColor = ImColor(255, 215, 33, 169);
    }

    color = ImColor(MathUtils::lerp(color.Value, targetColor.Value, ImGui::GetIO().DeltaTime * 7.5f));

    float daPadding = -25.f * anim;

    float max = pos.x + boxSize.x;
    ImVec2 bgMin = ImVec2(pos.x + boxSize.x * percentDone, pos.y);
    ImVec2 bgMax = ImVec2(pos.x + boxSize.x, pos.y + (boxSize.y + daPadding));
    ImVec2 progMax = ImVec2(pos.x + (boxSize.x * percentDone + 6.f), pos.y + (boxSize.y + daPadding));
    progMax.x = std::clamp(progMax.x, pos.x, max);

    float rounding = 99;

    if (percentDone > 0.001f)
    {
        drawList->AddShadowRect(ImVec2(pos.x, pos.y), progMax, ColorUtils::getThemedColor(0), 50.f, ImVec2(), ImDrawCornerFlags_All, rounding);
        drawList->PushClipRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + (boxSize.x * percentDone), pos.y + (boxSize.y - 10.f)));

        ImColor leftSide = color;
        drawList->AddRectFilledMultiColor(ImVec2(pos.x, pos.y), progMax, ColorUtils::getThemedColor(0), ColorUtils::getThemedColor(1), ColorUtils::getThemedColor(2), ColorUtils::getThemedColor(3), rounding, ImDrawCornerFlags_All);
        drawList->PopClipRect();
    }

    if (percentDone > 0.001f) drawList->PushClipRect(bgMin, bgMax);
    drawList->AddRectFilled(ImVec2(pos.x + boxSize.x * percentDone - 6, pos.y), bgMax, ImColor(0.f, 0.f, 0.f, 0.6f), rounding);
    if (percentDone > 0.001f) drawList->PopClipRect();

    FontHelper::pushPrefFont(true, true, true);

    float fontSize = 20.f * anim;
    ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.c_str());

    ImVec2 textPos = ImVec2(pos.x + (boxSize.x - textSize.x) / 2, pos.y + 1.6);

    ImRenderUtils::drawShadowText(drawList, text, textPos, ImColor(255, 255, 255, 255), fontSize);
    FontHelper::popPrefFont();

}

void Regen::DrawRotatedTriangle(ImDrawList* drawList, const ImVec2& center, float size, float rotation, const ImColor& color) {
    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);
    ImVec2 points[3];
    points[0] = ImVec2(center.x + cos_r * size, center.y + sin_r * size);
    points[1] = ImVec2(center.x + cos_r * -size + sin_r * size, center.y + sin_r * -size - cos_r * size);
    points[2] = ImVec2(center.x + cos_r * -size - sin_r * size, center.y + sin_r * -size + cos_r * size);
    drawList->AddTriangleFilled(points[0], points[1], points[2], color);
}

void Regen::DrawRotatedSquare(ImDrawList* drawList, const ImVec2& center, float size, float rotation, const ImColor& color) {
    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);
    ImVec2 points[4];
    points[0] = ImVec2(center.x + (cos_r - sin_r) * size, center.y + (sin_r + cos_r) * size);
    points[1] = ImVec2(center.x + (-cos_r - sin_r) * size, center.y + (-sin_r + cos_r) * size);
    points[2] = ImVec2(center.x + (-cos_r + sin_r) * size, center.y + (-sin_r - cos_r) * size);
    points[3] = ImVec2(center.x + (cos_r + sin_r) * size, center.y + (sin_r - cos_r) * size);
    drawList->AddQuadFilled(points[0], points[1], points[2], points[3], color);
}

void Regen::spawnParticle(const ImVec2& textPos) {
    if (mParticles.size() >= MAX_PARTICLES) return;

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    float fontSize = 20.f * anim;
    int absorptionValue = static_cast<int>(player->getAbsorption());
    std::string text = mAccurate.mValue ? "Mining (" + std::to_string(absorptionValue / 2) + "/5)" : "Mining (" + std::to_string(absorptionValue) + "/10)";

    ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.c_str());

    int shapesToSpawn = (mParticleShape.mValue == ParticleShape::Mixed) ? 3 : PARTICLES_PER_BURST;

    for (int i = 0; i < shapesToSpawn; i++) {
        if (mParticles.size() >= MAX_PARTICLES) break;

        ImColor themeColor = ColorUtils::getThemedColor(i * 30);

        float randomX = textPos.x + (static_cast<float>(rand()) / RAND_MAX) * textSize.x;
        float randomY = textPos.y + (static_cast<float>(rand()) / RAND_MAX) * textSize.y;
        ImVec2 spawnPos = ImVec2(randomX, randomY);

        Particle particle;
        particle.pos = spawnPos;
        particle.movingInward = false;

        float randomAngle = (static_cast<float>(rand()) / RAND_MAX) * 2.0f * 3.14159f;

        float baseSpeed = PARTICLE_SPEED + (static_cast<float>(rand()) / RAND_MAX * PARTICLE_SPEED_VARIANCE);
        particle.velocity = ImVec2(
            cos(randomAngle) * baseSpeed,
            sin(randomAngle) * baseSpeed
        );

        particle.life = PARTICLE_LIFE * (0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f);
        particle.maxLife = particle.life;
        particle.color = themeColor;
        particle.scale = 0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
        particle.rotation = static_cast<float>(rand()) / RAND_MAX * 2 * 3.14159f;
        particle.rotationSpeed = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * MAX_ROTATION_SPEED;

        if (mParticleShape.mValue == ParticleShape::Mixed) {
            particle.shape = static_cast<ParticleShape>(i % 3);
        }
        else {
            particle.shape = mParticleShape.mValue;
        }

        mParticles.push_back(particle);
    }
}

void Regen::updateParticles(float deltaTime, const ImVec2& sourcePos) {
    for (auto it = mParticles.begin(); it != mParticles.end();) {
        it->life -= deltaTime;
        if (it->life <= 0) {
            it = mParticles.erase(it);
            continue;
        }

        it->pos.x += it->velocity.x * deltaTime;
        it->pos.y += it->velocity.y * deltaTime;
        it->rotation += it->rotationSpeed * deltaTime;

        float lifePercent = it->life / it->maxLife;
        it->velocity.x *= 0.95f;
        it->velocity.y *= 0.95f;
        it->color.Value.w = lifePercent;

        ++it;
    }
}

void Regen::renderAeolusProgressBar()
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    static float lastProgress = 0.f;
    static EasingUtil inEase = EasingUtil();
    static float anim = 0.f;
    static float idleTime = 0.f;
    static float textFadeAnim = 0.f;
    constexpr float easeSpeed = 10.f;
    float delta = ImGui::GetIO().DeltaTime;

    float percentDone = mBreakingProgress / mCurrentDestroySpeed;
    if (percentDone < lastProgress) lastProgress = percentDone;
    percentDone = MathUtils::lerp(lastProgress, percentDone, delta * 30.f);
    lastProgress = percentDone;
    percentDone = MathUtils::clamp(percentDone, 0.f, 1.f);

    if (mEnabled && (mWasMiningBlock || mIsMiningBlock)) {
        inEase.incrementPercentage(delta * easeSpeed / 10);
        idleTime = 0.f;
    }
    else {
        idleTime += delta;
        if (idleTime > 1.0f) {
            inEase.decrementPercentage(delta * 2 * easeSpeed / 10);
        }
    }

    float inScale = inEase.easeOutExpo();
    if (inEase.isPercentageMax()) inScale = 0.996;
    inScale = MathUtils::clamp(inScale, 0.0f, 0.996);
    anim = MathUtils::lerp(anim, (mEnabled && (mWasMiningBlock || mIsMiningBlock)) ? 1.f : 0.f, delta * 10.f);

    textFadeAnim = MathUtils::lerp(textFadeAnim, (idleTime > 1.0f) ? 0.f : 1.f, delta * 12.f);
    textFadeAnim = MathUtils::clamp(textFadeAnim, 0.f, 1.f);

    if (anim < 0.001f) return;

    auto drawList = ImGui::GetBackgroundDrawList();
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImVec2 pos = ImVec2(displaySize.x / 2, displaySize.y / 2.5f);
    pos.y += pos.y / 1.14;
    ImVec2 boxSize = ImVec2(216 * anim, 48 * anim);
    pos.x -= boxSize.x / 2;
    pos.y -= boxSize.y / 2;
    float fontSize = 20.f * anim;

    int absorptionValue = static_cast<int>(player->getAbsorption());
    std::string text = mAccurate.mValue ? "Mining (" + std::to_string(absorptionValue / 2) + "/5)" : "Mining (" + std::to_string(absorptionValue) + "/10)";

    ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.c_str());
    ImVec2 textPos = ImVec2(pos.x + (boxSize.x - textSize.x) / 2, pos.y);

    if (mBackground.mValue) {
        drawList->AddShadowRect(ImVec2(textPos.x - 10, textPos.y - 5),
            ImVec2(textPos.x + textSize.x + 10, textPos.y + textSize.y + 5),
            ImColor(0.f, 0.f, 0.f, 0.45f * textFadeAnim),
            500, ImVec2(0, 0));
    }

    FontHelper::pushPrefFont(true, false, false);

    for (size_t i = 0; i < text.length(); i++) {
        float charProgress = static_cast<float>(i) / text.length();
        float progressOffset = percentDone - charProgress;
        float alpha = textFadeAnim;

        ImColor textColor;
        if (progressOffset >= 0 && progressOffset <= 1.0f) {

            float colorProgress = charProgress / percentDone;
            textColor = ColorUtils::getThemedColor(static_cast<int>(colorProgress * 100));
        }
        else {

            textColor = ImColor(255, 255, 255, static_cast<int>(255 * alpha));
        }

        std::string currentChar(1, text[i]);
        float charX = textPos.x + ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.substr(0, i).c_str()).x;
        ImVec2 charPos = ImVec2(charX, textPos.y);

        ImRenderUtils::drawShadowText(drawList, currentChar, charPos, textColor, fontSize);
    }

    FontHelper::popPrefFont();

    bool currentlyMining = mIsMiningBlock && mBreakingProgress > 0.001f;
    if (mLastMiningState && !currentlyMining && mShowParticles.mValue) {
        float currentTime = ImGui::GetTime();
        if (currentTime - mLastParticleSpawn > PARTICLE_SPAWN_RATE) {
            spawnParticle(textPos);
            mLastParticleSpawn = currentTime;
        }
    }
    mLastMiningState = currentlyMining;

    if (mShowParticles.mValue) {
        updateParticles(ImGui::GetIO().DeltaTime, textPos);
        for (const auto& particle : mParticles) {
            float baseSize = 3.0f * particle.scale;
            float size = baseSize * (particle.life / particle.maxLife);

            ImColor glowColor = particle.color;
            glowColor.Value.w *= 0.5f;

            switch (particle.shape) {
            case ParticleShape::Circle:
                drawList->AddCircleFilled(particle.pos, size * 2.0f, glowColor);
                drawList->AddCircleFilled(particle.pos, size, particle.color);
                break;

            case ParticleShape::Triangle:
                DrawRotatedTriangle(drawList, particle.pos, size * 2.0f, particle.rotation, glowColor);
                DrawRotatedTriangle(drawList, particle.pos, size, particle.rotation, particle.color);
                break;

            case ParticleShape::Square:
                DrawRotatedSquare(drawList, particle.pos, size * 2.0f, particle.rotation, glowColor);
                DrawRotatedSquare(drawList, particle.pos, size, particle.rotation, particle.color);
                break;
            }
        }
    }
}

void Regen::renderBlock()
{
    if (!mIsMiningBlock || !mEnabled) return;

    auto player = ClientInstance::get()->getLocalPlayer();

    static glm::vec3 lastBlockPos = mCurrentBlockPos;
    glm::vec3 blockPos = mCurrentBlockPos;

    if (mLerp.mValue) {
        glm::vec3 lastBlockPosVec3 = glm::vec3(lastBlockPos);
        glm::vec3 currentBlockPosVec3 = glm::vec3(mCurrentBlockPos);
        blockPos = MathUtils::lerp(lastBlockPosVec3, currentBlockPosVec3, static_cast<float>(ImGui::GetIO().DeltaTime * 10.f));
        lastBlockPos = blockPos;
    }
    else {
        lastBlockPos = mCurrentBlockPos;
    }

    auto size = glm::vec3(1.0f, 1.0f, 1.0f);
    blockPos.x += 0.5f - (size.x / 2.f);
    blockPos.y += 0.5f - (size.y / 2.f);
    blockPos.z += 0.5f - (size.z / 2.f);
    auto blockAABB = AABB(blockPos, size);

    static ImColor color = ImColor(255, 255, 0, 255);
    ImColor targetColor;

    if (mIsMiningBlock && 10 <= player->getAbsorption()) {
        targetColor = ImColor(0, 255, 255, 255);
    }
    else {
        targetColor = ImColor(255, 255, 0, 255);
    }

    color = ImColor(MathUtils::lerp(color.Value, targetColor.Value, ImGui::GetIO().DeltaTime * 10.f));

    RenderUtils::drawOutlinedAABB(blockAABB, true, color);
}

void Regen::renderFakeOres() {
    if (!mEnabled) return;

    if (!mFakePositions.empty()) {
        for (auto& pos : mFakePositions) {
            auto boxAABB = AABB(pos, glm::vec3(1.f, 1.f, 1.f));
            RenderUtils::drawOutlinedAABB(boxAABB, true);
        }
    }
}

void Regen::onPacketOutEvent(PacketOutEvent& event) {
    if (!mEnabled) return;

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    Block* blok = ClientInstance::get()->getBlockSource()->getBlock(mCurrentBlockPos);
    float percentDone = 1.f;

    percentDone = mBreakingProgress;
    percentDone /= mCurrentDestroySpeed;

    if (event.mPacket->getId() == PacketID::PlayerAuthInput) {
        auto paip = event.getPacket<PlayerAuthInputPacket>();

        if (mShouldRotate && !(mOnGround.mValue && mOffGround)) {
            player->swing();
            if (percentDone >= mPercentRot.mValue) {
                const glm::vec3 blockPos = mCurrentBlockPos;
                auto blockAABB = AABB(blockPos, glm::vec3(1, 1, 1));
                glm::vec2 rotations = MathUtils::getRots(*player->getPos(), blockAABB);
                paip->mRot = rotations;
                paip->mYHeadRot = rotations.y;
            }
            if (blok->getmLegacy()->isAir()) {
                mShouldRotate = false;
            }
        }

        if (mShouldRotateToPlacePos)
        {
            const glm::vec3 blockPos = mCurrentPlacePos;
            auto blockAABB = AABB(blockPos, glm::vec3(1, 1, 1));
            glm::vec2 rotations = MathUtils::getRots(*player->getPos(), blockAABB);
            paip->mRot = rotations;
            paip->mYHeadRot = rotations.y;
            if (!blok->getmLegacy()->isAir())
            {
                mShouldRotateToPlacePos = false;
            }
        }
    }
    else if (event.mPacket->getId() == PacketID::InventoryTransaction)
    {
        if (const auto it = event.getPacket<InventoryTransactionPacket>(); it->mTransaction->type ==
            ComplexInventoryTransaction::Type::ItemUseTransaction)
        {
            const auto transac = reinterpret_cast<ItemUseInventoryTransaction*>(it->mTransaction.get());
            if (transac->mActionType == ItemUseInventoryTransaction::ActionType::Place)
            {
                transac->mClickPos = BlockUtils::clickPosOffsets[transac->mFace];
                for (int i = 0; i < 3; i++)
                {
                    if (transac->mClickPos[i] == 0.5)
                    {
                        transac->mClickPos[i] = MathUtils::randomFloat(-0.49f, 0.49f);
                    }
                }
            }
        }
    }
    else if (event.mPacket->getId() == PacketID::MobEquipment)
    {
        auto mp = event.getPacket<MobEquipmentPacket>();
        if (mp->mSlot == mToolSlot) mShouldSpoofSlot = false;
        else mShouldSpoofSlot = true;
    }
}

void Regen::onPacketInEvent(class PacketInEvent& event) {
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    if (event.mPacket->getId() == PacketID::LevelEvent) {
        auto levelEvent = event.getPacket<LevelEventPacket>();
        if (levelEvent->mEventId == 3600) {

            AABB eventBlockAABB = AABB(levelEvent->mPos, glm::vec3(1, 1, 1));
            glm::vec3 closestPos = eventBlockAABB.getClosestPoint(*player->getPos());
            if (HARDCODED_RANGE < glm::distance(closestPos, *player->getPos())) {
                return;
            }

            auto blockAtPos = ClientInstance::get()->getBlockSource()->getBlock(levelEvent->mPos);

            if (!blockAtPos->mLegacy->mSolid || blockAtPos->mLegacy->getBlockId() == 10099) {
                return;
            }

            if (BlockUtils::isMiningPosition(glm::ivec3(levelEvent->mPos))) return;

            int exposedFace = BlockUtils::getExposedFace(levelEvent->mPos);
            if (exposedFace != -1) {
                for (auto& offset : mOffsetList) {
                    glm::ivec3 blockPos = glm::ivec3(levelEvent->mPos) + offset;

                    AABB targetBlockAABB = AABB(blockPos, glm::vec3(1, 1, 1));
                    glm::vec3 targetClosestPos = targetBlockAABB.getClosestPoint(*player->getPos());
                    if (HARDCODED_RANGE < glm::distance(targetClosestPos, *player->getPos())) {
                        continue;
                    }

                    if (isValidBlock(blockPos, true, false) && BlockUtils::getExposedFace(blockPos) == -1) {
                        mEnemyTargettingBlockPos = blockPos;
                        mLastEnemyLayerBlockPos = levelEvent->mPos;
                        mCanSteal = true;
                        mLastStealerUpdate = NOW;
                    }
                }
            }

            glm::ivec3 pos = glm::ivec3(levelEvent->mPos);
            if (pos == mTargettingBlockPos && pos != mCurrentBlockPos && mIsMiningBlock && mIsUncovering) {
                if (!mAntiSteal.mValue) {
                    mBlackListedOrePos = pos;
                    if (mDebug.mValue && mStealNotify.mValue) ChatUtils::displayClientMessage("§eenemy tried to steal your ore, switching ores!");
                }
                mLastStealerDetected = NOW;
            }
        }
        else if (levelEvent->mEventId == 3601) {
            if (mCanSteal && glm::ivec3(levelEvent->mPos) == mLastEnemyLayerBlockPos) {
                mCanSteal = false;
            }
            if (glm::ivec3(levelEvent->mPos) == mBlackListedOrePos) {
                if (mAntiSteal.mValue) {
                    mBlackListedOrePos = { INT_MAX, INT_MAX, INT_MAX };
                }
            }
        }
    }
}

void Regen::onSendImmediateEvent(SendImmediateEvent& event)
{
    uint8_t packetId = event.send[0];
    if (packetId == 0)
    {
        uint64_t timestamp = *reinterpret_cast<uint64_t*>(&event.send[1]);
        uint64_t timestamp64 = _byteswap_uint64(timestamp);
        uint64_t now = NOW;
        mEventDelay = now - timestamp64;
    }
}

void Regen::onPingUpdateEvent(PingUpdateEvent& event)
{
    mPing = event.mPing - mEventDelay;
}