#pragma once
//
// Created by alteik on 04/09/2024.
//

#include <Features/Modules/Setting.hpp>
#include <Features/Modules/Module.hpp>
#include <SDK/Minecraft/Actor/EntityId.hpp>

class TriggerBot : public ModuleBase<TriggerBot>
{
public:
    DividerSetting dSpeed = DividerSetting("- Speed -", "Settings for APS");
    NumberSetting mAPSMin = NumberSetting("APS Min", "The minimum APS to randomize", 12, 1, 20, 1);
    NumberSetting mAPSMax = NumberSetting("APS Max", "The maximum APS to randomize", 16, 1, 20, 1);
    DividerSetting dBypass = DividerSetting("- Bypass -", "Options for bypassing anticheat");
    BoolSetting mUseAntibot = BoolSetting("Use Antibot", "Whether or not to use antibot for exclude actors", true);
    BoolSetting mHive = BoolSetting("Hive", "Attempts to bypass hive's anticheat", true);

    TriggerBot() : ModuleBase<TriggerBot>("TriggerBot", "Automatically attacks the entity that you're aiming at", ModuleCategory::Combat, 0, false) {
        mNames = {
                {Lowercase, "triggerbot"},
                {LowercaseSpaced, "trigger bot"},
                {Normal, "TriggerBot"},
                {NormalSpaced, "Trigger Bot"}
        };

        addSettings
        (
            &dSpeed,
            &mAPSMin,
            &mAPSMax,

            &dBypass,
            &mUseAntibot,
            &mHive
        );
    }
    Actor* getActorFromEntityId(EntityId entityId);
    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
};