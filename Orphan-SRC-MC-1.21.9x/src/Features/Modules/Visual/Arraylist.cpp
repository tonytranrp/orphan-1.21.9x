//
// Created by vastrakai on 7/2/2024.
//

#include "Arraylist.hpp"

#include <Features/FeatureManager.hpp>
#include <Features/Modules/Visual/Interface.hpp>
#include "Features/Modules/ModuleCategory.hpp"

void Arraylist::onEnable()
{
    gFeatureManager->mDispatcher->listen<RenderEvent, &Arraylist::onRenderEvent>(this);
}

void Arraylist::onDisable()
{
    gFeatureManager->mDispatcher->deafen<RenderEvent, &Arraylist::onRenderEvent>(this);

    for (auto& mod : gFeatureManager->mModuleManager->getModules())
    {
        mod->mArrayListAnim = 0.f;
    }
}

void drawShadowTextW(ImDrawList* drawList, const std::string& text, ImVec2 pos, ImColor color, float size, bool shadow = true)
{
    ImVec2 shadowPos = pos;
    shadowPos.x += 1.f;
    shadowPos.y += 1.f;
    ImVec2 textPos = pos;

    for (int i = 0; i < text.length(); i++)
    {
        char c = text[i];
        if (shadow) drawList->AddText(ImGui::GetFont(), size, shadowPos, ImColor(color.Value.x * 0.03f, color.Value.y * 0.03f, color.Value.z * 0.03f, 0.9f), &c, &c + 1);
        drawList->AddText(ImGui::GetFont(), size, textPos, color, &c, &c + 1);
        textPos.x += ImGui::GetFont()->CalcTextSizeA(size, FLT_MAX, 0, &c, &c + 1).x;
        shadowPos.x += ImGui::GetFont()->CalcTextSizeA(size, FLT_MAX, 0, &c, &c + 1).x;
    }
}

struct TempRenderInfo
{
    std::string moduleName;
    ImVec2 start;
    ImVec2 end;
    ImColor color;
    Module* mod;
};

struct TempLineRenderInfo
{
    enum class Type
    {
        Top,
        Bottom,
        Left,
        Right
    };

    std::string moduleName;
    ImVec2 start;
    ImVec2 end;
    ImColor color;
    Module* mod;
};



void Arraylist::onRenderEvent(RenderEvent& event)
{
    bool glow = mGlow.mValue;
    bool displayGlow = mDisplayGlow.mValue;
    float glowStrength = mGlowStrength.mValue * 100.f;
    float colorSpeed = mColorSpeed.mValue * (mHorizontalColor.mValue ? 0.3f : 0.5f);
    float saturation = mSaturation.mValue;

    auto drawList = ImGui::GetForegroundDrawList();

    static auto daInterface = gFeatureManager->mModuleManager->getModule<Interface>();
    if (!daInterface) return;

    auto fontSel = daInterface->mFont.as<Interface::FontType>();
    if (fontSel == Interface::FontType::ProductSans) {
        FontHelper::pushPrefFont(true, true);
    }
    else {
        FontHelper::pushPrefFont(true, mBoldText.mValue, mSansFont.mValue);
    }

    static std::vector<std::shared_ptr<Module>> module;
    if (module.empty()) {
        module = gFeatureManager->mModuleManager->getModules();
    }

    float fontSize = mFontSize.mValue;
    ImVec2 displayRes = ImGui::GetIO().DisplaySize;
    ImVec2 pos = ImVec2(displayRes.x - 10, 5);

    std::ranges::sort(module, [this](const std::shared_ptr<Module>& a, const std::shared_ptr<Module>& b)
        {
            std::string aName = a->getName();
            std::string bName = b->getName();
            if (!a->getSettingDisplayText().empty() && mRenderMode.mValue) aName += " " + a->getSettingDisplayText();
            if (!b->getSettingDisplayText().empty() && mRenderMode.mValue) bName += " " + b->getSettingDisplayText();
            return ImGui::GetFont()->CalcTextSizeA(mFontSize.mValue, FLT_MAX, 0, aName.c_str()).x >
                ImGui::GetFont()->CalcTextSizeA(mFontSize.mValue, FLT_MAX, 0, bName.c_str()).x;
        });

    std::vector<TempRenderInfo> backgroundRects;
    ImVec4 totalRect = {
        displayRes.x,
        5.f,
        0.f,
        0.f
    };

    float totalHeight = 0.f;
    std::vector<std::pair<float, float>> moduleHeights;  
    
    for (auto& mod : module) {
        if (!mod->mVisibleInArrayList.mValue) continue;
        if (mVisibility.mValue == ModuleVisibility::Bound && mod->mKey == 0) continue;
        if (mExcludeVisual.mValue && mod->mCategory == ModuleCategory::Visual) continue;

        std::string name = mod->getName();
        std::string settingDisplay = mod->getSettingDisplayText();
        if (!mRenderMode.mValue) settingDisplay = "";
        if (!settingDisplay.empty()) settingDisplay = " " + settingDisplay;

        ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0, name.c_str());
        float moduleHeight = textSize.y + 3.0f;
        
        if (mod->mEnabled) {
            totalHeight += moduleHeight;
        }
        
        static std::map<Module*, float> heightAnimations;
        if (heightAnimations.find(mod.get()) == heightAnimations.end()) {
            heightAnimations[mod.get()] = mod->mEnabled ? moduleHeight : 0.f;
        }
        
        heightAnimations[mod.get()] = MathUtils::lerp(
            heightAnimations[mod.get()],
            mod->mEnabled ? moduleHeight : 0.f,
            ImGui::GetIO().DeltaTime * 6.f
        );
        
        moduleHeights.push_back({moduleHeight, heightAnimations[mod.get()]});
    }

    float currentY = 5.f;
    int moduleIndex = 0;
    
    for (auto& mod : module) {
        if (!mod->mVisibleInArrayList.mValue) continue;
        if (mVisibility.mValue == ModuleVisibility::Bound && mod->mKey == 0) continue;
        if (mExcludeVisual.mValue && mod->mCategory == ModuleCategory::Visual) continue;

        mod->mArrayListAnim = MathUtils::lerp(mod->mArrayListAnim, mod->mEnabled ? 1.f : 0.f, ImGui::GetIO().DeltaTime * 6.f);
        if (mod->mArrayListAnim < 0.005f && !mod->mEnabled) {
            moduleIndex++;
            continue;
        }

        std::string name = mod->getName();
        std::string settingDisplay = mod->getSettingDisplayText();
        if (!mRenderMode.mValue) settingDisplay = "";
        if (!settingDisplay.empty()) settingDisplay = " " + settingDisplay;

        ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0, name.c_str());
        ImVec2 textPos = ImVec2(pos.x, currentY);

        if (mDisplay.mValue == Display::Bar || mDisplay.mValue == Display::Split) {
            textPos.x -= 7;
        }

        ImVec2 displaySize = { 0, 0 };
        if (!settingDisplay.empty()) {
            displaySize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0, settingDisplay.c_str());
        }

        float endPos = textPos.x - textSize.x - displaySize.x;
        textPos.x = MathUtils::lerp(displayRes.x + 14.f, endPos, mod->mArrayListAnim);

        ImVec4 rect = {
            textPos.x - 3,
            textPos.y,
            textPos.x + textSize.x + displaySize.x + 3,
            textPos.y + textSize.y + 1.0f
        };

        totalRect.x = std::min(totalRect.x, rect.x);
        totalRect.z = std::max(totalRect.z, rect.z + 6.f);
        totalRect.w = pos.y + textSize.y;

        if (mBackground.mValue == BackgroundStyle::Shadow || mBackground.mValue == BackgroundStyle::Both) {
            ImVec4 shadowRectTall = ImVec4( // skinny tall one
                rect.x + 12.f,
                rect.y - 1.0f,
                rect.z - 12.f,
                rect.w + 1.0f
            );
            ImVec4 shadowRectWide = ImVec4( // fat short one
                rect.x + 6.f,
                rect.y + 12.f - 1.0f,
                rect.z - 6.f,
                rect.w - 12.f + 1.0f
            );

            // Left circle
            drawList->AddShadowCircle(
                ImVec2(rect.x + 12.f, rect.y + 12.f - 1.0f),
                12.5f,
                ImColor(0, 0, 0, int(255 * mBackgroundOpacity.mValue)),
                35.f + (mShadowOpacity.mValue * 15.f),
                ImVec2(0, 0),
                ImDrawFlags_None,
                25
            );

            // Right circle
            drawList->AddShadowCircle(
                ImVec2(rect.z - 12.f, rect.y + 12.f - 1.0f),
                12.5f,
                ImColor(0, 0, 0, int(255 * mBackgroundOpacity.mValue)),
                35.f + (mShadowOpacity.mValue * 15.f),
                ImVec2(0, 0),
                ImDrawFlags_None,
                25
            );

            // Tall shadow rectangles
            drawList->AddShadowRect(
                ImVec2(shadowRectTall.x, shadowRectTall.y),
                ImVec2(shadowRectTall.z, shadowRectTall.w),
                ImColor(0, 0, 0, int(255 * mBackgroundOpacity.mValue)),
                35.f + (mShadowOpacity.mValue * 15.f),
                ImVec2(0, 0),
                ImDrawFlags_None,
                25
            );
            drawList->AddShadowRect(
                ImVec2(shadowRectTall.x, shadowRectTall.y),
                ImVec2(shadowRectTall.z, shadowRectTall.w),
                ImColor(0, 0, 0, int(255 * mBackgroundOpacity.mValue)),
                35.f + (mShadowOpacity.mValue * 15.f),
                ImVec2(0, 0),
                ImDrawFlags_None,
                25
            );

            // Wide shadow rectangles
            drawList->AddShadowRect(
                ImVec2(shadowRectWide.x, shadowRectWide.y),
                ImVec2(shadowRectWide.z, shadowRectWide.w),
                ImColor(0, 0, 0, int(255 * mBackgroundOpacity.mValue)),
                35.f + (mShadowOpacity.mValue * 15.f),
                ImVec2(0, 0),
                ImDrawFlags_None,
                25
            );
            drawList->AddShadowRect(
                ImVec2(shadowRectWide.x, shadowRectWide.y),
                ImVec2(shadowRectWide.z, shadowRectWide.w),
                ImColor(0, 0, 0, int(255 * mBackgroundOpacity.mValue)),
                35.f + (mShadowOpacity.mValue * 15.f),
                ImVec2(0, 0),
                ImDrawFlags_None,
                25
            );
        }

        if (mBackground.mValue == BackgroundStyle::Opacity || mBackground.mValue == BackgroundStyle::Both) {
            drawList->AddRectFilled(
                ImVec2(rect.x, rect.y - 1.0f),
                ImVec2(rect.z, rect.w + 1.0f),
                ImColor(0.05f, 0.05f, 0.05f, mBackgroundOpacity.mValue),
                0.f,
                ImDrawFlags_None
            );
        }

        ImVec2 currentPos = textPos;
        float time = (float)ImGui::GetTime() * colorSpeed;

        for (int i = 0; i < name.length(); i++) 
        {
            char c = name[i];
            ImColor textColor;

            if (mHorizontalColor.mValue) 
            {
                float t = (float)i / (float)(name.length() - 1);
                if (mThemeColor.mValue) 
                {
                    ImColor culler = ColorUtils::getThemedColor(i*100);
                    ImColor culler2 = ColorUtils::getThemedColor(i*100+1);

                    float blend = fmodf(time + t, 1.0f);
                    textColor = ImColor(
                        MathUtils::lerp(culler.Value.x, culler2.Value.x, blend) * saturation,
                        MathUtils::lerp(culler.Value.y, culler2.Value.y, blend) * saturation,
                        MathUtils::lerp(culler.Value.z, culler2.Value.z, blend) * saturation,
                        1.0f
                    );
                }
                else if (mColors.mValue <= 1 && !mThemeColor.mValue) 
                {
                    textColor = mColor1.getAsImColor();
                    textColor = ImColor(
                        textColor.Value.x * saturation,
                        textColor.Value.y * saturation,
                        textColor.Value.z * saturation,
                        textColor.Value.w
                    );
                } 
                else if (!mThemeColor.mValue) 
                {
                    int currentColor = (int)(time + t) % (int)mColors.mValue;
                    int nextColor = (currentColor + 1) % (int)mColors.mValue;
                    float blend = fmodf(time + t, 1.0f);

                    ImColor color1, color2;
                    switch (currentColor) 
                    {
                        case 0: color1 = mColor1.getAsImColor(); break;
                        case 1: color1 = mColor2.getAsImColor(); break;
                        case 2: color1 = mColor3.getAsImColor(); break;
                        case 3: color1 = mColor4.getAsImColor(); break;
                        case 4: color1 = mColor5.getAsImColor(); break;
                        case 5: color1 = mColor6.getAsImColor(); break;
                    }
                    switch (nextColor) 
                    {
                        case 0: color2 = mColor1.getAsImColor(); break;
                        case 1: color2 = mColor2.getAsImColor(); break;
                        case 2: color2 = mColor3.getAsImColor(); break;
                        case 3: color2 = mColor4.getAsImColor(); break;
                        case 4: color2 = mColor5.getAsImColor(); break;
                        case 5: color2 = mColor6.getAsImColor(); break;
                    }

                    textColor = ImColor(
                        MathUtils::lerp(color1.Value.x, color2.Value.x, blend) * saturation,
                        MathUtils::lerp(color1.Value.y, color2.Value.y, blend) * saturation,
                        MathUtils::lerp(color1.Value.z, color2.Value.z, blend) * saturation,
                        1.0f
                    );
                }
            } 
            else 
            {
                textColor = ColorUtils::getThemedColor(currentY * 2);
            }

            if (glow && (!displayGlow || (displayGlow && mDisplay.mValue == Display::None))) 
            {
                drawList->AddShadowCircle(
                    ImVec2(currentPos.x + (ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0, &c, &c + 1).x / 2), 
                          currentPos.y + (fontSize / 2)),
                    fontSize / 3,
                    ImColor(textColor.Value.x, textColor.Value.y, textColor.Value.z, mod->mArrayListAnim * 0.75f),
                    glowStrength,
                    ImVec2(0.f, 0.f),
                    0,
                    12
                );
            }

            ImVec2 shadowPos = currentPos;
            shadowPos.x += 1.f;
            shadowPos.y += 1.f;
            drawList->AddText(ImGui::GetFont(), fontSize, shadowPos, ImColor(0.f, 0.f, 0.f, 0.5f), &c, &c + 1);
            drawList->AddText(ImGui::GetFont(), fontSize, currentPos, textColor, &c, &c + 1);
            currentPos.x += ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0, &c, &c + 1).x;
        }

        if (!settingDisplay.empty()) 
        {
            drawList->AddText(ImGui::GetFont(), fontSize, currentPos, ImColor(0.9f, 0.9f, 0.9f, 1.f), settingDisplay.c_str());
        }

        if (mDisplay.mValue == Display::Bar || mDisplay.mValue == Display::Split) 
        {
            ImColor barColor;
            float time = (float)ImGui::GetTime() * colorSpeed;

            if (mHorizontalColor.mValue) 
            {
                if (mThemeColor.mValue)
                {
                    float blend = fmodf(time, 1.0f) - 21.f;
                    ImColor culler = ColorUtils::getThemedColor(blend*100);
                    ImColor culler2 = ColorUtils::getThemedColor(blend*100+1);


                    barColor = ImColor(
                        MathUtils::lerp(culler.Value.x, culler2.Value.x, blend) * saturation,
                        MathUtils::lerp(culler.Value.y, culler2.Value.y, blend) * saturation,
                        MathUtils::lerp(culler.Value.z, culler2.Value.z, blend) * saturation,
                        1.0f
                    );
                }
                else if (mColors.mValue <= 1) 
                {
                    barColor = mColor1.getAsImColor();
                } 
                else 
                {
                    float t = fmodf(time, 1.0f);
                    int currentColor = (int)time % (int)mColors.mValue;
                    int nextColor = (currentColor + 1) % (int)mColors.mValue;

                    ImColor color1, color2;
                    switch (currentColor) {
                        case 0: color1 = mColor1.getAsImColor(); break;
                        case 1: color1 = mColor2.getAsImColor(); break;
                        case 2: color1 = mColor3.getAsImColor(); break;
                        case 3: color1 = mColor4.getAsImColor(); break;
                        case 4: color1 = mColor5.getAsImColor(); break;
                        case 5: color1 = mColor6.getAsImColor(); break;
                    }
                    switch (nextColor) {
                        case 0: color2 = mColor1.getAsImColor(); break;
                        case 1: color2 = mColor2.getAsImColor(); break;
                        case 2: color2 = mColor3.getAsImColor(); break;
                        case 3: color2 = mColor4.getAsImColor(); break;
                        case 4: color2 = mColor5.getAsImColor(); break;
                        case 5: color2 = mColor6.getAsImColor(); break;
                    }

                    barColor = ImColor(
                        MathUtils::lerp(color1.Value.x, color2.Value.x, t),
                        MathUtils::lerp(color1.Value.y, color2.Value.y, t),
                        MathUtils::lerp(color1.Value.z, color2.Value.z, t)
                    );
                }
            } 
            else 
            {
                barColor = ColorUtils::getThemedColor(currentY * 2);
            }

            barColor = ImColor(
                barColor.Value.x * saturation,
                barColor.Value.y * saturation,
                barColor.Value.z * saturation,
                barColor.Value.w
            );

            if (mDisplay.mValue == Display::Split) {
                float barHeight = textSize.y - 2.f;
                float barY = rect.y + 1.f;

                drawList->AddRectFilled(
                    ImVec2(rect.z, barY),
                    ImVec2(rect.z + mThickness.mValue, barY + barHeight),
                    barColor,
                    25.0f,
                    ImDrawFlags_RoundCornersRight
                );

                if (glow && (!displayGlow || (displayGlow && mDisplay.mValue == Display::Split))) {
                    drawList->AddShadowRect(
                        ImVec2(rect.z, barY),
                        ImVec2(rect.z + mThickness.mValue, barY + barHeight),
                        barColor,
                        glowStrength * mod->mArrayListAnim,
                        ImVec2(0.f, 0.f),
                        ImDrawFlags_None,
                        12
                    );
                }
            } 
            else if (mDisplay.mValue == Display::Bar) 
            {
                drawList->AddRectFilled(
                    ImVec2(rect.z, rect.y - 1.5f),
                    ImVec2(rect.z + mThickness.mValue, rect.w + 1.5f),
                    barColor,
                    5.0f
                );

                if (glow && (!displayGlow || (displayGlow && mDisplay.mValue == Display::Bar))) {
                    drawList->AddShadowRect(
                        ImVec2(rect.z, rect.y - 1.5f),
                        ImVec2(rect.z + mThickness.mValue, rect.w + 1.5f),
                        barColor,
                        glowStrength * mod->mArrayListAnim,
                        ImVec2(0.f, 0.f),
                        ImDrawFlags_None,
                        12
                    );
                }
            }
        }

        backgroundRects.push_back({name, ImVec2(rect.x, rect.y), ImVec2(rect.z, rect.w), ColorUtils::getThemedColor(currentY * 2), mod.get()});
        currentY += moduleHeights[moduleIndex].second;
        moduleIndex++;
    }

    FontHelper::popPrefFont();
}