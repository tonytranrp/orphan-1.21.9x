#pragma once
//
// Created by vastrakai on 7/22/2024.
//

#include <Hook/Hook.hpp>
#include <SDK/Minecraft/Actor/EntityContext.hpp>
#include <SDK/Minecraft/Actor/Components/CameraComponent.hpp>

class PanoramaReplace : public Hook {
public:
    PanoramaReplace() : Hook() {
        mName = "PanoramaRenderer::render";
    }

    static std::unique_ptr<Detour> mDetour;
    // symbol: void CameraDirectLookSystemUtil::_handleLookInput(EntityContext&, CameraComponent const&, CameraDirectLookComponent&, Vec2 const&)
    static void handlePanoramaReplace(struct PanoramaRenderer *that, struct MinecraftUIRenderContext *renderContext, ClientInstance *client, struct UIControl *owner);
    void init() override;
};
