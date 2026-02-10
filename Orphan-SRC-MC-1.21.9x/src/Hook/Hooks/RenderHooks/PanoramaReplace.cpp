//
// Created by vastrakai on 7/22/2024.
//

#include "PanoramaReplace.hpp"

#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/MinecraftSim.hpp>
#include <Features/Events/PanoramaRenderEvent.hpp>
#include <SDK/Minecraft/Options.hpp>

std::unique_ptr<Detour> PanoramaReplace::mDetour;

void PanoramaReplace::handlePanoramaReplace(
        PanoramaRenderer* that,
        MinecraftUIRenderContext* renderContext,
        ClientInstance* client,
        UIControl* owner)
{
    // Create event holder with exact parameters you got from IDA
    auto holder = nes::make_holder<PanoramaRenderEvent>(that, renderContext, client, owner);

    gFeatureManager->mDispatcher->trigger(holder);
    if (holder->isCancelled()) return;

    // Get the UIControl's dimensions
    float width = *(float*)((uintptr_t)owner + 72);
    float height = *(float*)((uintptr_t)owner + 76);

    // If dimensions are invalid, return early to prevent rendering
    if (width < 1.0f || height < 1.0f) return;

    // Call original function if we want to render
    auto original = mDetour->getOriginal<&PanoramaReplace::handlePanoramaReplace>();
    original(that, renderContext, client, owner);
}

void PanoramaReplace::init() {
    mDetour = std::make_unique<Detour>("PanoramaRenderer::render", reinterpret_cast<void*>(SigManager::Panorama_Render), &handlePanoramaReplace);
    mDetour->enable();
}
