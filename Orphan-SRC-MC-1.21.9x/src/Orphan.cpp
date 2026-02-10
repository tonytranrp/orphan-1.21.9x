//
// Created by vastrakai on 6/24/2024.
//

#include "Orphan.hpp"


#include <fstream>
#include <Features/FeatureManager.hpp>
#include <Features/Configs/ConfigManager.hpp>
#include <Hook/HookManager.hpp>
#include <Hook/Hooks/RenderHooks/D3DHook.hpp>

#include <SDK/OffsetProvider.hpp>

#include <SDK/SigManager.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/MinecraftGame.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>

#include <Features/Auth/Authorization.hpp>

#include "spdlog/sinks/stdout_color_sinks-inl.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <winrt/base.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <build/build_info.h>
#include <SDK/Minecraft/Rendering/GuiData.hpp>
//#include <Utils/OAuthUtils.hpp>
#include <Utils/SysUtils/xorstr.hpp>

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

std::string title = "[orphan client]";

void setTitle(std::string title)
{
    auto w = winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [title]() {
        winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView().Title(winrt::to_hstring(title));
        });
}

std::vector<unsigned char> gBpBytes = { 0x1c }; // Defines the new offset for mInHandSlot
DEFINE_PATCH_FUNC(patchInHandSlot, SigManager::ItemInHandRenderer_renderItem_bytepatch2 + 2, gBpBytes);

// called using winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [&]()
void Orphan::init(HMODULE hModule)
{
    // Not doing this could cause crashes if you inject too soon
    // Honestly, I don't think this helps much but it's not a bad idea to have it here
    while (ProcUtils::getModuleCount() < 130) std::this_thread::sleep_for(std::chrono::milliseconds(1));

    int64_t start = NOW;

    mModule = hModule;
    mInitialized = true;

    Logger::initialize();

    console = spdlog::stdout_color_mt(CC(21, 207, 148) + "orphan" + ANSI_COLOR_RESET, spdlog::color_mode::automatic);

    // Create a file logger sink
    std::string logFile = FileUtils::getOrphanDir() + xorstr_("orphan.log");

    // Don't use the file sink if the log file doesn't exist
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[" + CC(255, 135, 0) + "%H:%M:%S.%e" + ANSI_COLOR_RESET + "] [%n] [%^%l%$] %v");
    console_sink->set_level(spdlog::level::trace);
    console->set_level(spdlog::level::trace);
    console->set_pattern("[" + CC(255, 135, 0) + "%H:%M:%S.%e" + ANSI_COLOR_RESET + "] [%n] [%^%l%$] %v");
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(CC(21, 207, 148) + "orphan" + ANSI_COLOR_RESET, spdlog::sinks_init_list{ console_sink }));

    console->info("Welcome to " + CC(0, 255, 0) + "Orphan" + ANSI_COLOR_RESET + " by player5, EDest10, & rekitrelt!"
    );

    spdlog::info("Minecraft version: {}", ProcUtils::getVersion());

    ExceptionHandler::init();

    FileUtils::validateDirectories();

    // Change the window title

   /* Auth auth;
    if (title.contains("private") && !auth.isPrivateUser()) {
        title = "[orphan client]";
    }
    setTitle(title);*/


    sHWID = GET_HWID().toString();
    spdlog::info("HWID: {}", sHWID);

    if (MH_Initialize() != MH_OK)
    {
        console->critical("Failed to initialize MinHook!");
    }

    Prefs = PreferenceManager::load();

    HWND hwnd = ProcUtils::getMinecraftWindow(); // Cache the window handle

    if (Prefs->mEnforceDebugging)
    {
        while (!IsDebuggerPresent())
        {
            MessageBoxA(nullptr, "Please attach a debugger to continue.\nThis message is being shown because of your preference settings.", "Orphan", MB_OK | MB_ICONERROR);
            spdlog::info("Waiting for debugger...");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    //isTimeSyncedCheck();
    //killSwitchIfNeeded();

    console->info("initializing signatures...");
    int64_t sstart = NOW;
    OffsetProvider::initialize();
    SigManager::initialize();
    int64_t send = NOW;

    int failedSigs = 0;

    // Go through all signatures and print any that failed
    for (const auto& [sig, result] : SigManager::mSigs)
    {
        if (result == 0)
        {
            console->critical("[signatures] Failed to find signature: {}", sig);
            failedSigs++;
        }
    }

    for (const auto& [sig, result] : OffsetProvider::mSigs)
    {
        if (result == 0)
        {
            console->critical("[offsets] Failed to find offset: {}", sig);
            failedSigs++;
        }
    }

    if (failedSigs > 0)
    {
        console->critical("Failed to find {} signatures/offsets!", failedSigs);
        console->critical("Orphan should not be used in this state.");
        console->info("Type 'DEBUG' to continue, or press ENTER to exit.");
        std::string input;
        std::getline(std::cin, input);
        if (input != "DEBUG")
        {
            SigManager::deinitialize();
            OffsetProvider::deinitialize();
            Logger::deinitialize();

            setTitle("");
            FreeLibraryAndExitThread(hModule, 0);
        }
        ExceptionHandler::makeCrashLog("Failed to find signatures/offsets!", 0xFF01);

    }

    console->info("initialized signatures in {}ms", send - sstart);

    console->info("clientinstance addr @ 0x{:X}", reinterpret_cast<uintptr_t>(ClientInstance::get()));
    console->info("mcgame from clientinstance addr @ 0x{:X}", reinterpret_cast<uintptr_t>(ClientInstance::get()->getMinecraftGame()));
    console->info("localplayer addr @ 0x{:X}", reinterpret_cast<uintptr_t>(ClientInstance::get()->getLocalPlayer()));

    if (failedSigs > 0)
    {
        console->info("Press ENTER to continue...");
        std::string input;
        std::getline(std::cin, input);
    }


    gFeatureManager = std::make_shared<FeatureManager>();
    gFeatureManager->init();

    console->info("initializing hooks...");
    HookManager::init(false);

    console->info("initialized in {}ms", NOW - start);

    ClientInstance::get()->getMinecraftGame()->playUi("beacon.activate", 1, 1.0f);
    ChatUtils::displayClientMessage("Initialized!");

    console->info("Press END to eject dll.");
    mLastTick = NOW;

    mThread = std::thread(&Orphan::shutdownThread);
    mThread.detach();
}

void Orphan::shutdownThread()
{
    // Wait for the user to press END
    bool firstCall = true;
    bool isLpValid = false;
    while (!mRequestEject)
    {
        if (firstCall)
        {
            NotifyUtils::notify("Orphan initialized!", 5.0f, Notification::Type::Info);
            firstCall = false;

            /*std::string latestHash;
            latestHash = OAuthUtils::getLatestCommitHash();

            if(latestHash == xorstr_("403"))
            {
            }
            else if (!latestHash.empty())
            {
                if (latestHash != ORPHAN_BUILD_VERSION)
                {
                    console->warn("Orphan is out of date! Latest commit: {}", latestHash);
                    NotifyUtils::notify("There is a new version of Orphan available!\nIt is recommended to update.", 10.0f, Notification::Type::Warning);
                    ChatUtils::displayClientMessage("§aThere is a new version of Orphan available! Download it from the Discord server.");
                } else {
                    console->info("Orphan is up to date!");
                }
            }
            else if(latestHash.empty())
            {
            }*/
        }

        if (!isLpValid && ClientInstance::get()->getLocalPlayer())
        {
            isLpValid = true;
            HookManager::init(true); // Initialize the base tick hook

            if (!Prefs->mDefaultConfigName.empty())
            {
                if (ConfigManager::configExists(Prefs->mDefaultConfigName))
                {
                    ConfigManager::loadConfig(Prefs->mDefaultConfigName);
                }
                else
                {
                    console->warn("Default config does not exist! Clearing default config...");
                    NotifyUtils::notify("Default config does not exist! Clearing default config...", 10.0f, Notification::Type::Warning);
                    Prefs->mDefaultConfigName = "";
                    PreferenceManager::save(Prefs);
                }
            }
            else {
                console->warn("No default config set!");
            }
        }

        patchInHandSlot(ClientInstance::get()->getLocalPlayer() != nullptr);

        mLastTick = NOW;
        gFeatureManager->mModuleManager->onClientTick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    mRequestEject = true;

    setTitle("");

    patchInHandSlot(false);

    HookManager::shutdown();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    gFeatureManager->shutdown();

    // Shutdown
    console->warn("Shutting down...");

    ClientInstance::get()->getMinecraftGame()->playUi("beacon.deactivate", 1, 1.0f);
    ClientInstance::get()->getGuiData()->displayClientMessage("§aorphan§7 » §cEjected everywhere!");

    mInitialized = false;
    SigManager::deinitialize();
    OffsetProvider::deinitialize();

    // wait for threads to finishi
    Sleep(1000); // Give the user time to read the message

    Logger::deinitialize();
    FreeLibrary(mModule); // i don't understand this
}