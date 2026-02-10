//
// Created by vastrakai on 7/19/2024.
//

#include "NoFilter.hpp"

#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/MinecraftGame.hpp>
#include <SDK/Minecraft/UIProfanityContext.hpp>
#include <Features/Events/BaseTickEvent.hpp>

void NoFilter::onEnable()
{
    ClientInstance::get()->getMinecraftGame()->getProfanityContext()->setEnabled(false);
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &NoFilter::onBaseTickEvent>(this);
}

void NoFilter::onDisable()
{
    ClientInstance::get()->getMinecraftGame()->getProfanityContext()->setEnabled(true);
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &NoFilter::onBaseTickEvent>(this);
}
void NoFilter::onBaseTickEvent(BaseTickEvent& event) {
    /*auto player = ClientInstance::get()->getLocalPlayer();
    if(!player) return;
    spdlog::info("Min Health: " + std::to_string(player->getAttribute(AttributeId::Health)->mCurrentMinValue));
    spdlog::info("Health: " + std::to_string(player->getAttribute(AttributeId::Health)->mCurrentValue));
    spdlog::info("Max Health: " + std::to_string(player->getAttribute(AttributeId::Health)->mCurrentMaxValue));
    spdlog::info("Absorption: " + std::to_string(player->getAttribute(AttributeId::Absorption)->mCurrentValue));;
}*/
}