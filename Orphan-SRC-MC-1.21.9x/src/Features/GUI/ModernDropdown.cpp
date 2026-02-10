//
// Created by Tozic on 7/15/2024.
//

#include "ModernDropdown.hpp"
#include <Features/Modules/ModuleCategory.hpp>
#include <Features/Modules/Visual/ClickGui.hpp>
#include <Utils/FontHelper.hpp>
#include <Utils/MiscUtils/ImRenderUtils.hpp>
#include <Utils/MiscUtils/MathUtils.hpp>
#include <Features/Modules/Setting.hpp>
#include <Features/Modules/Visual/Interface.hpp>
#include <Features/Modules/Visual/ClickGui.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Rendering/GuiData.hpp>
#include <Utils/Keyboard.hpp>
#include <Utils/StringUtils.hpp>
#include <Utils/MiscUtils/ColorUtils.hpp>

// Initialize static members
bool ModernGui::isKeybindBinding = false;
KeybindSetting* ModernGui::lastKeybindSetting = nullptr;
bool ModernGui::isKeybindBindingActive = false;
bool ModernGui::justUnboundKeybind = false;

ImVec4 ModernGui::scaleToPoint(const ImVec4& _this, const ImVec4& point, float amount)
{
    return { point.x + (_this.x - point.x) * amount, point.y + (_this.y - point.y) * amount,
        point.z + (_this.z - point.z) * amount, point.w + (_this.w - point.w) * amount };
}

bool ModernGui::isMouseOver(const ImVec4& rect)
{
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    return mousePos.x >= rect.x && mousePos.y >= rect.y && mousePos.x < rect.z && mousePos.y < rect.w;
}

ImVec4 ModernGui::getCenter(ImVec4& vec)
{
    float centerX = (vec.x + vec.z) / 2.0f;
    float centerY = (vec.y + vec.w) / 2.0f;
    return { centerX, centerY, centerX, centerY };
}

void ModernGui::render(float animation, float inScale, int& scrollDirection, char* h, float blur, float midclickRounding, bool isPressingShift)
{
    static auto* clickGui = gFeatureManager->mModuleManager->getModule<ClickGui>();
    static auto interfaceMod = gFeatureManager->mModuleManager->getModule<Interface>();
    if (clickGui && clickGui->mEnabled) {
        ImGui::GetIO().WantCaptureKeyboard = true;
    }
    bool lowercase = interfaceMod->mNamingStyle.mValue == NamingStyle::Lowercase || interfaceMod->mNamingStyle.mValue == NamingStyle::LowercaseSpaced;

    FontHelper::pushPrefFont(true, interfaceMod->mBold.mValue, interfaceMod->mSansFont.mValue);

    ImVec2 screen = ImRenderUtils::getScreenSize();
    float deltaTime = ImGui::GetIO().DeltaTime;
    auto drawList = ImGui::GetBackgroundDrawList();

    if (resetPosition && NOW - lastReset > 100)
    {
        catPositions.clear();
        ImVec2 screen = ImRenderUtils::getScreenSize();
        auto categories = ModuleCategoryNames;
        if (catPositions.empty())
        {
            float centerX = screen.x / 2.f;
            float xPos = centerX - (categories.size() * (catWidth + catGap) / 2);
            for (std::string& category : categories)
            {
                CategoryPosition pos;
                pos.x = xPos;
                pos.y = catGap * 2;
                pos.x = std::round(pos.x / 2) * 2;
                pos.y = std::round(pos.y / 2) * 2;

                xPos += catWidth + catGap;
                catPositions.push_back(pos);
            }
        }
        resetPosition = false;
    }

    drawList->AddRectFilled(ImVec2(0, 0), ImVec2(screen.x, screen.y), IM_COL32(0, 0, 0, (clickGui->mBG.mValue * 255 * animation))); // background

    ImColor categoryColor = ImColor(0, 0, 0, 180);

    static std::vector<std::string> categories = ModuleCategoryNames;
    static std::vector<std::shared_ptr<Module>>& modules = gFeatureManager->mModuleManager->getModules();

    bool isEnabled = clickGui->mEnabled;
    std::string tooltip = "";

    float textSize = inScale;
    float textHeight = ImGui::GetFont()->CalcTextSizeA(textSize * 18, FLT_MAX, -1, "").y;

    int screenWidth = (int)screen.x;
    int screenHeight = 10;

    float windowWidth = 220.0f;
    float windowHeight = 190.0f;
    float yOffset = 50.0f;
    float windowX = (screenWidth - windowWidth) * 0.5f;
    float windowY = screenHeight;

    if (displayColorPicker && isEnabled)
    {
        FontHelper::pushPrefFont(false, false, true);
        ColorSetting* colorSetting = lastColorSetting;
        // Display the color picker in the bottom middle of the screen
        ImGui::SetNextWindowPos(ImVec2(screen.x / 2 - 200, screen.y / 2));
        ImGui::SetNextWindowSize(ImVec2(400, 400));

        ImGui::Begin("Color Picker", &displayColorPicker, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        {
            ImVec4 color = colorSetting->getAsImColor().Value;
            ImGui::ColorPicker4("Color", colorSetting->mValue, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
            ImGui::Button("Close");
            if (ImGui::IsItemClicked())
            {
                // Set the color setting to the new color
                colorSetting->setFromImColor(ImColor(color));
                displayColorPicker = false;
            }
        }
        ImGui::End();
        ImGui::PopFont();

        if (ImGui::IsMouseClicked(0) && !ImRenderUtils::isMouseOver(ImVec4(screen.x / 2 - 200, screen.y / 2, screen.x / 2 + 200, screen.y / 2 + 400)))
        {
            displayColorPicker = false;
        }
    }

    if (!isEnabled) displayColorPicker = false;

    if (catPositions.empty() && isEnabled)
    {
        float centerX = screen.x / 2.f;
        float xPos = centerX - (categories.size() * (catWidth + catGap) / 2);
        for (std::string& category : categories)
        {
            CategoryPosition pos;
            pos.x = xPos;
            pos.y = 0;
            pos.x = std::round(pos.x / 2) * 2;
            pos.y = std::round(pos.y / 2) * 2;
            xPos += catWidth + catGap;
            catPositions.push_back(pos);
        }
    }

    if (!catPositions.empty())
    {
        for (size_t i = 0; i < categories.size(); i++)
        {
            const float modWidth = catWidth;
            const float modHeight = catHeight;
            float moduleY = -catPositions[i].yOffset;
            float moduleX = catPositions[i].x;

            const auto& modsInCategory = gFeatureManager->mModuleManager->getModulesInCategory(i);

            ImVec4 catRect = ImVec4(catPositions[i].x, catPositions[i].y, catPositions[i].x + catWidth, catPositions[i].y + catHeight).scaleToPoint(ImVec4(screen.x / 2, screen.y / 2, screen.x / 2, screen.y / 2), inScale);

            catRect.w += 1.5f;
            ImRenderUtils::fillRectangle(catRect, darkBlack, animation, 15, ImGui::GetBackgroundDrawList(), clickGui->mRoundTop.mValue == true ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersNone);

            // Add subtle bottom glow for each category
            float glowHeight = 20.0f;
            float glowAlpha = 0.15f * animation;

            if (catPositions[i].isExtended) {
                ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(
                    ImVec2(catRect.x, catRect.w - glowHeight),
                    ImVec2(catRect.z, catRect.w + glowHeight),
                    IM_COL32(0, 0, 0, 0),
                    IM_COL32(0, 0, 0, 0),
                    IM_COL32(0, 0, 0, int(glowAlpha * 255)),
                    IM_COL32(0, 0, 0, int(glowAlpha * 255))
                );
            }

            float settingsHeight = 0;

            for (const auto& mod : modsInCategory)
            {
                std::string modLower = mod->getName();
                std::transform(modLower.begin(), modLower.end(), modLower.begin(), [](unsigned char c) { return std::tolower(c); });

                for (const auto& setting : mod->mSettings)
                {
                    switch (setting->mType)
                    {
                    case SettingType::Bool:
                        settingsHeight = MathUtils::lerp(settingsHeight, settingsHeight + modHeight, mod->cAnim);
                        break;
                    case SettingType::Enum:
                    {
                        EnumSetting* enumSetting = reinterpret_cast<EnumSetting*>(setting);
                        std::vector<std::string> enumValues = enumSetting->mValues;
                        int numValues = static_cast<int>(enumValues.size());

                        settingsHeight = MathUtils::lerp(settingsHeight, settingsHeight + modHeight, mod->cAnim);
                        if (setting->enumSlide > 0.01)
                        {
                            for (int j = 0; j < numValues; j++)
                                settingsHeight = MathUtils::lerp(settingsHeight, settingsHeight + modHeight, setting->enumSlide);
                        }
                        break;
                    }
                    case SettingType::Number:
                        settingsHeight = MathUtils::lerp(settingsHeight, settingsHeight + modHeight, mod->cAnim);
                        break;
                    case SettingType::Color:
                        settingsHeight = MathUtils::lerp(settingsHeight, settingsHeight + modHeight, mod->cAnim);
                        break;
                    case SettingType::Keybind:
                        settingsHeight = MathUtils::lerp(settingsHeight, settingsHeight + modHeight, mod->cAnim);
                        break;
                    }
                }
            }

            float catWindowHeight = catHeight + modHeight * modsInCategory.size() + settingsHeight;
            ImVec4 catWindow = ImVec4(catPositions[i].x, catPositions[i].y,
                catPositions[i].x + catWidth,
                catPositions[i].y + moduleY + catWindowHeight)
                .scaleToPoint(ImVec4(screen.x / 2, screen.y / 2, screen.x / 2, screen.y / 2), inScale);
            float absoluteY = catPositions[i].y;
            float absoluteX = catPositions[i].x;
            float colorOffset = i * 100.0f; // Increase offset to create more distinct category colors
            float totalHeight = screen.y; // Use screen height for color scaling

            // Base color for category
            ImColor rgb = ColorUtils::getThemedColor(clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal ? absoluteX : absoluteY);

            if (ImRenderUtils::isMouseOver(catWindow) && catPositions[i].isExtended)
            {
                if (scrollDirection > 0)
                {
                    catPositions[i].scrollEase += scrollDirection * catHeight;
                    if (catPositions[i].scrollEase > catWindowHeight - modHeight * 2)
                        catPositions[i].scrollEase = catWindowHeight - modHeight * 2;
                }
                else if (scrollDirection < 0)
                {
                    catPositions[i].scrollEase += scrollDirection * catHeight;
                    if (catPositions[i].scrollEase < 0)
                        catPositions[i].scrollEase = 0;
                }
                scrollDirection = 0;
            }

            if (!catPositions[i].isExtended)
            {
                catPositions[i].scrollEase = catWindowHeight - catHeight;
                catPositions[i].wasExtended = false;
            }
            else if (!catPositions[i].wasExtended)
            {
                catPositions[i].scrollEase = 0;
                catPositions[i].wasExtended = true;
            }

            catPositions[i].yOffset = MathUtils::animate(catPositions[i].scrollEase, catPositions[i].yOffset, ImRenderUtils::getDeltaTime() * 10.5);

            ImVec4 clipRect = ImVec4(catRect.x, catRect.w, catRect.z, screen.y);
            //drawList->PushClipRect(ImVec2(clipRect.x, clipRect.y), ImVec2(clipRect.z, clipRect.w), true);

            ImVec4 shadowRectTall = ImVec4( // skinny tall one
                catWindow.x + 12.f,
                catWindow.y + 6.f,
                catWindow.z - 12.f,
                catWindow.w
            );
            ImVec4 shadowRectWide = ImVec4( // fat short one
                catWindow.x,
                catWindow.y + 12.f,
                catWindow.z,
                catWindow.w - 12.f
            );

            if (clickGui->mShadow.mValue)
            {
                ImRenderUtils::fillShadowRectangle(
                    shadowRectTall,
                    ImColor(0, 0, 0, 255),
                    1000.f,
                    clickGui->mShadowThickness.mValue,
                    ImDrawFlags_RoundCornersAll,
                    25.f
                );
                ImRenderUtils::fillShadowRectangle(
                    shadowRectTall,
                    ImColor(0, 0, 0, 255),
                    1000.f,
                    clickGui->mShadowThickness.mValue,
                    ImDrawFlags_RoundCornersAll,
                    25.f
                );
                ImRenderUtils::fillShadowRectangle(
                    shadowRectWide,
                    ImColor(0, 0, 0, 255),
                    1000.f,
                    clickGui->mShadowThickness.mValue,
                    ImDrawFlags_RoundCornersAll,
                    25.f
                );
                ImRenderUtils::fillShadowRectangle(
                    shadowRectWide,
                    ImColor(0, 0, 0, 255),
                    1000.f,
                    clickGui->mShadowThickness.mValue,
                    ImDrawFlags_RoundCornersAll,
                    25.f
                );
            }

            int modIndex = 0;
            int modCount = modsInCategory.size();
            bool endMod = false;
            bool moduleToggled = false;
            for (const auto& mod : modsInCategory)
            {
                ImDrawFlags flags = clickGui->mRoundBot.mValue == true ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_RoundCornersNone;
                float radius = 0.f;
                if (modIndex == modsInCategory.size() - 1) {
                    endMod = true;
                    radius = clickGui->mRoundBot.mValue == true ? 15.f * (1.f - mod->cAnim) : 0.f;
                }

                std::string modLower = mod->getName();
                std::transform(modLower.begin(), modLower.end(), modLower.begin(), [](unsigned char c) { return std::tolower(c); });

                float currentY = absoluteY + moduleY;
                float currentX = moduleX;
                ImColor rgb = ColorUtils::getThemedColor(clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal ? currentX : currentY);

                if (mod->getCategory() == categories[i])
                {
                    ImVec4 modRect = ImVec4(catPositions[i].x,
                        catPositions[i].y + catHeight + moduleY,
                        catPositions[i].x + modWidth,
                        catPositions[i].y + catHeight + moduleY + modHeight)
                        .scaleToPoint(ImVec4(screen.x / 2, screen.y / 2, screen.x / 2, screen.y / 2), inScale);
                    modRect.y = std::floor(modRect.y);
                    modRect.x = std::floor(modRect.x);

                    float targetAnim = mod->showSettings ? 1.f : 0.f;
                    mod->cAnim = MathUtils::animate(targetAnim, mod->cAnim, ImRenderUtils::getDeltaTime() * 20.0);
                    mod->cAnim = MathUtils::clamp(mod->cAnim, 0.f, 1.f);

                    if (mod->cAnim > 0.001)
                    {
                        float settingY = modRect.w;
                        static bool wasDragging = false;
                        Setting* lastDraggedSetting = nullptr;
                        int sIndex = 0;
                        for (const auto& setting : mod->mSettings)
                        {
                            if (!setting->mIsVisible())
                            {
                                setting->sliderEase = 0;
                                setting->enumSlide = 0;
                                continue;
                            }

                            ImVec4 setRect = ImVec4(
                                modRect.x + 5,
                                settingY,
                                modRect.z - 5,
                                settingY + modHeight
                            ).scaleToPoint(ImVec4(screen.x / 2, screen.y / 2, screen.x / 2, screen.y / 2), inScale);

                            float radius = 0.f;
                            if (endMod && sIndex == mod->mSettings.size() - 1)
                                radius = 15.f;
                            else if (endMod)
                                radius = 15.f * (1.f - mod->cAnim);

                            bool endSetting = sIndex == mod->mSettings.size() - 1;
                            float setPadding = endSetting ? (-2.f * animation) : 0.f;

                            ImColor rgb = ColorUtils::getThemedColor(clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal ? currentX : currentY);
                            rgb.Value.w = animation;
                            float opacity = mod->cAnim * setting->visibilityAnim;
                            switch (setting->mType)
                            {
                            case SettingType::Bool:
                            {
                                BoolSetting* boolSetting = reinterpret_cast<BoolSetting*>(setting);
                                moduleY = MathUtils::lerp(moduleY, moduleY + modHeight, mod->cAnim);

                                ImVec4 rect = ImVec4(
                                    modRect.x, catPositions[i].y + catHeight + moduleY + setPadding, modRect.z,
                                    catPositions[i].y + catHeight + moduleY + modHeight)
                                    .scaleToPoint(
                                        ImVec4(modRect.x, screen.y / 2,
                                            modRect.z, screen.y / 2),
                                        inScale);
                                rect.y = std::floor(rect.y);
                                if (rect.y < modRect.y)
                                {
                                    rect.y = modRect.y;
                                }

                                if (rect.y > catRect.y + 0.5f)
                                {
                                    std::string setName = lowercase ? StringUtils::toLower(setting->mName) : setting->mName;
                                    ImRenderUtils::fillRectangle(rect, ImColor(30, 30, 30), animation, radius, ImGui::GetBackgroundDrawList(), clickGui->mRoundBot.mValue == true ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_RoundCornersNone);

                                    if (ImRenderUtils::isMouseOver(rect) && isEnabled && catPositions[i].isExtended)
                                    {
                                        tooltip = setting->mDescription;
                                        if (ImGui::IsMouseClicked(0) && !displayColorPicker && mod->showSettings)
                                        {
                                            boolSetting->mValue = !boolSetting->mValue;
                                        }

                                        if (ImGui::IsMouseClicked(2) && !displayColorPicker && catPositions[i].isExtended)
                                        {
                                            lastBoolSetting = boolSetting;
                                            isBoolSettingBinding = true;
                                            ClientInstance::get()->playUi("random.pop", 0.75f, 1.0f);
                                        }
                                    }

                                    setting->boolScale = MathUtils::animate(
                                        boolSetting->mValue ? 1 : 0, setting->boolScale,
                                        ImRenderUtils::getDeltaTime() * 10);

                                    float scaledWidth = rect.getWidth();
                                    float scaledHeight = rect.getHeight();

                                    ImVec2 center = ImVec2(rect.x + rect.getWidth() / 2.f, rect.y + rect.getHeight() / 2.f);
                                    ImVec4 scaledRect = ImVec4(center.x - scaledWidth / 2.f, center.y - scaledHeight / 2.f, center.x + scaledWidth / 2.f, center.y + scaledHeight / 2.f);

                                    float cmodRectCentreX = rect.x + ((rect.z - rect.x) - ImRenderUtils::getTextWidth(&setName, textSize)) / 2;
                                    float cmodRectCentreY = rect.y + ((rect.w - rect.y) - textHeight) / 2;

                                    ImVec4 smoothScaledRect = ImVec4(scaledRect.z - 19, scaledRect.y + 5, scaledRect.z - 5, scaledRect.w - 5);
                                    ImVec2 circleRect = ImVec2(smoothScaledRect.getCenter().x, smoothScaledRect.getCenter().y+2.f);

                                    ImColor targetShadowCol = ImColor(15, 15, 15);
                                    ImColor shadowCol = MathUtils::lerpImColor(targetShadowCol, rgb, setting->boolScale);

                                    if (!clickGui->miOS.mValue)
                                    {
                                        ImRenderUtils::fillShadowCircle(circleRect, 5, shadowCol, animation * mod->cAnim, 40, 0);
                                    }
                                    
                                    ImVec4 booleanRect = ImVec4(rect.z - 23.5f, cmodRectCentreY - 2.5f, rect.z - 5, cmodRectCentreY + 17.5f);
                                    booleanRect = booleanRect.scaleToPoint(ImVec4(rect.z, rect.y, rect.z, rect.w), animation);

                                    float rectXDiff = booleanRect.z - booleanRect.x;
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

                                    if (setting->boolScale > 0.01) {
                                        if (booleanRect.y < modRect.w) {
                                            booleanRect.y = modRect.w;
                                        }
                                        if (toggleAnim > 0.001f) {
                                            ImRenderUtils::fillCircle(ImVec2(booleanRect.getCenter().x, booleanRect.getCenter().y+2.f), 5.0f * setting->boolScale, rgb, animation, 12);
                                        }
                                    }

                                    if (clickGui->miOS.mValue)
                                    {
                                        if (booleanRect.y < modRect.w) {
                                            booleanRect.y = modRect.w;
                                        }

                                        ImVec4 sliderBg = ImVec4(
                                            booleanRect.z - 30,
                                            booleanRect.y + (booleanRect.w - booleanRect.y) / 2 - 7,
                                            booleanRect.z,
                                            booleanRect.y + (booleanRect.w - booleanRect.y) / 2 + 11
                                        );

                                        ImRenderUtils::fillRectangle(sliderBg, ImColor(40, 40, 40, int(255 * animation * opacity)), animation* opacity, 8.0f);

                                        float animWidth = (sliderBg.z - sliderBg.x) * toggleAnim;
                                        ImVec4 activeRect(
                                            sliderBg.x,
                                            sliderBg.y,
                                            sliderBg.x + animWidth,
                                            sliderBg.w
                                        );

                                        ImRenderUtils::fillRectangle(
                                            activeRect,
                                            ColorUtils::getThemedColor(clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal ? booleanRect.x : booleanRect.y),
                                            animation * opacity * toggleAnim,
                                            8.0f
                                        );

                                        float knobX = MathUtils::lerp(sliderBg.x + 8.f, sliderBg.z - 5.f * 2, knobPos);
                                        ImVec2 knobCenter(knobX + 1.f, (sliderBg.y + sliderBg.w) / 2);
                                        ImRenderUtils::fillShadowCircle(ImVec2(knobCenter.x, knobCenter.y), 8.f, ImColor(0, 0, 0), animation * 255, 5.f, ImDrawFlags_None);
                                        ImRenderUtils::fillCircle(knobCenter, 6.5f, ImColor(255, 255, 255), animation * opacity, 12);
                                    }

                                    ImRenderUtils::drawText(ImVec2(rect.x + 5.f, cmodRectCentreY), setName,
                                        ImColor(255, 255, 255), textSize, animation, true);
                                }
                                break;//c
                            }
                            case SettingType::Divider:
                            {
                                DividerSetting* dividerSetting = reinterpret_cast<DividerSetting*>(setting);
                                moduleY = MathUtils::lerp(moduleY, moduleY + modHeight, mod->cAnim);
                                ImVec4 rect = ImVec4(
                                    modRect.x, catPositions[i].y + catHeight + moduleY, modRect.z,
                                    catPositions[i].y + catHeight + moduleY + modHeight)
                                    .scaleToPoint(
                                        ImVec4(modRect.x, screen.y / 2,
                                            modRect.z, screen.y / 2),
                                        inScale);
                                rect.y = std::floor(rect.y);
                                if (rect.y < modRect.y)
                                {
                                    rect.y = modRect.y;
                                }
                                if (rect.y > catRect.y + 0.5f)
                                {
                                    ImRenderUtils::fillRectangle(rect, ImColor(30, 30, 30), animation, radius, ImGui::GetBackgroundDrawList(), clickGui->mRoundBot.mValue == true ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_RoundCornersNone);

                                    float textWidth = ImRenderUtils::getTextWidth(&dividerSetting->mName, textSize);
                                    float textHeight = ImRenderUtils::getTextHeight(textSize);
                                    float textX = rect.x + (rect.getWidth() - textWidth) / 2;
                                    float textY = rect.y + (modHeight - textHeight) / 2;

                                    float lineThickness = 1.5f;
                                    float lineRadius = 1.0f;
                                    float linePadding = 5.0f;
                                    float edgePadding = 8.0f;

                                    ImColor lineColor = ImColor(150, 150, 150);

                                    float lineY = textY + (textHeight / 2.0f);

                                    float totalWidth = rect.getWidth() - (edgePadding * 2);
                                    float sideLineLength = (totalWidth - textWidth - (linePadding * 2)) / 2;

                                    ImVec2 leftLineStart(rect.x + edgePadding, lineY);
                                    ImVec2 leftLineEnd(textX - linePadding, lineY);
                                    ImVec2 rightLineStart(textX + textWidth + linePadding, lineY);
                                    ImVec2 rightLineEnd(rect.z - edgePadding, lineY);

                                    ImRenderUtils::drawRoundRect(
                                        ImVec4(leftLineStart.x, lineY - lineThickness / 2, leftLineEnd.x, lineY + lineThickness / 2),
                                        ImDrawFlags_RoundCornersAll, lineRadius, lineColor, animation, lineThickness
                                    );

                                    ImRenderUtils::drawRoundRect(
                                        ImVec4(rightLineStart.x, lineY - lineThickness / 2, rightLineEnd.x, lineY + lineThickness / 2),
                                        ImDrawFlags_RoundCornersAll, lineRadius, lineColor, animation, lineThickness
                                    );

                                    ImRenderUtils::drawText(ImVec2(textX, textY), dividerSetting->mName,
                                        ImColor(150, 150, 150), textSize, animation, true);

                                    if (!dividerSetting->mDescription.empty() && ImRenderUtils::isMouseOver(rect))
                                    {
                                        tooltip = dividerSetting->mDescription;
                                    }
                                }
                                break;
                            }
                            case SettingType::Enum:
                            {
                                EnumSetting* enumSetting = reinterpret_cast<EnumSetting*>(setting);
                                std::string setName = lowercase ? StringUtils::toLower(setting->mName) : setting->mName;
                                std::vector<std::string> enumValues = enumSetting->mValues;
                                if (lowercase)
                                {
                                    for (std::string& value : enumValues)
                                    {
                                        value = StringUtils::toLower(value);
                                    }
                                }
                                int* iterator = &enumSetting->mValue;
                                int numValues = static_cast<int>(enumValues.size());

                                moduleY = MathUtils::lerp(moduleY, moduleY + modHeight, mod->cAnim);

                                ImVec4 rect = ImVec4(
                                    modRect.x, catPositions[i].y + catHeight + moduleY + setPadding, modRect.z,
                                    catPositions[i].y + catHeight + moduleY + modHeight)
                                    .scaleToPoint(
                                        ImVec4(modRect.x, screen.y / 2,
                                            modRect.z, screen.y / 2),
                                        inScale);
                                rect.y = std::floor(rect.y);
                                if (rect.y < modRect.y)
                                {
                                    rect.y = modRect.y;
                                }

                                float targetAnim = setting->enumExtended && mod->showSettings ? 1.f : 0.f;
                                setting->enumSlide = MathUtils::animate(
                                    targetAnim, setting->enumSlide, ImRenderUtils::getDeltaTime() * 10);
                                setting->enumSlide = MathUtils::clamp(setting->enumSlide, 0.f, 1.f);

                                if (setting->enumSlide > 0.001)
                                {
                                    for (int j = 0; j < numValues; j++)
                                    {
                                        std::string enumValue = enumValues[j];

                                        moduleY = MathUtils::lerp(moduleY, moduleY + modHeight, setting->enumSlide);

                                        ImVec4 rect2 = ImVec4(
                                            modRect.x, catPositions[i].y + catHeight + moduleY + setPadding, modRect.z,
                                            catPositions[i].y + catHeight + moduleY + modHeight)
                                            .scaleToPoint(
                                                ImVec4(modRect.x, screen.y / 2,
                                                    modRect.z, screen.y / 2),
                                                inScale);

                                        if (rect2.y > catRect.y + 0.5f)
                                        {
                                            float cmodRectCentreY = rect2.y + ((rect2.w - rect2.y) - textHeight)
                                                / 2;

                                            ImRenderUtils::fillRectangle(rect2, ImColor(20, 20, 20), animation, radius, ImGui::GetBackgroundDrawList(), clickGui->mRoundBot.mValue == true ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_RoundCornersNone);

                                            if (*iterator == j)
                                                ImRenderUtils::fillRectangle(
                                                    ImVec4(rect2.x, rect2.y, rect2.x + 1.5f, rect2.w),
                                                    rgb, animation);

                                            if (ImRenderUtils::isMouseOver(rect2) && ImGui::IsMouseClicked(0) &&
                                                isEnabled && !displayColorPicker && mod->showSettings)
                                            {
                                                *iterator = j;
                                            }

                                            ImRenderUtils::drawText(
                                                ImVec2(rect2.x + 5.f, cmodRectCentreY), enumValue,
                                                ImColor(255, 255, 255), textSize, animation, true);
                                        }
                                    }
                                }

                                if (rect.y > catRect.y + 0.5f)
                                {
                                    ImRenderUtils::fillRectangle(rect, ImColor(30, 30, 30), animation, radius, ImGui::GetBackgroundDrawList(), clickGui->mRoundBot.mValue == true ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_RoundCornersNone);

                                    if (ImRenderUtils::isMouseOver(rect) && isEnabled && catPositions[i].isExtended)
                                    {
                                        tooltip = setting->mDescription;
                                        if (ImGui::IsMouseClicked(0) && !displayColorPicker && mod->showSettings)
                                        {
                                            *iterator = (*iterator + 1) % enumValues.size();
                                        }
                                        else if (ImGui::IsMouseClicked(1) && mod->showSettings && !displayColorPicker && mod->showSettings)
                                        {
                                            setting->enumExtended = !setting->enumExtended;
                                        }
                                    }

                                    float cmodRectCentreY = rect.y + ((rect.w - rect.y) - textHeight) / 2;

                                    std::string enumValue = enumValues[*iterator];
                                    std::string settingName = setName;
                                    std::string settingString = enumValue;
                                    auto ValueLen = ImRenderUtils::getTextWidth(&settingString, textSize);

                                    ImRenderUtils::drawText(ImVec2(rect.x + 5.f, cmodRectCentreY),
                                        settingName, ImColor(255, 255, 255), textSize,
                                        animation, true);
                                    ImRenderUtils::drawText(
                                        ImVec2((rect.z - 5.f) - ValueLen, cmodRectCentreY),
                                        settingString, ImColor(170, 170, 170), textSize, animation, true);
                                }
                                if (rect.y > catRect.y - modHeight)
                                {
                                    ImRenderUtils::fillGradientOpaqueRectangle(
                                        ImVec4(rect.x, rect.w, rect.z,
                                            rect.w + 10.f * setting->enumSlide * animation),
                                        ImColor(0, 0, 0), ImColor(0, 0, 0), 0.F * animation, 0.55F * animation);
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

                                char str[32];
                                float roundedValue;

                                if (!isEditing[numSetting]) {
                                    roundedValue = std::round(value * 100.0f) / 100.0f;
                                    if (std::floor(roundedValue) == roundedValue) {
                                        sprintf_s(str, "%d", (int)roundedValue);
                                    }
                                    else {
                                        sprintf_s(str, 10, "%.2f", value);
                                        std::string rVal = str;
                                        while (rVal.back() == '0') rVal.pop_back();
                                        if (rVal.back() == '.') rVal += "0";
                                        strcpy_s(str, rVal.c_str());
                                    }
                                }
                                else {
                                    strcpy_s(str, editBuffer[numSetting].c_str());
                                }

                                std::string valueName = str;// rVal;

                                moduleY = MathUtils::lerp(moduleY, moduleY + modHeight, mod->cAnim);

                                ImVec4 backGroundRect = ImVec4(
                                    modRect.x, (catPositions[i].y + catHeight + moduleY), modRect.z,
                                    catPositions[i].y + catHeight + moduleY + modHeight)
                                    .scaleToPoint(
                                        ImVec4(modRect.x, screen.y / 2,
                                            modRect.z, screen.y / 2),
                                        inScale);

                                backGroundRect.y = std::floor(backGroundRect.y);
                                if (backGroundRect.y < modRect.y)
                                {
                                    backGroundRect.y = modRect.y;
                                }

                                ImVec4 rect = ImVec4(
                                    modRect.x + 7, (catPositions[i].y + catHeight + moduleY + setPadding), modRect.z - 7,
                                    catPositions[i].y + catHeight + moduleY + modHeight)
                                    .scaleToPoint(
                                        ImVec4(modRect.x, screen.y / 2,
                                            modRect.z, screen.y / 2),
                                        inScale);
                                rect.y = std::floor(rect.y);
                                if (rect.y < modRect.y)
                                {
                                    rect.y = modRect.y;
                                }

                                static float clickAnimation = 1.f;

                                float textY = backGroundRect.y - 1.0f;
                                float valueWidth = ImRenderUtils::getTextWidth(&valueName, inScale);

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

                                // If left click is down, lerp the alpha to 0.60f;
                                if (ImGui::IsMouseDown(0) && ImRenderUtils::isMouseOver(rect))
                                {
                                    clickAnimation = MathUtils::animate(0.60f, clickAnimation, ImRenderUtils::getDeltaTime() * 10);
                                }
                                else
                                {
                                    clickAnimation = MathUtils::animate(1.f, clickAnimation, ImRenderUtils::getDeltaTime() * 10);
                                }

                                if (backGroundRect.y > catRect.y + 0.5f)
                                {
                                    ImRenderUtils::fillRectangle(backGroundRect, ImColor(30, 30, 30), animation, radius, ImGui::GetBackgroundDrawList(), clickGui->mRoundBot.mValue == true ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_RoundCornersNone);

                                    const float sliderPos = (value - min) / (max - min) * (rect.z - rect.x);

                                    setting->sliderEase = MathUtils::animate(
                                        sliderPos, setting->sliderEase, ImRenderUtils::getDeltaTime() * 15);
                                    setting->sliderEase = std::clamp(setting->sliderEase, 0.f, rect.getWidth());
                                    
#pragma region Slider dragging
                                    if (ImRenderUtils::isMouseOver(rect) && isEnabled && catPositions[i].isExtended)
                                    {
                                        tooltip = setting->mDescription;
                                        if (ImGui::IsMouseDown(0) || ImGui::IsMouseDown(2))
                                        {
                                            setting->isDragging = true;
                                            lastDraggedSetting = setting;
                                        }
                                    }

                                    if (isEditing[numSetting]) {
                                        if (isSelected[numSetting]) {
                                            selectionAnim[numSetting] = MathUtils::lerp(selectionAnim[numSetting], 1.0f, ImGui::GetIO().DeltaTime * 8.0f);
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

                                        float maxCursorX = setRect.z - 2.5f - valueWidth + ImRenderUtils::getTextWidth(&valueName, inScale) + 3.f;

                                        float baseCursorX = setRect.z - 2.5f - valueWidth + ImRenderUtils::getTextWidth(&valueName, inScale) + 3.f;

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
                                    if (ImGui::IsMouseDown(0) && setting->isDragging && isEnabled)
                                    {
                                        if (lastDraggedSetting != setting)
                                        {
                                            setting->isDragging = false;
                                        }
                                        else
                                        {
                                            const float newValue = std::fmax(
                                                std::fmin(
                                                    (ImRenderUtils::getMousePos().x - rect.x) / (rect.z - rect.x) * (
                                                        max - min) + min, max), min);
                                            numSetting->setValue(newValue);
                                        }
                                    }
                                    else if (ImGui::IsMouseDown(2) && setting->isDragging && isEnabled)
                                    {
                                        if (lastDraggedSetting != setting)
                                        {
                                            setting->isDragging = false;
                                        }
                                        else
                                        {
                                            float newValue = std::fmax(
                                                std::fmin(
                                                    (ImRenderUtils::getMousePos().x - rect.x) / (rect.z - rect.x) * (
                                                        max - min) + min, max), min);
                                            // Round the value to the nearest value specified by midclickRounding
                                            newValue = std::round(newValue / midclickRounding) * midclickRounding;
                                            numSetting->mValue = newValue;
                                        }
                                    }
                                    else
                                    {
                                        setting->isDragging = false;
                                    }

                                    if (ImRenderUtils::isMouseOver(rect) && clickGui->mEnabled && !blockInputsUntilRelease) {
                                        tooltip = setting->mDescription;
                                        if ((ImGui::IsMouseDown(0) || ImGui::IsMouseDown(2)) && setting->visibilityAnim > 0.9f) {
                                            setting->isDragging = true;
                                            lastDraggedSetting = setting;
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

                                    if (!ImGui::IsMouseDown(0) && !ImGui::IsMouseDown(1) && !ImGui::IsMouseDown(2)) {
                                        blockInputsUntilRelease = false;
                                    }
#pragma endregion

                                    // "doesn't animate down when animating out" cuz its made by a 11 yr old smart nerd :yum:
                                    /* Original code (doesn't animate down when animating out)
                                    ImRenderUtils::fillRectangle(ImVec4(rect.x, (catPositions[i].y + catHeight + moduleY + modHeight) - 3, rect.x + setting->sliderEase, rect.w), rgb, animation);
                                    ImRenderUtils::fillShadowRectangle(ImVec4(rect.x, (catPositions[i].y + catHeight + moduleY + modHeight) - 3, rect.x + setting->sliderEase, rect.w), rgb, animation, 50.f, 0);*/

                                    float ySize = rect.w - rect.y;

                                    ImVec2 sliderBarMin = ImVec2(rect.x, rect.w - ySize / 8);
                                    ImVec2 sliderBarMax = ImVec2(rect.x + (setting->sliderEase * inScale), rect.w);
                                    sliderBarMin.y = sliderBarMax.y - 4 * inScale;

                                    ImVec4 sliderRect = ImVec4(sliderBarMin.x, sliderBarMin.y - 4.f, sliderBarMax.x, sliderBarMax.y - 4.f);

                                    // The slider bar
                                    ImRenderUtils::fillRectangle(sliderRect, rgb, animation, 15);

                                    // Circle (I am not sure)
                                    ImVec2 circlePos = ImVec2(sliderRect.z - 2.25f, sliderRect.getCenter().y);

                                    if (value <= min + 0.83f)
                                    {
                                        circlePos.x = sliderRect.z + 2.25f;
                                    }
                                    //ImRenderUtils::fillCircle(circlePos, 5.5f * clickAnimation * animation, rgb, animation, 12);

                                    // Push a clip rect to prevent the shadow from going outside the slider bar
                                    ImGui::GetBackgroundDrawList()->PushClipRect(ImVec2(sliderRect.x, sliderRect.y), ImVec2(sliderRect.z, sliderRect.w), true);

                                    ImRenderUtils::fillShadowRectangle(sliderRect, rgb, animation * 0.75f, 15.f, 0);

                                    ImGui::GetBackgroundDrawList()->PopClipRect();

                                    auto ValueLen = ImRenderUtils::getTextWidth(&valueName, textSize);
                                    ImRenderUtils::drawText(
                                        ImVec2((backGroundRect.z - 5.f) - ValueLen, backGroundRect.y + 2.5f), valueName,
                                        ImColor(170, 170, 170), textSize, animation, true);
                                    ImRenderUtils::drawText(ImVec2(backGroundRect.x + 5.f, backGroundRect.y + 2.5f),
                                        setName, ImColor(255, 255, 255), textSize,
                                        animation, true);
                                }
                                break;
                            }
                            case SettingType::Color:
                            {
                                ColorSetting* colorSetting = reinterpret_cast<ColorSetting*>(setting);
                                ImColor color = colorSetting->getAsImColor();
                                ImVec4 rgb = color.Value;
                                std::string setName = lowercase ? StringUtils::toLower(setting->mName) : setting->mName;

                                moduleY = MathUtils::lerp(moduleY, moduleY + modHeight, mod->cAnim);

                                ImVec4 rect = ImVec4(
                                    modRect.x, catPositions[i].y + catHeight + moduleY + setPadding, modRect.z,
                                    catPositions[i].y + catHeight + moduleY + modHeight)
                                    .scaleToPoint(
                                        ImVec4(modRect.x, screen.y / 2,
                                            modRect.z, screen.y / 2),
                                        inScale);
                                rect.y = std::floor(rect.y);
                                if (rect.y < modRect.y)
                                {
                                    rect.y = modRect.y;
                                }

                                if (rect.y > catRect.y + 0.5f)
                                {
                                    ImRenderUtils::fillRectangle(rect, ImColor(30, 30, 30), animation);

                                    if (ImRenderUtils::isMouseOver(rect) && isEnabled && catPositions[i].isExtended)
                                    {
                                        tooltip = setting->mDescription;
                                        if (ImGui::IsMouseClicked(0) && !displayColorPicker && mod->showSettings)
                                        {
                                            displayColorPicker = !displayColorPicker;
                                            lastColorSetting = colorSetting;
                                        }
                                    }

                                    float cmodRectCentreY = rect.y + ((rect.w - rect.y) - textHeight) / 2;
                                    ImRenderUtils::drawText(ImVec2(rect.x + 5.f, cmodRectCentreY), setName,
                                        ImColor(255, 255, 255), textSize, animation, true);

                                    ImVec2 colorRect = ImVec2(rect.z - 20, rect.y + 5);
                                    ImRenderUtils::fillRectangle(ImVec4(rect.z - 20, rect.y + 5, rect.z - 5, rect.w - 5),
                                        colorSetting->getAsImColor(), animation);
                                }
                                break;
                            }
                            case SettingType::Keybind:
                            {
                                KeybindSetting* keybind = reinterpret_cast<KeybindSetting*>(setting);
                                int displayKey = keybind->mKey != 0 ? keybind->mKey : mod->mKey;
                                std::string keyName = displayKey == 0 ? "NONE" : Keyboard::getKey(displayKey);
                                std::string label = "Bind:";
                                if (ModernGui::isKeybindBinding && ModernGui::lastKeybindSetting == keybind) {
                                    label += " Listening...";
                                }
                                ImColor boxColor = (ModernGui::isKeybindBinding && ModernGui::lastKeybindSetting == keybind) ? ImColor(80, 120, 255) : ImColor(40, 40, 40);
                                
                                moduleY = MathUtils::lerp(moduleY, moduleY + modHeight, mod->cAnim);

                                ImVec4 rect = ImVec4(
                                    modRect.x, catPositions[i].y + catHeight + moduleY + setPadding, modRect.z,
                                    catPositions[i].y + catHeight + moduleY + modHeight)
                                    .scaleToPoint(
                                        ImVec4(modRect.x, screen.y / 2,
                                            modRect.z, screen.y / 2),
                                        inScale);
                                rect.y = std::floor(rect.y);
                                if (rect.y < modRect.y)
                                {
                                    rect.y = modRect.y;
                                }

                                if (rect.y > catRect.y + 0.5f)
                                {
                                    ImRenderUtils::fillRectangle(rect, ImColor(30, 30, 30), animation, radius, ImGui::GetBackgroundDrawList(), clickGui->mRoundBot.mValue == true ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_RoundCornersNone);

                                    float setTextY = rect.y + ((rect.w - rect.y) - textHeight) / 2;
                                    ImRenderUtils::drawText(ImVec2(rect.x + 5.f, setTextY), label, ImColor(255, 255, 255), textSize, animation, true);
                                    
                                    float valueWidth = ImRenderUtils::getTextWidth(&keyName, textSize);
                                    ImVec4 keybindRect = ImVec4(rect.z - valueWidth - 15.f, rect.y + 2.f, rect.z - 5.f, rect.w - 2.f);
                                    ImRenderUtils::fillRectangle(keybindRect, boxColor, animation, 4.0f, ImGui::GetBackgroundDrawList(), ImDrawFlags_RoundCornersAll);
                                    ImRenderUtils::drawText(ImVec2(rect.z - valueWidth - 8.f, setTextY), keyName, ImColor(255, 255, 255), textSize, animation, true);
                                    
                                    if (ImRenderUtils::isMouseOver(rect) && isEnabled && catPositions[i].isExtended) {
                                        tooltip = setting->mDescription;
                                        if (ImGui::IsMouseClicked(0) && !displayColorPicker && mod->showSettings) {
                                            ModernGui::isKeybindBinding = true;
                                            ModernGui::lastKeybindSetting = keybind;
                                        }
                                    }
                                    
                                    if (ModernGui::isKeybindBinding && ModernGui::lastKeybindSetting == keybind) {
                                        for (const auto& key : Keyboard::mPressedKeys) {
                                            if (key.second) {
                                                keybind->mKey = (key.first == VK_ESCAPE ? 0 : key.first);
                                                ModernGui::isKeybindBinding = false;
                                                ModernGui::lastKeybindSetting = nullptr;
                                                ModernGui::justUnboundKeybind = true;
                                                ClientInstance::get()->playUi((key.first == VK_ESCAPE) ? "random.break" : "random.orb", 0.75f, 1.0f);
                                                break;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            }

                            sIndex++;
                        }
                    }

                    if (modRect.y > catRect.y + 0.5f)
                    {
                        // Draw the rect
                        if (mod->cScale <= 1)
                        {
                            if (mod->mEnabled)
                                ImRenderUtils::fillRectangle(modRect, rgb, animation, radius, ImGui::GetBackgroundDrawList(), ImDrawCornerFlags_BotRight | ImDrawCornerFlags_BotLeft);
                            else
                                ImRenderUtils::fillRectangle(modRect, ImColor(30, 30, 30), animation, radius, ImGui::GetBackgroundDrawList(), ImDrawCornerFlags_BotRight | ImDrawCornerFlags_BotLeft);
                            ImRenderUtils::fillRectangle(modRect, grayColor, animation, radius, ImGui::GetBackgroundDrawList(), ImDrawCornerFlags_BotRight | ImDrawCornerFlags_BotLeft);
                        }

                        std::string modName = mod->getName();

                        ImVec2 center = ImVec2(modRect.x + modRect.getWidth() / 2.f,
                            modRect.y + modRect.getHeight() / 2.f);

                        mod->cScale = MathUtils::animate(mod->mEnabled ? 1 : 0, mod->cScale,
                            ImRenderUtils::getDeltaTime() * 10);

                        float scaledWidth = modRect.getWidth();
                        float scaledHeight = modRect.getHeight();

                        ImVec4 scaledRect = ImVec4(center.x - scaledWidth / 2.f,
                            center.y - scaledHeight / 2.f,
                            center.x + scaledWidth / 2.f,
                            center.y + scaledHeight / 2.f);

                        if (mod->cScale > 0)
                        {
                            ImColor rgb1 = ColorUtils::getThemedColor(clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal ? modRect.x : modRect.y);
                            ImColor rgb2 = ColorUtils::getThemedColor(clickGui->mColorDirection.mValue == ClickGui::ClickGuiColorDirection::Horizontal ? modRect.z : modRect.w);
                            ImRenderUtils::fillRoundedGradientRectangle(scaledRect, rgb1, rgb2, radius, animation * mod->cScale, animation * mod->cScale, flags);
                        }

                        float cRectCentreX = modRect.x + ((modRect.z - modRect.x) - ImRenderUtils::getTextWidth(
                            &modName, textSize)) / 2;
                        float cRectCentreY = modRect.y + ((modRect.w - modRect.y) - textHeight) / 2;

                        ImVec2 modPosLerped = ImVec2(cRectCentreX, cRectCentreY);

                        ImRenderUtils::drawText(ImVec2(cRectCentreX, cRectCentreY), modName, ImColor(mod->mEnabled ? ImColor(255, 255, 255) : ImColor(180, 180, 180)).Lerp(mod->mEnabled ? ImColor(255, 255, 255) : ImColor(180, 180, 180), mod->cAnim), textSize, animation, true);

                        mod->symbolAnim = MathUtils::animate(mod->showSettings ? 1.0f : 0.0f, mod->symbolAnim, ImRenderUtils::getDeltaTime() * 15.0f);

                        float symbolSize = 8.0f * textSize;
                        float thickness = 1.5f * textSize;

                        float exactX = modRect.z - 15.0f;
                        float exactY = (modRect.y + modRect.w) / 2.0f;
                        ImVec2 symbolCenter(exactX, exactY);

                        auto* drawList = ImGui::GetBackgroundDrawList();
                        ImColor symbolColor = mod->mEnabled ? ImColor(1.0f, 1.0f, 1.0f, 1.0f) : ImColor(0.7f, 0.7f, 0.7f, 1.0f);

                        float rotationAngle = mod->symbolAnim * 3.14159f * 2.0f;

                        auto rotatePoint = [](const ImVec2& point, const ImVec2& center, float angle) -> ImVec2 {
                            float s = sin(angle);
                            float c = cos(angle);
                            ImVec2 translated = ImVec2(point.x - center.x, point.y - center.y);
                            return ImVec2(
                                center.x + (translated.x * c - translated.y * s),
                                center.y + (translated.x * s + translated.y * c)
                            );
                            };

                        float halfSize = symbolSize * 0.5f;

                        ImVec2 hStart = ImVec2(symbolCenter.x - halfSize, symbolCenter.y);
                        ImVec2 hEnd = ImVec2(symbolCenter.x + halfSize, symbolCenter.y);

                        float verticalScale = cos(mod->symbolAnim * 3.14159f);
                        verticalScale = verticalScale > 0 ? verticalScale : 0;
                        float halfHeight = halfSize * verticalScale;

                        ImVec2 vStart = ImVec2(symbolCenter.x, symbolCenter.y - halfHeight);
                        ImVec2 vEnd = ImVec2(symbolCenter.x, symbolCenter.y + halfHeight);

                        ImVec2 rotatedHStart = rotatePoint(hStart, symbolCenter, rotationAngle);
                        ImVec2 rotatedHEnd = rotatePoint(hEnd, symbolCenter, rotationAngle);
                        ImVec2 rotatedVStart = rotatePoint(vStart, symbolCenter, rotationAngle);
                        ImVec2 rotatedVEnd = rotatePoint(vEnd, symbolCenter, rotationAngle);

                        if (clickGui->mDropType.mValue == ClickGui::DropdownVisual::Plus)
                        {
                            drawList->AddLine(rotatedHStart, rotatedHEnd, symbolColor, thickness);
                            if (verticalScale > 0.01f)
                            {
                                drawList->AddLine(rotatedVStart, rotatedVEnd, ImColor(symbolColor.Value.x, symbolColor.Value.y, symbolColor.Value.z, verticalScale), thickness);
                            }
                        }
                        else if (clickGui->mDropType.mValue == ClickGui::DropdownVisual::Dot)
                        {
                            if (mod->showSettings)
                                mod->mEnabled ? drawList->AddCircle(symbolCenter, 4.f, symbolColor, 0, 2.f) : drawList->AddCircle(symbolCenter, 3.f, symbolColor, 0, 2.f);
                            else
                                mod->mEnabled ? drawList->AddCircleFilled(symbolCenter, 5.f, symbolColor, 0) : drawList->AddCircleFilled(symbolCenter, 2.5f, symbolColor, 0);
                        }
                        float renderx = screen.x / 2;
                        float rendery = (screen.y / 2) + 110;

                        if (ImRenderUtils::isMouseOver(modRect) && catPositions[i].isExtended && isEnabled && catPositions[i].isExtended)
                        {
                            if (ImRenderUtils::isMouseOver(catWindow) && catPositions[i].isExtended && catPositions[i].isExtended)
                            {
                                tooltip = mod->mDescription;
                            }

                            if (ImGui::IsMouseClicked(0) && !displayColorPicker && catPositions[i].isExtended)
                            {
                                if (!moduleToggled) mod->toggle();
                                //ClientInstance::get()->playUi("random.pop", 0.75f, 1.0f);
                                moduleToggled = true;
                            }
                            else if (ImGui::IsMouseClicked(1) && !displayColorPicker && catPositions[i].isExtended)
                            {
                                if (!mod->mSettings.empty()) mod->showSettings = !mod->showSettings;
                            }
                            else if (ImGui::IsMouseClicked(2) && !displayColorPicker && catPositions[i].isExtended)
                            {
                                lastMod = mod;
                                isBinding = true;
                                //ClientInstance::get()->playUi("random.pop", 0.75f, 1.0f);
                            }
                        }
                    }
                    if (modRect.y > catRect.y - modHeight)
                    {

                        ImRenderUtils::fillGradientOpaqueRectangle(
                            ImVec4(modRect.x, modRect.w, modRect.z,
                                modRect.w + 10.f * mod->cAnim * animation), ImColor(0, 0, 0),
                            ImColor(0, 0, 0), 0.F * animation, 0.55F * animation);
                    }
                    moduleY += modHeight;

                    modIndex++;
                }
            }
            //drawList->PopClipRect();

            if (isBinding)
            {
                tooltip = "Currently binding " + lastMod->getName() + "..." + " Press ESC to unbind.";
                for (const auto& key : Keyboard::mPressedKeys)
                {
                    if (key.second && lastMod)
                    {
                        //lastMod->setKeybind(key.first == Keys::ESC ? 7 : key.first);
                        lastMod->mKey = key.first == VK_ESCAPE ? 0 : key.first;
                        isBinding = false;
                        if (key.first == VK_ESCAPE)
                        {
                            ClientInstance::get()->playUi("random.break", 0.75f, 1.0f);
                        }
                        else
                        {
                            ClientInstance::get()->playUi("random.orb", 0.75f, 1.0f);
                        }
                    }
                }
            }

            if (isBoolSettingBinding)
            {
                tooltip = "Currently binding " + lastBoolSetting->mName + "... Press ESC to unbind.";
                for (const auto& key : Keyboard::mPressedKeys)
                {
                    if (key.second && lastBoolSetting)
                    {
                        lastBoolSetting->mKey = (key.first == VK_ESCAPE) ? 0 : key.first;
                        isBoolSettingBinding = false;

                        if (key.first == VK_ESCAPE)
                        {
                            ClientInstance::get()->playUi("random.break", 0.75f, 1.0f);
                        }
                        else
                        {
                            ClientInstance::get()->playUi("random.orb", 0.75f, 1.0f);
                        }
                    }
                }
            }

            std::string catName = lowercase ? StringUtils::toLower(categories[i]) : categories[i];

            if (ImRenderUtils::isMouseOver(catRect) && ImGui::IsMouseClicked(1))
                catPositions[i].isExtended = !catPositions[i].isExtended;

            catRect.w += 1.5f;
            ImRenderUtils::fillRectangle(catRect, darkBlack, animation, 15, ImGui::GetBackgroundDrawList(), clickGui->mRoundTop.mValue == true ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersNone);

            ImVec4 lineRect = ImVec4(catRect.x, catRect.w - 0.75f, catRect.z, catRect.w + 0.75f);


            FontHelper::pushPrefFont(true, interfaceMod->mBold.mValue, interfaceMod->mSansFont.mValue);

            float textHeight = ImGui::GetFont()->CalcTextSizeA(textSize * 18, FLT_MAX, -1, catName.c_str()).y;
            float cRectCentreX = catRect.x + ((catRect.z - catRect.x) - ImRenderUtils::getTextWidth(
                &catName, textSize * 1.15)) / 2;
            float cRectCentreY = catRect.y + ((catRect.w - catRect.y) - textHeight) / 2;

            std::string IconStr = "B";
            // TODO: please for the love of god make icon fkery like this into FontHelper.......
            // (also don't forget to check for case u idiot!!!!!!!111!!!!1)
            if (StringUtils::equalsIgnoreCase(catName, "Combat")) IconStr = "c";
            else if (StringUtils::equalsIgnoreCase(catName, "Movement")) IconStr = "f";
            else if (StringUtils::equalsIgnoreCase(catName, "Visual")) IconStr = "d";
            else if (StringUtils::equalsIgnoreCase(catName, "Player")) IconStr = "e";
            else if (StringUtils::equalsIgnoreCase(catName, "Misc")) IconStr = "a";


            ImGui::PushFont(FontHelper::Fonts["tenacity_icons_large"]);
            // Draw the icon
            ImRenderUtils::drawText(ImVec2(catRect.x + 10, cRectCentreY), IconStr, ImColor(255, 255, 255),
                textSize * 1.15, animation, true);
            ImGui::PopFont();

            // Draw the string
            ImRenderUtils::drawText(ImVec2(cRectCentreX, cRectCentreY), catName, ImColor(255, 255, 255),
                textSize * 1.15, animation, true);
            ImGui::PopFont();

            catPositions[i].x = std::clamp(catPositions[i].x, 0.f, screen.x - catWidth);
            catPositions[i].y = std::clamp(catPositions[i].y, 0.f, screen.y - catHeight);

#pragma region DraggingLogic
            static bool dragging = false;
            static ImVec2 dragOffset;
            if (catPositions[i].isDragging)
            {
                if (ImGui::IsMouseDown(0))
                {
                    if (!dragging)
                    {
                        dragOffset = ImVec2(ImRenderUtils::getMousePos().x - catRect.x,
                            ImRenderUtils::getMousePos().y - catRect.y);
                        dragging = true;
                    }
                    ImVec2 newPosition = ImVec2(ImRenderUtils::getMousePos().x - dragOffset.x,
                        ImRenderUtils::getMousePos().y - dragOffset.y);
                    newPosition.x = std::clamp(newPosition.x, 0.f,
                        screen.x - catWidth);
                    newPosition.y = std::clamp(newPosition.y, 0.f,
                        screen.y - catHeight);
                    // Round the position to an even number
                    newPosition.x = std::round(newPosition.x / 2) * 2;
                    newPosition.y = std::round(newPosition.y / 2) * 2;

                    catPositions[i].x = newPosition.x;
                    catPositions[i].y = newPosition.y;
                }
                else
                {
                    catPositions[i].isDragging = false;
                    dragging = false;
                }
            }
            else if (ImRenderUtils::isMouseOver(catRect) && ImGui::IsMouseClicked(0) && isEnabled)
            {
                catPositions[i].isDragging = true;
                dragOffset = ImVec2(ImRenderUtils::getMousePos().x - catRect.x,
                    ImRenderUtils::getMousePos().y - catRect.y);
            }
#pragma endregion
        }

        if (!tooltip.empty())
        {
            ImVec2 toolTipHeight = ImGui::GetFont()->CalcTextSizeA(textSize * 14.4f, FLT_MAX, 0, tooltip.c_str());
            float textWidth = ImRenderUtils::getTextWidth(&tooltip, textSize * 0.8f);
            float textHeight = toolTipHeight.y;
            float padding = 2.5f;
            float offset = 18.f;

            ImVec4 tooltipRect = ImVec4(
                ImRenderUtils::getMousePos().x + offset - padding,
                ImRenderUtils::getMousePos().y + textHeight / 2 - textHeight - padding,
                ImRenderUtils::getMousePos().x + offset + textWidth + padding * 2,
                ImRenderUtils::getMousePos().y + textHeight / 2 + padding
            ).scaleToPoint(ImVec4(
                screen.x / 2,
                screen.y / 2,
                screen.x / 2,
                screen.y / 2
            ), inScale);

            static float alpha = 1.f;

            // If mid or left click is down, lerp the alpha to 0.25f;
            if (ImGui::IsMouseDown(0) || ImGui::IsMouseDown(2))
            {
                alpha = MathUtils::animate(0.0f, alpha, ImRenderUtils::getDeltaTime() * 10);
            }
            else
            {
                alpha = MathUtils::animate(1.f, alpha, ImRenderUtils::getDeltaTime() * 10);
            }

            tooltipRect = tooltipRect.scaleToCenter(alpha);

            ImRenderUtils::fillRectangle(tooltipRect, ImColor(20, 20, 20), animation * alpha, 0.f, ImGui::GetForegroundDrawList());
            ImRenderUtils::drawText(ImVec2(tooltipRect.x + padding, tooltipRect.y + padding), tooltip,
                ImColor(255, 255, 255), (textSize * 0.8f) * alpha, animation * alpha, true, 0, ImGui::GetForegroundDrawList());
        }

        if (isEnabled)
        {
            scrollDirection = 0;
        }
    }
    ImGui::PopFont();
}

void ModernGui::onWindowResizeEvent(WindowResizeEvent& event)
{
    resetPosition = true;
    lastReset = NOW;
}
