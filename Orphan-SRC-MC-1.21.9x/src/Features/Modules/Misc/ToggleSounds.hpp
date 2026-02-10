#pragma once
//
// Created by vastrakai on 6/29/2024.
//

#include <Features/FeatureManager.hpp>

class ToggleSounds : public ModuleBase<ToggleSounds> {
public:
    enum class Sound {
        Celestial,
        Celestial2,
        Cowbell,
        Droplet,
        Lever,
        Nursultan,
        Poppet1,
        Poppet2,
        Sigma,
        Smooth
    };
    EnumSettingT<Sound> mSound = EnumSettingT<Sound>("Sound", "The sound to play on module toggle", Sound::Poppet1, "Celestial", "Celestial 2", "Cowbell", "Droplet", "Lever", "Nursultan", "Poppet 1", "Poppet 2", "Sigma", "Smooth");
    NumberSetting mVolume = NumberSetting("Volume", "The volume of the sound", 1.f, 0.f, 1.f, 0.1f);
    ToggleSounds() : ModuleBase("ToggleSounds", "Plays a sound on module toggle", ModuleCategory::Misc, 0, true) {
        addSetting(&mSound);
        addSetting(&mVolume);

        mNames = {
            {Lowercase, "togglesounds"},
            {LowercaseSpaced, "toggle sounds"},
            {Normal, "ToggleSounds"},
            {NormalSpaced, "Toggle Sounds"}
        };
    }

    void onEnable() override;
    void onDisable() override;

    void onModuleStateChange(ModuleStateChangeEvent& event);

    std::string getSettingDisplay() override {
        return mSound.mValues[mSound.as<int>()];
    }
};