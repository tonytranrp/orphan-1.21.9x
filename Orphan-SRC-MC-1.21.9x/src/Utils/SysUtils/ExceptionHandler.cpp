//
// Created by vastrakai on 7/7/2024.
//

#include "ExceptionHandler.hpp"

#include <build/build_info.h>
#include <build_info.h>
#include <Windows.h>
#include <string>
#include <spdlog/spdlog.h>
#include <Features/Configs/ConfigManager.hpp>
#include <iomanip>

#include "StackWalker.hpp"
#include <Utils/FileUtils.hpp>

LONG WINAPI TopLevelExceptionHandler(const PEXCEPTION_POINTERS pExceptionInfo)
{
    // Get the exception code
    auto exceptionCode = pExceptionInfo->ExceptionRecord->ExceptionCode;

    std::string text = "An unhandled exception occurred, your config was saved. (0x" + fmt::format("{:X}", exceptionCode) + ")\nStack Trace:\n";

    // Get the stack trace using stackwalker
    StackWalker sw;
    std::vector<std::string> stackTrace = sw.ShowCallstack(GetCurrentThread(), pExceptionInfo->ContextRecord);
    for (const auto& line : stackTrace)
    {
        text += line + "\n";
    }
    spdlog::error(text);

    // Save config on crash
    std::string crashConfigName;
    if (gFeatureManager && gFeatureManager->mModuleManager && !ConfigManager::LastLoadedConfig.empty()) {
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "crash_" << std::put_time(std::localtime(&nowTime), "%Y-%m-%d_%H-%M-%S");
        crashConfigName = ss.str();
        ExceptionHandler::makeCrashConfig(crashConfigName);
        // Update the exception message to include the config name
        text = "An unhandled exception occurred, your config was saved as: " + crashConfigName + ".json (0x" + fmt::format("{:X}", exceptionCode) + ")\nStack Trace:\n";
    }

    ExceptionHandler::makeCrashLog(text, exceptionCode);

    auto result = MessageBoxA(nullptr, LPCSTR(text.c_str()), "Unhandled Exception", MB_ABORTRETRYIGNORE | MB_ICONERROR);

    stackTrace.clear();
    sw.~StackWalker();


    switch (result)
    {
    case IDABORT:   // Terminates the process
        exit(1);
    case IDRETRY:
        return EXCEPTION_CONTINUE_EXECUTION;
    case IDIGNORE:
        return EXCEPTION_CONTINUE_SEARCH;
    default:
        return EXCEPTION_EXECUTE_HANDLER;
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void ExceptionHandler::init()
{
    SetUnhandledExceptionFilter(TopLevelExceptionHandler);
}

void ExceptionHandler::makeCrashLog(const std::string& text, DWORD exceptionCode)
{
    std::string excPath = FileUtils::getOrphanDir() + xorstr_("crash.log");

    std::ofstream excFile(excPath, std::ios::app);

    if (excFile.is_open())
    {
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        excFile << xorstr_("----------------- Crash at ") << std::put_time(std::localtime(&nowTime), xorstr_("%Y-%m-%d %X")) << xorstr_("\n");
        excFile << text << xorstr_("\n\n");


        auto modules = gFeatureManager ? gFeatureManager->mModuleManager->getModules() : std::vector<std::shared_ptr<Module>>();
        for (const auto& module : modules)
        {
            excFile << module->mName << xorstr_(" - ") << (module->mEnabled ? xorstr_("Enabled") : xorstr_("Disabled")) << xorstr_("\n");
        }
        if (gFeatureManager)
        {
            excFile << xorstr_("Module count: ") << modules.size() << xorstr_("\n");
        } else
        {
            excFile << xorstr_("(FeatureManager not initialized)\n");
        }

        excFile << xorstr_("Exception Code: 0x") << fmt::format(xorstr_("{:X}"), exceptionCode) << xorstr_("\n");
        excFile << xorstr_("Orphan commit: ") << ORPHAN_BUILD_VERSION << xorstr_("\n");
        excFile << xorstr_("Orphan branch: ") << ORPHAN_BUILD_BRANCH << xorstr_("\n");
        excFile << xorstr_("Orphan commit msg: ") << ORPHAN_BUILD_COMMIT_MESSAGE << xorstr_("\n");
        excFile << xorstr_("Minecraft version: ") << ProcUtils::getVersion() << xorstr_("\n");
        excFile << xorstr_("----------------------------------------\n");
        excFile.flush();
        excFile.close();
    }
}

void ExceptionHandler::makeCrashConfig(const std::string& name)
{
    std::string configDir = ConfigManager::getConfigPath();
    std::string crashConfigPath = configDir + name + ".json";
    try {
        FileUtils::createDirectory(configDir);
        spdlog::info("Ensured config directory exists: {}", configDir);
    } catch (const std::exception& e) {
        spdlog::error("Failed to create config directory: {} ({})", configDir, e.what());
        return;
    } catch (...) {
        spdlog::error("Failed to create config directory: {} (unknown error)", configDir);
        return;
    }
    nlohmann::json j;
    try {
        j = gFeatureManager->mModuleManager->serialize();
    } catch (const std::exception& e) {
        spdlog::error("Failed to serialize config: {}", e.what());
        return;
    } catch (...) {
        spdlog::error("Failed to serialize config: unknown error");
        return;
    }
    std::ofstream file(crashConfigPath);
    if (!file.is_open()) {
        spdlog::error("Failed to open crash config file for writing: {}", crashConfigPath);
        return;
    }
    try {
        file << j.dump(4);
        file.close();
        spdlog::info("Crash config saved successfully to {}", crashConfigPath);
    } catch (const std::exception& e) {
        spdlog::error("Failed to write crash config file: {} ({})", crashConfigPath, e.what());
    } catch (...) {
        spdlog::error("Failed to write crash config file: {} (unknown error)", crashConfigPath);
    }
}
