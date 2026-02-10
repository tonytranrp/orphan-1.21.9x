//
// Created by vastrakai on 6/28/2024.
//

#include <build/build_info.h>
#include <Features/Modules/ModuleManager.hpp>
//#include <Utils/OAuthUtils.hpp>

#include "Combat/Aura.hpp"
#include "Combat/Criticals.hpp"
#include "Combat/HitBoxes.hpp"
#include "Combat/InfiniteAura.hpp"
#include "Combat/Reach.hpp"
#include "Combat/TriggerBot.hpp"
#include "Combat/AutoClicker.hpp"

#include "Misc/AntiBot.hpp"
#include "Misc/Anticheat.hpp"
#include "Misc/AutoAccept.hpp"
#include "Misc/AutoDodge.hpp"
#include "Misc/AutoQueue.hpp"
#include "Misc/AutoSnipe.hpp"
#include "Misc/AutoVote.hpp"
#include "Misc/Desync.hpp"
#include "Misc/DeviceSpoof.hpp"
#include "Misc/Disabler.hpp"
#include "Misc/Friends.hpp"
#include "Misc/HackerAlert.hpp"
#include "Misc/KickSounds.hpp"
#include "Misc/NetSkip.hpp"
#include "Misc/NoPacket.hpp"
#include "Misc/PacketLogger.hpp"
#include "Misc/SkinStealer.hpp"
#include "Misc/StaffAlert.hpp"
#include "Misc/ToggleSounds.hpp"
//#include "Misc/AntiCheatDetector.hpp"
//#include "Misc/AutoCosmetic.hpp"
//#include "Misc/AutoMessage.hpp"
//#include "Misc/AutoReport.hpp"
//#include "Misc/CostumeSpammer.hpp"
//#include "Misc/IRC.hpp"
//#include "Misc/Killsults.hpp"
#include "Misc/NoFilter.hpp"
//#include "Misc/PartySpammer.hpp"
//#include "Misc/SkinBlinker.hpp"
//#include "Misc/Spammer.hpp"
//#include "Misc/TestModule.hpp"

#include "Movement/AirJump.hpp"
#include "Movement/AirSpeed.hpp"
#include "Movement/AntiImmobile.hpp"
#include "Movement/AutoPath.hpp"
#include "Movement/DamageBoost.hpp"
#include "Movement/DamageBoost2.hpp"
#include "Movement/DebugFly.hpp"
#include "Movement/FastStop.hpp"
#include "Movement/Fly.hpp"
#include "Movement/HiveFly.hpp"
#include "Movement/InventoryMove.hpp"
#include "Movement/Jesus.hpp"
#include "Movement/LongJump.hpp"
#include "Movement/NoJumpDelay.hpp"
#include "Movement/NoSlowDown.hpp"
#include "Movement/Phase.hpp"
#include "Movement/ReverseStep.hpp"
#include "Movement/SafeWalk.hpp"
#include "Movement/Speed.hpp"
#include "Movement/Spider.hpp"
#include "Movement/Sprint.hpp"
#include "Movement/Step.hpp"
#include "Movement/TargetStrafe.hpp"
#include "Movement/Velocity.hpp"
//#include "Movement/AutoWalk.hpp"
//#include "Movement/Jetpack.hpp"
//#include "Movement/ServerSneak.hpp"

#include "Player/AutoLootbox.hpp"
#include "Player/AutoSpellBook.hpp"
#include "Player/AutoTool.hpp"
//#include "Player/ChestAura.hpp"
#include "Player/ChestStealer.hpp"
#include "Player/ClickTp.hpp"
#include "Player/Derp.hpp"
#include "Player/Encaser.hpp"
#include "Player/Extinguisher.hpp"
#include "Player/FastEat.hpp"
#include "Player/FastMine.hpp"
#include "Player/Freecam.hpp"
#include "Player/InvManager.hpp"
#include "Player/MidclickAction.hpp"
#include "Player/NoFall.hpp"
#include "Player/NoRotate.hpp"
#include "Player/Nuker.hpp"
#include "Player/OreMiner.hpp"
#include "Player/Regen.hpp"
#include "Player/Scaffold.hpp"
#include "Player/Surround.hpp"
#include "Player/Teams.hpp"
#include "Player/Timer.hpp"
#include "Player/ZipLine.hpp"
//#include "Player/AntiVoid.hpp"
//#include "Player/AutoBoombox.hpp"
//#include "Player/AutoEat.hpp"
//#include "Player/AutoKick.hpp"
//#include "Player/RegenRecode.hpp"

#include "spdlog/spdlog.h"

#include "Visual/Animations.hpp"
#include "Visual/Arraylist.hpp"
#include "Visual/AutoScale.hpp"
#include "Visual/BlockESP.hpp"
#include "Visual/ChestESP.hpp"
#include "Visual/ClickGui.hpp"
#include "Visual/CustomChat.hpp"
#include "Visual/DestroyProgress.hpp"
#include "Visual/ESP.hpp"
#include "Visual/FullBright.hpp"
#include "Visual/Glint.hpp"
#include "Visual/HudEditor.hpp"
#include "Visual/Interface.hpp"
#include "Visual/ItemESP.hpp"
#include "Visual/Keystrokes.hpp"
#include "Visual/LevelInfo.hpp"
#include "Visual/NameProtect.hpp"
#include "Visual/Nametags.hpp"
#include "Visual/NoCameraClip.hpp"
#include "Visual/NoDebuff.hpp"
#include "Visual/NoHurtcam.hpp"
#include "Visual/NoRender.hpp"
#include "Visual/Notifications.hpp"
#include "Visual/PlaceHighlights.hpp"
#include "Visual/SessionInfo.hpp"
#include "Visual/TargetHUD.hpp"
#include "Visual/Tracers.hpp"
#include "Visual/UpdateForm.hpp"
#include "Visual/ViewModel.hpp"
#include "Visual/Watermark.hpp"
#include "Visual/Zoom.hpp"
//#include "Visual/BoneEsp.hpp"
//#include "Visual/ChinaHat.hpp"
//#include "Visual/Freelook.hpp"
//#include "Visual/ItemPhysics.hpp"
//#include "Visual/JumpCircles.hpp"
//#include "Visual/MotionBlur.hpp"
//#include "Visual/RobloxCamera.hpp"

void ModuleManager::init()
{
    // Visual (must be initialized first)
    mModules.emplace_back(std::make_shared<HudEditor>());

    // Combat
    mModules.emplace_back(std::make_shared<Aura>());
    mModules.emplace_back(std::make_shared<AutoClicker>());
    mModules.emplace_back(std::make_shared<Criticals>());
    mModules.emplace_back(std::make_shared<HitBoxes>());
    mModules.emplace_back(std::make_shared<InfiniteAura>());
    mModules.emplace_back(std::make_shared<Reach>());
    mModules.emplace_back(std::make_shared<TriggerBot>());

    // Movement
    mModules.emplace_back(std::make_shared<AirJump>());
    mModules.emplace_back(std::make_shared<AirSpeed>());
    mModules.emplace_back(std::make_shared<AntiImmobile>());
    mModules.emplace_back(std::make_shared<DamageBoost>());
    if (Auth::isPrivateUser()) { mModules.emplace_back(std::make_shared<DamageBoost2>()); }
    mModules.emplace_back(std::make_shared<FastStop>());
    mModules.emplace_back(std::make_shared<Fly>());
    mModules.emplace_back(std::make_shared<HiveFly>()); // Flareon V2 boombox fly
    mModules.emplace_back(std::make_shared<Jesus>());
    mModules.emplace_back(std::make_shared<LongJump>());
    mModules.emplace_back(std::make_shared<NoJumpDelay>());
    mModules.emplace_back(std::make_shared<NoSlowDown>());
    mModules.emplace_back(std::make_shared<Phase>());
    //mModules.emplace_back(std::make_shared<SafeWalk>());
    mModules.emplace_back(std::make_shared<Speed>());
    mModules.emplace_back(std::make_shared<Spider>());
    mModules.emplace_back(std::make_shared<Sprint>());
    mModules.emplace_back(std::make_shared<Step>());
    mModules.emplace_back(std::make_shared<TargetStrafe>());
    mModules.emplace_back(std::make_shared<Velocity>());
    //mModules.emplace_back(std::make_shared<AutoWalk>());
    mModules.emplace_back(std::make_shared<InventoryMove>());
    //mModules.emplace_back(std::make_shared<Jetpack>());
    //mModules.emplace_back(std::make_shared<ReverseStep>());
    //mModules.emplace_back(std::make_shared<ServerSneak>());

    // Player
    mModules.emplace_back(std::make_shared<AutoLootbox>());
    mModules.emplace_back(std::make_shared<AutoSpellBook>());
    mModules.emplace_back(std::make_shared<AutoTool>());
    mModules.emplace_back(std::make_shared<ChestStealer>());
    mModules.emplace_back(std::make_shared<ClickTp>());
    mModules.emplace_back(std::make_shared<Derp>());
    mModules.emplace_back(std::make_shared<Encaser>());
    mModules.emplace_back(std::make_shared<Extinguisher>());
    mModules.emplace_back(std::make_shared<FastEat>());
    mModules.emplace_back(std::make_shared<FastMine>());
    mModules.emplace_back(std::make_shared<Freecam>());
    mModules.emplace_back(std::make_shared<InvManager>());
    mModules.emplace_back(std::make_shared<MidclickAction>());
    mModules.emplace_back(std::make_shared<NoFall>());
    mModules.emplace_back(std::make_shared<NoRotate>());
    mModules.emplace_back(std::make_shared<Nuker>());
    mModules.emplace_back(std::make_shared<OreMiner>());
    mModules.emplace_back(std::make_shared<Regen>());
    mModules.emplace_back(std::make_shared<Scaffold>());
    mModules.emplace_back(std::make_shared<Surround>());
    mModules.emplace_back(std::make_shared<Teams>());
    mModules.emplace_back(std::make_shared<Timer>());
    mModules.emplace_back(std::make_shared<ZipLine>());
    //mModules.emplace_back(std::make_shared<AntiVoid>());
    //mModules.emplace_back(std::make_shared<AutoBoombox>());
    //mModules.emplace_back(std::make_shared<AutoEat>());
    //mModules.emplace_back(std::make_shared<ChestAura>());
    //mModules.emplace_back(std::make_shared<RegenRecode>());

    // Misc
    mModules.emplace_back(std::make_shared<AntiBot>());
    mModules.emplace_back(std::make_shared<AutoAccept>());
    mModules.emplace_back(std::make_shared<AutoDodge>());
    mModules.emplace_back(std::make_shared<AutoQueue>());
    mModules.emplace_back(std::make_shared<AutoSnipe>());
    mModules.emplace_back(std::make_shared<AutoVote>());
    mModules.emplace_back(std::make_shared<DeviceSpoof>());
    mModules.emplace_back(std::make_shared<Desync>()); // needs troubleshooting
    mModules.emplace_back(std::make_shared<Disabler>());
    mModules.emplace_back(std::make_shared<Friends>());
    mModules.emplace_back(std::make_shared<HackerAlert>());
    mModules.emplace_back(std::make_shared<KickSounds>());
    mModules.emplace_back(std::make_shared<NetSkip>());
    mModules.emplace_back(std::make_shared<NoPacket>());
    mModules.emplace_back(std::make_shared<PacketLogger>());
    mModules.emplace_back(std::make_shared<SkinStealer>());
    mModules.emplace_back(std::make_shared<StaffAlert>());
    mModules.emplace_back(std::make_shared<ToggleSounds>());
    //mModules.emplace_back(std::make_shared<AntiCheatDetector>());
    //mModules.emplace_back(std::make_shared<AutoCosmetic>());
    //mModules.emplace_back(std::make_shared<AutoMessage>());
    //mModules.emplace_back(std::make_shared<AutoReport>());
    //mModules.emplace_back(std::make_shared<CostumeSpammer>());
    //mModules.emplace_back(std::make_shared<IRC>());
    //mModules.emplace_back(std::make_shared<Killsults>());
    mModules.emplace_back(std::make_shared<NoFilter>());
    //mModules.emplace_back(std::make_shared<PartySpammer>());
    //mModules.emplace_back(std::make_shared<Spammer>());
    //mModules.emplace_back(std::make_shared<TestModule>());

    // Visual
    mModules.emplace_back(std::make_shared<Animations>());
    mModules.emplace_back(std::make_shared<Anticheat>()); // Private for now cuz its not really good
    mModules.emplace_back(std::make_shared<Arraylist>());
    mModules.emplace_back(std::make_shared<AutoScale>());
    mModules.emplace_back(std::make_shared<BlockESP>());
    mModules.emplace_back(std::make_shared<ChestESP>());
    mModules.emplace_back(std::make_shared<ClickGui>());
    mModules.emplace_back(std::make_shared<CustomChat>());
    mModules.emplace_back(std::make_shared<DestroyProgress>());
    mModules.emplace_back(std::make_shared<ESP>());
    mModules.emplace_back(std::make_shared<FullBright>());
    mModules.emplace_back(std::make_shared<Glint>());
    mModules.emplace_back(std::make_shared<Interface>());
    mModules.emplace_back(std::make_shared<ItemESP>());
    mModules.emplace_back(std::make_shared<Keystrokes>());
    mModules.emplace_back(std::make_shared<LevelInfo>());
    mModules.emplace_back(std::make_shared<Nametags>());
    mModules.emplace_back(std::make_shared<NoCameraClip>());
    mModules.emplace_back(std::make_shared<NoDebuff>());
    mModules.emplace_back(std::make_shared<NoHurtcam>());
    mModules.emplace_back(std::make_shared<NoRender>());
    mModules.emplace_back(std::make_shared<Notifications>());
    mModules.emplace_back(std::make_shared<PlaceHighlights>());
    mModules.emplace_back(std::make_shared<SessionInfo>());
    mModules.emplace_back(std::make_shared<TargetHUD>());
    mModules.emplace_back(std::make_shared<Tracers>());
    mModules.emplace_back(std::make_shared<ViewModel>());
    mModules.emplace_back(std::make_shared<Watermark>());
    mModules.emplace_back(std::make_shared<Zoom>());
    //mModules.emplace_back(std::make_shared<ChinaHat>());
    //mModules.emplace_back(std::make_shared<Freelook>());
    //mModules.emplace_back(std::make_shared<JumpCircles>());
    //mModules.emplace_back(std::make_shared<MotionBlur>());
    //mModules.emplace_back(std::make_shared<NameProtect>());
    //mModules.emplace_back(std::make_shared<RobloxCamera>());

    // Determine if we should add UpdateForm
    //std::string oldHash = OAuthUtils::getLastCommitHash();
    std::string latestHash = ORPHAN_BUILD_VERSION;
    //if (oldHash != latestHash && oldHash != "")
    //{
    //    spdlog::info("Adding UpdateForm module, oldHash: {}, latestHash: {}", oldHash, latestHash);
    //    mModules.emplace_back(std::make_shared<UpdateForm>());
    //} else
    //{
    //    spdlog::info("Not adding UpdateForm module, oldHash: {}, latestHash: {}", oldHash, latestHash);
    //}

    for (auto &module: mModules) {
        try {
            module->onInit();
        } catch (const std::exception &e) {
            spdlog::error("Failed to initialize module {}: {}", module->mName, e.what());
        } catch (...) {
            spdlog::error("Failed to initialize module {}: unknown", module->mName);
        }
    }
}

void ModuleManager::shutdown() {
    for (auto &module : mModules)
    {
        if (module->mEnabled)
        {
            module->mEnabled = false;
            module->onDisable();
        }
    }

    mModules.clear();
}

void ModuleManager::registerModule(const std::shared_ptr<Module>& module)
{
    mModules.push_back(module);
}

std::vector<std::shared_ptr<Module>>& ModuleManager::getModules()
{
    return mModules;
}

Module* ModuleManager::getModule(const std::string& name) const
{
    for (const auto& module : mModules)
    {
        if (StringUtils::equalsIgnoreCase(module->mName, name))
        {
            return module.get();
        }
    }
    return nullptr;
}

void ModuleManager::removeModule(const std::string& name)
{
    for (auto it = mModules.begin(); it != mModules.end(); ++it)
    {
        if (StringUtils::equalsIgnoreCase((*it)->mName, name))
        {
            mModules.erase(it);
            return;
        }
    }
}

std::vector<std::shared_ptr<Module>>& ModuleManager::getModulesInCategory(int catId)
{
    static std::unordered_map<int, std::vector<std::shared_ptr<Module>>> categoryMap = {};
    if (categoryMap.contains(catId))
    {
        return categoryMap[catId];
    }

    // Cache category
    std::vector<std::shared_ptr<Module>> modules;
    for (const auto& module : mModules)
    {
        if (static_cast<int>(module->mCategory) == catId)
        {
            modules.push_back(module);
        }
    }

    categoryMap[catId] = modules;
    return categoryMap[catId];
}

std::unordered_map<std::string, std::shared_ptr<Module>> ModuleManager::getModuleCategoryMap()
{
    static std::unordered_map<std::string, std::shared_ptr<Module>> map;

    if (!map.empty())
    {
        return map;
    }

    for (const auto& module : mModules)
    {
        map[module->getCategory()] = module;
    }

    return map;
}

void ModuleManager::onClientTick()
{
    for (auto& module : mModules)
    {
        try
        {
            if (module->mWantedState != module->mEnabled)
            {
                module->mEnabled = module->mWantedState;
                spdlog::trace("onClientTick: calling {} on module {}", module->mEnabled ? "onEnable" : "onDisable", module->mName);
                if (module->mEnabled)
                {
                    module->onEnable();
                }
                else
                {
                    module->onDisable();
                }
            }
        } catch (const nlohmann::json::exception &e) {
            spdlog::error("Failed to enable/disable module {}: JSON error - {}", module->mName, e.what());
        } catch (const std::exception &e) {
            spdlog::error("Failed to enable/disable module {}: {}", module->mName, e.what());
        } catch (...)
        {
            spdlog::error("Failed to enable/disable module {}: unknown", module->mName);
        }
    }

    //auto holder = nes::make_holder<ClientTickEvent>();
    //gFeatureManager->mDispatcher->trigger(holder);
}

nlohmann::json ModuleManager::serialize() const
{
    nlohmann::json j;
    j["client"] = "Orphan";
    //j["version"] = ORPHAN_VERSION;
    j["modules"] = nlohmann::json::array();

    for (const auto& module : mModules)
    {
        j["modules"].push_back(module->serialize());
    }

    return j;
}

nlohmann::json ModuleManager::serializeModule(Module* module)
{
    // same as above but only for the specified module
    nlohmann::json j;
    j["client"] = "Orphan";
    //j["version"] = ORPHAN_VERSION;
    j["modules"] = nlohmann::json::array();

    j["modules"].push_back(module->serialize());

    return j;
}

static bool safeGetDivider(const nlohmann::json& j, const std::string& key, bool defaultValue = true) {
    if (!j.contains(key) || j[key].is_null()) return defaultValue;
    if (j[key].is_boolean()) return j[key].get<bool>();
    if (j[key].is_number_integer() || j[key].is_number_unsigned()) return j[key].get<int>() != 0;
    if (j[key].is_string()) {
        std::string val = j[key].get<std::string>();
        return val == "true" || val == "1";
    }
    return defaultValue;
}

void ModuleManager::deserialize(const nlohmann::json& j, bool showMessages)
{
    std::string version;
    if (j.contains("version") && !j["version"].is_null()) {
        if (j["version"].is_string()) version = j["version"].get<std::string>();
        else if (j["version"].is_number_integer() || j["version"].is_number_unsigned()) version = std::to_string(j["version"].get<int>());
        else if (j["version"].is_number_float()) version = std::to_string(j["version"].get<double>());
        else version = "";
    } else {
        version = "";
    }
    //std::string currentVersion = ORPHAN_VERSION;

    //if (version != currentVersion)
    //{
    //    spdlog::warn("Config version mismatch. Expected: {}, Got: {}", currentVersion, version);
    //    ChatUtils::displayClientMessage("§eWarning: The specified config is from a different version of Orphan. §cSome settings may not be loaded§e.");
    //}

    int modulesLoaded = 0;
    int settingsLoaded = 0;
    std::vector<std::string> moduleNames;
    for (const auto& module : mModules)
    {
        moduleNames.push_back(module->mName);
    }
    for (const auto& module : j["modules"])
    {
        std::string name;
        if (module.contains("name") && !module["name"].is_null()) {
            if (module["name"].is_string()) name = module["name"].get<std::string>();
            else if (module["name"].is_number_integer() || module["name"].is_number_unsigned()) name = std::to_string(module["name"].get<int>());
            else if (module["name"].is_number_float()) name = std::to_string(module["name"].get<double>());
            else name = "";
        } else {
            name = "";
        }
        std::erase(moduleNames, name);
        bool enabled = false;
        if (module.contains("enabled") && !module["enabled"].is_null()) {
            if (module["enabled"].is_boolean()) enabled = module["enabled"].get<bool>();
            else if (module["enabled"].is_number_integer() || module["enabled"].is_number_unsigned()) enabled = module["enabled"].get<int>() != 0;
            else if (module["enabled"].is_string()) {
                std::string val = module["enabled"].get<std::string>();
                enabled = (val == "true" || val == "1");
            }
        }
        int keybind = 0;
        if (module.contains("key") && !module["key"].is_null()) {
            if (module["key"].is_number_integer() || module["key"].is_number_unsigned()) keybind = module["key"].get<int>();
            else if (module["key"].is_string()) {
                try { keybind = std::stoi(module["key"].get<std::string>()); } catch (...) { keybind = 0; }
            }
        }

        auto* mod = getModule(name);
        if (mod)
        {
            mod->mWantedState = enabled;
            mod->mKey = keybind;
            std::vector<std::string> settingNames;
            for (const auto& setting : mod->mSettings)
            {
                settingNames.push_back(setting->mName);
            }
            if (module.contains("settings"))
            {
                for (const auto& setting : module["settings"].items())
                {
                    try
                    {
                        const auto& settingValue = setting.value();
                        std::string settingName;
                        if (settingValue.contains("name") && !settingValue["name"].is_null()) {
                            if (settingValue["name"].is_string()) settingName = settingValue["name"].get<std::string>();
                            else if (settingValue["name"].is_number_integer() || settingValue["name"].is_number_unsigned()) settingName = std::to_string(settingValue["name"].get<int>());
                            else if (settingValue["name"].is_number_float()) settingName = std::to_string(settingValue["name"].get<double>());
                            else settingName = "";
                        } else {
                            settingName = "";
                        }
                        auto normalize = [](const std::string& s) {
                            std::string out;
                            for (char c : s) {
                                if (c != '-' && c != ' ')
                                    out += std::tolower(static_cast<unsigned char>(c));
                            }
                            return out;
                        };
                        bool erased = std::erase(settingNames, settingName) > 0;
                        if (!erased) {
                            for (auto it = settingNames.begin(); it != settingNames.end(); ++it) {
                                if (normalize(*it) == normalize(settingName)) {
                                    settingNames.erase(it);
                                    break;
                                }
                            }
                        }

                        auto* set = mod->getSetting(settingName);
                        if (set)
                        {
                            if (set->mType == SettingType::Bool)
                            {
                                auto* boolSetting = static_cast<BoolSetting*>(set);
                                bool boolValue = false;
                                if (settingValue.contains("boolValue") && !settingValue["boolValue"].is_null()) {
                                    if (settingValue["boolValue"].is_boolean()) boolValue = settingValue["boolValue"].get<bool>();
                                    else if (settingValue["boolValue"].is_number_integer() || settingValue["boolValue"].is_number_unsigned()) boolValue = settingValue["boolValue"].get<int>() != 0;
                                    else if (settingValue["boolValue"].is_string()) {
                                        std::string val = settingValue["boolValue"].get<std::string>();
                                        boolValue = (val == "true" || val == "1");
                                    }
                                }
                                boolSetting->mValue = boolValue;
                                int key = -1;
                                if (settingValue.contains("key") && !settingValue["key"].is_null()) {
                                    if (settingValue["key"].is_number_integer() || settingValue["key"].is_number_unsigned()) key = settingValue["key"].get<int>();
                                    else if (settingValue["key"].is_string()) {
                                        try { key = std::stoi(settingValue["key"].get<std::string>()); } catch (...) { key = -1; }
                                    }
                                }
                                boolSetting->mKey = key;
                            }
                            else if (set->mType == SettingType::Number)
                            {
                                auto* numberSetting = static_cast<NumberSetting*>(set);
                                if (settingValue.contains("numberValue") && !settingValue["numberValue"].is_null()) {
                                    if (settingValue["numberValue"].is_number())
                                        numberSetting->mValue = settingValue["numberValue"].get<float>();
                                    else if (settingValue["numberValue"].is_string())
                                        numberSetting->mValue = std::stof(settingValue["numberValue"].get<std::string>());
                                }
                            }
                            else if (set->mType == SettingType::Divider)
                            {
                                auto* dividerSetting = static_cast<DividerSetting*>(set);
                                dividerSetting->mDivider = safeGetDivider(settingValue, "divider", true);
                            }
                            else if (set->mType == SettingType::Enum)
                            {
                                auto* enumSetting = static_cast<EnumSetting*>(set);
                                int enumValue = 0;
                                if (settingValue.contains("enumValue") && !settingValue["enumValue"].is_null()) {
                                    if (settingValue["enumValue"].is_number_integer() || settingValue["enumValue"].is_number_unsigned()) enumValue = settingValue["enumValue"].get<int>();
                                    else if (settingValue["enumValue"].is_string()) {
                                        try { enumValue = std::stoi(settingValue["enumValue"].get<std::string>()); } catch (...) { enumValue = 0; }
                                    }
                                }
                                if (enumValue >= 0 && enumValue < enumSetting->mValues.size())
                                    enumSetting->mValue = enumValue;
                                else
                                {
                                    spdlog::warn("Invalid enum value for setting {} in module {}", settingName, name);
                                    if (showMessages) ChatUtils::displayClientMessage("§cInvalid enum value for setting §6" + settingName + "§c in module §6" + name + "§c.");
                                }
                            } else if (set->mType == SettingType::Color)
                            {
                                auto* colorSetting = static_cast<ColorSetting*>(set);
                                if (settingValue.contains("colorValue") && settingValue["colorValue"].is_array() && settingValue["colorValue"].size() == 4) {
                                    for (int i = 0; i < 4; i++) {
                                        if (settingValue["colorValue"][i].is_number())
                                            colorSetting->mValue[i] = settingValue["colorValue"][i].get<float>();
                                        else if (settingValue["colorValue"][i].is_string())
                                            colorSetting->mValue[i] = std::stof(settingValue["colorValue"][i].get<std::string>());
                                    }
                                }
                            }
                            else if (set->mType == SettingType::Keybind)
                            {
                                auto* keybindSetting = static_cast<KeybindSetting*>(set);
                                int key = 0;
                                if (settingValue.contains("keybind") && !settingValue["keybind"].is_null()) {
                                    if (settingValue["keybind"].is_number_integer() || settingValue["keybind"].is_number_unsigned())
                                        key = settingValue["keybind"].get<int>();
                                    else if (settingValue["keybind"].is_string()) {
                                        try { key = std::stoi(settingValue["keybind"].get<std::string>()); } catch (...) { key = 0; }
                                    }
                                }
                                keybindSetting->mKey = key;
                            }

                            settingsLoaded++;
                        } else
                        {
                            spdlog::warn("Setting {} not found for module {}", setting.key(), name);
                            if (showMessages) ChatUtils::displayClientMessage("§cSetting §6" + settingName + "§c not found for module §6" + name + "§c.");
                        }
                    } catch (const std::exception& e)
                    {
                        spdlog::warn("Failed to load setting {} for module {}: {}", setting.key(), name, e.what());
                        if (showMessages) ChatUtils::displayClientMessage("§cFailed to load setting §6" + setting.key() + "§c for module §6" + name + "§c.");
                    }
                }

                modulesLoaded++;
            }

            for (const auto& settingName : settingNames)
            {
                auto* set = mod->getSetting(settingName);
                if (set && set->mType == SettingType::Divider)
                    continue;
                spdlog::warn("Setting {} not found for module {}, default value will be used", settingName, name);
                if (showMessages) ChatUtils::displayClientMessage("§cSetting §6" + settingName + "§c not found for module §6" + name + "§c, default value will be used.");
            }
        } else
        {
            spdlog::warn("Module {} not found", name);
            if (showMessages) ChatUtils::displayClientMessage("§cModule §6" + name + "§c not found.");
        }
    }

    for (const auto& moduleName : moduleNames)
    {
        spdlog::warn("Module {} not found in config, using default settings", moduleName);
        if (showMessages) ChatUtils::displayClientMessage("§cModule §6" + moduleName + "§c not found in config, using default settings.");
    }

    spdlog::info("Loaded {} modules and {} settings from config", modulesLoaded, settingsLoaded);
}