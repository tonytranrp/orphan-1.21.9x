#include "AuditDropdown.hpp"

void AuditDropdown::render(float animation, float inScale, int& scrollDirection, char* h, float blur, float midclickRounding, bool isPressingShift) {
    // Animate blurAnim towards open/close based on animation (1 = open, 0 = closed)
    float speed = 8.0f;
    float delta = ImGui::GetIO().DeltaTime;
    float target = (animation > 0.01f || isFadingOut) ? 1.0f : 0.0f;
    if (blurAnim != target) {
        if (blurAnim < target) {
            blurAnim += speed * delta;
            if (blurAnim > target) blurAnim = target;
        } else {
            blurAnim -= speed * delta;
            if (blurAnim < target) blurAnim = target;
        }
    }
    if (blurAnim > 0.01f) {
        ImVec2 screen = ImRenderUtils::getScreenSize();
        ImVec4 blurRect(0, 0, screen.x, screen.y);
        ImRenderUtils::addBlur(blurRect, blurAnim * blur);
    }
    // If fully faded out, stop fading
    if (isFadingOut && blurAnim <= 0.01f) {
        isFadingOut = false;
    }
}

void AuditDropdown::onWindowResizeEvent(class WindowResizeEvent& event) {
    // No-op for now
}
