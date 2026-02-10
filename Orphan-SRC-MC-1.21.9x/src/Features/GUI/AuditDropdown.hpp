#pragma once
#include <Utils/MiscUtils/ImRenderUtils.hpp>

class AuditDropdown {
public:
    float blurAnim = 0.0f; // 0 = hidden, 1 = fully shown
    bool isOpen = false;
    bool isBinding = false;
    bool isFadingOut = false;

    void render(float animation, float inScale, int& scrollDirection, char* h, float blur, float midclickRounding, bool isPressingShift);
    void onWindowResizeEvent(class WindowResizeEvent& event);
};

using AuditGui = AuditDropdown;
