#include "TargetHUD.hpp"

#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Modules/Combat/Aura.hpp>
#include <Hook/Hooks/RenderHooks/D3DHook.hpp>
#include <SDK/Minecraft/Actor/SerializedSkin.hpp>
#include <SDK/Minecraft/Actor/Components/ActorOwnerComponent.hpp>
#include <SDK/Minecraft/Network/Packets/ActorEventPacket.hpp>
#include <SDK/Minecraft/Rendering/GuiData.hpp>

TargetHUD::TargetHUD() : ModuleBase("TargetHUD", "Shows target information", ModuleCategory::Visual, 0, false)
{
    addSettings(
        &mStyle,
        &mFontSize,
        &mHealthCalculation,
        &mShowParticles,
        &mDynamicMode,
        &mParticleShape
    );

    mNames = {
        {Lowercase, "targethud"},
        {LowercaseSpaced, "target hud"},
        {Normal, "TargetHUD"},
        {NormalSpaced, "Target HUD"},
    };

    gFeatureManager->mDispatcher->listen<RenderEvent, &TargetHUD::onRenderEvent, nes::event_priority::LAST>(this);

    mElement = std::make_unique<HudElement>();
    mElement->mPos = { 500, 500 };
    const char* ModuleBaseType = ModuleBase<TargetHUD>::getTypeID();;
    mElement->mParentTypeIdentifier = const_cast<char*>(ModuleBaseType);
    HudEditor::gInstance->registerElement(mElement.get());
}

void TargetHUD::onEnable()
{
    gFeatureManager->mDispatcher->listen<BaseTickEvent, &TargetHUD::onBaseTickEvent, nes::event_priority::VERY_LAST>(this);
    gFeatureManager->mDispatcher->listen<PacketInEvent, &TargetHUD::onPacketInEvent>(this);

    mElement->mVisible = true;
}

void TargetHUD::onDisable()
{
    gFeatureManager->mDispatcher->deafen<BaseTickEvent, &TargetHUD::onBaseTickEvent>(this);
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &TargetHUD::onPacketInEvent>(this);

    for (auto& [actor, textureHolder] : mTargetTextures)
    {
        if (textureHolder.texture) textureHolder.texture->Release();
    }
    mTargetTextures.clear();

    mElement->mVisible = false;
}

void TargetHUD::onBaseTickEvent(BaseTickEvent& event)
{
    validateTextures();
    if (mHealthCalculation.mValue) calculateHealths();

    if (Aura::sHasTarget && Aura::sTarget && Aura::sTarget->getMobHurtTimeComponent())
    {
        if (!Aura::sTarget->getActorTypeComponent())
        {
            spdlog::warn("TargetHUD: Target has no ActorTypeComponent");
            Aura::sTarget = nullptr;
            return;
        }
        mHealth = Aura::sTarget->getHealth();
        if (mHealthCalculation.mValue) mHealth = mHealths[Aura::sTarget->getRawName()].health;
        mMaxHealth = Aura::sTarget->getMaxHealth();
        mAbsorption = Aura::sTarget->getAbsorption();
        mMaxAbsorption = Aura::sTarget->getMaxAbsorption();
        if (!Aura::sTarget->isPlayer())
        {
            mLastPlayerName = "Mob";
            return;
        }
        mLastHurtTime = mHurtTime;
        mHurtTime = static_cast<float>(Aura::sTarget->getMobHurtTimeComponent()->mHurtTime);
        mLastHealth = mHealth;
        mLastAbsorption = mAbsorption;
        mLastMaxHealth = mMaxHealth;
        mLastMaxAbsorption = mMaxAbsorption;
        mLastPlayerName = Aura::sTarget->getRawName();

        if (mHurtTime > mLastHurtTime)
        {
            mLastHurtTime = mHurtTime;
        }
    }
}

void TargetHUD::calculateHealths() {
    auto player = ClientInstance::get()->getLocalPlayer();
    auto actors = ActorUtils::getActorList(true, true);

    bool heal = 4000 <= NOW - mLastHealTime;
    if (heal) mLastHealTime = NOW;

    for (auto actor : actors) {
        if (actor == player) continue;
        if (!actor->getMobHurtTimeComponent() || !actor->getActorTypeComponent()) continue;
        auto info = &mHealths[actor->getRawName()];
        float absorption = actor->getAbsorption();
        int hurtTime = actor->getMobHurtTimeComponent()->mHurtTime;
        if (0 < hurtTime) {
            float damage = 0;
            if (absorption < info->lastAbsorption) {
                if (0 < absorption) {
                    info->damage = abs(info->lastAbsorption - absorption);
                    damage = 0;
                }
                else if (0 < info->lastAbsorption) {
                    damage = abs(info->damage - info->lastAbsorption);
                }
            }
            else if (hurtTime == 9)
            {
                damage = info->damage;
            }

            if (absorption == 0 && 0 < damage) {
                if (info->health - damage < 0) info->health = 0;
                else info->health -= damage;
            }
        }
        if (heal) {
            if (info->health + 1 > 20) info->health = 20;
            else info->health++;
        }
        info->lastAbsorption = absorption;
    }
}

void TargetHUD::validateTextures()
{
    auto player = ClientInstance::get()->getLocalPlayer();
    std::vector<EntityId> foundEntities;
    for (auto&& [daId, moduleOwner, typeComponent] : player->mContext.mRegistry->view<ActorOwnerComponent, ActorTypeComponent>().each())
    {
        foundEntities.push_back(moduleOwner.mActor->mContext.mEntityId);
    }

    for (auto it = mTargetTextures.begin(); it != mTargetTextures.end();)
    {
        if (std::ranges::find(foundEntities, it->second.associatedEntity) == foundEntities.end())
        {
            if (it->second.texture) it->second.texture->Release();
            it = mTargetTextures.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

ID3D11ShaderResourceView* TargetHUD::getActorSkinTex(Actor* actor)
{
    auto player = ClientInstance::get()->getLocalPlayer();

    if (!mTargetTextures.contains(actor)) mTargetTextures[actor] = TargetTextureHolder();

    auto& [texture, loaded, id] = mTargetTextures[actor];

    if (actor)
    {
        auto skin = actor->getSkin();
        if (skin)
        {
            if (!loaded) {
                bool isPlayer = true;
                if (actor->isValid() && !actor->isPlayer())
                {
                    isPlayer = false;
                    skin = player->getSkin();
                    spdlog::warn("Falling back to default LP skin for actor");
                }

                int headSize = skin->impl->mObject.skinWidth / 8;
                int headOffsetX = skin->impl->mObject.skinWidth / 8;
                int headOffsetY = skin->impl->mObject.skinHeight / 8;

                std::vector<uint8_t> headData(headSize * headSize * 4);

                for (int y = 0; y < headSize; y++) {
                    for (int x = 0; x < headSize; x++) {
                        int srcIndex = ((y + headOffsetY) * skin->impl->mObject.skinWidth + (x + headOffsetX)) * 4;
                        int dstIndex = (y * headSize + x) * 4;
                        std::copy_n(skin->impl->mObject.skinData + srcIndex, 4, headData.data() + dstIndex);
                    }
                }

                int scalingFactor = 8;
                std::vector<uint8_t> scaledHeadData(headSize * scalingFactor * headSize * scalingFactor * 4);

                for (int y = 0; y < headSize * scalingFactor; y++) {
                    for (int x = 0; x < headSize * scalingFactor; x++) {
                        int srcX = x / scalingFactor;
                        int srcY = y / scalingFactor;
                        int srcIndex = (srcY * headSize + srcX) * 4;
                        int dstIndex = (y * headSize * scalingFactor + x) * 4;
                        std::copy_n(headData.data() + srcIndex, 4, scaledHeadData.data() + dstIndex);
                    }
                }

                headSize *= scalingFactor;

                headData = std::move(scaledHeadData);
                spdlog::info("Loading skin texture for {}", isPlayer ? actor->getRawName() : "Mob");
                D3DHook::createTextureFromData(headData.data(), headSize, headSize, &texture);
                loaded = true;
                id = actor->mContext.mEntityId;
            }
        }
    }

    return texture;
}

void TargetHUD::spawnParticle(const ImVec2& sourcePos) {
    if (mParticles.size() >= MAX_PARTICLES) return;

    int shapesToSpawn = (mParticleShape.mValue == ParticleShape::Mixed) ? 3 : PARTICLES_PER_BURST;

    for (int i = 0; i < shapesToSpawn; i++) {
        if (mParticles.size() >= MAX_PARTICLES) break;

        ImColor themeColor = ColorUtils::getThemedColor(0);

        float angle = static_cast<float>(rand()) / RAND_MAX * 2 * 3.14159f;
        float spreadAngle = angle + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * PARTICLE_SPREAD;
        ImVec2 headSize2 = ImVec2(60, 60);
        float radius = headSize2.x / 2.0f;

        Particle particle;

        particle.pos.x = sourcePos.x + cos(angle) * radius;
        particle.pos.y = sourcePos.y + sin(angle) * radius;

        particle.movingInward = (static_cast<float>(rand()) / RAND_MAX < INWARD_PARTICLE_CHANCE);

        float baseSpeed = PARTICLE_SPEED + (static_cast<float>(rand()) / RAND_MAX * PARTICLE_SPEED_VARIANCE);
        if (particle.movingInward) {
            particle.velocity = ImVec2(-cos(spreadAngle) * baseSpeed * 0.5f, -sin(spreadAngle) * baseSpeed * 0.5f);
        }
        else {
            particle.velocity = ImVec2(cos(spreadAngle) * baseSpeed, sin(spreadAngle) * baseSpeed);
        }

        particle.life = PARTICLE_LIFE * (0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f);
        particle.maxLife = particle.life;
        particle.color = themeColor;
        particle.scale = 0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
        particle.rotation = static_cast<float>(rand()) / RAND_MAX * 2 * 3.14159f;
        particle.rotationSpeed = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * MAX_ROTATION_SPEED;

        if (mParticleShape.mValue == ParticleShape::Mixed) {

            particle.shape = static_cast<ParticleShape>(i % 3);
        }
        else {
            particle.shape = mParticleShape.mValue;
        }

        mParticles.push_back(particle);
    }
}

void TargetHUD::updateParticles(float deltaTime, const ImVec2& sourcePos) {
    for (auto it = mParticles.begin(); it != mParticles.end();) {
        it->life -= deltaTime;
        if (it->life <= 0) {
            it = mParticles.erase(it);
            continue;
        }

        it->pos.x += it->velocity.x * deltaTime;
        it->pos.y += it->velocity.y * deltaTime;

        it->rotation += it->rotationSpeed * deltaTime;

        ImVec2 toSource = ImVec2(sourcePos.x - it->pos.x, sourcePos.y - it->pos.y);
        float distToSource = sqrt(toSource.x * toSource.x + toSource.y * toSource.y);

        float groupingForce = 0.3f;
        if (distToSource > 0) {
            toSource.x /= distToSource;
            toSource.y /= distToSource;

            it->velocity.x += toSource.x * groupingForce;
            it->velocity.y += toSource.y * groupingForce;
        }

        float lifePercent = it->life / it->maxLife;

        if (it->movingInward) {
            float acceleration = 2000.0f * deltaTime;
            it->velocity.x += toSource.x * acceleration;
            it->velocity.y += toSource.y * acceleration;
        }
        else {

            it->velocity.x *= (0.92f + lifePercent * 0.03f);
            it->velocity.y *= (0.92f + lifePercent * 0.03f);
        }

        it->color.Value.w = lifePercent * lifePercent;

        ++it;
    }
}

void DrawRotatedTriangle(ImDrawList* drawList, const ImVec2& center, float size, float rotation, const ImColor& color) {
    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);
    ImVec2 points[3];
    points[0] = ImVec2(center.x + cos_r * size, center.y + sin_r * size);
    points[1] = ImVec2(center.x + cos_r * -size + sin_r * size, center.y + sin_r * -size - cos_r * size);
    points[2] = ImVec2(center.x + cos_r * -size - sin_r * size, center.y + sin_r * -size + cos_r * size);
    drawList->AddTriangleFilled(points[0], points[1], points[2], color);
}

void DrawRotatedSquare(ImDrawList* drawList, const ImVec2& center, float size, float rotation, const ImColor& color) {
    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);
    ImVec2 points[4];
    points[0] = ImVec2(center.x + (cos_r - sin_r) * size, center.y + (sin_r + cos_r) * size);
    points[1] = ImVec2(center.x + (-cos_r - sin_r) * size, center.y + (-sin_r + cos_r) * size);
    points[2] = ImVec2(center.x + (-cos_r + sin_r) * size, center.y + (-sin_r - cos_r) * size);
    points[3] = ImVec2(center.x + (cos_r + sin_r) * size, center.y + (sin_r - cos_r) * size);
    drawList->AddQuadFilled(points[0], points[1], points[2], points[3], color);
}

void TargetHUD::updateAnimation(float deltaTime, bool visible, const ImVec2& targetPos) {

    float targetVisibility = visible ? 1.0f : 0.0f;
    mVisibilityAnimation = MathUtils::lerp(mVisibilityAnimation, targetVisibility, deltaTime * ANIMATION_SPEED);

    if (visible) {
        mTargetPosition = targetPos;
    }
    else {

        mTargetPosition.y -= 50.0f * deltaTime;
    }

    mCurrentPosition.x = MathUtils::lerp(mCurrentPosition.x, mTargetPosition.x, deltaTime * POSITION_LERP_SPEED);
    mCurrentPosition.y = MathUtils::lerp(mCurrentPosition.y, mTargetPosition.y, deltaTime * POSITION_LERP_SPEED);
}

void TargetHUD::onRenderEvent(RenderEvent& event)
{
    auto ci = ClientInstance::get();
    if (!ci || !ci->getLocalPlayer()) return;

    float deltaTime = ImGui::GetIO().DeltaTime;
    bool hasTarget = (Aura::sTarget != nullptr && Aura::sHasTarget);

    ImVec2 targetPos;
    if (hasTarget && mDynamicMode.mValue) {
        auto target = Aura::sTarget;
        if (target) {
            auto posComp = target->getRenderPositionComponent();
            if (posComp) {
                glm::vec3 targetPosVec = posComp->mPosition;
                targetPosVec.y += 2.0f;
                targetPosVec.x += 0.5f;

                ImVec2 screen;
                bool isOnScreen = RenderUtils::worldToScreen(targetPosVec, screen);

                if (isOnScreen) {
                    targetPos = screen;

                    float distance = glm::distance(RenderUtils::transform.mOrigin, targetPosVec);
                    float scale = 1.0f / (distance * 0.1f);
                    scale = glm::clamp(scale, 0.5f, 1.0f);

                    mFontSize.mValue = 20.0f * scale;
                }
                else {

                    glm::vec3 viewDir = glm::normalize(targetPosVec - RenderUtils::transform.mOrigin);

                    float screenWidth = ci->getGuiData()->mResolution.x;
                    float screenHeight = ci->getGuiData()->mResolution.y;

                    targetPos = ImVec2(
                        screenWidth * 0.95f,
                        screenHeight * 1.5f + (glm::sign(viewDir.y) * screenHeight * 0.2f)
                    );

                    mFontSize.mValue = 20.0f;
                }
            }
        }
    }
    else {
        targetPos = ImVec2(mElement->getPos().x, mElement->getPos().y);
    }

    updateAnimation(deltaTime, hasTarget, targetPos);

    if (mVisibilityAnimation <= 0.001f && !hasTarget) return;

    auto renderPos = mCurrentPosition;

    float anim = mVisibilityAnimation;

    auto target = Aura::sTarget;
    if (!target || !Aura::sHasTarget) return;

    if (target->isDead() || target->getHealth() <= 0) {
        Aura::sTarget = nullptr;
        Aura::sHasTarget = false;
        return;
    }

    auto drawList = ImGui::GetBackgroundDrawList();
    auto resolution = ci->getGuiData()->mResolution;

    if (mDynamicMode.mValue) {
        auto posComp = target->getRenderPositionComponent();
        if (posComp) {
            glm::vec3 targetPos = posComp->mPosition;

            targetPos.y += 2.0f;
            targetPos.x += 0.5f;

            ImVec2 screen;
            bool isOnScreen = RenderUtils::worldToScreen(targetPos, screen);

            if (isOnScreen) {

                renderPos = screen;

                float distance = glm::distance(RenderUtils::transform.mOrigin, targetPos);
                float scale = 1.0f / (distance * 0.1f);
                scale = glm::clamp(scale, 0.5f, 1.0f);

                mFontSize.mValue = 20.0f * scale;
            }
            else {

                glm::vec3 viewDir = glm::normalize(targetPos - RenderUtils::transform.mOrigin);

                float screenWidth = resolution.x;
                float screenHeight = resolution.y;

                renderPos = ImVec2(
                    screenWidth * 0.95f,
                    screenHeight * 1.5f + (glm::sign(viewDir.y) * screenHeight * 0.2f)
                );

                mFontSize.mValue = 20.0f;
            }
        }
    }
    else {

        renderPos = ImVec2(mElement->getPos().x, mElement->getPos().y);
    }

    auto boxPos = ImVec2(renderPos.x, renderPos.y);

    if (mStyle.mValue == Style::EDest)
    {
        bool hasTarget = Aura::sHasTarget;

        if (mElement->mSampleMode && !hasTarget)
        {
            target = ClientInstance::get()->getLocalPlayer();
            hasTarget = true;
            mHealth = target->getHealth();
            mMaxHealth = target->getMaxHealth();
            mAbsorption = target->getAbsorption();
            mMaxAbsorption = target->getMaxAbsorption();
            mLastPlayerName = target->getRawName();
            mLastHurtTime = mHurtTime;
            mHurtTime = target->getMobHurtTimeComponent()->mHurtTime;
        }

        static float anim = 0.f;
        float delta = ImGui::GetIO().DeltaTime;

        float lerpedHurtTime = MathUtils::lerp(mLastHurtTime / 10.f, mHurtTime / 10.f, delta);

        static float hurtTimeAnimPerc = 0.f;
        static float healthAnimPerc = 0.f;
        static float absorptionAnimPerc = 0.f;

        if (mLastTarget != target)
        {
            mLastTarget = target;
            mLastHealth = mHealth;
            mLastAbsorption = mAbsorption;
            mLastMaxHealth = mMaxHealth;
            mLastMaxAbsorption = mMaxAbsorption;
            healthAnimPerc = mHealth / mMaxHealth;
            absorptionAnimPerc = mAbsorption / 20.f;
            mLerpedHealth = mHealth;
            mLerpedAbsorption = mAbsorption;
            spdlog::info("Recalcing health and absorption");
        }

        mLastTarget = target;

        hurtTimeAnimPerc = MathUtils::lerp(hurtTimeAnimPerc, lerpedHurtTime, delta * 20.f);

        float perc = mLastHealth / mLastMaxHealth;
        healthAnimPerc = MathUtils::lerp(healthAnimPerc, perc, delta * 6.f);
        float perc2 = mLastAbsorption / 20.f;
        absorptionAnimPerc = MathUtils::lerp(absorptionAnimPerc, perc2, delta * 6.f);

        mLerpedHealth = MathUtils::lerp(mLerpedHealth, mHealth, delta * 10.f);
        mLerpedAbsorption = MathUtils::lerp(mLerpedAbsorption, mAbsorption, delta * 10.f);

        bool showing = mEnabled && hasTarget && target;

        anim = MathUtils::lerp(anim, showing ? 1.f : 0.f, ImGui::GetIO().DeltaTime * 10.f);

        float scaleFactor = mFontSize.mValue / 20.0f;

        float xpad = 5 * scaleFactor;
        float ypad = 5 * scaleFactor;

        auto screenSize = ImGui::GetIO().DisplaySize;

        auto headSize = ImVec2(60 * anim * scaleFactor, 60 * anim * scaleFactor);
        auto boxSize = ImVec2(230 * anim * scaleFactor, 70 * anim * scaleFactor);

        auto boxPos = ImVec2(renderPos.x, renderPos.y);
        boxPos.x -= boxSize.x / 2;
        boxPos.y -= boxSize.y / 2;

        mElement->mSize = glm::vec2(boxSize.x, boxSize.y);
        mElement->mCentered = true;

        auto headPos = ImVec2(boxPos.x + xpad * anim, boxPos.y + ypad * anim);
        auto headSize2 = ImVec2(MathUtils::lerp(headSize.x, 40 * anim * scaleFactor, hurtTimeAnimPerc),
            MathUtils::lerp(headSize.y, 40 * anim * scaleFactor, hurtTimeAnimPerc));

        float daTopYdiff = headPos.y - boxPos.y;

        ImRenderUtils::addBlur(ImVec4(boxPos.x, boxPos.y, boxPos.x + boxSize.x, boxPos.y + boxSize.y), 15.0f, 10.0f, drawList, true);
        drawList->AddRectFilled(boxPos, ImVec2(boxPos.x + boxSize.x, boxPos.y + boxSize.y), ImColor(0.f, 0.f, 0.f, 0.5f * anim), 15.f * anim);

        ID3D11ShaderResourceView* texture = nullptr;
        static bool loaded = false;
        texture = getActorSkinTex(target);
        loaded = true;

        auto imageColor = ImColor(1.f, 1.f, 1.f, 1.f * anim);

        imageColor.Value.x = MathUtils::lerp(imageColor.Value.x, 1.f, hurtTimeAnimPerc);
        imageColor.Value.y = MathUtils::lerp(imageColor.Value.y, 1.f - hurtTimeAnimPerc, hurtTimeAnimPerc);
        imageColor.Value.z = MathUtils::lerp(imageColor.Value.z, 1.f - hurtTimeAnimPerc, hurtTimeAnimPerc);

        std::string name = mLastPlayerName;
        auto textNameSize = ImGui::GetFont()->CalcTextSizeA(mFontSize.mValue * anim, FLT_MAX, 0, name.c_str());

        auto textNamePos = ImVec2(headPos.x + headSize.x + xpad * anim, boxPos.y + ypad * anim);
        ImRenderUtils::drawShadowText(drawList, name, textNamePos, ImColor(255, 255, 255, static_cast<int>(255 * anim)), mFontSize.mValue * anim, false);

        std::string statusStr;
        ImU32 statusColor;

        float totalHealth = mHealth + mAbsorption;
        float targetTotalHealth = target->getHealth() + target->getAbsorption();

        mTargetStatus = (totalHealth > targetTotalHealth) ? "Winning" : "Losing";

        if (mCurrentStatus != mTargetStatus) {
            mStatusTransition = 0.0f;
            mCurrentStatus = mTargetStatus;
        }

        mStatusTransition = MathUtils::lerp(mStatusTransition, 1.0f, delta * 5.0f);

        statusStr = mCurrentStatus;
        statusColor = (mCurrentStatus == "Winning") ?
            IM_COL32(255, 0, 0, static_cast<int>(255 * mStatusTransition)) :
            IM_COL32(255, 255, 0, static_cast<int>(255 * mStatusTransition));

        auto textStatusSize = ImGui::GetFont()->CalcTextSizeA(mFontSize.mValue * anim, FLT_MAX, 0, statusStr.c_str());
        auto textStatusPos = ImVec2(textNamePos.x, textNamePos.y + textNameSize.y + 5 * anim * scaleFactor);

        ImRenderUtils::drawShadowText(drawList, name, textNamePos, ImColor(255, 255, 255, static_cast<int>(255 * anim)), mFontSize.mValue * anim, false);
        ImRenderUtils::drawShadowText(drawList, statusStr, textStatusPos, ImColor(200, 200, 200, static_cast<int>(255 * anim * mStatusTransition)), mFontSize.mValue * anim, false);

        float healthStartY = textStatusPos.y + textStatusSize.y + ypad * anim * scaleFactor;
        float ysize = 10 * anim * scaleFactor;
        auto healthBarStart = ImVec2(textStatusPos.x, healthStartY);
        int barSizeX = boxSize.x - headSize.x - xpad * 3;
        auto healthBarEnd = ImVec2(healthBarStart.x + barSizeX, healthStartY + ysize);

        std::string healthStr = "+" + std::to_string((int)mAbsorption);
        auto textHealthSize = ImGui::GetFont()->CalcTextSizeA(mFontSize.mValue * anim, FLT_MAX, 0, healthStr.c_str());
        auto textHealthPos = ImVec2(healthBarStart.x + xpad * anim, healthStartY);

        headPos.x += (headSize.x - headSize2.x) / 2;
        headPos.y += (headSize.y - headSize2.y) / 2;

        if (texture) {
            ImVec2 headCenter = ImVec2(
                headPos.x + headSize2.x / 2,
                headPos.y + headSize2.y / 2
            );

            if (mShowParticles.mValue) {
                float currentTime = ImGui::GetTime();

                static float lastHurtTimePerc = 0.0f;
                if (hurtTimeAnimPerc > lastHurtTimePerc && hurtTimeAnimPerc > 0.01f && currentTime - mLastParticleSpawn > PARTICLE_SPAWN_RATE) {
                    if (mParticles.size() < MAX_PARTICLES) {
                        spawnParticle(headCenter);
                    }
                    mLastParticleSpawn = currentTime;
                }
                lastHurtTimePerc = hurtTimeAnimPerc;

                updateParticles(ImGui::GetIO().DeltaTime, headCenter);

                for (const auto& particle : mParticles) {
                    float baseSize = 3.0f * particle.scale * scaleFactor;
                    float size = baseSize * (particle.life / particle.maxLife);

                    ImColor glowColor = particle.color;
                    glowColor.Value.w *= 0.5f;

                    float glowScale = particle.movingInward ? 1.5f : 2.0f;

                    switch (particle.shape) {
                    case ParticleShape::Circle:
                        drawList->AddCircleFilled(particle.pos, size * glowScale, glowColor);
                        drawList->AddCircleFilled(particle.pos, size, particle.color);
                        break;

                    case ParticleShape::Triangle:
                        DrawRotatedTriangle(drawList, particle.pos, size * glowScale, particle.rotation, glowColor);
                        DrawRotatedTriangle(drawList, particle.pos, size, particle.rotation, particle.color);
                        break;

                    case ParticleShape::Square:
                        DrawRotatedSquare(drawList, particle.pos, size * glowScale, particle.rotation, glowColor);
                        DrawRotatedSquare(drawList, particle.pos, size, particle.rotation, particle.color);
                        break;
                    }
                }
            }

            drawList->AddImageRounded(texture, headPos, headPos + headSize2, ImVec2(0, 0), ImVec2(1, 1), imageColor, 10.f * anim * scaleFactor);
        }

        auto textStartPos = textNamePos;
        auto textEndPos = textHealthPos + textHealthSize;
        textEndPos.x = boxPos.x + boxSize.x - xpad * anim;
        drawList->PushClipRect(textStartPos, textEndPos, true);
        drawList->PopClipRect();

        float daBottomYdiff = boxPos.y + boxSize.y - healthBarEnd.y;

        drawList->AddRectFilled(healthBarStart, healthBarEnd, ImColor(100, 100, 100, (int)((float)170 * anim)), 10.f);

        float healthPerc = mLerpedHealth / mMaxHealth;
        healthPerc = MathUtils::clamp(healthPerc, 0.f, 1.f);
        auto healthEnd = ImVec2(healthBarEnd.x, healthBarEnd.y);
        healthEnd.x = MathUtils::lerp(healthBarStart.x, healthBarEnd.x, healthPerc);

        float absorptionPerc = mLerpedAbsorption / 20.f;
        absorptionPerc = MathUtils::clamp(absorptionPerc, 0.f, 1.f);
        auto absorpEnd = ImVec2(healthBarEnd.x, healthBarEnd.y);
        absorpEnd.x = MathUtils::lerp(healthBarStart.x, healthBarEnd.x, absorptionPerc);

        float endXDiff = healthBarEnd.x - healthBarStart.x;
        ImColor startColor = ColorUtils::getThemedColor(0);
        ImColor endColor = ColorUtils::getThemedColor(endXDiff * 2);
        startColor.Value.w *= anim;
        endColor.Value.w *= anim;

        if (healthPerc > 0.01f)
        {
            drawList->PushClipRect(healthBarStart, healthEnd, true);
            drawList->AddRectFilledMultiColor(healthBarStart, healthBarEnd, startColor, endColor, endColor, startColor, 10.f, ImDrawCornerFlags_All);
            drawList->PopClipRect();
        }

        if (absorptionPerc > 0.01f)
        {
            drawList->PushClipRect(healthBarStart, absorpEnd, true);
            drawList->AddRectFilled(healthBarStart, healthBarEnd, ImColor(244, 204, 0, (int)(255 * anim)), 10.f);
            drawList->PopClipRect();
        }
    }
    else
    {
        Actor* target = Aura::sTarget;
        bool hasTarget = Aura::sHasTarget;

        if (mElement->mSampleMode && !hasTarget) {
            target = ClientInstance::get()->getLocalPlayer();
            hasTarget = true;
            mHealth = target->getHealth();
            mMaxHealth = target->getMaxHealth();
            mAbsorption = target->getAbsorption();
            mMaxAbsorption = target->getMaxAbsorption();
            mLastPlayerName = target->getRawName();
            mLastHurtTime = mHurtTime;
            mHurtTime = target->getMobHurtTimeComponent()->mHurtTime;
        }

        static float anim = 0.f;
        float delta = ImGui::GetIO().DeltaTime;

        float lerpedHurtTime = MathUtils::lerp(mLastHurtTime / 10.f, mHurtTime / 10.f, delta);

        static float hurtTimeAnimPerc = 0.f;
        static float healthAnimPerc = 0.f;
        static float absorptionAnimPerc = 0.f;

        if (mLastTarget != target)
        {
            mLastTarget = target;
            mLastHealth = mHealth;
            mLastAbsorption = mAbsorption;
            mLastMaxHealth = mMaxHealth;
            mLastMaxAbsorption = mMaxAbsorption;
            healthAnimPerc = mHealth / mMaxHealth;
            absorptionAnimPerc = mAbsorption / 20.f;
            mLerpedHealth = mHealth;
            mLerpedAbsorption = mAbsorption;
            spdlog::info("Recalcing health and absorption");
        }

        mLastTarget = target;

        hurtTimeAnimPerc = MathUtils::lerp(hurtTimeAnimPerc, lerpedHurtTime, delta * 20.f);

        float perc = mLastHealth / mLastMaxHealth;
        healthAnimPerc = MathUtils::lerp(healthAnimPerc, perc, delta * 6.f);
        float perc2 = mLastAbsorption / 20.f;
        absorptionAnimPerc = MathUtils::lerp(absorptionAnimPerc, perc2, delta * 6.f);

        mLerpedHealth = MathUtils::lerp(mLerpedHealth, mHealth, delta * 10.f);
        mLerpedAbsorption = MathUtils::lerp(mLerpedAbsorption, mAbsorption, delta * 10.f);

        bool showing = mEnabled && hasTarget && target;

        anim = MathUtils::lerp(anim, showing ? 1.f : 0.f, ImGui::GetIO().DeltaTime * 10.f);

        float scaleFactor = mFontSize.mValue / 20.0f;

        float xpad = 5 * scaleFactor;
        float ypad = 5 * scaleFactor;

        auto screenSize = ImGui::GetIO().DisplaySize;

        auto headSize = ImVec2(60 * anim * scaleFactor, 60 * anim * scaleFactor);
        auto boxSize = ImVec2(230 * anim * scaleFactor, 70 * anim * scaleFactor);

        auto boxPos = ImVec2(renderPos.x, renderPos.y);
        boxPos.x -= boxSize.x / 2;
        boxPos.y -= boxSize.y / 2;

        mElement->mSize = glm::vec2(boxSize.x, boxSize.y);
        mElement->mCentered = true;

        auto headPos = ImVec2(boxPos.x + xpad * anim, boxPos.y + ypad * anim);

        float headQuartY = headSize.y / 4;
        auto headSize2 = ImVec2(MathUtils::lerp(headSize.x, 40 * anim * scaleFactor, hurtTimeAnimPerc), MathUtils::lerp(headSize.y, 40 * anim * scaleFactor, hurtTimeAnimPerc));

        float daTopYdiff = headPos.y - boxPos.y;

        drawList->AddRectFilled(boxPos, ImVec2(boxPos.x + boxSize.x, boxPos.y + boxSize.y), ImColor(0.f, 0.f, 0.f, 0.5f * anim), 15.f * anim);

        ID3D11ShaderResourceView* texture = nullptr;
        static bool loaded = false;
        texture = getActorSkinTex(target);
        loaded = true;

        auto imageColor = ImColor(1.f, 1.f, 1.f, 1.f * anim);

        imageColor.Value.x = MathUtils::lerp(imageColor.Value.x, 1.f, hurtTimeAnimPerc);
        imageColor.Value.y = MathUtils::lerp(imageColor.Value.y, 1.f - hurtTimeAnimPerc, hurtTimeAnimPerc);
        imageColor.Value.z = MathUtils::lerp(imageColor.Value.z, 1.f - hurtTimeAnimPerc, hurtTimeAnimPerc);

        float healthStartY = boxPos.y + boxSize.y - (ypad + 2) * anim - 25 * anim;
        float ysize = 20 * anim * scaleFactor;
        auto healthBarStart = ImVec2(boxPos.x + headSize.x + (xpad * 2) * anim, healthStartY);
        int barSizeX = boxSize.x - xpad;
        auto healthBarEnd = ImVec2(boxPos.x + barSizeX, healthStartY + ysize);

        std::string name = mLastPlayerName;
        auto textNameSize = ImGui::GetFont()->CalcTextSizeA(mFontSize.mValue * anim, FLT_MAX, 0, name.c_str());

        float ydiff = healthBarStart.y - boxPos.y;
        auto textNamePos = ImVec2(headPos.x + headSize.x + xpad * anim, boxPos.y + ydiff / 2 - textNameSize.y / 2 + (ypad * anim));

        std::string healthStr = "+" + std::to_string((int)mAbsorption);
        auto textHealthSize = ImGui::GetFont()->CalcTextSizeA(mFontSize.mValue * anim, FLT_MAX, 0, healthStr.c_str());
        auto textHealthPos = ImVec2(healthBarStart.x + xpad * anim, healthStartY);

        headPos.x += (headSize.x - headSize2.x) / 2;
        headPos.y += (headSize.y - headSize2.y) / 2;

        if (texture) {
            ImVec2 headCenter = ImVec2(
                headPos.x + headSize2.x / 2,
                headPos.y + headSize2.y / 2
            );

            if (mShowParticles.mValue) {
                float currentTime = ImGui::GetTime();

                static float lastHurtTimePerc = 0.0f;
                if (hurtTimeAnimPerc > lastHurtTimePerc && hurtTimeAnimPerc > 0.01f && currentTime - mLastParticleSpawn > PARTICLE_SPAWN_RATE) {
                    if (mParticles.size() < MAX_PARTICLES) {
                        spawnParticle(headCenter);
                    }
                    mLastParticleSpawn = currentTime;
                }
                lastHurtTimePerc = hurtTimeAnimPerc;

                updateParticles(ImGui::GetIO().DeltaTime, headCenter);

                for (const auto& particle : mParticles) {
                    float baseSize = 3.0f * particle.scale * scaleFactor;
                    float size = baseSize * (particle.life / particle.maxLife);

                    ImColor glowColor = particle.color;
                    glowColor.Value.w *= 0.5f;

                    float glowScale = particle.movingInward ? 1.5f : 2.0f;

                    switch (particle.shape) {
                    case ParticleShape::Circle:
                        drawList->AddCircleFilled(particle.pos, size * glowScale, glowColor);
                        drawList->AddCircleFilled(particle.pos, size, particle.color);
                        break;

                    case ParticleShape::Triangle:
                        DrawRotatedTriangle(drawList, particle.pos, size * glowScale, particle.rotation, glowColor);
                        DrawRotatedTriangle(drawList, particle.pos, size, particle.rotation, particle.color);
                        break;

                    case ParticleShape::Square:
                        DrawRotatedSquare(drawList, particle.pos, size * glowScale, particle.rotation, glowColor);
                        DrawRotatedSquare(drawList, particle.pos, size, particle.rotation, particle.color);
                        break;
                    }
                }
            }

            drawList->AddImageRounded(texture, headPos, headPos + headSize2, ImVec2(0, 0), ImVec2(1, 1), imageColor, 10.f * anim * scaleFactor);
        }

        auto textStartPos = textNamePos;
        auto textEndPos = textHealthPos + textHealthSize;
        textEndPos.x = boxPos.x + boxSize.x - xpad * anim;
        drawList->PushClipRect(textStartPos, textEndPos, true);
        ImRenderUtils::drawShadowText(drawList, name, textNamePos, ImColor(255, 255, 255, static_cast<int>(255 * anim)), mFontSize.mValue * anim, false);
        drawList->PopClipRect();

        float daBottomYdiff = boxPos.y + boxSize.y - healthBarEnd.y;

        drawList->AddRectFilled(healthBarStart, healthBarEnd, ImColor(100, 100, 100, (int)((float)170 * anim)), 10.f);

        float healthPerc = mLerpedHealth / mMaxHealth;
        healthPerc = MathUtils::clamp(healthPerc, 0.f, 1.f);
        auto healthEnd = ImVec2(healthBarEnd.x, healthBarEnd.y);
        healthEnd.x = MathUtils::lerp(healthBarStart.x, healthBarEnd.x, healthPerc);

        float absorptionPerc = mLerpedAbsorption / 20.f;
        absorptionPerc = MathUtils::clamp(absorptionPerc, 0.f, 1.f);
        auto absorpEnd = ImVec2(healthBarEnd.x, healthBarEnd.y);
        absorpEnd.x = MathUtils::lerp(healthBarStart.x, healthBarEnd.x, absorptionPerc);

        float endXDiff = healthBarEnd.x - healthBarStart.x;
        ImColor startColor = ColorUtils::getThemedColor(0);
        ImColor endColor = ColorUtils::getThemedColor(endXDiff * 2);
        startColor.Value.w *= anim;
        endColor.Value.w *= anim;

        if (healthPerc > 0.01f)
        {
            drawList->PushClipRect(healthBarStart, healthEnd, true);
            drawList->AddRectFilledMultiColor(healthBarStart, healthBarEnd, startColor, endColor, endColor, startColor, 10.f, ImDrawCornerFlags_All);
            drawList->PopClipRect();
        }

        if (absorptionPerc > 0.01f)
        {
            drawList->PushClipRect(healthBarStart, absorpEnd, true);
            drawList->AddRectFilled(healthBarStart, healthBarEnd, ImColor(244, 204, 0, (int)(255 * anim)), 10.f);
            drawList->PopClipRect();
        }

        if (mAbsorption != 0)
        {
            drawList->PushClipRect(textStartPos, textEndPos, true);

            drawList->PopClipRect();
        }
    }
}

void TargetHUD::onPacketInEvent(PacketInEvent& event)
{
    if (event.mPacket->getId() == PacketID::ActorEvent)
    {
        auto packet = event.getPacket<ActorEventPacket>();

        if (packet->mEvent != ActorEvent::HURT) return;

        Actor* target = ActorUtils::getActorFromRuntimeID(packet->mRuntimeID);
        if (!target) return;
    }
    else if (event.mPacket->getId() == PacketID::ChangeDimension) {

        for (auto& [actor, textureHolder] : mTargetTextures)
        {
            if (textureHolder.texture) textureHolder.texture->Release();
        }
        mTargetTextures.clear();

        mLastHealTime = NOW;
        mHealths.clear();
    }
}