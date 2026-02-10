#pragma once
#include <Features/Events/ActorRenderEvent.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Events/ModuleStateChangeEvent.hpp>
#include <Features/Events/DrawImageEvent.hpp>
#include <Features/Events/PreGameCheckEvent.hpp>
//
// Created by vastrakai on 7/1/2024.
// Edited by player5 (1/24/2025)
//


class Interface : public ModuleBase<Interface>
{
public:
    float buttonYOffset = 0.0f;
    bool initialized = false;
    bool wasValidScreen = false;
    bool isConfigMenuOpen = false;
    bool isTabMenuOpen = false;
    float tabMenuAnim = 0.0f;
    float tabMenuYLerped = 0.0f;
    std::vector<Actor*> tabPlayerActors;
    std::vector<std::string> tabPlayerNames;
    std::string tabMenuServerName;
    std::string tabMenuServerIP;
    bool prevTabMenuOpen = false;
    std::string lastConnectedServerIP = "Localhost";
    bool isPregameTabMenu = false;

    enum ColorTheme {
        Rainbow,
        Bubblegum,
        Sunset,
        Poison,
        Custom
    };

    enum class FontType {
        Mojangles,
        ProductSans,
        OpenSans,
        Comfortaa,
        SFProDisplay
    };

    ColorTheme randomTheme = (ColorTheme)MathUtils::random(0, ColorTheme::Custom);

    DividerSetting dText = DividerSetting("- Text -", "Settings for text");
    EnumSettingT<NamingStyle> mNamingStyle = EnumSettingT<NamingStyle>("Naming", "The style of the module names", NamingStyle::NormalSpaced, "lowercase", "lower spaced", "Normal", "Spaced");
    EnumSettingT<FontType> mFont = EnumSettingT<FontType>("Font", "The font of the interface", FontType::ProductSans, "Mojangles", "Product Sans", "Open Sans", "Comfortaa", "SF Pro Display");
    BoolSetting mSansFont = BoolSetting("Sans Font", "Use Product Sans for the ClickGUI", true);
    BoolSetting mBold = BoolSetting("Bold Font", "Use a bold font", false);

    DividerSetting dColor = DividerSetting("- Color -", "Settings for color");
    EnumSettingT<ColorTheme> mMode = EnumSettingT<ColorTheme>("Theme", "The mode of the interface", ColorTheme::Rainbow, "Rainbow", "Bubblegum", "Sunset", "Poison", "Custom");
    NumberSetting mColors = NumberSetting("Colors", "The amount of colors in the interface", 3, 1, 6, 1);
    ColorSetting mColor1 = ColorSetting("Color 1", "The first color of the interface", 0xFFFF0000);
    ColorSetting mColor2 = ColorSetting("Color 2", "The second color of the interface", 0xFFFF7F00);
    ColorSetting mColor3 = ColorSetting("Color 3", "The third color of the interface", 0xFFFFD600);
    ColorSetting mColor4 = ColorSetting("Color 4", "The fourth color of the interface", 0xFF00FF00);
    ColorSetting mColor5 = ColorSetting("Color 5", "The fifth color of the interface", 0xFF0000FF);
    ColorSetting mColor6 = ColorSetting("Color 6", "The sixth color of the interface", 0xFF8B00FF);
    NumberSetting mColorSpeed = NumberSetting("Color Speed", "The speed of the color change", 3.f, 0.1f, 20.f, 0.1);
    NumberSetting mSaturation = NumberSetting("Saturation", "The saturation of the interface", 1.f, 0.f, 1.f, 0.1);

    DividerSetting dOther = DividerSetting("- Other -", "Other settings");
    BoolSetting mSlotEasing = BoolSetting("Slot Easing", "Eases the selection of slots", true);
    NumberSetting mSlotEasingSpeed = NumberSetting("Easing Speed", "The speed of the slot easing", 20.f, 0.1f, 20.f, 0.1f);
    BoolSetting mForcePackSwitching = BoolSetting("Force Pack Switching", "Allows pack switching in-game", false);
    BoolSetting mHoveredItem = BoolSetting("Custom Hover", "Customizes the hovered item gui", false);

    DividerSetting dTabList = DividerSetting("- Tab List -", "Settings for tab list");
    NumberSetting mTabListOpacity = NumberSetting("Background Opacity", "The opacity of the tab list background", 0.75f, 0.0f, 1.0f, 0.01f);
    NumberSetting mTabListBlur = NumberSetting("Background Blur", "The blur amount of the tab list background", 10.0f, 0.0f, 20.0f, 0.1f);

    // Add a keybind for the tab list
    KeybindSetting mTabListKey = KeybindSetting("Tab List Key", "Keybind for opening the tab list", 0);

    Interface() : ModuleBase("Interface", "Customize the visuals!", ModuleCategory::Visual, 0, true) {
        gFeatureManager->mDispatcher->listen<ModuleStateChangeEvent, &Interface::onModuleStateChange, nes::event_priority::FIRST>(this);
        gFeatureManager->mDispatcher->listen<RenderEvent, &Interface::onRenderEvent, nes::event_priority::NORMAL>(this);
        gFeatureManager->mDispatcher->listen<ActorRenderEvent, &Interface::onActorRenderEvent, nes::event_priority::NORMAL>(this);
        gFeatureManager->mDispatcher->listen<BaseTickEvent, &Interface::onBaseTickEvent>(this);
        gFeatureManager->mDispatcher->listen<PacketOutEvent, &Interface::onPacketOutEvent, nes::event_priority::ABSOLUTE_LAST>(this);
        gFeatureManager->mDispatcher->listen<DrawImageEvent, &Interface::onDrawImageEvent>(this);
        gFeatureManager->mDispatcher->listen<PreGameCheckEvent, &Interface::onPregameCheckEvent>(this);

        addSettings(
            &dText,
            &mNamingStyle,
            &mFont,
            &mSansFont,
            &mBold,

            &dColor,
            &mMode,
            &mColors,
            &mColor1,
            &mColor2,
            &mColor3,
            &mColor4,
            &mColor5,
            &mColor6,
            &mColorSpeed,
            &mSaturation,

            &dOther,
            &mSlotEasing,
            &mSlotEasingSpeed,
            &mForcePackSwitching,
            &mHoveredItem,
            &dTabList,
            &mTabListOpacity,
            &mTabListBlur,
            &mTabListKey
        );

        VISIBILITY_CONDITION(mColors, mMode.mValue == Custom);
        VISIBILITY_CONDITION(mColor1, mMode.mValue == Custom && mColors.mValue >= 1);
        VISIBILITY_CONDITION(mColor2, mMode.mValue == Custom && mColors.mValue >= 2);
        VISIBILITY_CONDITION(mColor3, mMode.mValue == Custom && mColors.mValue >= 3);
        VISIBILITY_CONDITION(mColor4, mMode.mValue == Custom && mColors.mValue >= 4);
        VISIBILITY_CONDITION(mColor5, mMode.mValue == Custom && mColors.mValue >= 5);
        VISIBILITY_CONDITION(mColor6, mMode.mValue == Custom && mColors.mValue >= 6);

        VISIBILITY_CONDITION(mSlotEasingSpeed, mSlotEasing.mValue);

        mNames = {
            {Lowercase, "interface"},
            {LowercaseSpaced, "interface"},
            {Normal, "Interface"},
            {NormalSpaced, "Interface"}
        };
    }

    static inline std::unordered_map<int, std::vector<ImColor>> ColorThemes = {

{Rainbow,   {}},

{Bubblegum, {
    ImColor(255, 99, 202, 255),
    ImColor(255, 195, 195, 255),
    ImColor(146, 245, 255, 255),
    ImColor(249, 255, 148, 255),
    ImColor(135, 255, 176, 255),
}},

{Sunset, {
    ImColor(213,32,0, 255),
    ImColor(239, 118, 39, 255),
    ImColor(255, 154, 86, 255),
    ImColor(255, 255, 255, 255),
    ImColor(209, 98, 164, 255),
    ImColor(181, 86, 144, 255),
}},

{Poison, {
    ImColor(115,222,70, 255),
    ImColor(67, 201, 89, 255),
    ImColor(41, 230, 94, 255),
    ImColor(12, 210, 83, 255),
    ImColor(87, 211, 72, 255),
    ImColor(57, 210, 124, 255),
}},

{Custom,    {}}
    };

    std::vector<ImColor> getCustomColors() {
        auto result = std::vector<ImColor>();
        if (mColors.mValue >= 1) result.push_back(mColor1.getAsImColor());
        if (mColors.mValue >= 2) result.push_back(mColor2.getAsImColor());
        if (mColors.mValue >= 3) result.push_back(mColor3.getAsImColor());
        if (mColors.mValue >= 4) result.push_back(mColor4.getAsImColor());
        if (mColors.mValue >= 5) result.push_back(mColor5.getAsImColor());
        if (mColors.mValue >= 6) result.push_back(mColor6.getAsImColor());
        return result;
    }

    void onEnable() override;
    void onDisable() override;
    void renderHoverText();
    void onModuleStateChange(ModuleStateChangeEvent& event);
    void onPregameCheckEvent(class PreGameCheckEvent& event);
    void onRenderEvent(class RenderEvent& event);
    void onActorRenderEvent(class ActorRenderEvent& event);
    void onDrawImageEvent(class DrawImageEvent& event);
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
    void updateTabPlayerList();
    void renderTabMenu();
    ID3D11ShaderResourceView* getTabPlayerHeadTex(Actor* actor);
    void onConnectionRequestEvent(class ConnectionRequestEvent& event);

};

class BodyYaw
{
public:
    static inline float bodyYaw = 0.f;
    static inline glm::vec3 posOld = glm::vec3(0, 0, 0);
    static inline glm::vec3 pos = glm::vec3(0, 0, 0);

    static inline void updateRenderAngles(Actor* plr, float headYaw)
    {
        posOld = pos;
        pos = *plr->getPos();

        float diffX = pos.x - posOld.x;
        float diffZ = pos.z - posOld.z;
        float diff = diffX * diffX + diffZ * diffZ;

        float body = bodyYaw;
        if (diff > 0.0025000002F)
        {
            float anglePosDiff = atan2f(diffZ, diffX) * 180.f / 3.14159265358979323846f - 90.f;
            float degrees = abs(wrapAngleTo180_float(headYaw) - anglePosDiff);
            if (95.f < degrees && degrees < 265.f)
            {
                body = anglePosDiff - 180.f;
            }
            else
            {
                body = anglePosDiff;
            }
        }

        turnBody(body, headYaw);
    };

    static inline void turnBody(float bodyRot, float headYaw)
    {
        float amazingDegreeDiff = wrapAngleTo180_float(bodyRot - bodyYaw);
        bodyYaw += amazingDegreeDiff * 0.3f;
        float bodyDiff = wrapAngleTo180_float(headYaw - bodyYaw);
        if (bodyDiff < -75.f)
            bodyDiff = -75.f;

        if (bodyDiff >= 75.f)
            bodyDiff = 75.f;

        bodyYaw = headYaw - bodyDiff;
        if (bodyDiff * bodyDiff > 2500.f)
        {
            bodyYaw += bodyDiff * 0.2f;
        }
    };

    static inline float wrapAngleTo180_float(float value)
    {
        value = fmodf(value, 360.f);

        if (value >= 180.0F)
        {
            value -= 360.0F;
        }

        if (value < -180.0F)
        {
            value += 360.0F;
        }

        return value;
    };
};