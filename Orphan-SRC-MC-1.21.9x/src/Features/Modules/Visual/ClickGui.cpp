//
// Created by vastrakai on 6/29/2024.
//

#include "ClickGui.hpp"

#include <Features/Events/MouseEvent.hpp>
#include <Features/Events/KeyEvent.hpp>
#include <Features/GUI/ModernDropdown.hpp>
#include <Features/GUI/SimpleDropdown.hpp>
#include <Features/GUI/AuditDropdown.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>

static bool lastMouseState = false;
static bool isPressingShift = false;
static ModernGui modernGui = ModernGui();
static SimpleGui simpleGui = SimpleGui();
static AuditGui auditGui = AuditGui();

void ClickGui::onEnable()//oopsies
{
    auto cii = ClientInstance::get();
    lastMouseState = !cii->getMouseGrabbed();

    cii->releaseMouse();

    gFeatureManager->mDispatcher->listen<MouseEvent, &ClickGui::onMouseEvent>(this);
    gFeatureManager->mDispatcher->listen<KeyEvent, &ClickGui::onKeyEvent, nes::event_priority::FIRST>(this);
}

void ClickGui::onDisable()
{
    gFeatureManager->mDispatcher->deafen<MouseEvent, &ClickGui::onMouseEvent>(this);
    gFeatureManager->mDispatcher->deafen<KeyEvent, &ClickGui::onKeyEvent>(this);

    // Only grab mouse when in game world, let other screens handle mouse state naturally
    auto ci = ClientInstance::get();
    if (ci && ci->getScreenName() == "hud_screen") {
        ci->grabMouse();
    }
}

void ClickGui::onWindowResizeEvent(WindowResizeEvent& event)
{
    if (mStyle.mValue == ClickGuiStyle::Modern) {
        modernGui.onWindowResizeEvent(event);
    } else if (mStyle.mValue == ClickGuiStyle::Simple) {
        simpleGui.onWindowResizeEvent(event);
    }
}

void ClickGui::onMouseEvent(MouseEvent& event)
{
    event.mCancelled = true;
}

void ClickGui::onKeyEvent(KeyEvent& event)
{
    // Prevent processing the next key event after keybind binding/unbinding
    if (ModernGui::justUnboundKeybind || SimpleGui::justUnboundKeybind) {
        ModernGui::justUnboundKeybind = false;
        SimpleGui::justUnboundKeybind = false;
        event.mCancelled = true;
        return;
    }
    // Block Escape from closing UI if SimpleDropdown is listening for a keybind
    if (SimpleGui::isKeybindBindingActive && event.mKey == VK_ESCAPE && event.mPressed) {
        event.mCancelled = true;
        return;
    }
    // Allow toggling ClickGui with its keybind even if ImGui is capturing keyboard
    if (event.mKey == mKey && event.mPressed) {
        this->toggle();
        event.mCancelled = true;
        return;
    }
    if (event.mKey == VK_ESCAPE) {
        if ((!modernGui.isBinding && !simpleGui.isBinding && !auditGui.isBinding) && event.mPressed) this->toggle();
        event.mCancelled = true;
    }

    if (modernGui.isBinding || simpleGui.isBinding || auditGui.isBinding) {
        event.mCancelled = true;
        return;
    }

    if (event.mKey == VK_SHIFT && event.mPressed) {
        isPressingShift = true;
        event.mCancelled = true;
    }
    else {
        isPressingShift = false;
    }
}

float ClickGui::getEaseAnim(EasingUtil ease, int mode) {
    switch (mode) {
        case 0: return ease.easeOutExpo();
        case 1: return mEnabled ? ease.easeOutElastic() : ease.easeOutBack();
        default: return ease.easeOutExpo();
    }
}

void ClickGui::onRenderEvent(RenderEvent& event)
{
    if (mEnabled) {
        ImGui::GetIO().WantCaptureKeyboard = true;
    }
    static float animation = 0;
    static int styleMode = 0;
    static int scrollDirection = 0;
    static char h[2] = { 0 };
    static EasingUtil inEase = EasingUtil();

    float delta = ImGui::GetIO().DeltaTime;

    this->mEnabled ? inEase.incrementPercentage(delta * mEaseSpeed.mValue / 10)
                   : inEase.decrementPercentage(delta * 2 * mEaseSpeed.mValue / 10);
    float inScale = getEaseAnim(inEase, mAnimation.as<int>());
    if (inEase.isPercentageMax()) inScale = 0.996;
    if (mAnimation.mValue == ClickGuiAnimation::Zoom) inScale = MathUtils::clamp(inScale, 0.0f, 0.996);
    animation = MathUtils::lerp(0, 1, inEase.easeOutExpo());

    if (animation < 0.0001f) {
        return;
    }

    if (ImGui::GetIO().MouseWheel > 0) {
        scrollDirection = -1;
    }
    else if (ImGui::GetIO().MouseWheel < 0) {
        scrollDirection = 1;
    }
    else {
        scrollDirection = 0;
    }

    if (mStyle.mValue == ClickGuiStyle::Modern) {
        modernGui.render(animation, inScale, scrollDirection, h, mBlurStrength.mValue, mMidclickRounding.mValue, isPressingShift);
    } else if (mStyle.mValue == ClickGuiStyle::Simple) {
        simpleGui.render(animation, inScale, scrollDirection, h, mBlurStrength.mValue, mMidclickRounding.mValue, isPressingShift);
    }
}