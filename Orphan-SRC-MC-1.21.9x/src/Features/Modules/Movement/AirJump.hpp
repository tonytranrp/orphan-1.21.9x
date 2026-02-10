//
// Created by alteik on 04/09/2024.
// Edited by player5(1/24/2025)
//
#pragma once

#include "Features/Modules/Module.hpp"

class AirJump : public ModuleBase<AirJump>
{
public:
    AirJump() : ModuleBase<AirJump>("AirJump", "Allows you to jump while in the air", ModuleCategory::Movement, 0, false) {
        mNames = {
                {Lowercase, "airjump"},
                {LowercaseSpaced, "air jump"},
                {Normal, "AirJump"},
                {NormalSpaced, "Air Jump"}
        };
    }

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
};