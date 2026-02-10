//
// Created by vastrakai on 7/10/2024.
//

#include "Speed.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <Features/Events/RunUpdateCycleEvent.hpp>
#include <Features/Modules/Player/Scaffold.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/KeyboardMouseSettings.hpp>
#include <SDK/Minecraft/MinecraftSim.hpp>
#include <SDK/Minecraft/Inventory/PlayerInventory.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerAuthInputPacket.hpp>
#include <SDK/Minecraft/Network/Packets/MobEffectPacket.hpp>
#include <SDK/Minecraft/Network/Packets/SetActorMotionPacket.hpp>
#include <random>

void Speed::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &Speed::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketOutEvent, &Speed::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->listen<RunUpdateCycleEvent, &Speed::onRunUpdateCycleEvent>(this);
    mDamageBoostVal = 1.f;
    mDamageTimerApplied = false;
}

void Speed::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &Speed::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketOutEvent, &Speed::onPacketOutEvent>(this);
    gFeatureManager->mDispatcher->deafen<RunUpdateCycleEvent, &Speed::onRunUpdateCycleEvent>(this);
    ClientInstance::get()->getMinecraftSim()->setSimTimer(20.f);
    mDamageBoostVal = 1.f;
    mDamageTimerApplied = false;
}

void Speed::onRunUpdateCycleEvent(RunUpdateCycleEvent& event)
{
    if (event.isCancelled() || event.mApplied) return;

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    static auto scaffold = gFeatureManager->mModuleManager->getModule<Scaffold>();
    if (scaffold->mEnabled) return;

    bool applyNetskip = false;

    if (!applyNetskip) return;
    Sleep(101);
}

/*bool Speed::tickSwiftness() {
    auto player = ClientInstance::get()->getLocalPlayer();
    bool hasSpeed = mEffectTimers.contains(EffectType::Speed);
    for (auto &[effect, time]: mEffectTimers) {
        if (time < NOW) mEffectTimers.erase(effect);
        if (time > NOW && effect == EffectType::Speed) {
            if (time - NOW < 100) hasSpeed = false;
            else hasSpeed = true;
        }
    }

    static bool lastSpace = false;
    auto &keyboard = *ClientInstance::get()->getKeyboardSettings();
    bool space = Keyboard::mPressedKeys[keyboard["key.jump"]];

    int spellbook = ItemUtils::getSwiftnessSpellbook(mSwiftnessHotbar.mValue);
    if (spellbook == -1 && !hasSpeed) return false;

    static auto scaffold = gFeatureManager->mModuleManager->getModule<Scaffold>();

    if (space && !lastSpace && !hasSpeed && spellbook != -1 && !scaffold->mEnabled) {
        ItemUtils::useItem(spellbook);
        NotifyUtils::notify("Using swiftness!", 5.f, Notification::Type::Info);
    }

    if (space && hasSpeed || !mHoldSpace.mValue && hasSpeed) {
        auto input = player->getMoveInputComponent();
        input->mIsJumping = false;

        glm::vec3 velocity = player->getStateVectorComponent()->mVelocity;
        float movementSpeed = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);

        float movementYaw = atan2(velocity.z, velocity.x);
        float movementYawDegrees = movementYaw * (180.0f / M_PI) - 90.f;

        float playerYawDegrees = player->getActorRotationComponent()->mYaw + MathUtils::getRotationKeyOffset();

        float yawDifference = playerYawDegrees - movementYawDegrees;
        float yawDifferenceRadians = yawDifference * (M_PI / 180.0f);
        float newMovementYaw = movementYaw + yawDifferenceRadians;
        static float fric = 1.f;
        if (!player->isOnGround()) fric *= mSwiftnessFriction.as<float>();
        else fric = 1.f;

        if (Keyboard::isUsingMoveKeys()) {
            movementSpeed = mSwiftnessSpeed.mValue * fric;
        }
        glm::vec3 newVelocity = {cos(newMovementYaw) * movementSpeed, velocity.y, sin(newMovementYaw) * movementSpeed};
        if (mSwiftnessSpeed.mValue != 0) player->getStateVectorComponent()->mVelocity = newVelocity;

        lastSpace = space;
        return true;
    }

    lastSpace = space;
    return false;
}*/

void Speed::onBaseTickEvent(BaseTickEvent& event)
{
    auto player = event.mActor;
    if (!player) return;

    if (player->getFlag<RedirectCameraInputComponent>()) return;

    /*if (mSwiftness.mValue)
    {
        tickSwiftness();
    }*/

    if (mMode.mValue == Mode::Friction)
    {
        tickFriction(player);
    }
    else if (mMode.mValue == Mode::Legit)
    {
        tickLegit(player);
    }
    else if (mMode.mValue == Mode::HiveSafe)
    {
        static float friction = 1.0f;
        static int tick = 0;

        if (player->isOnGround())
        {
            friction = 1.0f;
            tick = 0;
            ClientInstance::get()->getMinecraftSim()->setSimTimer(20.f);
        }
        else
        {
            friction *= 1.0f;
            tick++;
        }

        float speed = 3.39f;
        if (Keyboard::isStrafing())
            speed = 2.5f;

        glm::vec2 motion = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, ((speed * mDamageBoostVal) / 10) * friction, true);
        auto stateVector = player->getStateVectorComponent();
        stateVector->mVelocity = { motion.x, stateVector->mVelocity.y, motion.y };

        bool usingMoveKeys = Keyboard::isUsingMoveKeys();
        if (usingMoveKeys && mJumpType.mValue == JumpType::Vanilla) {
            if (player->isOnGround()) {
                player->jumpFromGround();
                player->getStateVectorComponent()->mVelocity.y = mJumpHeight.as<float>();
            }
        }
        else if (usingMoveKeys && mJumpType.mValue == JumpType::Motion) {
            if (player->isOnGround()) {
                player->getStateVectorComponent()->mVelocity.y = mJumpHeight.as<float>();
            }
        }
    }
}

void Speed::onPacketInEvent(PacketInEvent& event)
{
    if (event.mPacket->getId() == PacketID::MobEffect)
    {
        auto player = ClientInstance::get()->getLocalPlayer();
        if (!player) return;

        auto mep = event.getPacket<MobEffectPacket>();

        if (mep->mRuntimeId != player->getRuntimeID()) return;

        auto effect = std::string(mep->getEffectName());
        auto eventName = std::string(mep->getEventName());
        uint64_t time = mep->mEffectDurationTicks * 50;

        if (mep->mEventId == MobEffectPacket::Event::Add) {
            mEffectTimers[mep->mEffectId] = NOW + time;
        }
        else if (mep->mEventId == MobEffectPacket::Event::Remove) {
            mEffectTimers.erase(mep->mEffectId);
        }

        spdlog::info("Effect: {} Event: {} Time: {}", effect, eventName, time);
    }

    if (!mEnabled) return;
}

void Speed::onPacketOutEvent(PacketOutEvent& event)
{
    if (event.mPacket->getId() == PacketID::PlayerAuthInput)
    {
        auto paip = event.getPacket<PlayerAuthInputPacket>();
        auto player = ClientInstance::get()->getLocalPlayer();
        if (Keyboard::isUsingMoveKeys()) paip->mInputData |= AuthInputAction::JUMPING | AuthInputAction::WANT_UP | AuthInputAction::JUMP_DOWN;
        if (!player->isOnGround() && player->wasOnGround() && Keyboard::isUsingMoveKeys()) {
            paip->mInputData |= AuthInputAction::START_JUMPING;
        }

        if (mStrafeMode.mValue == StrafeMode::Custom || (mStrafeMode.mValue == StrafeMode::Ground && player->isOnGround())) {

            auto pkt = event.getPacket<PlayerAuthInputPacket>();
            glm::vec2 moveVec = pkt->mMove;
            glm::vec2 xzVel = { pkt->mPosDelta.x, pkt->mPosDelta.z };
            float yaw = pkt->mRot.y;
            yaw = -yaw;

            if (moveVec.x == 0 && moveVec.y == 0 && xzVel.x == 0 && xzVel.y == 0) return;

            float moveVecYaw = atan2(moveVec.x, moveVec.y);
            moveVecYaw = glm::degrees(moveVecYaw);

            float movementYaw = atan2(xzVel.x, xzVel.y);
            float movementYawDegrees = movementYaw * (180.0f / M_PI);

            float yawDiff = movementYawDegrees - yaw;

            float newMoveVecX = sin(glm::radians(yawDiff));
            float newMoveVecY = cos(glm::radians(yawDiff));
            glm::vec2 newMoveVec = { newMoveVecX, newMoveVecY };

            if (abs(newMoveVec.x) < 0.001) newMoveVec.x = 0;
            if (abs(newMoveVec.y) < 0.001) newMoveVec.y = 0;
            if (moveVec.x == 0 && moveVec.y == 0) newMoveVec = { 0, 0 };

            // Remove all old flags
            pkt->mInputData &= ~AuthInputAction::UP;
            pkt->mInputData &= ~AuthInputAction::DOWN;
            pkt->mInputData &= ~AuthInputAction::LEFT;
            pkt->mInputData &= ~AuthInputAction::RIGHT;
            pkt->mInputData &= ~AuthInputAction::UP_RIGHT;
            pkt->mInputData &= ~AuthInputAction::UP_LEFT;

            pkt->mMove = newMoveVec;
            pkt->mVehicleRotation = newMoveVec; // ???? wtf mojang
            pkt->mInputMode = InputMode::MotionController;

            // Get the move direction
            bool forward = newMoveVec.y > 0;
            bool backward = newMoveVec.y < 0;
            bool left = newMoveVec.x < 0;
            bool right = newMoveVec.x > 0;

            static bool isSprinting = false;
            bool startedThisTick = false;
            // if the flags contain isSprinting, set the flag
            if (pkt->hasInputData(AuthInputAction::START_SPRINTING)) {
                isSprinting = true;
                startedThisTick = true;
            }
            else if (pkt->hasInputData(AuthInputAction::STOP_SPRINTING)) {
                isSprinting = false;
            }

            if (!forward) {
                // Remove all sprint flags
                pkt->mInputData &= ~AuthInputAction::START_SPRINTING;
                if (isSprinting && !startedThisTick) {
                    pkt->mInputData |= AuthInputAction::STOP_SPRINTING;
                }

                pkt->mInputData &= ~AuthInputAction::SPRINTING;
                pkt->mInputData &= ~AuthInputAction::START_SNEAKING;

                // Stop the player from sprinting
                player->getMoveInputComponent()->setmIsSprinting(false);
            }
        }
    }
}

void Speed::tickLegit(Actor* player)
{
    static int tick = 0;
    if (player->isOnGround()) {
        tick = 0;
        ClientInstance::get()->getMinecraftSim()->setSimTimer(20.f);
    }
    else {
        tick++;
    }

    if (mFastFall.mValue == FastfallMode::Predict)
    {
        if (tick == mFallTicks.as<int>())
        {
            auto fallSpeed = mFallSpeed.as<float>();
            auto stateVector = player->getStateVectorComponent();
            for (int i = 0; i < fallSpeed; i++)
            {
                stateVector->mVelocity.y = (stateVector->mVelocity.y - 0.08f) * 0.98f;
            }
        }
        if (mFastFall2.mValue && tick == mFallTicks2.as<int>())
        {
            auto fallSpeed = mFallSpeed2.as<float>();
            auto stateVector = player->getStateVectorComponent();
            for (int i = 0; i < fallSpeed; i++)
            {
                stateVector->mVelocity.y = (stateVector->mVelocity.y - 0.08f) * 0.98f;
            }
        }
    }
    else if (mFastFall.mValue == FastfallMode::SetVel)
    {
        if (tick == mFallTicks.as<int>())
        {
            auto fallSpeed = mFallSpeed.as<float>() / 10;
            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity.y = -fallSpeed;
        }
        if (mFastFall2.mValue && tick == mFallTicks2.as<int>())
        {
            auto fallSpeed = mFallSpeed2.as<float>() / 10;
            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity.y = -fallSpeed;
        }
    }
    else if (mFastFall.mValue == FastfallMode::Timer)
    {
        if (tick == mFallTicks.as<int>())
        {
            ClientInstance::get()->getMinecraftSim()->setSimTimer(mTimerFallSpeed.as<float>());
        }
        if (mFastFall2.mValue && tick == mFallTicks2.as<int>())
        {
            ClientInstance::get()->getMinecraftSim()->setSimTimer(mTimerFallSpeed2.as<float>());
        }
    }
    else if (mFastFall.mValue == FastfallMode::Multiply)
    {
        if (tick == mFallTicks.as<int>())
        {
            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity.y *= mMultiplyFallValue.as<float>();
        }
        if (mFastFall2.mValue && tick == mFallTicks2.as<int>())
        {
            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity.y *= mMultiplyFallValue2.as<float>();
        }
    }

    glm::vec3 velocity = player->getStateVectorComponent()->mVelocity;
    float movementSpeed = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);

    if (!player->isOnGround() && Keyboard::isUsingMoveKeys() && player->wasOnGround())
    {
    }
    else if (Keyboard::isUsingMoveKeys() && !player->isOnGround() && !player->wasOnGround() && player->getFallDistance() > 0) {
        movementSpeed *= mFriction.as<float>();
    }

    float movementYaw = atan2(velocity.z, velocity.x);
    float movementYawDegrees = movementYaw * (180.0f / M_PI) - 90.f;
    float newMovementYaw = movementYaw;

    glm::vec3 newVelocity = { cos(newMovementYaw) * movementSpeed, velocity.y, sin(newMovementYaw) * movementSpeed };
    if (mSpeed.mValue != 0)
        player->getStateVectorComponent()->mVelocity = newVelocity;

    bool usingMoveKeys = Keyboard::isUsingMoveKeys();
    if (usingMoveKeys && mJumpType.mValue == JumpType::Vanilla) {
        if (player->isOnGround())
        {
            player->jumpFromGround();
            player->getStateVectorComponent()->mVelocity.y = mJumpHeight.as<float>();
        }
    }
    else if (usingMoveKeys && mJumpType.mValue == JumpType::Motion) {
        if (player->isOnGround()) {
            player->getStateVectorComponent()->mVelocity.y = mJumpHeight.as<float>();
        }
    }
}

void Speed::tickFriction(Actor* player)
{
    static auto friction = mFriction.as<float>();
    static int tick = 0;

    if (player->isOnGround())
    {
        friction = 1.f;
        tick = 0;
        ClientInstance::get()->getMinecraftSim()->setSimTimer(20.f);
    }
    else
    {
        friction *= mFriction.as<float>();
        tick++;
    }

    if (mFastFall.mValue == FastfallMode::Predict)
    {
        if (tick == mFallTicks.as<int>())
        {
            auto fallSpeed = mFallSpeed.as<float>();
            auto stateVector = player->getStateVectorComponent();
            for (int i = 0; i < fallSpeed; i++)
            {
                stateVector->mVelocity.y = (stateVector->mVelocity.y - 0.08f) * 0.98f;
            }
        }
        if (mFastFall2.mValue && tick == mFallTicks2.as<int>())
        {
            auto fallSpeed = mFallSpeed2.as<float>();
            auto stateVector = player->getStateVectorComponent();
            for (int i = 0; i < fallSpeed; i++)
            {
                stateVector->mVelocity.y = (stateVector->mVelocity.y - 0.08f) * 0.98f;
            }
        }
    }
    else if (mFastFall.mValue == FastfallMode::SetVel)
    {
        if (tick == mFallTicks.as<int>())
        {
            auto fallSpeed = mFallSpeed.as<float>() / 10;
            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity.y = -fallSpeed;
        }
        if (mFastFall2.mValue && tick == mFallTicks2.as<int>())
        {
            auto fallSpeed = mFallSpeed2.as<float>() / 10;
            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity.y = -fallSpeed;
        }
    }
    else if (mFastFall.mValue == FastfallMode::Timer)
    {
        if (tick == mFallTicks.as<int>())
        {
            ClientInstance::get()->getMinecraftSim()->setSimTimer(mTimerFallSpeed.as<float>());
        }
        if (mFastFall2.mValue && tick == mFallTicks2.as<int>())
        {
            ClientInstance::get()->getMinecraftSim()->setSimTimer(mTimerFallSpeed2.as<float>());
        }
    }
    else if (mFastFall.mValue == FastfallMode::Multiply)
    {
        static float multiplier = 1.0f;
        if (tick == mFallTicks.as<int>())
        {
            auto stateVector = player->getStateVectorComponent();
            multiplier += (mMultiplyFallValue.as<float>() - 1.0f) * 0.1f; // Gradually increase
            stateVector->mVelocity.y *= multiplier;
        }
        if (mFastFall2.mValue && tick == mFallTicks2.as<int>())
        {
            auto stateVector = player->getStateVectorComponent();
            multiplier += (mMultiplyFallValue2.as<float>() - 1.0f) * 0.1f; // Gradually increase
            stateVector->mVelocity.y *= multiplier;
        }
        if (player->isOnGround()) {
            multiplier = 1.0f; // Reset multiplier when landing
        }
    }

    float speed;
    speed = mSpeed.as<float>();

    if (Keyboard::isStrafing() && mStrafeMode.mValue != StrafeMode::None)
        speed = (mStrafeMode.mValue == StrafeMode::Hive ? 2.5f : mStrafeSpeed.mValue);

    glm::vec2 motion;
    bool shouldStrafe = mStrafeMode.mValue != StrafeMode::None;
    static glm::vec2 lastGroundMotion = { 0, 0 };
    static float targetYaw = player->getActorRotationComponent()->mYaw;

    auto moveInput = player->getMoveInputComponent();
    if (Keyboard::isStrafing()) {
        if (moveInput->mBackward) targetYaw = player->getActorRotationComponent()->mYaw + 180.0f;
        else if (moveInput->mLeft) targetYaw = player->getActorRotationComponent()->mYaw + 90.0f;
        else if (moveInput->mRight) targetYaw = player->getActorRotationComponent()->mYaw - 90.0f;
    }

    // Smoothly rotate to target yaw
    float currentYaw = player->getActorRotationComponent()->mYaw;
    float yawDiff = targetYaw - currentYaw;
    while (yawDiff > 180.0f) yawDiff -= 360.0f;
    while (yawDiff < -180.0f) yawDiff += 360.0f;

    /*if (mStrafeMode.mValue == StrafeMode::Ground && Keyboard::isStrafing()) {
        player->getActorRotationComponent()->mYaw = currentYaw + (yawDiff * 0.3f);
    }*/

    if (player->isOnGround()) {
        // Calculate and store ground motion
        lastGroundMotion = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, ((speed * mDamageBoostVal) / 10) * friction, shouldStrafe);
        motion = lastGroundMotion;
    }
    else if (mStrafeMode.mValue == StrafeMode::Ground) {
        // Use stored ground motion in air
        motion = lastGroundMotion;
    }
    else {
        // Normal strafe calculation if ground strafe is disabled
        motion = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, ((speed * mDamageBoostVal) / 10) * friction, shouldStrafe);
    }

    auto stateVector = player->getStateVectorComponent();
    stateVector->mVelocity = { motion.x, stateVector->mVelocity.y, motion.y };

    bool usingMoveKeys = Keyboard::isUsingMoveKeys();
    if (usingMoveKeys && mJumpType.mValue == JumpType::Vanilla) {
        if (player->isOnGround()) {
            player->jumpFromGround();
            player->getStateVectorComponent()->mVelocity.y = mJumpHeight.as<float>();
        }
    }
    else if (usingMoveKeys && mJumpType.mValue == JumpType::Motion) {
        if (player->isOnGround()) {
            player->getStateVectorComponent()->mVelocity.y = mJumpHeight.as<float>();
        }
    }
}

void Speed::tickFrictionPreset(FrictionPreset& preset)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;
    float frictionSetting = preset.friction;
    float speedSetting = preset.speed;
    bool strafeSetting = preset.strafe;
    FastfallMode fastFallSetting = preset.fastFall;
    int fallTicksSetting = preset.fallTicks;
    float fallSpeedSetting = preset.fallSpeed;
    bool fastFall2Setting = preset.fastFall2;
    int fallTicks2Setting = preset.fallTicks2;
    float fallSpeed2Setting = preset.fallSpeed2;
    JumpType jumpTypeSetting = preset.jumpType;
    float jumpHeightSetting = preset.jumpHeight;

    if (Keyboard::isStrafing() && mStrafeMode.mValue != StrafeMode::None)
        speedSetting = (mStrafeMode.mValue == StrafeMode::Hive ? 2.5f : mStrafeSpeed.mValue);

    static float friction = frictionSetting;
    static int tick = 0;

    if (player->isOnGround())
    {
        friction = 1.f;
        tick = 0;
    }
    else
    {
        friction *= frictionSetting;
        tick++;
    }

    if (fastFallSetting == FastfallMode::Predict)
    {
        if (tick == fallTicksSetting)
        {
            auto fallSpeed = fallSpeedSetting;
            auto stateVector = player->getStateVectorComponent();
            for (int i = 0; i < fallSpeed; i++)
            {
                stateVector->mVelocity.y = (stateVector->mVelocity.y - 0.08f) * 0.98f;
            }
        }
        if (fastFall2Setting && tick == fallTicks2Setting)
        {
            auto fallSpeed = fallSpeed2Setting;
            auto stateVector = player->getStateVectorComponent();
            for (int i = 0; i < fallSpeed; i++)
            {
                stateVector->mVelocity.y = (stateVector->mVelocity.y - 0.08f) * 0.98f;
            }
        }
    }
    else if (fastFallSetting == FastfallMode::SetVel)
    {
        if (tick == fallTicksSetting)
        {
            auto fallSpeed = fallSpeedSetting / 10;
            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity.y = -fallSpeed;
        }
        if (fastFall2Setting && tick == fallTicks2Setting)
        {
            auto fallSpeed = fallSpeed2Setting / 10;
            auto stateVector = player->getStateVectorComponent();
            stateVector->mVelocity.y = -fallSpeed;
        }
    }

    glm::vec2 motion = MathUtils::getMotion(player->getActorRotationComponent()->mYaw, ((speedSetting / 10) * mDamageBoostVal) * friction, strafeSetting);
    auto stateVector = player->getStateVectorComponent();
    stateVector->mVelocity = { motion.x, stateVector->mVelocity.y, motion.y };

    bool usingMoveKeys = Keyboard::isUsingMoveKeys();
    if (usingMoveKeys && jumpTypeSetting == JumpType::Vanilla) {
        if (player->isOnGround())
        {
            player->jumpFromGround();
            player->getStateVectorComponent()->mVelocity.y = jumpHeightSetting;
        }
    }
    else if (usingMoveKeys && jumpTypeSetting == JumpType::Motion) {
        if (player->isOnGround()) {
            player->getStateVectorComponent()->mVelocity.y = jumpHeightSetting;
        }
    }
}