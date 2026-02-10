//
// Created by vastrakai on 7/12/2024.
//

#include "InventoryMove.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/RenderEvent.hpp>
#include <SDK/SigManager.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/KeyboardMouseSettings.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <SDK/Minecraft/Network/Packets/ContainerClosePacket.hpp>

// Restore patches with NOP instructions
//std::vector<unsigned char> gClrBytes = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }; // Using 6 NOPs to be safe
//DEFINE_PATCH_FUNC(InventoryMove::patchFunc, SigManager::PlayerMovement_clearInputStateInlined, gClrBytes);

void InventoryMove::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &InventoryMove::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->listen<RenderEvent, &InventoryMove::onRenderEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketInEvent, &InventoryMove::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->listen<PacketOutEvent, &InventoryMove::onPacketOutEvent>(this);
    //patchFunc(true);
    auto player = ClientInstance::get()->getLocalPlayer();
    if (player) player->getMoveInputComponent()->mIsMoveLocked = false;
}

void InventoryMove::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &InventoryMove::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<RenderEvent, &InventoryMove::onRenderEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &InventoryMove::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketOutEvent, &InventoryMove::onPacketOutEvent>(this);
    //patchFunc(false);
    auto player = ClientInstance::get()->getLocalPlayer();
    if (player) player->getMoveInputComponent()->mIsMoveLocked = false;
}

void InventoryMove::onBaseTickEvent(BaseTickEvent& event)
{
    if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantTextInput) return;
    auto player = event.mActor;
    bool isUsingFreecam = player->getFlag<RenderCameraComponent>();
    if (isUsingFreecam) return;

    auto input = player->getMoveInputComponent();
    auto& keyboard = *ClientInstance::get()->getKeyboardSettings();

    bool w = Keyboard::mPressedKeys[keyboard["key.forward"]];
    bool a = Keyboard::mPressedKeys[keyboard["key.left"]];
    bool s = Keyboard::mPressedKeys[keyboard["key.back"]];
    bool d = Keyboard::mPressedKeys[keyboard["key.right"]];
    bool space = Keyboard::mPressedKeys[keyboard["key.jump"]];
    bool shift = Keyboard::mPressedKeys[keyboard["key.sneak"]];
    bool pressed = w || a || s || d || space || shift;

    std::string screenName = ClientInstance::get()->getScreenName();
    bool isInChatScreen = screenName == "chat_screen";

    if (isInChatScreen || !pressed)
    {
        input->mForward = false;
        input->mBackward = false;
        input->mLeft = false;
        input->mRight = false;
        input->mIsJumping = false;
        input->mIsJumping2 = false;
        input->mMoveVector = glm::vec2(0.f, 0.f);
        input->mIsSneakDown = false;
        return;
    }

    if (screenName != "hud_screen")
    {
        input->mIsMoveLocked = false;  // Force unlock movement
    }

    input->mForward = w;
    input->mBackward = s;
    input->mLeft = a;
    input->mRight = d;

    if (screenName != "hud_screen")
    {
        input->mIsJumping = space;
        input->mIsJumping2 = space;
    }

    if(mHasOpenContainer)
    {
        input->mIsSneakDown = mDisallowShift.mValue ? false : shift;
    }
    input->mMoveVector = MathUtils::getMovement();
}

void InventoryMove::onRenderEvent(RenderEvent& event)
{
    if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantTextInput) return;
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;
    bool isUsingFreecam = player->getFlag<RenderCameraComponent>();
    if (isUsingFreecam) return;

    auto input = player->getMoveInputComponent();
    auto& keyboard = *ClientInstance::get()->getKeyboardSettings();

    bool w = Keyboard::mPressedKeys[keyboard["key.forward"]];
    bool a = Keyboard::mPressedKeys[keyboard["key.left"]];
    bool s = Keyboard::mPressedKeys[keyboard["key.back"]];
    bool d = Keyboard::mPressedKeys[keyboard["key.right"]];
    bool space = Keyboard::mPressedKeys[keyboard["key.jump"]];
    bool shift = Keyboard::mPressedKeys[keyboard["key.sneak"]];
    bool pressed = w || a || s || d || space || shift;

    std::string screenName = ClientInstance::get()->getScreenName();
    bool isInChatScreen = screenName == "chat_screen";

    if (isInChatScreen || !pressed)
    {
        input->mForward = false;
        input->mBackward = false;
        input->mLeft = false;
        input->mRight = false;
        input->mIsJumping = false;
        input->mIsJumping2 = false;
        input->mMoveVector = glm::vec2(0.f, 0.f);
        input->mIsSneakDown = false;
        return;
    }

    if (screenName != "hud_screen")
    {
        input->mIsMoveLocked = false;  // Force unlock movement
    }

    input->mForward = w;
    input->mBackward = s;
    input->mLeft = a;
    input->mRight = d;

    if (screenName != "hud_screen")
    {
        input->mIsJumping = space;
        input->mIsJumping2 = space;
    }

    if(mHasOpenContainer)
    {
        input->mIsSneakDown = mDisallowShift.mValue ? false : shift;
    }
    input->mMoveVector = MathUtils::getMovement();
}

void InventoryMove::onPacketInEvent(PacketInEvent& event)
{
    if (event.mPacket->getId() == PacketID::ContainerOpen)
    {
        auto packet = event.getPacket<ContainerOpenPacket>();
        mHasOpenContainer = true;
    }
    if (event.mPacket->getId() == PacketID::ContainerClose)
    {
        mHasOpenContainer = false;
    }
}

void InventoryMove::onPacketOutEvent(PacketOutEvent& event)
{
    if (event.mPacket->getId() == PacketID::ContainerClose)
    {
        mHasOpenContainer = false;
    }
    else if (event.mPacket->getId() == PacketID::ContainerOpen)
    {
        auto packet = event.getPacket<ContainerOpenPacket>();
        mHasOpenContainer = true;
    }
}