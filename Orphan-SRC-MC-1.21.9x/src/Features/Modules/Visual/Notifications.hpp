#pragma once
//
// Created by vastrakai on 7/4/2024.
//
#include <Features/FeatureManager.hpp>
#include <Features/Events/NotifyEvent.hpp>
#include <Features/Modules/Module.hpp>

class Notifications : public ModuleBase<Notifications> {
public:
    enum class Style {
        Solaris,
        EDest,
    };
    DividerSetting dVisual = DividerSetting("- Visual -", "Settings for visuals");
    EnumSettingT<Style> mStyle = EnumSettingT<Style>("Style", "The style of the notifications", Style::Solaris, "Solaris", "EDest");
    BoolSetting mShowOnToggle = BoolSetting("Show on toggle", "Show a notification when a module is toggled", true);
    BoolSetting mShowOnJoin = BoolSetting("Show on join", "Show a notification when you join a server", true);
    //BoolSetting mColorGradient = BoolSetting("Color gradient", "Enable a color gradient on the notifications", false);

    DividerSetting dLimit = DividerSetting("- Limit -", "Settings for visuals");
    NumberSetting mNotificationDuration = NumberSetting("Notification Duration", "The maximum number of notifications shown at one time", 2, 1, 10, 1);
    BoolSetting mLimitNotifications = BoolSetting("Limit notifications", "Limit the number of notifications shown at one time", false);
    NumberSetting mMaxNotifications = NumberSetting("Max notifications", "The maximum number of notifications shown at one time", 6, 1, 25, 1);
    Notifications() : ModuleBase("Notifications", "Shows notifications on module toggle and other events", ModuleCategory::Visual, 0, true) {
        addSetting(&dVisual);
        addSetting(&mStyle);
        addSetting(&mShowOnToggle);
        addSetting(&mShowOnJoin);
        //addSetting(&mColorGradient);

        addSetting(&dLimit);
        addSetting(&mNotificationDuration);
        addSetting(&mLimitNotifications);
        addSetting(&mMaxNotifications);//test build
        VISIBILITY_CONDITION(mMaxNotifications, mLimitNotifications.mValue == true);
        mNames = {
            {Lowercase, "notifications"},
            {LowercaseSpaced, "notifications"},
            {Normal, "Notifications"},
            {NormalSpaced, "Notifications"}
        };
        gFeatureManager->mDispatcher->listen<RenderEvent, &Notifications::onRenderEvent, nes::event_priority::VERY_LAST>(this);
    }
    std::vector<Notification> mNotifications;
    void onEnable() override;
    void onDisable() override;
    void onRenderEvent(class RenderEvent& event);
    void renderSolarisStyle();
    void renderEDestStyle();
    void onModuleStateChange(ModuleStateChangeEvent& event);
    void onConnectionRequestEvent(class ConnectionRequestEvent& event);
    void onNotifyEvent(class NotifyEvent& event);
    std::string getSettingDisplay() override {
        return mStyle.mValues[mStyle.mValue == Style::Solaris ? 0 : 1];
    }
};