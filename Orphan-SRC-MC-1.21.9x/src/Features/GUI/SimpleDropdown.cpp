#include "SimpleDropdown.hpp"
#include <Features/Modules/ModuleCategory.hpp>
#include <Features/Modules/Visual/ClickGui.hpp>
#include <Utils/FontHelper.hpp>
#include <Utils/MiscUtils/ImRenderUtils.hpp>
#include <Utils/MiscUtils/MathUtils.hpp>
#include <Features/Modules/Setting.hpp>
#include <Features/Modules/Visual/Interface.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <Utils/StringUtils.hpp>
#include <Utils/MiscUtils/ColorUtils.hpp>

bool SimpleGui::isKeybindBinding = false;
KeybindSetting* SimpleGui::lastKeybindSetting = nullptr;
bool SimpleGui::isKeybindBindingActive = false;
bool SimpleGui::justUnboundKeybind = false;

ImVec4 SimpleGui::scaleToPoint(const ImVec4& _this, const ImVec4& point, float amount) {
    return { point.x + (_this.x - point.x) * amount, point.y + (_this.y - point.y) * amount,
             point.z + (_this.z - point.z) * amount, point.w + (_this.w - point.w) * amount };
}

bool SimpleGui::isMouseOver(const ImVec4& rect) {
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    return mousePos.x >= rect.x && mousePos.y >= rect.y && mousePos.x < rect.z && mousePos.y < rect.w;
}

ImVec4 SimpleGui::getCenter(ImVec4& vec) {
    float centerX = (vec.x + vec.z) / 2.0f;
    float centerY = (vec.y + vec.w) / 2.0f;
    return { centerX, centerY, centerX, centerY };
}

void SimpleGui::render(float animation, float inScale, int& scrollDirection, char* h, float blur, float midclickRounding, bool isPressingShift) {
    static auto* clickGui = gFeatureManager->mModuleManager->getModule<ClickGui>();
    static auto interfaceMod = gFeatureManager->mModuleManager->getModule<Interface>();
    bool lowercase = interfaceMod->mNamingStyle.mValue == NamingStyle::Lowercase || interfaceMod->mNamingStyle.mValue == NamingStyle::LowercaseSpaced;
    std::string tooltip = "";
    bool moduleToggled = false;

    FontHelper::pushPrefFont(true, interfaceMod->mBold.mValue, interfaceMod->mSansFont.mValue);

    ImVec2 screen = ImRenderUtils::getScreenSize();
    float deltaTime = ImGui::GetIO().DeltaTime;
    auto drawList = ImGui::GetBackgroundDrawList();

    static float blurAnimation = 0.0f;
    static bool wasEnabled = false;
    static float currentBlur = 0.0f;
    static float maxBlur = blur * 2.0f;

    if (clickGui->mEnabled) {
        if (!wasEnabled) {
            wasEnabled = true;
            blurAnimation = 0.0f;
            currentBlur = 0.0f;
        }
        blurAnimation = MathUtils::animate(1.0f, blurAnimation, deltaTime * 8.0f);

        if (currentBlur < maxBlur) {
            currentBlur = MathUtils::animate(maxBlur, currentBlur, deltaTime * 15.0f);
        }
    }
    else {
        wasEnabled = false;
        blurAnimation = MathUtils::animate(0.0f, blurAnimation, deltaTime * 8.0f);
        currentBlur = MathUtils::animate(0.0f, currentBlur, deltaTime * 8.0f);
    }

    if (blurAnimation > 0.001f) {
        float finalBlur = std::min(currentBlur, maxBlur) * blurAnimation;

    }

    static float colorPickerAlpha = 0.0f;
    static bool wasColorPickerDisplayed = false;

    if (displayColorPicker && clickGui->mEnabled) {
        colorPickerAlpha = MathUtils::animate(1.0f, colorPickerAlpha, ImRenderUtils::getDeltaTime() * 12.0f);
        wasColorPickerDisplayed = true;
    }
    else if (wasColorPickerDisplayed) {
        colorPickerAlpha = MathUtils::animate(0.0f, colorPickerAlpha, ImRenderUtils::getDeltaTime() * 16.0f);
        if (colorPickerAlpha <= 0.001f) {
            wasColorPickerDisplayed = false;
        }
    }

    if (resetPosition || catPositions.empty()) {
        catPositions.clear();
        float centerX = screen.x / 2.f;
        float totalWidth = (ModuleCategoryNames.size() * catWidth) + ((ModuleCategoryNames.size() - 1) * catGap);
        float startX = centerX - (totalWidth / 2.f);

        for (size_t i = 0; i < ModuleCategoryNames.size(); i++) {
            CategoryPosition pos;
            pos.x = startX + (i * (catWidth + catGap));
            pos.y = catGap;
            catPositions.push_back(pos);
        }
        resetPosition = false;
        lastReset = NOW;
    }

    if (SimpleGui::justUnboundKeybind) {
        SimpleGui::isKeybindBindingActive = false;
        SimpleGui::justUnboundKeybind = false;
    }
    if (SimpleGui::isKeybindBinding) {
        ImGui::GetIO().WantCaptureKeyboard = true;
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {

            return;
        }
    }

    float textSize = ImGui::GetFont()->FontSize;

    for (size_t i = 0; i < ModuleCategoryNames.size(); i++) {

        if (catPositions[i].isExtended) {
            auto modsInCategory = gFeatureManager->mModuleManager->getModulesInCategory(i);

            float totalContentHeight = 0;
            for (const auto& mod : modsInCategory) {
                totalContentHeight += catHeight + 2.0f;
                if (mod->showSettings) {
                    for (const auto& setting : mod->mSettings) {
                        if (setting->mIsVisible()) totalContentHeight += catHeight;
                    }
                }
            }

            bool scrollingNeeded = totalContentHeight > screen.y - catHeight * 2;

            if (scrollingNeeded && ImRenderUtils::isMouseOver(ImVec4(
                    catPositions[i].x,
                    catPositions[i].y,
                    catPositions[i].x + catWidth,
                    screen.y))) {
                catPositions[i].targetScroll += ImGui::GetIO().MouseWheel * 60.0f;
            }
            else if (!scrollingNeeded) {
                catPositions[i].targetScroll = 0.0f;
            }
        }

        catPositions[i].currentScroll = MathUtils::lerp(
                catPositions[i].currentScroll,
                catPositions[i].targetScroll,
                ImRenderUtils::getDeltaTime() * 12.0f
        );

        float maxScroll = 0;
        auto modsInCategory = gFeatureManager->mModuleManager->getModulesInCategory(i);
        for (const auto& mod : modsInCategory) {
            maxScroll += catHeight + 2.0f;
            if (mod->showSettings) {
                for (const auto& setting : mod->mSettings) {
                    if (setting->mIsVisible()) maxScroll += catHeight;
                }
            }
        }
        maxScroll = std::max(0.0f, maxScroll - screen.y + catHeight * 2);
        catPositions[i].targetScroll = std::clamp(catPositions[i].targetScroll, -maxScroll, 0.0f);

        if (catPositions[i].isExtended != catPositions[i].wasExtended) {
            catPositions[i].wasExtended = catPositions[i].isExtended;
        }

        const float modWidth = catWidth;
        const float modHeight = catHeight;
        float moduleY = catPositions[i].currentScroll;

        ImVec4 catRect = ImVec4(
                catPositions[i].x, catPositions[i].y,
                catPositions[i].x + catWidth, catPositions[i].y + catHeight
        ).scaleToPoint(ImVec4(screen.x / 2, screen.y / 2, screen.x / 2, screen.y / 2), inScale);
        ImVec4 shadowRectTallCat = ImVec4(
                catRect.x + 12.f,
                catRect.y,
                catRect.z - 12.f,
                catRect.y + 5.f
        );
        ImVec4 shadowRectWideCat = ImVec4(
                catRect.x + 6.f,
                catRect.y + 12.f,
                catRect.z - 6.f,
                catRect.w
        );
        if (clickGui->mShadow.mValue)
        {
            ImRenderUtils::fillShadowRectangle(
                    shadowRectTallCat,
                    ImColor(0, 0, 0, 255),
                    1000.f,
                    clickGui->mShadowThickness.mValue,
                    ImDrawFlags_None
            );
            ImRenderUtils::fillShadowRectangle(
                    shadowRectWideCat,
                    ImColor(0, 0, 0, 255),
                    1000.f,
                    clickGui->mShadowThickness.mValue,
                    ImDrawFlags_None
            );
            ImRenderUtils::fillShadowRectangle(
                    shadowRectTallCat,
                    ImColor(0, 0, 0, 255),
                    1000.f,
                    clickGui->mShadowThickness.mValue,
                    ImDrawFlags_None
            );
            ImRenderUtils::fillShadowRectangle(
                    shadowRectWideCat,
                    ImColor(0, 0, 0, 255),
                    1000.f,
                    clickGui->mShadowThickness.mValue,
                    ImDrawFlags_None
            );
        }
        catRect.w += 1.0f;
        ImRenderUtils::fillRectangle(catRect, ImColor(28, 28, 28), animation, 15.0f, ImGui::GetBackgroundDrawList(), ImDrawFlags_RoundCornersTop);

        std::string catName = lowercase ? StringUtils::toLower(ModuleCategoryNames[i]) : ModuleCategoryNames[i];
        float textHeight = ImGui::GetFont()->CalcTextSizeA(inScale * 18, FLT_MAX, -1, catName.c_str()).y;
        float textWidth = ImRenderUtils::getTextWidth(&catName, inScale * 1.15);

        std::string moduleCount = "[" + std::to_string(modsInCategory.size()) + "]";
        float countWidth = ImRenderUtils::getTextWidth(&moduleCount, inScale * 1.15);

        float centerX = catRect.x + 10.0f;

        ImRenderUtils::drawText(
                ImVec2(centerX, catRect.y + (catRect.getHeight() - textHeight) / 2),
                catName,
                textColor,
                inScale * 1.15,
                animation,
                true
        );

        ImRenderUtils::drawText(
                ImVec2(catRect.z - countWidth - 10.0f, catRect.y + (catRect.getHeight() - textHeight) / 2),
                moduleCount,
                textColor,
                inScale * 1.15,
                animation,
                true
        );

        if (ImRenderUtils::isMouseOver(catRect)) {
            if (ImGui::IsMouseClicked(1) && clickGui->mEnabled) {
                catPositions[i].isExtended = !catPositions[i].isExtended;
                ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
            }
            else if (ImGui::IsMouseClicked(0) && clickGui->mEnabled) {
                catPositions[i].isDragging = true;
                catPositions[i].dragStartPos = ImVec2(catPositions[i].x, catPositions[i].y);
                catPositions[i].dragStartMouse = ImGui::GetIO().MousePos;
                catPositions[i].lastMousePos = ImGui::GetIO().MousePos;
                catPositions[i].dragVelocity = glm::vec2(0, 0);
            }
        }

        if (ImGui::IsMouseDown(0) && catPositions[i].isDragging && clickGui->mEnabled) {
            ImVec2 currentMouse = ImGui::GetIO().MousePos;
            ImVec2 mouseDelta = ImVec2(
                    currentMouse.x - catPositions[i].lastMousePos.x,
                    currentMouse.y - catPositions[i].lastMousePos.y
            );

            catPositions[i].dragVelocity.x = MathUtils::lerp(catPositions[i].dragVelocity.x, mouseDelta.x, ImRenderUtils::getDeltaTime() * 15.0f);
            catPositions[i].dragVelocity.y = MathUtils::lerp(catPositions[i].dragVelocity.y, mouseDelta.y, ImRenderUtils::getDeltaTime() * 15.0f);

            catPositions[i].x += catPositions[i].dragVelocity.x;
            catPositions[i].y += catPositions[i].dragVelocity.y;

            catPositions[i].x = std::clamp(catPositions[i].x, 0.f, screen.x - catWidth);
            catPositions[i].y = std::clamp(catPositions[i].y, 0.f, screen.y - catHeight);

            catPositions[i].lastMousePos = currentMouse;
        }
        else if (!ImGui::IsMouseDown(0)) {
            catPositions[i].isDragging = false;
            catPositions[i].dragVelocity = glm::vec2(0, 0);
        }

        if (catPositions[i].isExtended) {
            float totalSettingsHeight = 0;

            float containerHeight = 0;
            for (const auto& mod : modsInCategory) {
                containerHeight += modHeight;
                if (mod->showSettings) {
                    for (const auto& setting : mod->mSettings) {
                        if (!setting->mIsVisible()) continue;
                        containerHeight += modHeight;
                    }
                }
                containerHeight += 2.0f;
            }

            ImVec4 containerRect = ImVec4(
                    catRect.x,
                    catRect.w - 2.0f,
                    catRect.z,
                    catRect.w + containerHeight - 1.0f
            ).scaleToPoint(ImVec4(screen.x / 2, screen.y / 2, screen.x / 2, screen.y / 2), inScale);

            float dropdownOffset = 5.0f;
            ImVec4 shadowRect = ImVec4(
                    containerRect.x - dropdownOffset,
                    containerRect.y - dropdownOffset,
                    containerRect.z + dropdownOffset,
                    containerRect.w + dropdownOffset
            );
            ImVec4 shadowRectTall = ImVec4(
                    containerRect.x + 12.f,
                    containerRect.y + 6.f,
                    containerRect.z - 12.f,
                    containerRect.w
            );
            ImVec4 shadowRectWide = ImVec4(
                    containerRect.x,
                    containerRect.y + 12.f,
                    containerRect.z,
                    containerRect.w - 12.f
            );
            if (clickGui->mShadow.mValue)
            {
                ImRenderUtils::fillShadowRectangle(
                        shadowRectTall,
                        ImColor(0, 0, 0, 255),
                        1000.f,
                        clickGui->mShadowThickness.mValue,
                        ImDrawFlags_None
                );
                ImRenderUtils::fillShadowRectangle(
                        shadowRectTall,
                        ImColor(0, 0, 0, 255),
                        1000.f,
                        clickGui->mShadowThickness.mValue,
                        ImDrawFlags_None
                );
                ImRenderUtils::fillShadowRectangle(
                        shadowRectWide,
                        ImColor(0, 0, 0, 255),
                        1000.f,
                        clickGui->mShadowThickness.mValue,
                        ImDrawFlags_None
                );
                ImRenderUtils::fillShadowRectangle(
                        shadowRectWide,
                        ImColor(0, 0, 0, 255),
                        1000.f,
                        clickGui->mShadowThickness.mValue,
                        ImDrawFlags_None
                );
            }
            bool isHorizontal = clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal;
            float bottomRounding = 25.0f;
            ImDrawFlags roundingFlags = ImDrawFlags_RoundCornersBottom;

            ImRenderUtils::fillRectangle(containerRect, categoryBgColor, animation, bottomRounding, drawList, roundingFlags);

            if (clickGui->mSort.mValue == ClickGui::ClickGuiSort::Length) {
                std::sort(modsInCategory.begin(), modsInCategory.end(),
                          [](const std::shared_ptr<Module>& a, const std::shared_ptr<Module>& b) {
                              return a->getName().length() > b->getName().length();
                          });
            }

            ImVec4 clipRect = ImVec4(
                    catRect.x,
                    catRect.w,
                    catRect.z,
                    screen.y
            );

            drawList->PushClipRect(
                    ImVec2(clipRect.x, clipRect.y),
                    ImVec2(clipRect.z, clipRect.w),
                    true
            );

            for (size_t moduleIndex = 0; moduleIndex < modsInCategory.size(); moduleIndex++) {
                const auto& mod = modsInCategory[moduleIndex];
                bool isLastModule = moduleIndex == modsInCategory.size() - 1;

                float targetAnim = mod->showSettings ? 1.0f : 0.0f;
                mod->cAnim = MathUtils::animate(targetAnim, mod->cAnim, ImRenderUtils::getDeltaTime() * 15.0f);

                ImVec4 modRect = ImVec4(
                        catRect.x + 5,
                        catRect.w + moduleY,
                        catRect.z - 5,
                        catRect.w + moduleY + modHeight
                ).scaleToPoint(ImVec4(screen.x / 2, screen.y / 2, screen.x / 2, screen.y / 2), inScale);

                ImColor bgColor = mod->mEnabled ? enabledModuleBg : disabledModuleBg;
                ImDrawFlags roundingFlags = isLastModule ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_None;

                static std::map<std::shared_ptr<Module>, float> enableAnimations;
                if (enableAnimations.find(mod) == enableAnimations.end()) {
                    enableAnimations[mod] = mod->mEnabled ? 1.0f : 0.0f;
                }
                float& enableAnim = enableAnimations[mod];
                if (mod->mEnabled) {
                    enableAnim = std::min(1.0f, enableAnim + ImGui::GetIO().DeltaTime * 6.0f);
                }
                else {
                    enableAnim = std::max(0.0f, enableAnim - ImGui::GetIO().DeltaTime * 6.0f);
                }

                if (clickGui->mModuleColorStyle.mValue == ClickGui::ModuleColorStyle::Full && mod->mEnabled) {

                    ImColor rgb1, rgb2;
                    if (clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal) {
                        rgb1 = ColorUtils::getThemedColor(modRect.x);
                        rgb2 = ColorUtils::getThemedColor(modRect.z);
                    }
                    else {
                        rgb1 = ColorUtils::getThemedColor(modRect.y);
                        rgb2 = ColorUtils::getThemedColor(modRect.w);
                    }

                    ImRenderUtils::fillRoundedGradientRectangle(
                            modRect,
                            rgb1,
                            rgb2,
                            isLastModule ? 25.0f : 0.0f,
                            animation,
                            animation,
                            isLastModule ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_None
                    );

                    drawList->AddRect(
                            ImVec2(modRect.x, modRect.y),
                            ImVec2(modRect.z, modRect.w),
                            ImColor(40, 40, 40, int(255 * animation)),
                            isLastModule ? 25.0f : 0.0f,
                            isLastModule ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_None,
                            1.0f
                    );
                }
                else if (clickGui->mModuleColorStyle.mValue == ClickGui::ModuleColorStyle::Accent) {

                    ImRenderUtils::fillRectangle(
                            modRect,
                            ImColor(28, 28, 28),
                            animation,
                            isLastModule ? 25.0f : 0.0f,
                            drawList,
                            isLastModule ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_None
                    );

                    if (clickGui->mModuleColorStyle.mValue != ClickGui::ModuleColorStyle::Accent) {
                        if (moduleIndex > 0) {
                            drawList->AddRect(
                                    ImVec2(modRect.x, modRect.y - 1),
                                    ImVec2(modRect.z, modRect.w),
                                    ImColor(40, 40, 40, int(255 * animation)),
                                    isLastModule ? 25.0f : 0.0f,
                                    isLastModule ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_None,
                                    1.0f
                            );
                        }
                        else {
                            drawList->AddRect(
                                    ImVec2(modRect.x, modRect.y),
                                    ImVec2(modRect.z, modRect.w),
                                    ImColor(40, 40, 40, int(255 * animation)),
                                    isLastModule ? 25.0f : 0.0f,
                                    isLastModule ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_None,
                                    1.0f
                            );
                        }
                    }

                    if (enableAnim > 0.01f) {

                        ImRenderUtils::fillRectangle(
                                modRect,
                                ImColor(40, 40, 40, int(255 * enableAnim)),
                                animation,
                                isLastModule ? 25.0f : 0.0f,
                                drawList,
                                isLastModule ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_None
                        );
                    }

                    if (mod->mEnabled) {
                        ImVec4 accentBar(
                                modRect.x,
                                modRect.y,
                                modRect.x + 2.0f,
                                modRect.w
                        );

                        ImRenderUtils::fillRectangle(
                                accentBar,
                                ImColor(255, 255, 255),
                                animation,
                                0.0f,
                                drawList,
                                ImDrawFlags_None
                        );
                    }
                }

                float symbolSize = 8.0f * inScale;
                ImVec2 symbolPos(modRect.z - symbolSize - 8.0f, modRect.y + (modRect.w - modRect.y - symbolSize) * 0.5f);

                ImColor symbolColor = mod->mEnabled ?
                                      ImColor(255, 255, 255, int(255 * animation)) :
                                      ImColor(180, 180, 180, int(255 * animation));

                static std::map<std::shared_ptr<Module>, float> rotationAnimations;
                if (rotationAnimations.find(mod) == rotationAnimations.end()) {
                    rotationAnimations[mod] = mod->showSettings ? 360.0f : 0.0f;
                }

                float& rotation = rotationAnimations[mod];
                float targetRotation = mod->showSettings ? 360.0f : 0.0f;
                rotation = MathUtils::animate(targetRotation, rotation, ImRenderUtils::getDeltaTime() * 12.0f);
                float rotationAngle = rotation;
                float radians = (rotationAngle * 3.14159f) / 180.0f;

                ImVec2 symbolCenter = symbolPos;
                float symbolThickness = 2.0f;

                ImVec2 hStart = ImVec2(symbolCenter.x - symbolSize * 0.5f, symbolCenter.y);
                ImVec2 hEnd = ImVec2(symbolCenter.x + symbolSize * 0.5f, symbolCenter.y);

                float verticalScale = mod->showSettings ? 0.0f : 1.0f;
                float vOffset = symbolSize * 0.5f * verticalScale;
                ImVec2 vStart = ImVec2(symbolCenter.x, symbolCenter.y - vOffset);
                ImVec2 vEnd = ImVec2(symbolCenter.x, symbolCenter.y + vOffset);

                auto rotatePoint = [](const ImVec2& point, const ImVec2& center, float angle) -> ImVec2 {
                    float s = sin(angle);
                    float c = cos(angle);
                    ImVec2 translated = ImVec2(point.x - center.x, point.y - center.y);
                    return ImVec2(
                            center.x + (translated.x * c - translated.y * s),
                            center.y + (translated.x * s + translated.y * c)
                    );
                };

                ImVec2 rotatedHStart = rotatePoint(hStart, symbolPos, radians);
                ImVec2 rotatedHEnd = rotatePoint(hEnd, symbolPos, radians);
                ImVec2 rotatedVStart = rotatePoint(vStart, symbolPos, radians);
                ImVec2 rotatedVEnd = rotatePoint(vEnd, symbolPos, radians);

                if (clickGui->mDropType.mValue == ClickGui::DropdownVisual::Dot) {
                    if (mod->showSettings)
                        if (mod->mEnabled) {
                            drawList->AddCircle(symbolCenter, 5.f, ColorUtils::getThemedColor(symbolCenter.y), 0, 2.f);
                            drawList->AddCircle(symbolCenter, 4.f, symbolColor, 0, 2.f);
                        }
                        else {
                            drawList->AddCircle(symbolCenter, 3.f, symbolColor, 0, 2.f);
                        }
                    else
                    if (mod->mEnabled) {
                        drawList->AddCircleFilled(symbolCenter, 5.f, ColorUtils::getThemedColor(symbolCenter.y), 0);
                        drawList->AddCircleFilled(symbolCenter, 4.f, symbolColor, 0);
                    }
                    else {
                        drawList->AddCircleFilled(symbolCenter, 2.5f, symbolColor, 0);
                    }
                }
                else if (clickGui->mDropType.mValue == ClickGui::DropdownVisual::Plus) {
                    drawList->AddLine(rotatedHStart, rotatedHEnd, symbolColor, symbolThickness);
                    if (verticalScale > 0.01f) {
                        drawList->AddLine(rotatedVStart, rotatedVEnd, symbolColor, symbolThickness);
                    }
                }

                std::string modName = mod->getName();
                float modTextWidth = ImRenderUtils::getTextWidth(&modName, inScale);
                float modTextX = modRect.x + (modRect.z - modRect.x - modTextWidth) / 2;
                float modTextY = modRect.y + ((modRect.w - modRect.y) - textHeight) / 2;

                ImColor textCol = mod->mEnabled ?
                                  (clickGui->mModuleColorStyle.mValue == ClickGui::ModuleColorStyle::Full ? textColor : textColor) :
                                  ImColor(180, 180, 180);

                if (ImRenderUtils::isMouseOver(modRect) && clickGui->mEnabled && !displayColorPicker) {
                    if (ImGui::IsMouseClicked(0)) {
                        mod->toggle();
                        ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
                    }
                    else if (ImGui::IsMouseClicked(1)) {
                        mod->showSettings = !mod->showSettings;
                        ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
                    }
                    else if (ImGui::IsMouseClicked(2) && catPositions[i].isExtended) {
                        lastMod = mod;
                        isBinding = true;
                        ClientInstance::get()->playUi("random.pop", 0.75f, 1.0f);
                    }
                }

                if (modRect.y + 2.0f >= clipRect.y) {
                    ImRenderUtils::drawText(
                            ImVec2(modTextX, modTextY),
                            modName,
                            textCol,
                            inScale,
                            animation,
                            true,
                            0,
                            drawList
                    );
                }

                if (mod->showSettings) {
                    float settingY = modRect.w;
                    float visibleSettingCount = 0;

                    for (const auto& setting : mod->mSettings) {
                        bool isVisible = setting->mIsVisible();
                        float targetVisibility = isVisible ? 1.0f : 0.0f;

                        if (!setting->hasVisibilityAnim) {
                            setting->visibilityAnim = isVisible ? 1.0f : 0.0f;
                            setting->hasVisibilityAnim = true;
                        }

                        float animSpeed = isVisible ? 15.0f : 8.0f;
                        setting->visibilityAnim = MathUtils::animate(targetVisibility, setting->visibilityAnim, ImRenderUtils::getDeltaTime() * animSpeed);

                        if (setting->visibilityAnim > 0.001f) {
                            visibleSettingCount++;
                        }
                    }

                    for (const auto& setting : mod->mSettings) {
                        if (setting->visibilityAnim <= 0.001f && !setting->mIsVisible()) continue;

                        ImVec4 setRect = ImVec4(
                                modRect.x + 5,
                                settingY,
                                modRect.z - 5,
                                settingY + modHeight
                        ).scaleToPoint(ImVec4(screen.x / 2, screen.y / 2, screen.x / 2, screen.y / 2), inScale);

                        float yOffset = (1.0f - setting->visibilityAnim) * modHeight;
                        setRect.y -= yOffset;
                        setRect.w -= yOffset;

                        ImColor bgColor = settingBgColor;
                        bgColor.Value.w *= mod->cAnim * setting->visibilityAnim;
                        if (isLastModule && setting == mod->mSettings.back() && setting->visibilityAnim > 0.001f) {
                            float rounding = 25.0f;
                            ImRenderUtils::fillRectangle(
                                    setRect,
                                    settingBgColor,
                                    animation * setting->visibilityAnim,
                                    rounding,
                                    drawList,
                                    ImDrawFlags_RoundCornersBottom
                            );
                        }
                        else {
                            ImRenderUtils::fillRectangle(
                                    setRect,
                                    settingBgColor,
                                    animation * setting->visibilityAnim,
                                    0.0f,
                                    drawList
                            );
                        }

                        float opacity = mod->cAnim * setting->visibilityAnim;

                        switch (setting->mType) {
                            case SettingType::Bool: {
                                BoolSetting* boolSetting = reinterpret_cast<BoolSetting*>(setting);
                                std::string setName = lowercase ? StringUtils::toLower(setting->mName) : setting->mName;
                                float setTextY = setRect.y + ((setRect.w - setRect.y) - textHeight) / 2;

                                ImRenderUtils::drawText(
                                        ImVec2(setRect.x + 5.f, setTextY),
                                        setName,
                                        textColor,
                                        inScale,
                                        animation * opacity,
                                        true
                                );

                                static std::map<BoolSetting*, float> toggleAnimations;
                                static std::map<BoolSetting*, float> knobPositions;
                                if (toggleAnimations.find(boolSetting) == toggleAnimations.end()) {
                                    toggleAnimations[boolSetting] = boolSetting->mValue ? 1.0f : 0.0f;
                                    knobPositions[boolSetting] = boolSetting->mValue ? 1.0f : 0.0f;
                                }

                                float& toggleAnim = toggleAnimations[boolSetting];
                                float& knobPos = knobPositions[boolSetting];

                                if (boolSetting->mValue) {
                                    toggleAnim = MathUtils::animate(1.0f, toggleAnim, ImGui::GetIO().DeltaTime * 12.0f);
                                    knobPos = MathUtils::lerp(knobPos, 1.0f, ImGui::GetIO().DeltaTime * 12.0f);
                                }
                                else {
                                    toggleAnim = MathUtils::animate(0.0f, toggleAnim, ImGui::GetIO().DeltaTime * 12.0f);
                                    knobPos = MathUtils::lerp(knobPos, 0.0f, ImGui::GetIO().DeltaTime * 12.0f);
                                }

                                ImVec4 sliderBg = ImVec4(
                                        setRect.z - 40,
                                        setRect.y + (setRect.w - setRect.y) / 2 - 8,
                                        setRect.z - 5,
                                        setRect.y + (setRect.w - setRect.y) / 2 + 8
                                );

                                ImRenderUtils::fillRectangle(sliderBg, ImColor(40, 40, 40, int(255 * animation * opacity)), animation * opacity, 8.0f);

                                if (toggleAnim > 0.001f) {
                                    float animWidth = (sliderBg.z - sliderBg.x) * 1.0f;
                                    ImVec4 activeRect(
                                            sliderBg.x,
                                            sliderBg.y,
                                            sliderBg.x + animWidth,
                                            sliderBg.w
                                    );

                                    ImRenderUtils::fillRectangle(
                                            activeRect,
                                            ColorUtils::getThemedColor(setRect.y),
                                            animation * opacity * toggleAnim,
                                            8.0f
                                    );
                                }

                                float baseKnobSize = 7.0f;
                                float knobSize = MathUtils::lerp(baseKnobSize - 1.0f, baseKnobSize, toggleAnim);
                                float knobX = MathUtils::lerp(sliderBg.x + 2, sliderBg.z - knobSize * 2 - 2, knobPos);
                                ImVec2 knobCenter(knobX + knobSize, (sliderBg.y + sliderBg.w) / 2);

                                float r = MathUtils::lerp(85.f, 255.f, toggleAnim);
                                float g = MathUtils::lerp(85.f, 255.f, toggleAnim);
                                float b = MathUtils::lerp(85.f, 255.f, toggleAnim);
                                ImColor knobColor(r / 255.f, g / 255.f, b / 255.f, 1.0f);

                                ImRenderUtils::fillCircle(knobCenter, knobSize, knobColor, animation * opacity, 12);

                                if (ImRenderUtils::isMouseOver(setRect) && clickGui->mEnabled && setting->visibilityAnim > 0.9f) {
                                    tooltip = setting->mDescription;
                                    if (ImGui::IsMouseClicked(0) && !displayColorPicker && setting->visibilityAnim > 0.9f) {
                                        boolSetting->mValue = !boolSetting->mValue;
                                        ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
                                    }
                                }

                                break;
                            }
                            case SettingType::Number:
                            {
                                NumberSetting* numSetting = reinterpret_cast<NumberSetting*>(setting);
                                const float value = numSetting->mValue;
                                const float min = numSetting->mMin;
                                const float max = numSetting->mMax;

                                std::string setName = lowercase ? StringUtils::toLower(setting->mName) : setting->mName;

                                static std::map<NumberSetting*, bool> isEditing;
                                static std::map<NumberSetting*, std::string> editBuffer;
                                static std::map<NumberSetting*, float> cursorAnim;
                                static std::map<NumberSetting*, bool> keyReleased;
                                static std::map<NumberSetting*, float> textScaleAnim;
                                static std::map<NumberSetting*, float> textPosAnim;
                                static std::map<NumberSetting*, float> cursorAppearAnim;
                                static std::map<NumberSetting*, float> selectionAnim;
                                static std::map<NumberSetting*, bool> isSelected;
                                static bool blockInputsUntilRelease = false;

                                if (isEditing.find(numSetting) == isEditing.end()) {
                                    isEditing[numSetting] = false;
                                    editBuffer[numSetting] = "";
                                    cursorAnim[numSetting] = 0.0f;
                                    keyReleased[numSetting] = true;
                                    textScaleAnim[numSetting] = 1.0f;
                                    textPosAnim[numSetting] = 0.0f;
                                    cursorAppearAnim[numSetting] = 0.0f;
                                    selectionAnim[numSetting] = 0.0f;
                                    isSelected[numSetting] = false;
                                }

                                char valueStr[32];
                                float roundedValue;

                                if (!isEditing[numSetting]) {
                                    roundedValue = std::round(value * 100.0f) / 100.0f;
                                    if (std::floor(roundedValue) == roundedValue) {
                                        sprintf_s(valueStr, "%d", (int)roundedValue);
                                    }
                                    else {
                                        sprintf_s(valueStr, "%.2f", roundedValue);
                                        std::string valueText = valueStr;
                                        while (valueText.back() == '0') valueText.pop_back();
                                        if (valueText.back() == '.') valueText += "0";
                                        strcpy_s(valueStr, valueText.c_str());
                                    }
                                }
                                else {
                                    strcpy_s(valueStr, editBuffer[numSetting].c_str());
                                }

                                std::string valueName = valueStr;

                                float sliderWidth = setRect.getWidth() - 14.0f;
                                float progress = (value - min) / (max - min);
                                setting->sliderEase = MathUtils::animate(progress * sliderWidth, setting->sliderEase, ImRenderUtils::getDeltaTime() * 15);
                                setting->sliderEase = std::clamp(setting->sliderEase, 0.f, sliderWidth);

                                float sliderY = setRect.y + setRect.getHeight() * 0.7f;
                                ImVec4 sliderBg = ImVec4(
                                        setRect.x + 5,
                                        sliderY - 2,
                                        setRect.x + 5 + sliderWidth,
                                        sliderY + 2
                                );

                                ImRenderUtils::fillRectangle(sliderBg, ImColor(40, 40, 40), animation * opacity, 2.0f);

                                ImVec4 filledSlider = ImVec4(
                                        sliderBg.x,
                                        sliderBg.y,
                                        sliderBg.x + setting->sliderEase,
                                        sliderBg.w
                                );
                                ImRenderUtils::fillRectangle(
                                        filledSlider,
                                        ColorUtils::getThemedColor(clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal ? filledSlider.x : filledSlider.y),
                                        animation * opacity,
                                        2.0f
                                );

                                float knobX = filledSlider.z;
                                float knobY = (sliderBg.y + sliderBg.w) / 2;
                                float knobSize = setting->isDragging ? 4.0f : 5.0f;

                                ImRenderUtils::fillCircle(
                                        ImVec2(knobX, knobY),
                                        knobSize + 1.0f,
                                        ColorUtils::getThemedColor(clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal ? knobX : knobY),
                                        animation * opacity,
                                        12
                                );

                                ImRenderUtils::fillCircle(
                                        ImVec2(knobX, knobY),
                                        knobSize,
                                        ImColor(255, 255, 255),
                                        animation * opacity,
                                        12
                                );

                                float textY = setRect.y - 1.0f;
                                float valueWidth = ImRenderUtils::getTextWidth(&valueName, inScale);

                                ImRenderUtils::drawText(
                                        ImVec2(setRect.x + 5.f, textY),
                                        setName,
                                        ImColor(255, 255, 255),
                                        inScale,
                                        animation * opacity,
                                        true
                                );

                                float targetScale = 1.0f;
                                float targetPos = 0.0f;

                                if (isEditing[numSetting]) {
                                    textScaleAnim[numSetting] = 1.0f;
                                    textPosAnim[numSetting] = MathUtils::lerp(textPosAnim[numSetting], targetPos, ImGui::GetIO().DeltaTime * 12.0f);
                                    cursorAppearAnim[numSetting] = MathUtils::lerp(cursorAppearAnim[numSetting], 1.0f, ImGui::GetIO().DeltaTime * 12.0f);
                                }
                                else {
                                    textScaleAnim[numSetting] = 1.0f;
                                    textPosAnim[numSetting] = MathUtils::lerp(textPosAnim[numSetting], 0.0f, ImGui::GetIO().DeltaTime * 12.0f);
                                    cursorAppearAnim[numSetting] = MathUtils::lerp(cursorAppearAnim[numSetting], 0.0f, ImGui::GetIO().DeltaTime * 12.0f);
                                }

                                ImVec2 textPos = ImVec2(
                                        setRect.z - valueWidth - 5.f + textPosAnim[numSetting],
                                        textY
                                );

                                if (isEditing[numSetting] && isSelected[numSetting]) {
                                    float selectionAlpha = selectionAnim[numSetting];
                                    ImVec4 selectionRect(
                                            textPos.x,
                                            textPos.y,
                                            textPos.x + valueWidth,
                                            textPos.y + ImGui::GetFont()->FontSize * inScale
                                    );

                                    ImRenderUtils::fillRectangle(
                                            selectionRect,
                                            ImColor(100, 150, 255, int(80 * selectionAlpha * animation * opacity)),
                                            animation * opacity,
                                            0.0f
                                    );
                                }

                                ImRenderUtils::drawText(
                                        textPos,
                                        valueName,
                                        ImColor(170, 170, 170),
                                        inScale,
                                        animation * opacity,
                                        true
                                );

                                if (isEditing[numSetting]) {
                                    if (isSelected[numSetting]) {
                                        selectionAnim[numSetting] = MathUtils::lerp(selectionAnim[numSetting], 1.0f, ImGui::GetIO().DeltaTime * 8.0f);
                                    }
                                    else {
                                        selectionAnim[numSetting] = MathUtils::lerp(selectionAnim[numSetting], 0.0f, ImGui::GetIO().DeltaTime * 8.0f);
                                    }

                                    if (GetAsyncKeyState(VK_CONTROL) & 0x8000 && ImGui::IsKeyPressed(ImGuiKey_A)) {
                                        isSelected[numSetting] = true;
                                    }

                                    for (const auto& key : Keyboard::mPressedKeys) {
                                        if (key.second && key.first != VK_CONTROL) {
                                            isSelected[numSetting] = false;
                                            break;
                                        }
                                    }

                                    cursorAnim[numSetting] = std::fmod(cursorAnim[numSetting] + ImGui::GetIO().DeltaTime * 2.0f, 2.0f);
                                    float cursorAlpha = (std::cos(cursorAnim[numSetting] * 3.14159f) + 1.0f) * 0.5f;

                                    float cursorHeight = ImGui::GetFont()->FontSize * 0.4f * inScale;
                                    float cursorY = textY + (ImGui::GetFont()->FontSize * 0.08f * inScale);

                                    float maxCursorX = setRect.z - 5.f - valueWidth + ImRenderUtils::getTextWidth(&valueName, inScale) + 3.f;

                                    float baseCursorX = setRect.z - 5.f - valueWidth + ImRenderUtils::getTextWidth(&valueName, inScale) + 3.f;

                                    float cursorX = std::min(maxCursorX, baseCursorX + textPosAnim[numSetting]);

                                    ImRenderUtils::fillRectangle(
                                            ImVec4(cursorX, cursorY, cursorX + 1.0f, cursorY + cursorHeight),
                                            ImColor(255, 255, 255, int(255 * cursorAlpha * cursorAppearAnim[numSetting])),
                                            animation * opacity,
                                            0.0f
                                    );

                                    if (ImGui::IsMouseClicked(0)) {
                                        if (!editBuffer[numSetting].empty()) {
                                            try {
                                                if (numSetting->parse(editBuffer[numSetting])) {
                                                    ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
                                                }
                                            }
                                            catch (...) {}
                                        }
                                        isEditing[numSetting] = false;
                                        editBuffer[numSetting] = "";
                                        blockInputsUntilRelease = true;
                                    }

                                    for (const auto& key : Keyboard::mPressedKeys) {
                                        if (!key.second || !keyReleased[numSetting]) continue;

                                        if (key.first == VK_ESCAPE) {
                                            isEditing[numSetting] = false;
                                            editBuffer[numSetting] = "";
                                            keyReleased[numSetting] = false;
                                        }
                                        else if (key.first == VK_RETURN) {
                                            if (!editBuffer[numSetting].empty()) {
                                                try {
                                                    if (numSetting->parse(editBuffer[numSetting])) {
                                                        ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
                                                    }
                                                }
                                                catch (...) {}
                                            }
                                            isEditing[numSetting] = false;
                                            editBuffer[numSetting] = "";
                                            keyReleased[numSetting] = false;
                                        }
                                        else if (key.first == VK_BACK) {
                                            if (!editBuffer[numSetting].empty()) {
                                                editBuffer[numSetting].pop_back();

                                                textPosAnim[numSetting] = std::max(-5.0f, textPosAnim[numSetting] - 5.0f);
                                            }
                                            keyReleased[numSetting] = false;
                                        }
                                        else {
                                            char c = 0;
                                            if (key.first >= '0' && key.first <= '9') c = key.first;
                                            else if (key.first == VK_OEM_PERIOD) c = '.';
                                            else if (key.first == VK_OEM_MINUS) c = '-';
                                            else if (key.first == 'E' || key.first == 'e') c = 'e';
                                            else if (key.first == VK_OEM_PLUS) c = '+';

                                            if (c != 0) {
                                                if ((c == '.' && editBuffer[numSetting].find('.') != std::string::npos) ||
                                                    (c == '-' && !editBuffer[numSetting].empty() && editBuffer[numSetting][0] != 'e') ||
                                                    (c == '+' && (!editBuffer[numSetting].empty() &&
                                                                  (editBuffer[numSetting].back() != 'e' && editBuffer[numSetting].back() != 'E'))) ||
                                                    ((c == 'e') && (editBuffer[numSetting].empty() ||
                                                                    editBuffer[numSetting].find('e') != std::string::npos ||
                                                                    editBuffer[numSetting].find('E') != std::string::npos))) {
                                                    keyReleased[numSetting] = false;
                                                    break;
                                                }

                                                editBuffer[numSetting] += c;

                                                textPosAnim[numSetting] = std::min(5.0f, textPosAnim[numSetting] + 5.0f);
                                            }
                                            keyReleased[numSetting] = false;
                                        }
                                    }

                                    bool anyKeyPressed = false;
                                    for (const auto& key : Keyboard::mPressedKeys) {
                                        if (key.second) {
                                            anyKeyPressed = true;
                                            break;
                                        }
                                    }
                                    if (!anyKeyPressed) {
                                        keyReleased[numSetting] = true;
                                    }
                                }

                                if (ImRenderUtils::isMouseOver(setRect) && clickGui->mEnabled && !blockInputsUntilRelease) {
                                    tooltip = setting->mDescription;
                                    bool isOverSlider = ImGui::GetIO().MousePos.y >= sliderY - 10 &&
                                                        ImGui::GetIO().MousePos.y <= sliderY + 10;

                                    if ((ImGui::IsMouseDown(0) || ImGui::IsMouseDown(2)) &&
                                        setting->visibilityAnim > 0.9f && isOverSlider) {
                                        setting->isDragging = true;
                                        lastDraggedSetting = setting;
                                        isEditing[numSetting] = false;
                                        editBuffer[numSetting] = "";
                                    }
                                    else if (ImGui::IsMouseClicked(1) && setting->visibilityAnim > 0.9f) {
                                        isEditing[numSetting] = true;
                                        editBuffer[numSetting] = valueName;
                                        cursorAnim[numSetting] = 0.0f;
                                        keyReleased[numSetting] = true;
                                        textPosAnim[numSetting] = 0.0f;
                                        cursorAppearAnim[numSetting] = 0.0f;
                                        selectionAnim[numSetting] = 0.0f;
                                        isSelected[numSetting] = false;
                                    }
                                }

                                if (ImGui::IsMouseDown(0) && setting->isDragging && clickGui->mEnabled && setting->visibilityAnim > 0.9f) {
                                    if (lastDraggedSetting != setting) {
                                        setting->isDragging = false;
                                    }
                                    else {
                                        const float newValue = std::fmax(
                                                std::fmin((ImRenderUtils::getMousePos().x - setRect.x) / (setRect.z - setRect.x) * (max - min) + min, max), min);
                                        numSetting->setValue(newValue);
                                    }
                                }
                                else if (ImGui::IsMouseDown(2) && setting->isDragging && clickGui->mEnabled)
                                {
                                    if (lastDraggedSetting != setting)
                                    {
                                        setting->isDragging = false;
                                    }
                                    else
                                    {
                                        float newValue = std::fmax(
                                                std::fmin((ImRenderUtils::getMousePos().x - setRect.x) / (setRect.z - setRect.x) * (max - min) + min, max), min);

                                        newValue = std::round(newValue / midclickRounding) * midclickRounding;
                                        numSetting->setValue(newValue);
                                    }
                                }
                                else {
                                    setting->isDragging = false;
                                }

                                if (!ImGui::IsMouseDown(0) && !ImGui::IsMouseDown(1) && !ImGui::IsMouseDown(2)) {
                                    blockInputsUntilRelease = false;
                                }

                                if (isLastModule && setting->visibilityAnim <= 0.001f) {
                                    ImRenderUtils::fillRectangle(setRect, ImColor(30, 30, 30), animation, 25.0f, ImGui::GetBackgroundDrawList(), ImDrawFlags_RoundCornersBottom);
                                }

                                break;
                            }
                            case SettingType::Enum: {
                                EnumSetting* enumSetting = reinterpret_cast<EnumSetting*>(setting);
                                std::string setName = lowercase ? StringUtils::toLower(setting->mName) : setting->mName;
                                std::string currentValue = enumSetting->mValues[enumSetting->mValue];
                                if (lowercase) currentValue = StringUtils::toLower(currentValue);

                                static std::map<EnumSetting*, float> fadeAnimations;
                                static std::map<EnumSetting*, std::string> lastEnumValues;

                                if (fadeAnimations.find(enumSetting) == fadeAnimations.end()) {
                                    fadeAnimations[enumSetting] = 1.0f;
                                    lastEnumValues[enumSetting] = currentValue;
                                }

                                float& fadeAnim = fadeAnimations[enumSetting];

                                if (lastEnumValues[enumSetting] != currentValue && enumSetting->mValues.size() > 1) {
                                    fadeAnim = 0.0f;
                                    lastEnumValues[enumSetting] = currentValue;
                                }

                                fadeAnim = MathUtils::animate(1.0f, fadeAnim, deltaTime * 12.0f);

                                float setTextY = setRect.y + ((setRect.w - setRect.y) - textHeight) / 2;
                                float valueWidth = ImRenderUtils::getTextWidth(&currentValue, inScale);

                                ImRenderUtils::drawText(
                                        ImVec2(setRect.x + 5.f, setTextY),
                                        setName + ": ",
                                        textColor,
                                        inScale,
                                        animation * opacity,
                                        true
                                );

                                ImColor valueColor = ImColor(170, 170, 170);
                                valueColor.Value.w = fadeAnim * animation * opacity;

                                float valueY = setRect.y + ((setRect.w - setRect.y) - textHeight) / 2;
                                ImRenderUtils::drawText(
                                        ImVec2(setRect.z - valueWidth - 5.f, valueY),
                                        currentValue,
                                        valueColor,
                                        inScale,
                                        animation * opacity,
                                        true
                                );

                                if (ImRenderUtils::isMouseOver(setRect) && clickGui->mEnabled && setting->visibilityAnim > 0.9f) {
                                    tooltip = setting->mDescription;
                                    if (ImGui::IsMouseClicked(0) && !displayColorPicker && setting->visibilityAnim > 0.9f && enumSetting->mValues.size() > 1) {
                                        enumSetting->mValue = (enumSetting->mValue + 1) % enumSetting->mValues.size();
                                        ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
                                    }
                                    else if (ImGui::IsMouseClicked(1) && !displayColorPicker && setting->visibilityAnim > 0.9f && enumSetting->mValues.size() > 1) {
                                        enumSetting->mValue = (enumSetting->mValue - 1 + enumSetting->mValues.size()) % enumSetting->mValues.size();
                                        ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
                                    }
                                }
                                break;
                            }
                            case SettingType::Color: {
                                ColorSetting* colorSetting = reinterpret_cast<ColorSetting*>(setting);
                                std::string setName = lowercase ? StringUtils::toLower(setting->mName) : setting->mName;
                                float setTextY = setRect.y + ((setRect.w - setRect.y) - textHeight) / 2;

                                ImRenderUtils::drawText(
                                        ImVec2(setRect.x + 10, setTextY),
                                        setName,
                                        textColor,
                                        inScale,
                                        animation * opacity,
                                        true
                                );

                                ImVec4 colorRect = ImVec4(
                                        setRect.z - 30,
                                        setRect.y + 5,
                                        setRect.z - 10,
                                        setRect.w - 5
                                );

                                ImRenderUtils::fillRectangle(
                                        colorRect,
                                        colorSetting->getAsImColor(),
                                        animation * opacity,
                                        4.0f,
                                        drawList,
                                        ImDrawFlags_RoundCornersAll
                                );

                                if (ImRenderUtils::isMouseOver(setRect) && clickGui->mEnabled && setting->visibilityAnim > 0.9f) {
                                    tooltip = setting->mDescription;
                                    if (ImGui::IsMouseClicked(0) && setting->visibilityAnim > 0.9f) {
                                        lastColorSetting = colorSetting;
                                        displayColorPicker = !displayColorPicker;
                                        ClientInstance::get()->playUi("random.click", 0.75f, 1.0f);
                                    }
                                }
                                break;
                            }
                            case SettingType::Divider: {
                                DividerSetting* dividerSetting = reinterpret_cast<DividerSetting*>(setting);
                                std::string setName = lowercase ? StringUtils::toLower(setting->mName) : setting->mName;

                                float textWidth = ImRenderUtils::getTextWidth(&setName, inScale);
                                float setTextY = setRect.y + ((setRect.w - setRect.y) - textHeight) / 2;
                                float lineY = setRect.y + (setRect.w - setRect.y) / 2;
                                float padding = 10.0f;
                                float availableWidth = setRect.getWidth() - (padding * 2);
                                float textStart = setRect.x + (setRect.getWidth() - textWidth) / 2;

                                float lineThickness = 1.0f;
                                ImColor lineColor = ImColor(100, 100, 100);
                                float fadeWidth = 20.0f;

                                float leftLineStart = setRect.x + padding;
                                float leftLineEnd = textStart - 5.0f;
                                for (float x = leftLineStart; x < leftLineEnd; x += 1.0f) {
                                    float alpha = 1.0f;
                                    if (x < leftLineStart + fadeWidth)
                                        alpha = (x - leftLineStart) / fadeWidth;
                                    else if (x > leftLineEnd - fadeWidth)
                                        alpha = (leftLineEnd - x) / fadeWidth;

                                    drawList->AddLine(
                                            ImVec2(x, lineY),
                                            ImVec2(x + 1.0f, lineY),
                                            ImColor(lineColor.Value.x, lineColor.Value.y, lineColor.Value.z, alpha * animation * opacity),
                                            lineThickness
                                    );
                                }

                                float rightLineStart = textStart + textWidth + 5.0f;
                                float rightLineEnd = setRect.z - padding;
                                for (float x = rightLineStart; x < rightLineEnd; x += 1.0f) {
                                    float alpha = 1.0f;
                                    if (x < rightLineStart + fadeWidth)
                                        alpha = (x - rightLineStart) / fadeWidth;
                                    else if (x > rightLineEnd - fadeWidth)
                                        alpha = (rightLineEnd - x) / fadeWidth;

                                    drawList->AddLine(
                                            ImVec2(x, lineY),
                                            ImVec2(x + 1.0f, lineY),
                                            ImColor(lineColor.Value.x, lineColor.Value.y, lineColor.Value.z, alpha * animation * opacity),
                                            lineThickness
                                    );
                                }

                                ImRenderUtils::drawText(
                                        ImVec2(textStart, setTextY),
                                        setName,
                                        ImColor(170, 170, 170),
                                        inScale,
                                        animation * opacity,
                                        true
                                );
                                break;
                            }
                            case SettingType::Keybind: {
                                KeybindSetting* keybind = reinterpret_cast<KeybindSetting*>(setting);
                                int displayKey = keybind->mKey != 0 ? keybind->mKey : mod->mKey;
                                std::string keyName = displayKey == 0 ? "NONE" : Keyboard::getKey(displayKey);
                                std::string label = "Bind:";
                                if (SimpleGui::isKeybindBinding && SimpleGui::lastKeybindSetting == keybind) {
                                    label += " Listening...";
                                }
                                ImColor boxColor = (SimpleGui::isKeybindBinding && SimpleGui::lastKeybindSetting == keybind) ? ImColor(80, 120, 255) : ImColor(40, 40, 40);
                                
                                float setTextY = setRect.y + ((setRect.w - setRect.y) - textHeight) / 2;
                                ImRenderUtils::drawText(ImVec2(setRect.x + 5.f, setTextY), label, textColor, inScale, animation * opacity, true);
                                
                                float valueWidth = ImRenderUtils::getTextWidth(&keyName, inScale);
                                ImVec4 keybindRect = ImVec4(setRect.z - valueWidth - 15.f, setRect.y + 2.f, setRect.z - 5.f, setRect.w - 2.f);
                                ImRenderUtils::fillRectangle(keybindRect, boxColor, animation * opacity, 4.0f, drawList, ImDrawFlags_RoundCornersAll);
                                ImRenderUtils::drawText(ImVec2(setRect.z - valueWidth - 8.f, setTextY), keyName, ImColor(255, 255, 255), inScale, animation * opacity, true);
                                
                                if (ImRenderUtils::isMouseOver(setRect) && clickGui->mEnabled && setting->visibilityAnim > 0.9f && ImGui::IsMouseClicked(0)) {
                                    SimpleGui::isKeybindBinding = true;
                                    SimpleGui::lastKeybindSetting = keybind;
                                }
                                if (SimpleGui::isKeybindBinding && SimpleGui::lastKeybindSetting == keybind) {
                                    for (const auto& key : Keyboard::mPressedKeys) {
                                        if (key.second) {
                                            keybind->mKey = (key.first == VK_ESCAPE ? 0 : key.first);
                                            SimpleGui::isKeybindBinding = false;
                                            SimpleGui::lastKeybindSetting = nullptr;
                                            SimpleGui::justUnboundKeybind = true;
                                            ClientInstance::get()->playUi((key.first == VK_ESCAPE) ? "random.break" : "random.orb", 0.75f, 1.0f);
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                        }

                        settingY += modHeight * setting->visibilityAnim;
                    }
                }

                moduleY += modHeight;
                if (mod->showSettings || mod->cAnim > 0.001f) {
                    for (const auto& setting : mod->mSettings) {
                        if (setting->mIsVisible()) {
                            moduleY += modHeight * mod->cAnim;
                        }
                    }
                }
                moduleY += 2.0f;
            }

            drawList->PopClipRect();
        }
    }

    static float tooltipTimer = 0.0f;
    static float tooltipAlpha = 0.0f;
    static std::string currentTooltip = "";
    static bool isHoveringTooltip = false;

    if (!tooltip.empty()) {
        if (tooltip != currentTooltip) {
            tooltipTimer = 0.0f;
            tooltipAlpha = 0.0f;
            currentTooltip = tooltip;
            isHoveringTooltip = true;
        }

        if (isHoveringTooltip) {
            tooltipTimer += ImGui::GetIO().DeltaTime;
            if (tooltipTimer >= 0.5f) {
                tooltipAlpha = MathUtils::animate(1.0f, tooltipAlpha, ImGui::GetIO().DeltaTime * 8.0f);
            }
        }

        if (tooltipAlpha > 0.001f) {
            ImVec2 toolTipSize = ImGui::GetFont()->CalcTextSizeA(inScale * 14.4f, FLT_MAX, 0, tooltip.c_str());
            float padding = 5.0f;

            ImVec4 tooltipRect = ImVec4(
                    ImGui::GetIO().MousePos.x + 15,
                    ImGui::GetIO().MousePos.y - toolTipSize.y / 2 - padding,
                    ImGui::GetIO().MousePos.x + 15 + toolTipSize.x + padding * 2,
                    ImGui::GetIO().MousePos.y + toolTipSize.y / 2 + padding
            );

            ImRenderUtils::fillRectangle(tooltipRect, ImColor(20, 20, 20, int(200 * tooltipAlpha)), animation, 5.0f, drawList, ImDrawFlags_RoundCornersAll);
            ImRenderUtils::drawText(
                    ImVec2(tooltipRect.x + padding, tooltipRect.y + padding),
                    tooltip,
                    ImColor(textColor.Value.x, textColor.Value.y, textColor.Value.z, tooltipAlpha),
                    inScale * 0.8f,
                    animation,
                    true
            );
        }
    }
    else {
        tooltipTimer = 0.0f;
        tooltipAlpha = 0.0f;
        currentTooltip = "";
        isHoveringTooltip = false;
    }

    if (isBinding) {

        ImVec2 screen = ImRenderUtils::getScreenSize();
        std::string bindingText = "Currently Binding...";

        if (lastMod) {
            for (const auto& key : Keyboard::mPressedKeys) {
                if (key.second) {
                    if (key.first == VK_ESCAPE) {
                        lastMod->mKey = 0;
                        bindingText = lastMod->getName() + " unbound";
                    }
                    else {
                        lastMod->mKey = key.first;
                        bindingText = lastMod->getName() + " bound to " + Keyboard::getKey(key.first);
                    }
                    isBinding = false;
                    ClientInstance::get()->playUi(key.first == VK_ESCAPE ? "random.break" : "random.orb", 0.75f, 1.0f);
                }
            }
        }

        float padding = 20.0f;
        float textWidth = ImRenderUtils::getTextWidth(&bindingText, inScale * 1.2f);
        float notifWidth = textWidth + padding * 2;
        float notifHeight = 40.0f;
        float bottomOffset = 60.0f;

        ImVec4 notifRect(
                (screen.x - notifWidth) / 2,
                screen.y - bottomOffset - notifHeight,
                (screen.x + notifWidth) / 2,
                screen.y - bottomOffset
        );

        static float notifAnim = 0.0f;
        notifAnim = MathUtils::animate(1.0f, notifAnim, deltaTime * 8.0f);

        drawList->AddRectFilled(
                ImVec2(notifRect.x - 2, notifRect.y - 2),
                ImVec2(notifRect.z + 2, notifRect.w + 2),
                ImColor(0, 0, 0, int(100 * notifAnim)),
                8.0f
        );

        ImRenderUtils::fillRectangle(
                notifRect,
                ImColor(28, 28, 28, int(255 * notifAnim)),
                notifAnim,
                8.0f
        );

        ImRenderUtils::drawText(
                ImVec2(notifRect.x + (notifRect.z - notifRect.x - textWidth) / 2,
                       notifRect.y + (notifRect.w - notifRect.y - ImGui::GetFont()->FontSize) / 2),
                bindingText,
                ImColor(255, 255, 255, int(255 * notifAnim)),
                inScale * 1.2f,
                notifAnim,
                true
        );
    }

    if (wasColorPickerDisplayed) {
        FontHelper::pushPrefFont(false, false, true);
        ColorSetting* colorSetting = lastColorSetting;

        drawList->AddRectFilled(
                ImVec2(0, 0),
                screen,
                ImColor(0, 0, 0, int(60 * colorPickerAlpha))
        );

        ImGui::SetNextWindowPos(ImVec2(screen.x / 2 - 200, screen.y / 2 - 225));
        ImGui::SetNextWindowSize(ImVec2(400, 450));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, colorPickerAlpha);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.11f, 0.11f, colorPickerAlpha));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.20f, 0.20f, colorPickerAlpha));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.11f, 0.11f, 0.11f, colorPickerAlpha));

        ImGui::Begin("Color Picker", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoSavedSettings);

        bool colorChanged = ImGui::ColorPicker4("##picker", colorSetting->mValue,
                                                ImGuiColorEditFlags_NoSidePreview |
                                                ImGuiColorEditFlags_NoSmallPreview |
                                                ImGuiColorEditFlags_NoAlpha |
                                                ImGuiColorEditFlags_NoLabel |
                                                ImGuiColorEditFlags_NoTooltip |
                                                ImGuiColorEditFlags_NoInputs |
                                                ImGuiColorEditFlags_PickerHueBar |
                                                ImGuiColorEditFlags_NoOptions |
                                                ImGuiColorEditFlags_DisplayRGB);

        char colorCode[32];
        sprintf_s(colorCode, "RGB: %d, %d, %d",
                  int(colorSetting->mValue[0] * 255),
                  int(colorSetting->mValue[1] * 255),
                  int(colorSetting->mValue[2] * 255));

        char hexCode[32];
        sprintf_s(hexCode, "HEX: #%02X%02X%02X",
                  int(colorSetting->mValue[0] * 255),
                  int(colorSetting->mValue[1] * 255),
                  int(colorSetting->mValue[2] * 255));

        ImGui::Text("%s", colorCode);
        ImGui::Text("%s", hexCode);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, colorPickerAlpha));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, colorPickerAlpha));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.35f, 0.35f, colorPickerAlpha));

        if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 30))) {
            displayColorPicker = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Copy", ImVec2(ImGui::GetContentRegionAvail().x, 30))) {
            ImGui::SetClipboardText(hexCode + 5);
        }

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::End();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(3);
        ImGui::PopFont();

        if (ImGui::IsMouseClicked(0)) {
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            ImVec2 pickerMin(screen.x / 2 - 200, screen.y / 2 - 225);
            ImVec2 pickerMax(screen.x / 2 + 200, screen.y / 2 + 225);
            if (mousePos.x < pickerMin.x || mousePos.x > pickerMax.x ||
                mousePos.y < pickerMin.y || mousePos.y > pickerMax.y) {
                displayColorPicker = false;
            }
        }
    }

    ImGui::PopFont();
}

void SimpleGui::onWindowResizeEvent(WindowResizeEvent& event) {
    resetPosition = true;
    lastReset = NOW;
}

std::string SimpleGui::getCategoryIcon(const std::string& category) {
    if (StringUtils::equalsIgnoreCase(category, "Combat")) return "c";
    if (StringUtils::equalsIgnoreCase(category, "Movement")) return "f";
    if (StringUtils::equalsIgnoreCase(category, "Visual")) return "d";
    if (StringUtils::equalsIgnoreCase(category, "Player")) return "e";
    if (StringUtils::equalsIgnoreCase(category, "Misc")) return "a";
    return "B";
}