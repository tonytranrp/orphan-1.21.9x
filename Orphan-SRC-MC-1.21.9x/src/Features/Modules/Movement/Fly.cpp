#include "Fly.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <Features/Events/RenderEvent.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/MinecraftSim.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Actor/Components/StateVectorComponent.hpp>
#include <SDK/Minecraft/Actor/Components/ActorRotationComponent.hpp>
#include <SDK/Minecraft/World/Level.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerAuthInputPacket.hpp>
#include <SDK/Minecraft/Network/Packets/SetActorMotionPacket.hpp>

class PlayerAuthInputPacket;

void Fly::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &Fly::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketOutEvent, &Fly::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->listen<RenderEvent, &Fly::onRenderEvent>(this);

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    if (mMode.mValue == Mode::Flareon)
    {

        mDistanceTraveled = 0.f;
        mLastPos = *player->getPos();
        mNetSkipEnabled = true;
        mCurrentStage = 1;
        mOriginalY = player->getPos()->y;
        mDebugPositions.clear();

        if (mKeepMotion.mValue)
        {
            mInitialMotion = player->getStateVectorComponent()->mVelocity;
            if (mDebug.mValue)
                displayDebug("Stored initial motion: " + std::to_string(mInitialMotion.x) + ", " +
                    std::to_string(mInitialMotion.y) + ", " + std::to_string(mInitialMotion.z));
        }

        if (mDebug.mValue)
        {
            displayDebug("Flareon mode enabled - Waiting for clip distance");
        }
        return;
    }

    if (mMode.mValue != Mode::Jump) return;
    mCurrentY = player->getPos()->y;
    mLastJump = NOW;
    displayDebug("Jump fly enabled");
}

void Fly::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &Fly::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketOutEvent, &Fly::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->deafen<RenderEvent, &Fly::onRenderEvent>(this);
    if (mTimerBoost.mValue) ClientInstance::get()->getMinecraftSim()->setSimTimer(20);

    auto player = ClientInstance::get()->getLocalPlayer();
    if (player && mMode.mValue == Mode::Flareon)
    {

        glm::vec3 currentPos = *player->getPos();
        currentPos.y = mOriginalY;
        player->setPosition(currentPos);

        if (mKeepMotion.mValue)
        {

            player->getStateVectorComponent()->mVelocity = mInitialMotion;
            if (mDebug.mValue)
                displayDebug("Restored initial motion");
        }
        else
        {

            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity *= 0.1f;

        }

        if (mDebug.mValue)
        {
            displayDebug("Disabled - Returned to original Y position" + std::string(mKeepMotion.mValue ? " and kept initial motion" : " with safe landing"));
        }
        return;
    }

    if (mMode.mValue != Mode::Jump) return;

    displayDebug("Jump fly disabled");
}

void Fly::displayDebug(const std::string& message) const
{
    if (mDebug.mValue)
    {
        ChatUtils::displayClientMessageSub("§6Fly", message);
        spdlog::debug("[Fly] {}", message);
    }
}

void Fly::onBaseTickEvent(BaseTickEvent& event)
{
    static bool setLast = false;
    bool applyTimer = true;

    auto player = event.mActor;
    if (mMode.mValue == Mode::Motion || mMode.mValue == Mode::Elytra || mMode.mValue == Mode::Flareon)
    {
        glm::vec3 motion = glm::vec3(0, 0, 0);

        if (Keyboard::isUsingMoveKeys(true))
        {
            glm::vec2 calc = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, mSpeed.mValue / 10);
            motion.x = calc.x;
            motion.z = calc.y;

            bool isJumping = player->getMoveInputComponent()->mIsJumping;
            bool isSneaking = player->getMoveInputComponent()->mIsSneakDown;

            if (isJumping)
                motion.y += mSpeed.mValue / 10;
            else if (isSneaking)
                motion.y -= mSpeed.mValue / 10;
        }

        player->getStateVectorComponent()->mVelocity = motion;
    }
    else if (mMode.mValue == Mode::Jump)
    {
        applyTimer = tickJump(player);
    }

    if (mTimerBoost.mValue && applyTimer)
    {
        setLast = true;
        ClientInstance::get()->getMinecraftSim()->setSimTimer(mTimerBoostValue.mValue);
    }
    else if (!mTimerBoost.mValue && setLast || !applyTimer)
    {
        setLast = false;
        ClientInstance::get()->getMinecraftSim()->setSimTimer(20);
    }
}

bool Fly::tickJump(Actor* player)
{
    static int flyTicks = 0;
    static bool wasFlying = false;

    if (mDamageOnly.mValue && mLastDamage < NOW)
    {
        if (wasFlying)
        {
            mCurrentY = player->getPos()->y;
            if (mResetOnDisable.mValue)
                player->getStateVectorComponent()->mVelocity = glm::vec3(0.f, player->getStateVectorComponent()->mVelocity.y, 0.f);

            wasFlying = false;
        }

        mCurrentFriction = 1.f;
        flyTicks = 0;
        return false;
    }

    wasFlying = true;

    if (player->isOnGround())
    {
        displayDebug("Resetting friction [OnGround]");
        mCurrentFriction = 1.f;
    }
    else if (mSpeedFriction.mValue)
    {
        mCurrentFriction *= mFriction.mValue;

        glm::vec2 motion = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, (mSpeed.mValue * mCurrentFriction) / 10);
        player->getStateVectorComponent()->mVelocity = glm::vec3(motion.x, player->getStateVectorComponent()->mVelocity.y, motion.y);
    }

    if (player->getPos()->y < mCurrentY && !player->isOnGround())
    {
        if (NOW - mLastJump > static_cast<uint64_t>(mJumpDelay.mValue) * 1000)
        {
            jump();
            mLastJump = NOW;
        }

        displayDebug("Height: " + std::to_string(player->getPos()->y - mCurrentY));
        mCurrentY -= mHeightLoss.mValue;
    }

    if (player->isOnGround() && mResetOnGround.mValue)
    {
        mCurrentY = player->getPos()->y;
        displayDebug("Resetting height [OnGround]");
    }

    flyTicks++;
    return true;
}

void Fly::jump()
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (player == nullptr)
        return;

    bool onGround = player->isOnGround();
    player->setOnGround(true);
    player->jumpFromGround();
    player->setOnGround(onGround);

    glm::vec2 motion = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, mSpeed.mValue / 10);
    player->getStateVectorComponent()->mVelocity = glm::vec3(motion.x, player->getStateVectorComponent()->mVelocity.y, motion.y);

    displayDebug("Jumping");
    mCurrentFriction = 1.f;
    displayDebug("Resetting friction [Jump]");
}

void Fly::onPacketOutEvent(PacketOutEvent& event)
{
    if (event.mPacket->getId() == PacketID::PlayerAuthInput)
    {
        auto player = ClientInstance::get()->getLocalPlayer();
        if (player == nullptr)
            return;

        auto packet = event.getPacket<PlayerAuthInputPacket>();

        if (mMode.mValue == Mode::Flareon)
        {
            glm::vec3 currentPos = *player->getPos();

            float deltaX = currentPos.x - mLastPos.x;
            float deltaZ = currentPos.z - mLastPos.z;
            float horizontalDistance = sqrt(deltaX * deltaX + deltaZ * deltaZ);

            mDistanceTraveled += horizontalDistance;

            if (mPacketAllowTicks > 0)
            {
                mPacketAllowTicks--;
                if (mPacketAllowTicks == 0)
                {
                    mNetSkipEnabled = true;
                    if (mDebug.mValue)
                        displayDebug("Re-enabling packet delays");
                }
            }

            if (mDistanceTraveled >= static_cast<float>(mClipDistance.mValue))
            {
                auto stateVector = player->getStateVectorComponent();

                switch (mCurrentStage) {
                case 1:
                    stateVector->mVelocity.y = static_cast<float>(mClipDownAmount.mValue);
                    mCurrentStage = 2;
                    mNetSkipEnabled = true;
                    mPacketAllowTicks = 1;
                    if (mDebug.mValue)
                        displayDebug("Stage 1: Down");
                    break;

                case 2:
                    stateVector->mVelocity.y = static_cast<float>(mClipUpAmount.mValue);
                    mCurrentStage = 3;
                    mNetSkipEnabled = false;
                    mPacketAllowTicks = 1;
                    if (mDebug.mValue)
                        displayDebug("Stage 2: Up");
                    break;

                case 3:
                    mCurrentStage = 1;
                    if (mDebug.mValue)
                        displayDebug("Stage 3: Reset");
                    break;
                }

                if (mDebug.mValue)
                {
                    mDebugPositions.push_back(currentPos);
                    if (mDebugPositions.size() > 10)
                        mDebugPositions.erase(mDebugPositions.begin());
                }

                mDistanceTraveled = 0.f;
            }
            event.mCancelled = true;

            if (!event.mCancelled && mEnabled && Keyboard::isUsingMoveKeys(true))
            {
                glm::vec2 calc = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, mSpeed.mValue / 10);
                packet->mPos.x += calc.x;
                packet->mPos.z += calc.y;

                bool isJumping = player->getMoveInputComponent()->mIsJumping;
                bool isSneaking = player->getMoveInputComponent()->mIsSneakDown;

                if (isJumping)
                    packet->mPos.y += mSpeed.mValue / 10;
                else if (isSneaking)
                    packet->mPos.y -= mSpeed.mValue / 10;
            }

            mLastPos = currentPos;
            return;
        }

        if (mMode.mValue == Mode::Motion && mApplyGlideFlags.mValue)
        {
            packet->mInputData |= AuthInputAction::START_GLIDING;
            packet->mInputData &= ~AuthInputAction::STOP_GLIDING;
        }
        if (mMode.mValue == Mode::Elytra)
        {
            static bool alternating = false;
            alternating = !alternating;
            packet->mInputData |= AuthInputAction::START_GLIDING | AuthInputAction::ASCEND | AuthInputAction::WANT_UP | AuthInputAction::STOP_GLIDING;

            if (alternating)
                packet->mInputData |= AuthInputAction::JUMPING | AuthInputAction::START_JUMPING | AuthInputAction::JUMP_DOWN;
            packet->mInputData &= ~AuthInputAction::DESCEND | AuthInputAction::WANT_DOWN | AuthInputAction::SNEAKING | AuthInputAction::SNEAK_TOGGLE_DOWN | AuthInputAction::START_SNEAKING;
        }
    }
}

void Fly::onPacketInEvent(PacketInEvent& event)
{
    if (mMode.mValue != Mode::Jump) return;

    if (event.mPacket->getId() == PacketID::SetActorMotion)
    {
        auto player = ClientInstance::get()->getLocalPlayer();
        auto sem = event.getPacket<SetActorMotionPacket>();
        if (sem->mRuntimeID == player->getRuntimeID())
        {
            mLastDamage = NOW + static_cast<uintptr_t>(mFlyTime.mValue) * 1000;
            mCurrentY = player->getPos()->y;
            if (mEnabled) displayDebug("Damage taken");
        }
    }

}

void Fly::onRenderEvent(RenderEvent& event)
{
    if (!mDebug.mValue || mMode.mValue != Mode::Flareon) return;

    auto drawList = ImGui::GetBackgroundDrawList();
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    AABB currentBox;
    currentBox.mMin = *player->getPos() - glm::vec3(0.3f);
    currentBox.mMax = *player->getPos() + glm::vec3(0.3f);

    std::vector<ImVec2> currentPoints = MathUtils::getImBoxPoints(currentBox);
    ImColor currentColor = ImColor(0.0f, 1.0f, 0.0f, 1.0f);
    drawList->AddPolyline(currentPoints.data(), currentPoints.size(), currentColor, 0, 2.0f);

    AABB originalBox;
    originalBox.mMin = glm::vec3(player->getPos()->x - 0.3f, mOriginalY - 0.3f, player->getPos()->z - 0.3f);
    originalBox.mMax = glm::vec3(player->getPos()->x + 0.3f, mOriginalY + 0.3f, player->getPos()->z + 0.3f);

    std::vector<ImVec2> originalPoints = MathUtils::getImBoxPoints(originalBox);
    ImColor originalColor = ImColor(1.0f, 1.0f, 0.0f, 1.0f);
    drawList->AddPolyline(originalPoints.data(), originalPoints.size(), originalColor, 0, 2.0f);

    for (const auto& pos : mDebugPositions)
    {
        AABB box;
        box.mMin = pos - glm::vec3(0.3f);
        box.mMax = pos + glm::vec3(0.3f);

        std::vector<ImVec2> points = MathUtils::getImBoxPoints(box);
        ImColor color = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
        drawList->AddPolyline(points.data(), points.size(), color, 0, 2.0f);
    }
}