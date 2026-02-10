#include "Encaser.hpp"
#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Modules/Combat/Aura.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Inventory/PlayerInventory.hpp>
#include <SDK/Minecraft/World/Level.hpp>
#include <Features/Events/RenderEvent.hpp>
#include <SDK/Minecraft/Network/PacketID.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerAuthInputPacket.hpp>
#include <SDK/Minecraft/Network/Packets/MovePlayerPacket.hpp>
#include <Utils/GameUtils/ActorUtils.hpp>

// Initialize all static variables
std::string Encaser::mTextToBuild;
std::unordered_map<char, Encaser::CharacterPattern> Encaser::characterPatterns;
AABB Encaser::mTargetedAABB = AABB({0, 0, 0}, {0, 0, 0});
bool Encaser::mRotating = false;
bool Encaser::isTargetTrapped = false;
glm::vec3 Encaser::mBuildPos = {INT_MAX, INT_MAX, INT_MAX};
bool Encaser::mShouldRotate = false;
glm::vec3 Encaser::mLastBlock = {0, 0, 0};
int Encaser::mLastFace = 0;
float Encaser::mPlaceTimer = 0.f;
int64_t Encaser::mLastSwitchTime = 0;
int Encaser::placeIndex = 0;
glm::vec3 Encaser::mBuildStartPos = {INT_MAX, INT_MAX, INT_MAX};
bool Encaser::mBuildInProgress = false;
int Encaser::mLastSlot = -1;

void Encaser::onEnable() {
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &Encaser::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketOutEvent, &Encaser::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketInEvent, &Encaser::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->listen<RenderEvent, &Encaser::onRenderEvent>(this);
    isTargetTrapped = false;
    mBuildPos = {INT_MAX, INT_MAX, INT_MAX};
    mStoredBasePos = {INT_MAX, INT_MAX, INT_MAX};
    mLastSlot = -1;
    if (!mBuildInProgress) {
        placeIndex = 0;
        mBuildStartPos = {INT_MAX, INT_MAX, INT_MAX};
    }
}

void Encaser::onDisable() {
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &Encaser::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketOutEvent, &Encaser::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &Encaser::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->deafen<RenderEvent, &Encaser::onRenderEvent>(this);
    
    // Reset all position-related variables
    isTargetTrapped = false;
    mBuildPos = {INT_MAX, INT_MAX, INT_MAX};
    mStoredBasePos = {INT_MAX, INT_MAX, INT_MAX};
    mBuildStartPos = {INT_MAX, INT_MAX, INT_MAX};
    placeIndex = 0;
    mBuildInProgress = false;
    pattern.clear();
}

Actor* Encaser::getClosestActor() {
    auto player = ClientInstance::get()->getLocalPlayer();
    auto actors = ActorUtils::getActorList(true, true);

    if (!player || actors.empty()) return nullptr;

    Actor* closestActor = nullptr;
    float closestDistance = -1.f;

    for (auto actor : actors) {
        if (actor == player) continue;

        float distance = actor->distanceTo(player);
        if (closestDistance == -1.f || distance < closestDistance) {
            closestDistance = distance;
            closestActor = actor;
        }
    }

    if (!closestActor) return nullptr;
    if (closestDistance > mRange.mValue) return nullptr;

    return closestActor;
}

std::vector<glm::vec3> Encaser::getCollidingBlocks() {
    auto closestActor = getClosestActor();
    if (!closestActor) return {};

    // Get both position and AABB
    glm::vec3 pos = *closestActor->getPos();
    AABB aabb = closestActor->getAABB();
    
    // Get the fractional parts to check if entity is between blocks
    float fracX = pos.x - floor(pos.x);
    float fracZ = pos.z - floor(pos.z);

    // Get the base block position
    glm::vec3 basePos = {
        floor(pos.x),
        floor(pos.y),
        floor(pos.z)
    };

    std::vector<glm::vec3> blocks;
    blocks.push_back(basePos);

    if (fracX > 0.3f || (aabb.mMax.x - floor(aabb.mMax.x)) > 0.3f) {
        blocks.push_back({basePos.x + 1, basePos.y, basePos.z});
    }
    if (fracZ > 0.3f || (aabb.mMax.z - floor(aabb.mMax.z)) > 0.3f) {
        blocks.push_back({basePos.x, basePos.y, basePos.z + 1});
        // If we already added the X+1 block, also add the diagonal
        if (blocks.size() > 1) {
            blocks.push_back({basePos.x + 1, basePos.y, basePos.z + 1});
        }
    }

    return blocks;
}

std::vector<glm::vec3> Encaser::getSurroundingBlocks(bool airOnly) {
    auto collidingBlocks = getCollidingBlocks();
    std::vector<glm::vec3> surroundingBlocks;

    if (collidingBlocks.empty()) return surroundingBlocks;

    // Find the bounds of the area to surround
    glm::vec3 min = collidingBlocks[0];
    glm::vec3 max = collidingBlocks[0];
    
    for (const auto& block : collidingBlocks) {
        min.x = std::min(min.x, block.x);
        min.z = std::min(min.z, block.z);
        max.x = std::max(max.x, block.x);
        max.z = std::max(max.z, block.z);
    }

    // Add bottom layer walls
    for (float x = min.x - 1; x <= max.x + 1; x++) {
        surroundingBlocks.push_back({x, min.y, min.z - 1}); // Front
        surroundingBlocks.push_back({x, min.y, max.z + 1}); // Back
    }
    for (float z = min.z; z <= max.z; z++) {
        surroundingBlocks.push_back({min.x - 1, min.y, z}); // Left
        surroundingBlocks.push_back({max.x + 1, min.y, z}); // Right
    }

    // Add top layer walls
    for (float x = min.x - 1; x <= max.x + 1; x++) {
        surroundingBlocks.push_back({x, min.y + 1, min.z - 1}); // Front
        surroundingBlocks.push_back({x, min.y + 1, max.z + 1}); // Back
    }
    for (float z = min.z; z <= max.z; z++) {
        surroundingBlocks.push_back({min.x - 1, min.y + 1, z}); // Left
        surroundingBlocks.push_back({max.x + 1, min.y + 1, z}); // Right
    }

    // Add top cover
    for (float x = min.x; x <= max.x; x++) {
        for (float z = min.z; z <= max.z; z++) {
            surroundingBlocks.push_back({x, min.y + 2, z});
        }
    }

    if (airOnly) {
        // Keep track of failed positions to expand around them
        std::vector<glm::vec3> failedPositions;
        for (size_t i = 0; i < surroundingBlocks.size(); i++) {
            if (!BlockUtils::isAirBlock(surroundingBlocks[i]) || 
                !BlockUtils::isValidPlacePos(surroundingBlocks[i])) {
                failedPositions.push_back(surroundingBlocks[i]);
                surroundingBlocks.erase(surroundingBlocks.begin() + i);
                i--;
            }
        }

        // For each failed position, try to add blocks around it
        for (const auto& failedPos : failedPositions) {
            surroundingBlocks.push_back({failedPos.x + 1, failedPos.y, failedPos.z});
            surroundingBlocks.push_back({failedPos.x - 1, failedPos.y, failedPos.z});
            surroundingBlocks.push_back({failedPos.x, failedPos.y, failedPos.z + 1});
            surroundingBlocks.push_back({failedPos.x, failedPos.y, failedPos.z - 1});
        }

        // Remove duplicates and check air blocks again
        std::sort(surroundingBlocks.begin(), surroundingBlocks.end());
        surroundingBlocks.erase(std::unique(surroundingBlocks.begin(), surroundingBlocks.end()), surroundingBlocks.end());

        for (size_t i = 0; i < surroundingBlocks.size(); i++) {
            if (!BlockUtils::isAirBlock(surroundingBlocks[i]) || 
                !BlockUtils::isValidPlacePos(surroundingBlocks[i])) {
                surroundingBlocks.erase(surroundingBlocks.begin() + i);
                i--;
            }
        }
    }

    // Sort by Y level
    std::sort(surroundingBlocks.begin(), surroundingBlocks.end(),
        [](const glm::vec3& a, const glm::vec3& b) {
            return a.y < b.y;
        });

    return surroundingBlocks;
}

glm::vec3 Encaser::getNextPlacePos() {
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return {FLT_MAX, FLT_MAX, FLT_MAX};

    //if (mMode.mValue == Mode::Encase) {
    //    Actor* target = getClosestActor();
    //    if (!target) {
    //        placeIndex = 0;
    //        return {FLT_MAX, FLT_MAX, FLT_MAX};
    //    }

    //    // Get target's position and AABB
    //    glm::vec3 pos = *target->getPos();
    //    AABB targetAABB = target->getAABB();
    //    
    //    // Calculate base position (at feet level)
    //    glm::vec3 basePos = {
    //        floor(pos.x),
    //        floor(pos.y),
    //        floor(pos.z)
    //    };

    //    static std::vector<glm::vec3> originalPositions;
    //    static std::vector<glm::vec3> failedPositions;
    //    static std::vector<glm::vec3> attemptedPositions;
    //    static std::vector<glm::vec3> lastFailedPos;  // Track the last failed position

    //    // Helper function to check if a position intersects with target's position
    //    auto intersectsWithTarget = [&](const glm::vec3& checkPos) {
    //        // Check if any part of the entity is in this block space
    //        bool inX = pos.x >= checkPos.x && pos.x < checkPos.x + 1.0f;
    //        bool inZ = pos.z >= checkPos.z && pos.z < checkPos.z + 1.0f;
    //        bool inY = pos.y >= checkPos.y && pos.y < checkPos.y + 1.0f;
    //        
    //        return inX && inZ && inY;
    //    };

    //    // Helper function to check if a block exists (not air)
    //    auto hasBlock = [](const glm::vec3& pos) {
    //        return !BlockUtils::isAirBlock(pos);
    //    };

    //    // Helper function to add surrounding positions
    //    auto addSurroundingPositions = [&](const glm::vec3& center) {
    //        std::vector<glm::vec3> surrounding = {
    //            {center.x + 1, center.y, center.z},
    //            {center.x - 1, center.y, center.z},
    //            {center.x, center.y, center.z + 1},
    //            {center.x, center.y, center.z - 1}
    //        };

    //        for (const auto& pos : surrounding) {
    //            // Skip if we've already tried this position or if it has a block
    //            if (std::find(attemptedPositions.begin(), attemptedPositions.end(), pos) != attemptedPositions.end() ||
    //                hasBlock(pos)) {
    //                continue;
    //            }
    //            pattern.push_back(pos);
    //        }
    //    };

    //    // Reset patterns if we're starting fresh
    //    if (placeIndex == 0) {
    //        originalPositions.clear();
    //        failedPositions.clear();
    //        pattern.clear();
    //        attemptedPositions.clear();
    //        lastFailedPos.clear();

    //        // Get the fractional parts to check if entity is between blocks
    //        float fracX = pos.x - floor(pos.x);
    //        float fracZ = pos.z - floor(pos.z);
    //        
    //        // Determine if we need to expand based on position and hitbox
    //        bool expandX = (fracX > 0.3f) || (targetAABB.mMax.x - targetAABB.mMin.x > 0.8f);
    //        bool expandZ = (fracZ > 0.3f) || (targetAABB.mMax.z - targetAABB.mMin.z > 0.8f);

    //        // Calculate the height needed (exactly one block above entity)
    //        float height = targetAABB.mMax.y - targetAABB.mMin.y;
    //        int wallHeight = static_cast<int>(floor(height));  // This will be the exact entity height

    //        // First layer - Ground level walls
    //        pattern.push_back({basePos.x + 1, basePos.y, basePos.z});  // East wall
    //        pattern.push_back({basePos.x - 1, basePos.y, basePos.z});  // West wall
    //        pattern.push_back({basePos.x, basePos.y, basePos.z + 1});  // North wall
    //        pattern.push_back({basePos.x, basePos.y, basePos.z - 1});  // South wall

    //        // Second layer - Build walls first
    //        pattern.push_back({basePos.x + 1, basePos.y + 1, basePos.z});  // East wall
    //        pattern.push_back({basePos.x - 1, basePos.y + 1, basePos.z});  // West wall
    //        pattern.push_back({basePos.x, basePos.y + 1, basePos.z + 1});  // North wall
    //        pattern.push_back({basePos.x, basePos.y + 1, basePos.z - 1});  // South wall

    //        // Build roof - start from edges and work inward
    //        pattern.push_back({basePos.x, basePos.y + 1, basePos.z});      // Center roof block
    //        pattern.push_back({basePos.x - 1, basePos.y + 1, basePos.z + 1});      
    //        pattern.push_back({basePos.x + 1, basePos.y + 1, basePos.z});  // East roof
    //        pattern.push_back({basePos.x - 1, basePos.y + 1, basePos.z});  // West roof
    //        pattern.push_back({basePos.x, basePos.y + 1, basePos.z + 1});  // North roof
    //        pattern.push_back({basePos.x, basePos.y + 1, basePos.z - 1});  // South roof

    //        originalPositions = pattern;
    //    }

    //    // Check if we need to handle a failed placement from last attempt
    //    if (!lastFailedPos.empty()) {
    //        // Check if the block was actually placed
    //        if (!hasBlock(lastFailedPos[0])) {
    //            // Block wasn't placed, treat as failed and expand around it
    //            failedPositions.push_back(lastFailedPos[0]);
    //            addSurroundingPositions(lastFailedPos[0]);
    //        }
    //        lastFailedPos.clear();
    //    }

    //    // Try to place next block
    //    while (placeIndex < pattern.size()) {
    //        glm::vec3 placePos = pattern[placeIndex];
    //        placeIndex++;

    //        // Add to attempted positions
    //        attemptedPositions.push_back(placePos);

    //        // Skip if this position was already handled or has a block
    //        if (std::find(failedPositions.begin(), failedPositions.end(), placePos) != failedPositions.end() ||
    //            hasBlock(placePos)) {
    //            continue;
    //        }

    //        bool isBlocked = !BlockUtils::isValidPlacePos(placePos) || 
    //                        intersectsWithTarget(placePos);

    //        if (isBlocked) {
    //            // Add to failed positions and expand around it
    //            failedPositions.push_back(placePos);
    //            addSurroundingPositions(placePos);
    //            continue;
    //        }

    //        // Check for adjacent block support
    //        bool hasSupport = hasBlock({placePos.x - 1, placePos.y, placePos.z}) || 
    //                        hasBlock({placePos.x + 1, placePos.y, placePos.z}) ||
    //                        hasBlock({placePos.x, placePos.y, placePos.z - 1}) ||
    //                        hasBlock({placePos.x, placePos.y, placePos.z + 1}) ||
    //                        hasBlock({placePos.x, placePos.y - 1, placePos.z});

    //        if (!hasSupport) {
    //            // If no support, treat as blocked and expand around it
    //            failedPositions.push_back(placePos);
    //            addSurroundingPositions(placePos);
    //            continue;
    //        }

    //        // Store this position in case the placement fails
    //        lastFailedPos = {placePos};
    //        return placePos;
    //    }

    //    // If we've reached here, we've gone through all positions in the pattern
    //    // Count how many blocks are actually placed
    //    int placedBlocks = 0;
    //    int totalBlocks = pattern.size();
    //    
    //    for (const auto& pos : pattern) {
    //        if (hasBlock(pos) || std::find(failedPositions.begin(), failedPositions.end(), pos) != failedPositions.end()) {
    //            placedBlocks++;
    //        }
    //    }

    //    // Only reset and disable if we've placed all blocks or truly can't place any more
    //    if (placedBlocks == totalBlocks) {
    //        placeIndex = 0;
    //        pattern.clear();
    //        if (mToggle.mValue) {
    //            setEnabled(false);
    //        }
    //        return {FLT_MAX, FLT_MAX, FLT_MAX};
    //    }

    //    // If we haven't placed all blocks but ran out of current positions, reset index to try again
    //    if (placeIndex >= pattern.size()) {
    //        placeIndex = 0;
    //    }

    //    return {FLT_MAX, FLT_MAX, FLT_MAX};
    //} 
    //else 
    if (mMode.mValue == Mode::Threeline) {
        auto player = ClientInstance::get()->getLocalPlayer();
        if (!player) return {FLT_MAX, FLT_MAX, FLT_MAX};

        // Get player's position and look direction
        glm::vec3 pos = *player->getPos();
        float yaw = player->getActorRotationComponent()->mYaw;
        while (yaw > 180.0f) yaw -= 360.0f;
        while (yaw < -180.0f) yaw += 360.0f;

        // Convert yaw to direction
        int direction = (int)round(yaw / 90.0f);
        if (direction == -2) direction = 2;

        // Calculate or use stored base position
        if (placeIndex == 0) {
            mStoredBasePos = {
                floor(pos.x),
                floor(pos.y) - 1,  // Place at feet level
                floor(pos.z)
            };
        }

        // Use stored position instead of recalculating
        glm::vec3 basePos = mStoredBasePos;

        if (placeIndex == 0) {
            pattern.clear();
            switch(direction) {
                case 0:  // South (+Z)
                    // First layer: [ x ] [ x ] [ x ] [ _ ] [ x ]
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z + 2}); // Left
                    pattern.push_back({basePos.x - 1, basePos.y, basePos.z + 2}); // Left-center
                    pattern.push_back({basePos.x, basePos.y, basePos.z + 2});     // Center
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z + 2}); // Right
                    // Second layer: [ _ ] [ _ ] [ x ] [ _ ] [ x ]
                    pattern.push_back({basePos.x, basePos.y + 1, basePos.z + 2});     // Center
                    pattern.push_back({basePos.x + 2, basePos.y + 1, basePos.z + 2}); // Right
                    // Third layer: [ x ] [ x ] [ x ] [ x ] [ x ]
                    pattern.push_back({basePos.x - 2, basePos.y + 2, basePos.z + 2}); // Left
                    pattern.push_back({basePos.x - 1, basePos.y + 2, basePos.z + 2}); // Left-center
                    pattern.push_back({basePos.x, basePos.y + 2, basePos.z + 2});     // Center
                    pattern.push_back({basePos.x + 1, basePos.y + 2, basePos.z + 2}); // Right-center
                    pattern.push_back({basePos.x + 2, basePos.y + 2, basePos.z + 2}); // Right
                    // Fourth layer: [ x ] [ _ ] [ x ] [ _ ] [ _ ]
                    pattern.push_back({basePos.x - 2, basePos.y + 3, basePos.z + 2}); // Left
                    pattern.push_back({basePos.x, basePos.y + 3, basePos.z + 2});     // Center
                    // Fifth layer: [ x ] [ _ ] [ x ] [ x ] [ x ]
                    pattern.push_back({basePos.x - 2, basePos.y + 4, basePos.z + 2}); // Left
                    pattern.push_back({basePos.x, basePos.y + 4, basePos.z + 2});     // Center
                    pattern.push_back({basePos.x + 1, basePos.y + 4, basePos.z + 2}); // Right-center
                    pattern.push_back({basePos.x + 2, basePos.y + 4, basePos.z + 2}); // Right
                    break;
                case 1:  // West (-X)
                    // First layer: [ x ] [ x ] [ x ] [ _ ] [ x ]
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z - 2}); // Left
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z - 1}); // Left-center
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z});     // Center
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z + 2}); // Right
                    // Second layer: [ _ ] [ _ ] [ x ] [ _ ] [ x ]
                    pattern.push_back({basePos.x - 2, basePos.y + 1, basePos.z});     // Center
                    pattern.push_back({basePos.x - 2, basePos.y + 1, basePos.z + 2}); // Right
                    // Third layer: [ x ] [ x ] [ x ] [ x ] [ x ]
                    pattern.push_back({basePos.x - 2, basePos.y + 2, basePos.z - 2}); // Left
                    pattern.push_back({basePos.x - 2, basePos.y + 2, basePos.z - 1}); // Left-center
                    pattern.push_back({basePos.x - 2, basePos.y + 2, basePos.z});     // Center
                    pattern.push_back({basePos.x - 2, basePos.y + 2, basePos.z + 1}); // Right-center
                    pattern.push_back({basePos.x - 2, basePos.y + 2, basePos.z + 2}); // Right
                    // Fourth layer: [ x ] [ _ ] [ x ] [ _ ] [ _ ]
                    pattern.push_back({basePos.x - 2, basePos.y + 3, basePos.z - 2}); // Left
                    pattern.push_back({basePos.x - 2, basePos.y + 3, basePos.z});     // Center
                    // Fifth layer: [ x ] [ _ ] [ x ] [ x ] [ x ]
                    pattern.push_back({basePos.x - 2, basePos.y + 4, basePos.z - 2}); // Left
                    pattern.push_back({basePos.x - 2, basePos.y + 4, basePos.z});     // Center
                    pattern.push_back({basePos.x - 2, basePos.y + 4, basePos.z + 1}); // Right-center
                    pattern.push_back({basePos.x - 2, basePos.y + 4, basePos.z + 2}); // Right
                    break;
                case 2:  // North (-Z)
                    // First layer: [ x ] [ x ] [ x ] [ _ ] [ x ]
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z - 2}); // Left
                    pattern.push_back({basePos.x + 1, basePos.y, basePos.z - 2}); // Left-center
                    pattern.push_back({basePos.x, basePos.y, basePos.z - 2});     // Center
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z - 2}); // Right
                    // Second layer: [ _ ] [ _ ] [ x ] [ _ ] [ x ]
                    pattern.push_back({basePos.x, basePos.y + 1, basePos.z - 2});     // Center
                    pattern.push_back({basePos.x - 2, basePos.y + 1, basePos.z - 2}); // Right
                    // Third layer: [ x ] [ x ] [ x ] [ x ] [ x ]
                    pattern.push_back({basePos.x + 2, basePos.y + 2, basePos.z - 2}); // Left
                    pattern.push_back({basePos.x + 1, basePos.y + 2, basePos.z - 2}); // Left-center
                    pattern.push_back({basePos.x, basePos.y + 2, basePos.z - 2});     // Center
                    pattern.push_back({basePos.x - 1, basePos.y + 2, basePos.z - 2}); // Right-center
                    pattern.push_back({basePos.x - 2, basePos.y + 2, basePos.z - 2}); // Right
                    // Fourth layer: [ x ] [ _ ] [ x ] [ _ ] [ _ ]
                    pattern.push_back({basePos.x + 2, basePos.y + 3, basePos.z - 2}); // Left
                    pattern.push_back({basePos.x, basePos.y + 3, basePos.z - 2});     // Center
                    // Fifth layer: [ x ] [ _ ] [ x ] [ x ] [ x ]
                    pattern.push_back({basePos.x + 2, basePos.y + 4, basePos.z - 2}); // Left
                    pattern.push_back({basePos.x, basePos.y + 4, basePos.z - 2});     // Center
                    pattern.push_back({basePos.x - 1, basePos.y + 4, basePos.z - 2}); // Right-center
                    pattern.push_back({basePos.x - 2, basePos.y + 4, basePos.z - 2}); // Right
                    break;
                case -1:  // East (+X)
                    // First layer: [ x ] [ x ] [ x ] [ _ ] [ x ]
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z + 2}); // Left
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z + 1}); // Left-center
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z});     // Center
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z - 2}); // Right
                    // Second layer: [ _ ] [ _ ] [ x ] [ _ ] [ x ]
                    pattern.push_back({basePos.x + 2, basePos.y + 1, basePos.z});     // Center
                    pattern.push_back({basePos.x + 2, basePos.y + 1, basePos.z - 2}); // Right
                    // Third layer: [ x ] [ x ] [ x ] [ x ] [ x ]
                    pattern.push_back({basePos.x + 2, basePos.y + 2, basePos.z + 2}); // Left
                    pattern.push_back({basePos.x + 2, basePos.y + 2, basePos.z + 1}); // Left-center
                    pattern.push_back({basePos.x + 2, basePos.y + 2, basePos.z});     // Center
                    pattern.push_back({basePos.x + 2, basePos.y + 2, basePos.z - 1}); // Right-center
                    pattern.push_back({basePos.x + 2, basePos.y + 2, basePos.z - 2}); // Right
                    // Fourth layer: [ x ] [ _ ] [ x ] [ _ ] [ _ ]
                    pattern.push_back({basePos.x + 2, basePos.y + 3, basePos.z + 2}); // Left
                    pattern.push_back({basePos.x + 2, basePos.y + 3, basePos.z});     // Center
                    // Fifth layer: [ x ] [ _ ] [ x ] [ x ] [ x ]
                    pattern.push_back({basePos.x + 2, basePos.y + 4, basePos.z + 2}); // Left
                    pattern.push_back({basePos.x + 2, basePos.y + 4, basePos.z});     // Center
                    pattern.push_back({basePos.x + 2, basePos.y + 4, basePos.z - 1}); // Right-center
                    pattern.push_back({basePos.x + 2, basePos.y + 4, basePos.z - 2}); // Right
                    break;
            }
        }

        while (placeIndex < pattern.size()) {
            glm::vec3 placePos = pattern[placeIndex];
            placeIndex++;

            if (!BlockUtils::isAirBlock(placePos)) continue;
            if (!BlockUtils::isValidPlacePos(placePos)) continue;

            return placePos;
        }

        // Count how many blocks are actually placed
        int placedBlocks = 0;
        int totalBlocks = pattern.size();
        
        for (const auto& pos : pattern) {
            if (!BlockUtils::isAirBlock(pos)) {
                placedBlocks++;
            }
        }

        // Only reset and disable if we've placed all blocks
        if (placedBlocks == totalBlocks) {
            placeIndex = 0;
            pattern.clear();
            if (mToggle.mValue) {
                setEnabled(false);
            }
            return {FLT_MAX, FLT_MAX, FLT_MAX};
        }

        // If we haven't placed all blocks but ran out of current positions, reset index to try again
        if (placeIndex >= pattern.size()) {
            placeIndex = 0;
        }

        return {FLT_MAX, FLT_MAX, FLT_MAX};
    } else if (mMode.mValue == Mode::Penis) {
        auto player = ClientInstance::get()->getLocalPlayer();
        if (!player) return {FLT_MAX, FLT_MAX, FLT_MAX};

        // Get player's position and look direction
        glm::vec3 pos = *player->getPos();
        float yaw = player->getActorRotationComponent()->mYaw;
        while (yaw > 180.0f) yaw -= 360.0f;
        while (yaw < -180.0f) yaw += 360.0f;

        // Convert yaw to direction
        int direction = (int)round(yaw / 90.0f);
        if (direction == -2) direction = 2;

        // Calculate or use stored base position
        if (placeIndex == 0) {
            mStoredBasePos = {
                floor(pos.x),
                floor(pos.y) - 1,  // Place at feet level
                floor(pos.z)
            };
        }

        // Use stored position instead of recalculating
        glm::vec3 basePos = mStoredBasePos;

        if (placeIndex == 0) {
            pattern.clear();
            switch(direction) {
                case 0:  // South
                    // Bottom layer (3 blocks)
                    pattern.push_back({basePos.x - 1, basePos.y, basePos.z + 2});
                    pattern.push_back({basePos.x, basePos.y, basePos.z + 2});
                    pattern.push_back({basePos.x + 1, basePos.y, basePos.z + 2});
                    // Middle blocks
                    pattern.push_back({basePos.x, basePos.y + 1, basePos.z + 2});
                    pattern.push_back({basePos.x, basePos.y + 2, basePos.z + 2});
                    break;
                case 1:  // West
                    // Bottom layer (3 blocks)
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z - 1});
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z});
                    pattern.push_back({basePos.x - 2, basePos.y, basePos.z + 1});
                    // Middle blocks
                    pattern.push_back({basePos.x - 2, basePos.y + 1, basePos.z});
                    pattern.push_back({basePos.x - 2, basePos.y + 2, basePos.z});
                    break;
                case 2:  // North
                    // Bottom layer (3 blocks)
                    pattern.push_back({basePos.x - 1, basePos.y, basePos.z - 2});
                    pattern.push_back({basePos.x, basePos.y, basePos.z - 2});
                    pattern.push_back({basePos.x + 1, basePos.y, basePos.z - 2});
                    // Middle blocks
                    pattern.push_back({basePos.x, basePos.y + 1, basePos.z - 2});
                    pattern.push_back({basePos.x, basePos.y + 2, basePos.z - 2});
                    break;
                case -1:  // East
                    // Bottom layer (3 blocks)
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z - 1});
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z});
                    pattern.push_back({basePos.x + 2, basePos.y, basePos.z + 1});
                    // Middle blocks
                    pattern.push_back({basePos.x + 2, basePos.y + 1, basePos.z});
                    pattern.push_back({basePos.x + 2, basePos.y + 2, basePos.z});
                    break;
            }
        }

        while (placeIndex < pattern.size()) {
            glm::vec3 placePos = pattern[placeIndex];
            placeIndex++;

            if (!BlockUtils::isAirBlock(placePos)) continue;
            if (!BlockUtils::isValidPlacePos(placePos)) continue;

            return placePos;
        }

        int placedBlocks = 0;
        int totalBlocks = pattern.size();
        
        for (const auto& pos : pattern) {
            if (!BlockUtils::isAirBlock(pos)) {
                placedBlocks++;
            }
        }

        if (placedBlocks == totalBlocks) {
            placeIndex = 0;
            pattern.clear();
            if (mToggle.mValue) {
                setEnabled(false);
            }
            return {FLT_MAX, FLT_MAX, FLT_MAX};
        }

        // If we haven't placed all blocks but ran out of current positions, reset index to try again
        if (placeIndex >= pattern.size()) {
            placeIndex = 0;
        }

        return {FLT_MAX, FLT_MAX, FLT_MAX};
    } else if (mMode.mValue == Mode::Text) {
        auto player = ClientInstance::get()->getLocalPlayer();
        if (!player) return {FLT_MAX, FLT_MAX, FLT_MAX};

        // Get player's position and look direction
        glm::vec3 pos = *player->getPos();
        float yaw = player->getActorRotationComponent()->mYaw;
        while (yaw > 180.0f) yaw -= 360.0f;
        while (yaw < -180.0f) yaw += 360.0f;

        // Convert yaw to direction
        int direction = (int)round(yaw / 90.0f);
        if (direction == -2) direction = 2;

        // If we're not building, initialize the build
        if (!mBuildInProgress) {
            mStoredBasePos = {
                floor(pos.x),
                floor(pos.y) - 1,  // Place at feet level
                floor(pos.z)
            };
            
            // Calculate start position based on direction
            glm::vec3 startPos;
            switch(direction) {
                case 0:  // South (+Z)
                    startPos = {mStoredBasePos.x - (mTextToBuild.length() * 3) / 2, mStoredBasePos.y, mStoredBasePos.z + 2};
                    break;
                case 1:  // West (-X)
                    startPos = {mStoredBasePos.x - 2, mStoredBasePos.y, mStoredBasePos.z - (mTextToBuild.length() * 3) / 2};
                    break;
                case 2:  // North (-Z)
                    startPos = {mStoredBasePos.x - (mTextToBuild.length() * 3) / 2, mStoredBasePos.y, mStoredBasePos.z - 2};
                    break;
                case -1:  // East (+X)
                    startPos = {mStoredBasePos.x + 2, mStoredBasePos.y, mStoredBasePos.z - (mTextToBuild.length() * 3) / 2};
                    break;
            }

            mBuildStartPos = startPos;
            textToPattern(mTextToBuild, startPos);
            mBuildInProgress = true;
            placeIndex = 0;
        }

        // Check if any remaining blocks are in range
        bool anyInRange = false;
        for (size_t i = placeIndex; i < pattern.size(); i++) {
            if (isInRange(pattern[i])) {
                anyInRange = true;
                break;
            }
        }

        if (!anyInRange) {
            return {FLT_MAX, FLT_MAX, FLT_MAX};  // Wait for player to get in range
        }

        while (placeIndex < pattern.size()) {
            glm::vec3 placePos = pattern[placeIndex];
            
            // Skip if not in range
            if (!isInRange(placePos)) {
                placeIndex++;
                continue;
            }

            if (!BlockUtils::isAirBlock(placePos) || !BlockUtils::isValidPlacePos(placePos)) {
                placeIndex++;
                continue;
            }

            return placePos;
        }

        // Count how many blocks are actually placed
        int placedBlocks = 0;
        int totalBlocks = pattern.size();
        
        for (const auto& pos : pattern) {
            if (!BlockUtils::isAirBlock(pos)) {
                placedBlocks++;
            }
        }

        // Only reset and disable if we've placed all blocks
        if (placedBlocks == totalBlocks) {
            placeIndex = 0;
            pattern.clear();
            mBuildInProgress = false;
            mBuildStartPos = {INT_MAX, INT_MAX, INT_MAX};
            if (mToggle.mValue) {
                setEnabled(false);
            }
            return {FLT_MAX, FLT_MAX, FLT_MAX};
        }

        // If we haven't placed all blocks but ran out of current positions, reset index to try again
        if (placeIndex >= pattern.size()) {
            placeIndex = 0;
        }

        return {FLT_MAX, FLT_MAX, FLT_MAX};
    }
    
    // Keep other modes as they are...
    return {FLT_MAX, FLT_MAX, FLT_MAX};
}

void Encaser::onBaseTickEvent(BaseTickEvent& event) {
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    // Update place timer
    mPlaceTimer += 1.0f;
    float placeDelay = 1.0f / mBlocksPerTick.mValue;
    
    if (mPlaceTimer < placeDelay) return;
    mPlaceTimer = 0.f;

    // Get next position to place
    glm::vec3 placePos = getNextPlacePos();
    if (placePos.x == FLT_MAX) return;  // No valid position found

    // Try to place block
    int face = BlockUtils::getBlockPlaceFace(placePos);
    if (face != -1) {
        if (mLastSlot == -1) mLastSlot = player->getSupplies()->mSelectedSlot;
        int lastSlot = player->getSupplies()->mSelectedSlot;

        if (mSwitchMode.mValue != SwitchMode::None) {
            int slot = ItemUtils::getPlaceableItemOnBlock(placePos, true, false);
            if (slot == -1) return;  // Changed from returning vec3 to just returning
            
            if (mSwitchMode.mValue == SwitchMode::Full) {
                player->getSupplies()->mSelectedSlot = slot;
            } else if (mSwitchMode.mValue == SwitchMode::Spoof) {
                player->getSupplies()->mSelectedSlot = slot;
                PacketUtils::spoofSlot(slot);
            }
        }

        mLastBlock = placePos;
        mLastFace = face;
        mShouldRotate = true;

        BlockUtils::placeBlock(placePos, face);

        if (mSwitchMode.mValue == SwitchMode::Spoof) {
            player->getSupplies()->mSelectedSlot = lastSlot;
            PacketUtils::spoofSlot(lastSlot);
        }

        player->swing();
    }
}

void Encaser::onPacketOutEvent(PacketOutEvent& event) {
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    // Add click position randomization
    if (event.mPacket->getId() == PacketID::InventoryTransaction) {
        if (const auto it = event.getPacket<InventoryTransactionPacket>(); it->mTransaction->type ==
            ComplexInventoryTransaction::Type::ItemUseTransaction) {
            const auto transac = reinterpret_cast<ItemUseInventoryTransaction*>(it->mTransaction.get());
            if (transac->mActionType == ItemUseInventoryTransaction::ActionType::Place) {
                transac->mClickPos = BlockUtils::clickPosOffsets[transac->mFace];
                for (int i = 0; i < 3; i++) {
                    if (transac->mClickPos[i] == 0.5) {
                        transac->mClickPos[i] = MathUtils::randomFloat(-0.49f, 0.49f);
                    }
                }
            }
        }
    }

    if (event.mPacket->getId() == PacketID::PlayerAuthInput) {
        auto paip = event.getPacket<PlayerAuthInputPacket>();

        // Add ground spoof
        paip->mPos.y -= 0.01f;

        if (!mRotate.mValue || !mShouldRotate) return;

        glm::vec3 side = BlockUtils::blockFaceOffsets[mLastFace] * 0.5f;
        glm::vec3 target = mLastBlock + side;
        auto pos = *player->getPos();
        
        glm::vec2 rotations = MathUtils::getRots(pos, target);
        
        // Normalize yaw to -180/180 range
        while (rotations.y > 180.0f) rotations.y -= 360.0f;
        while (rotations.y < -180.0f) rotations.y += 360.0f;

        // Add combat integration
        auto auraMod = gFeatureManager->mModuleManager->getModule<Aura>();
        if (auraMod->sHasTarget) {
            mShouldRotate = false;
            return;
        }
        
        paip->mRot = rotations;
        paip->mYHeadRot = rotations.y;
        
        auto rotComp = player->getActorRotationComponent();
        rotComp->mPitch = rotations.x;
        rotComp->mYaw = rotations.y;
    }
}

void Encaser::onPacketInEvent(PacketInEvent& event) {
    if (event.mPacket->getId() == PacketID::PlayerAuthInput) {
        mShouldRotate = false;
    }
}

void Encaser::onRenderEvent(RenderEvent& event) {
    //if (!mEnabled || !mDebug.mValue) return;
	if (mEnabled) return;
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    
    return;

    // Get target and check validity
    Actor* target = getClosestActor();
    if (!target) return;

    // Get target's position and AABB
    glm::vec3 pos = *target->getPos();
    AABB aabb = target->getAABB();
    
    // Calculate entity dimensions
    float width = aabb.mMax.x - aabb.mMin.x;
    float height = aabb.mMax.y - aabb.mMin.y;
    
    // Get fractional parts to check if entity is between blocks
    float fracX = pos.x - floor(pos.x);
    float fracZ = pos.z - floor(pos.z);
    
    // Determine if we need to expand the box based on position and size
    bool expandX = (fracX > 0.3f) || (width > 0.8f);
    bool expandZ = (fracZ > 0.3f) || (width > 0.8f);
    
    // Calculate base position (at feet level)
    glm::vec3 basePos = {
        floor(pos.x),
        floor(pos.y),
        floor(pos.z)
    };

    // Create placement pattern
    std::vector<glm::vec3> pattern;
    
    // Calculate box dimensions
    int boxWidth = 1;
    if (expandX) boxWidth++;
    if (expandZ) boxWidth++;
    
    float wallHeight = ceil(height);
    float topY = basePos.y + wallHeight;

    // Generate pattern same as in getNextPlacePos
    // Bottom layer
    if (expandX && expandZ) {
        pattern.push_back(basePos);
        pattern.push_back({basePos.x + 1, basePos.y, basePos.z});
        pattern.push_back({basePos.x, basePos.y, basePos.z + 1});
        pattern.push_back({basePos.x + 1, basePos.y, basePos.z + 1});
    } else if (expandX) {
        pattern.push_back(basePos);
        pattern.push_back({basePos.x + 1, basePos.y, basePos.z});
    } else if (expandZ) {
        pattern.push_back(basePos);
        pattern.push_back({basePos.x, basePos.y, basePos.z + 1});
    } else {
        pattern.push_back(basePos);
    }

    // Middle layers
    for (int y = 1; y < wallHeight; y++) {
        if (expandX && expandZ) {
            pattern.push_back({basePos.x, basePos.y + y, basePos.z});
            pattern.push_back({basePos.x + 1, basePos.y + y, basePos.z});
            pattern.push_back({basePos.x, basePos.y + y, basePos.z + 1});
            pattern.push_back({basePos.x + 1, basePos.y + y, basePos.z + 1});
        } else if (expandX) {
            pattern.push_back({basePos.x, basePos.y + y, basePos.z});
            pattern.push_back({basePos.x + 1, basePos.y + y, basePos.z});
        } else if (expandZ) {
            pattern.push_back({basePos.x, basePos.y + y, basePos.z});
            pattern.push_back({basePos.x, basePos.y + y, basePos.z + 1});
        } else {
            pattern.push_back({basePos.x, basePos.y + y, basePos.z});
        }
    }

    // Top layer
    if (expandX && expandZ) {
        pattern.push_back({basePos.x, topY, basePos.z});
        pattern.push_back({basePos.x + 1, topY, basePos.z});
        pattern.push_back({basePos.x, topY, basePos.z + 1});
        pattern.push_back({basePos.x + 1, topY, basePos.z + 1});
    } else if (expandX) {
        pattern.push_back({basePos.x, topY, basePos.z});
        pattern.push_back({basePos.x + 1, topY, basePos.z});
    } else if (expandZ) {
        pattern.push_back({basePos.x, topY, basePos.z});
        pattern.push_back({basePos.x, topY, basePos.z + 1});
    } else {
        pattern.push_back({basePos.x, topY, basePos.z});
    }

    // Render all potential placement positions
    for (size_t i = 0; i < pattern.size(); i++) {
        const auto& pos = pattern[i];
        auto boxAABB = AABB(pos, pos + glm::vec3(1.f, 1.f, 1.f));
        
        ImColor color;
        if (BlockUtils::isValidPlacePos(pos) && BlockUtils::isAirBlock(pos)) {
            if (i == placeIndex) {
                color = ImColor(0, 255, 255, 200);  // Cyan for next placement
            } else {
                color = ImColor(0, 255, 0, 128);  // Green for valid positions
            }
        } else {
            color = ImColor(255, 0, 0, 128);  // Red for invalid positions
        }
        
        RenderUtils::drawOutlinedAABB(boxAABB, true, color);
    }

    // Render target's AABB
    RenderUtils::drawOutlinedAABB(aabb, true, ImColor(255, 255, 0, 200));  // Yellow for target
}
