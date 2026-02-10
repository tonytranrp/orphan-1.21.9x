//
// Created by vastrakai on 7/7/2024.
//

#include "SetupAndRenderHook.hpp"

#include <SDK/Minecraft/mce.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/Rendering/LevelRenderer.hpp>
#include <Features/Events/DrawImageEvent.hpp>

#include <glm/glm.hpp>
#include <SDK/Minecraft/Rendering/MinecraftUIRenderContext.hpp>

#include "D3DHook.hpp"
#include "src/Features/Modules/Visual/ClickGui.hpp"
#include "src/Features/Modules/Visual/NoRender.hpp"
#include <SDK/Minecraft/Rendering/GuiData.hpp>

std::unique_ptr<Detour> SetupAndRenderHook::mSetupAndRenderDetour;
std::unique_ptr<Detour> SetupAndRenderHook::mDrawImageDetour;



namespace UILayer
{
    static std::string Ingame_CanMoveScreen("UnknownLayer");

    // ingame menus
    static std::string Ingame_ChatScreen("chat.chat_screen");
    static std::string Ingame_HudScreen("hud.hud_screen");
    static std::string Ingame_PauseScreen("pause.pause_screen");
    static std::string Ingame_InventoryScreen("crafting.inventory_screen");
    static std::string Ingame_DeathScreen("death.death_screen");
    static std::string Ingame_Progress_ModalProcessScreen("progress.modal_progress_screen");

    // ingame container menus
    static std::string Ingame_SmithingTableScreen("smithing_table.smithing_table_screen");
    static std::string Ingame_BlastFurnaceScreen("blast_furnace.blast_furnace_screen");
    static std::string Ingame_BrewingStandScreen("brewing_stand.brewing_stand_screen");
    static std::string Ingame_StoneCutterScreen("stonecutter.stonecutter_screen");
    static std::string Ingame_CartographyScreen("cartography.cartography_screen");
    static std::string Ingame_SmallChestScreen("chest.small_chest_screen");
    static std::string Ingame_LargeChestScreen("chest.large_chest_screen");
    static std::string Ingame_ShulkerBoxScreen("chest.shulker_box_screen");
    static std::string Ingame_EnderChestScreen("chest.ender_chest_screen");
    static std::string Ingame_BarrelScreen("chest.barrel_screen");
    static std::string Ingame_CraftingScreen("crafting.crafting_screen");
    static std::string Ingame_EnchantingScreen("enchanting.enchanting_screen");
    static std::string Ingame_BeaconScreen("beacon.beacon_screen");
    static std::string Ingame_FurnaceScreen("furnace.furnace_screen");
    static std::string Ingame_SmokerScreen("smoker.smoker_screen");
    static std::string Ingame_DispenserScreen("redstone.dispenser_screen");
    static std::string Ingame_DropperScreen("redstone.dropper_screen");
    static std::string Ingame_HopperScreen("redstone.hopper_screen");
    static std::string Ingame_InBedScreen("bed.in_bed_screen");
    static std::string Ingame_LoomScreen("loom.loom_screen");
    static std::string Ingame_BookScreen("book.book_screen");
    static std::string Ingame_GrindStoneScreen("grindstone.grindstone_screen");
    static std::string Ingame_AnvilScreen("anvil.anvil_screen");
    static std::string Ingame_ThirdPartyScreen("server_form.third_party_server_screen");

    static std::string Toast_ToastScreen("toast_screen.toast_screen");

    static std::string Debug_DebugScreen("debug_screen.debug_screen");

    static std::string ProfileCard_ProfileCardScreen("profile_card.profile_card_screen");

    static std::string Start_StartScreen("start.start_screen");

    static std::string Play_PlayScreen("play.play_screen");

    static std::string XBL_FriendFinderScreen("xbl_friend_finder.xbl_friend_finder");

    static std::string Events_GatheringInfoScreen("gathering_info.gathering_info_screen");

    static std::string WorldTemplates_WorldTemplateScreen("world_templates.world_templates_screen");

    static std::string Feed_FeedScreen("feed.feed_screen");
    static std::string Feed_CommentScreen("comment.comment_screen");

    static std::string Progress_LoadingScreen("progress.world_loading_progress_screen");
    static std::string Progress_SavingScreen("progress.world_saving_progress_screen");
    static std::string Progress_RealmLoadingScreen("progress.realms_loading_progress_screen");

    static std::string Settings_ScreenControls("settings.screen_controls_and_settings");
    static std::string Settings_WorldEditScreen("settings.screen_world_edit");
    static std::string Settings_HowToScreen("how_to_play.how_to_play_screen");
    static std::string Settings_SafeZoneScreen("safe_zone.safe_zone_screen");
    static std::string Settings_AddExternalServerEditScreen("add_external_server.add_external_server_screen_edit");

    static std::string Store_DataDrivenScreen("store_layout.store_data_driven_screen");
    static std::string Store_InventoryScreen("store_inventory.store_inventory_screen");

    static std::string RealmsPlus_RealmsPlusPDPScreen("realmsPlus.realms_plus_pdp_screen");
    static std::string RealmsPlus_SlotsScreen("realms_slots.realms_slots_screen");
    static std::string RealmsPlus_WorldEditScreen("settings.screen_world_slot_edit");
    static std::string RealmsPlus_LocalWorldPickerScreen("local_world_picker.local_world_picker_screen");
    static std::string RealmsPlus_RealmManageScreen("settings.screen_realm_manage");

    static std::string Persona_ProfileScreen("profile.profile_screen");
    static std::string Persona_PersonaScreen("persona.persona_screen");

    static std::vector<std::string> MainMenu_Screen = {
            Start_StartScreen,
            Play_PlayScreen,
            XBL_FriendFinderScreen,
            WorldTemplates_WorldTemplateScreen,
            //Progress_LoadingScreen, // all but the realms one cuz that needs special care love from tozic
            //Progress_SavingScreen,
            Settings_AddExternalServerEditScreen, // all settings menu
            Settings_WorldEditScreen,
            Settings_HowToScreen,
            Settings_ScreenControls,
            Settings_SafeZoneScreen,
            Persona_PersonaScreen, // all of persona stuff
            Persona_ProfileScreen,
            Feed_CommentScreen, // realms comments stuff
            Feed_FeedScreen,
            RealmsPlus_LocalWorldPickerScreen, // all realms management stuff
            RealmsPlus_RealmManageScreen,
            RealmsPlus_RealmsPlusPDPScreen,
            RealmsPlus_SlotsScreen,
            RealmsPlus_WorldEditScreen,
            Store_DataDrivenScreen, // all of the marketplace store
            Store_InventoryScreen
    };

    bool Is(ScreenView* screen, std::string screenType)
    {
        return screen->mTree->mRootUIControl->getLayerName() == screenType;
    }

    bool Is(ScreenView* screen, std::vector<std::string> screenTypes)
    {
        std::string currentScreenType = screen->mTree->mRootUIControl->getLayerName();

        for (std::string type : screenTypes)
        {
            if (currentScreenType == type)
            {
                return true;
            }
        }

        return false;
    }

    bool IsNot(ScreenView* screen, std::string screenType)
    {
        return screen->mTree->mRootUIControl->getLayerName() != screenType;
    }

    bool IsNot(ScreenView* screen, std::vector<std::string> screenTypes)
    {
        std::string currentScreenType = screen->mTree->mRootUIControl->getLayerName();

        for (std::string type : screenTypes)
        {
            if (currentScreenType == type)
            {
                return false;
            }
        }

        return true;
    }
}

class ResourceLocation {
public:
    uint64_t    mFileSystem;        //0x0000
    std::string mFilePath;  //0x0008
    uint64_t    mPathHash{};
    uint64_t    mFullHash{};

    ResourceLocation() = default;

    ResourceLocation(bool external, std::string filePath) {
        this->mFilePath = filePath;
        if (external)
            this->mFileSystem = 2;
        else this->mFileSystem = 0;

        _computeHashes();
    };

    bool operator==(const ResourceLocation& other) const {
        if (this->mFileSystem != other.mFileSystem || this->mPathHash != other.mPathHash) return false;
        return this->mFilePath == other.mFilePath;
    }

    bool operator<(const ResourceLocation& other) const {
        return this->mFilePath < other.mFilePath;
    }
private:
    void _computeHashes() // Should add a Local variable for path hash computation
    {
        const uint64_t FNV_OFFSET_BASIS = 0xCBF29CE484222325u;
        const uint64_t FNV_PRIME = 0x100000001B3u;

        uint64_t _pathHash = FNV_OFFSET_BASIS;
        if (!this->mFilePath.empty()) {
            for (char c : this->mFilePath) {
                _pathHash *= FNV_PRIME;
                _pathHash ^= c;
            }
        }
        else {
            _pathHash = 0;
        }

        uint64_t fileSystemHash = FNV_OFFSET_BASIS ^ static_cast<uint8_t>(this->mFileSystem);
        fileSystemHash *= FNV_PRIME;

        this->mPathHash = _pathHash;
        this->mFullHash = _pathHash ^ fileSystemHash;
    }
};


void* SetupAndRenderHook::onSetupAndRender(ScreenView* screenView, MinecraftUIRenderContext* mcuirc)
{
    auto ci = ClientInstance::get();
    auto original = mSetupAndRenderDetour->getOriginal<&SetupAndRenderHook::onSetupAndRender>();

    static bool once = false;
    if (!once)
    {
        once = true;
        initVt(mcuirc);
    }

    if (!ci) return original(screenView, mcuirc);

    static auto* clickGui = gFeatureManager->mModuleManager->getModule<ClickGui>();
    std::string screenName = ci->getScreenName();

    Textures::LoadAll(mcuirc);

    if (clickGui && clickGui->mEnabled) {
        ci->releaseMouse();
    } else if (screenName == "hud_screen") {
        ci->grabMouse();
    }

    auto player = ClientInstance::get()->getLocalPlayer();

    glm::vec3 origin = glm::vec3(0, 0, 0);
    glm::vec3 playerPos = glm::vec3(0, 0, 0);

    if (player && ci->getLevelRenderer())
    {
        origin = *ci->getLevelRenderer()->getRendererPlayer()->getCameraPos();
        playerPos = player->getRenderPositionComponent()->mPosition;
    }

    if (D3DHook::FrameTransforms) D3DHook::FrameTransforms->push({ ci->getViewMatrix(), origin, playerPos, ci->getFov() });

    if (UILayer::Is(screenView, UILayer::MainMenu_Screen))
    {
        static bool bannerDownloaded = false;
        if (!bannerDownloaded) {
            FileUtils::downloadFile("https://i.imgur.com/qKfRMRg.jpeg", FileUtils::getOrphanDir() + "\\banner.png");
            bannerDownloaded = true;
        }
        bool textureExternal = true; // true = uses an external texture
        std::string filePath = FileUtils::getOrphanDir() + "banner.png";
        mce::TexturePtr* texturePtr = new mce::TexturePtr();
        mcuirc->getTexture(texturePtr, new ResourceLocation(textureExternal, filePath));
        glm::vec2 pos = glm::vec2(0, 0);
        glm::vec2 size = glm::vec2(ClientInstance::get()->getGuiData()->mResolution.x, ClientInstance::get()->getGuiData()->mResolution.y);
        glm::vec2 uv = glm::vec2(0, 0);
        glm::vec2 uv2 = glm::vec2(1, 1);
        ImColor imageColor = ImColor(255, 255, 255);
        mcuirc->drawImage(texturePtr->mClientTexture.get(), &pos, &size, &uv, &uv2, false);
        static HashedString flushString = HashedString(0xA99285D21E94FC80, "ui_flush");
        mcuirc->flushImages(imageColor, 1, flushString);


    }

    return original(screenView, mcuirc);
}

/*void SetupAndRenderHook::onDrawText(MinecraftUIRenderContext* _this, Font* font, glm::vec4 const& pos, std::string* str, mce::Color const& colour, float alpha, float alinM, struct TextMeasureData* const& textdata, struct CaretMeasureData* const& caretdata)
{
    if (!str) return;
    auto original = mDrawImageDetour->getOriginal<&SetupAndRenderHook::onDrawText>();
    original(_this, font, pos, str, colour, alpha, alinM, textdata, caretdata);
}*/

void* SetupAndRenderHook::onDrawImage(MinecraftUIRenderContext* context, BedrockTextureData* texture, glm::vec2* imagePos, glm::vec2* size, glm::vec2* uv, mce::Color* color, void* unk)
{
    auto original = mDrawImageDetour->getOriginal<&SetupAndRenderHook::onDrawImage>();

    static bool yourGayEdest = false;

    if (texture == Textures::mTitle)
    {
        return nullptr;
    }

    static auto* noRender = gFeatureManager->mModuleManager->getModule<NoRender>();
    if (noRender) {

        if (noRender->mNoFire.mValue && (texture == Textures::mFire0 || texture == Textures::mFire1)) {
            return nullptr;
        }
    }

    nes::event_holder<DrawImageEvent> holder = nes::make_holder<DrawImageEvent>(context, texture, imagePos, size, uv, color);
    gFeatureManager->mDispatcher->trigger(holder);
    if (holder->isCancelled()) return nullptr;

    return original(context, texture, imagePos, size, uv, color, unk);
}

void SetupAndRenderHook::initVt(void* ctx)
{
    const auto vtable = *static_cast<uintptr_t**>(ctx);
    mDrawImageDetour = std::make_unique<Detour>("MinecraftUIRenderContext::drawImage", reinterpret_cast<void*>(vtable[OffsetProvider::MinecraftUIRenderContext_drawImage]), &SetupAndRenderHook::onDrawImage);
    //mDrawImageDetour = std::make_unique<Detour>("MinecraftUIRenderContext::drawText", reinterpret_cast<void*>(vtable[OffsetProvider::MinecraftUIRenderContext_drawText]), &SetupAndRenderHook::onDrawText);
    mDrawImageDetour->enable();
}

void SetupAndRenderHook::init()
{
    mSetupAndRenderDetour = std::make_unique<Detour>("ScreenView::setupAndRender", reinterpret_cast<void*>(SigManager::ScreenView_setupAndRender), &SetupAndRenderHook::onSetupAndRender);
}
