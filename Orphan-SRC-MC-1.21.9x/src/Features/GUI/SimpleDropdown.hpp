#pragma once
#include <vector>
#include <Features/FeatureManager.hpp>
#include <Features/Modules/Setting.hpp>

class SimpleGui 
{
public:
    struct CategoryPosition 
    {
        float x = 0.f, y = 0.f;
        bool isDragging = false, isExtended = true, wasExtended = true;
        float currentScroll = 0.0f, targetScroll = 0.0f;
        glm::vec2 dragVelocity = glm::vec2(0, 0);
        ImVec2 lastMousePos = ImVec2(0, 0), dragStartPos = ImVec2(0, 0), dragStartMouse = ImVec2(0, 0);
        float dragStartTime = 0.0, lastX = 0.0f, lastY = 0.0f;
    };

    const float catWidth = 200.f, catHeight = 30.f, catGap = 40.f;
    float settingsHeight = 500.0f, knobSize = 5.0f, enumTransition = 1.0f;
    std::string lastEnumValue = "";
    int lastDragged = -1;
    std::vector<CategoryPosition> catPositions;
    std::shared_ptr<Module> lastMod = nullptr;
    Setting* lastDraggedSetting = nullptr;
    bool isBinding = false, isBindingNumber = false, isBoolSettingBinding = false;
    BoolSetting* lastBoolSetting = nullptr;
    ColorSetting* lastColorSetting = nullptr;
    bool displayColorPicker = false;
    bool resetPosition = false;
    uint64_t lastReset = 0;

    ImColor textColor = ImColor(255, 255, 255);
    ImColor bgColor = ImColor(15, 15, 15, 180);
    ImColor categoryBgColor = ImColor(28, 28, 28);
    ImColor enabledModuleBg = ImColor(35, 35, 35);
    ImColor disabledModuleBg = ImColor(25, 25, 25);
    ImColor accentColor = ImColor(97, 232, 255);
    ImColor settingBgColor = ImColor(32, 32, 32);

    ImVec4 scaleToPoint(const ImVec4& _this, const ImVec4& point, float amount);
    bool isMouseOver(const ImVec4& rect);
    ImVec4 getCenter(ImVec4& vec);
    void render(float animation, float inScale, int& scrollDirection, char* h, float blur, float midclickRounding, bool isPressingShift);
    void onWindowResizeEvent(class WindowResizeEvent& event);
    std::string getCategoryIcon(const std::string& category);
    SimpleGui() {
    }

    // Keybind binding state for SimpleGui
    static bool isKeybindBinding;
    static KeybindSetting* lastKeybindSetting;
    static bool isKeybindBindingActive;
    static bool justUnboundKeybind;
}; 