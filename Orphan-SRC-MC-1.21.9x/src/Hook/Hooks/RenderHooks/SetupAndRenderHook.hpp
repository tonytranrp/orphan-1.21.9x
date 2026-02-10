#pragma once
//
// Created by vastrakai on 7/7/2024.
//
#include <Hook/Hook.hpp>
#include <Hook/HookManager.hpp>
#include <SDK/Minecraft/mce.hpp>
#include <SDK/Minecraft/Rendering/MinecraftUIRenderContext.hpp>

static const uintptr_t ExecutableBaseAddress = reinterpret_cast<uintptr_t>(GetModuleHandleA("Minecraft.Windows.exe"));

class UIControl
{
public:
    /*std::string& getLayerName() {
        return hat::member_at<std::string>(this, 0x20);
    }*/

    std::string getLayerName()
    {
        const char* ptr = reinterpret_cast<const char*>(this);

        if (ptr == nullptr || reinterpret_cast<uintptr_t>(ptr) >= ExecutableBaseAddress)
            return std::string("UnknownLayer");

        std::string result;
        while (*ptr != '\0')
        {
            result += *ptr;
            ptr++;
        }
        return result;
    }
};

class VisualTree
{
public:
PAD(0x28);
    UIControl *mRootUIControl;
};

class ScreenView
{
public:
PAD(0x48);
    VisualTree*       mTree;
};

class SetupAndRenderHook : public Hook {
public:
    SetupAndRenderHook() : Hook() {
        mName = "ScreenView::setupAndRender";
    }

    static std::unique_ptr<Detour> mSetupAndRenderDetour;
    static std::unique_ptr<Detour> mDrawImageDetour;


    static void* onSetupAndRender(ScreenView* screenView, MinecraftUIRenderContext* mcuirc);
    static void* onDrawImage(MinecraftUIRenderContext* context, BedrockTextureData* texture, glm::vec2* pos, glm::vec2* size, glm::vec2* uv, mce::Color* color, void* unk);
    //static void* onDrawImageDetour(void* context, mce::TexturePtr* texture, glm::vec2* imagePos, glm::vec2* size, glm::vec2* uv, mce::Color* color, void* unk);
    static inline bool sDrawingTitleImage = false;
    static std::unique_ptr<Detour> mDrawTextDetour;
    static void* onDrawText(
        MinecraftUIRenderContext* context,
        Font* font,
        glm::vec4 const& pos,
        std::string* str,
        mce::Color const& colour,
        float alpha,
        float alinM,
        TextMeasureData* const& textdata,
        CaretMeasureData* const& caretdata);

    static void initVt(void* ctx);
    void init() override;

};

