//
// Created by vastrakai on 6/29/2024.
//

#include "ToggleSounds.hpp"

#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/MinecraftGame.hpp>

void ToggleSounds::onEnable()
{
    gFeatureManager->mDispatcher->listen<ModuleStateChangeEvent, &ToggleSounds::onModuleStateChange>(this);
}

void ToggleSounds::onDisable()
{
    gFeatureManager->mDispatcher->deafen<ModuleStateChangeEvent, &ToggleSounds::onModuleStateChange>(this);
}

void ToggleSounds::onModuleStateChange(ModuleStateChangeEvent& event)
{
    if (event.isCancelled()) return;

    if (mSound.mValue == Sound::Celestial) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "celestial_on.wav" : "celestial_off.wav", mVolume.mValue);
    else if (mSound.mValue == Sound::Celestial2) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "celestial2_on.wav" : "celestial2_off.wav", mVolume.mValue);
    else if (mSound.mValue == Sound::Cowbell) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "cowbell_on.wav" : "cowbell_off.wav", mVolume.mValue);
    else if (mSound.mValue == Sound::Droplet) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "droplet_on.wav" : "droplet_off.wav", mVolume.mValue);
    else if (mSound.mValue == Sound::Lever) ClientInstance::get()->getMinecraftGame()->playUi("random.lever_click", mVolume.mValue, event.mEnabled ? 0.6f : 0.5f);
    else if (mSound.mValue == Sound::Nursultan) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "nursultan_on.wav" : "nursultan_off.wav", mVolume.mValue);
    else if (mSound.mValue == Sound::Poppet1) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "poppet1_on.wav" : "poppet1_off.wav", mVolume.mValue);
    else if (mSound.mValue == Sound::Poppet2) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "poppet2_on.wav" : "poppet2_off.wav", mVolume.mValue);
    else if (mSound.mValue == Sound::Sigma) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "sigma_on.wav" : "sigma_off.wav", mVolume.mValue);
    else if (mSound.mValue == Sound::Smooth) SoundUtils::playSoundFromEmbeddedResource(event.mEnabled ? "smooth_on.wav" : "smooth_off.wav", mVolume.mValue);
}
