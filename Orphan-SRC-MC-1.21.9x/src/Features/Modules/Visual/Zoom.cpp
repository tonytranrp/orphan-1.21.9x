//
// Created by alteik on 15/10/2024.
//

#include "Zoom.hpp"
#include <SDK/Minecraft/Actor/Actor.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Options.hpp>

void Zoom::onEnable()
{
    mPastFov = ClientInstance::get()->getOptions()->mGfxFieldOfView->mValue;
    misEnabled = true;

    if (mSmooth.mValue)
        mStartValue = mPastFov; // Start from the current FOV
    else
        mCurrentValue = mZoomValue.mValue;

    mZoomingIn = true; // Flag to indicate zooming in
    ClientInstance::get()->getOptions()->mGfxFieldOfView->mMinimum = 10.f;

    gFeatureManager->mDispatcher->listen<MouseEvent, &Zoom::onMouseEvent>(this);
    gFeatureManager->mDispatcher->listen<RenderEvent, &Zoom::onRenderEvent>(this);

}

void Zoom::onDisable()
{
    misEnabled = false;
    mZoomingIn = false;
    mStartValue = mZoomValue.mValue;
    gFeatureManager->mDispatcher->deafen<MouseEvent, &Zoom::onMouseEvent>(this);
    gFeatureManager->mDispatcher->deafen<RenderEvent, &Zoom::onRenderEvent>(this);

    ClientInstance::get()->getOptions()->mGfxFieldOfView->mMinimum = 30.f;
    ClientInstance::get()->getOptions()->mGfxFieldOfView->mValue = mPastFov; // Reset to original FOV
}

void Zoom::onMouseEvent(MouseEvent& event)
{
    if (ClientInstance::get()->getMouseGrabbed()) return;
    if (!mScroll.mValue) return;

    if (event.mActionButtonId == 4)
    {
        if (event.mButtonData == 0x78 || event.mButtonData == 0x7F)
        {
            mZoomValue.mValue -= mScrollIncrement.mValue;
            event.cancel();
        }
        else if (event.mButtonData == 0x88 || event.mButtonData == 0x80 || event.mButtonData == -0x78)
        {
            mZoomValue.mValue += mScrollIncrement.mValue;
            event.cancel();
        }
    }

    if (mZoomValue.mValue < 10.f) mZoomValue.mValue = 10.f;
    if (mPastFov < mZoomValue.mValue) mZoomValue.mValue = mPastFov;
    if (mCurrentValue < 10.f) mCurrentValue = 10.f;
}

void Zoom::onRenderEvent(RenderEvent& event)
{

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    if (mSmooth.mValue)
    {
        ClientInstance::get()->getOptions()->mGfxFieldOfView->mValue = MathUtils::lerp(ClientInstance::get()->getOptions()->mGfxFieldOfView->mValue, mZoomValue.mValue, ImGui::GetIO().DeltaTime * 20.f);
    }
    else
        ClientInstance::get()->getOptions()->mGfxFieldOfView->mValue = mZoomValue.mValue;
}