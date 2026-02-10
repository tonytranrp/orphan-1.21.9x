//
// Created by vastrakai on 7/29/2024.
//

#include "BuildInfoCommand.hpp"
#include <build/build_info.h>

void BuildInfoCommand::execute(const std::vector<std::string>& args)
{
    ChatUtils::displayClientMessage("§6Orphan build info");
    ChatUtils::displayClientMessage("§6- §eBuild date§7: {}", __DATE__);
    ChatUtils::displayClientMessage("§6- §eBuild time§7: {}", __TIME__);
    ChatUtils::displayClientMessage("§6- §eOrphan version§7: {}", std::string(ORPHAN_BUILD_VERSION_SHORT));
    ChatUtils::displayClientMessage("§6- §eMinecraft version§7: {}", ProcUtils::getVersion());
    ChatUtils::displayClientMessage("§6- §eIntended Minecraft version§7: {}", std::strcmp(ORPHAN_INTENDED_VERSION, "") != 0 ? ORPHAN_INTENDED_VERSION : ORPHAN_BUILD_VERSION_SHORT);
    ChatUtils::displayClientMessage("§6- §eCommit message§7: {}", std::string(ORPHAN_BUILD_COMMIT_MESSAGE));
    ChatUtils::displayClientMessage("§6- §eCommit hash§7: {}", std::string(ORPHAN_BUILD_VERSION));
    //ChatUtils::displayClientMessage("§6- §eCompiled by§7: {}", std::string(ORPHAN_PC_USERNAME));
    ChatUtils::displayClientMessage("§6- §eBranch§7: {}", std::string(ORPHAN_BUILD_BRANCH));

    // so it doesn't get optimized out
    // auto idHolder = std::make_unique<IdHolder>(DISCORD_USER_ID);
    // idHolder.reset();
    ChatUtils::displayClientMessage("§e{}§6 files changed locally compared to the last commit", STRING(ORPHAN_FILES_CHANGED_COUNT));
}

std::vector<std::string> BuildInfoCommand::getAliases() const
{
    return {"bi"};
}

std::string BuildInfoCommand::getDescription() const
{
    return "Displays information about the current client build";
}

std::string BuildInfoCommand::getUsage() const
{
    return "Usage: .buildinfo";
}