#pragma once
//
// Created by vastrakai on 7/7/2024.
//

#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/World/Chunk/ChunkSource.hpp>


class ChestESP : public ModuleBase<ChestESP>
{
public:
    NumberSetting mChestRadius = NumberSetting("Chest Radius", "The radius of the chest esp", 20.f, 1.f, 100.f, 1.f);

    ChestESP() : ModuleBase("ChestESP", "Draws a box around chests", ModuleCategory::Visual, 0, false) {
        addSettings(
            &mChestRadius
        );

        mNames = {
            {Lowercase, "chestesp"},
            {LowercaseSpaced, "chest esp"},
            {Normal, "ChestESP"},
            {NormalSpaced, "Chest ESP"}
        };
    }

    void onEnable() override;
    void onDisable() override;
};