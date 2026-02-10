#include <Features/Events/ActorRenderEvent.hpp>
#include <Features/Events/BaseTickEvent.hpp>
#include <Features/Configs/ConfigManager.hpp>
#include <Features/Events/DrawImageEvent.hpp>
#include <Features/Events/ModuleStateChangeEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <Features/Events/PacketOutEvent.hpp>
#include <Features/Events/DrawImageEvent.hpp>
#include <Features/Events/PreGameCheckEvent.hpp>
#include <Features/Events/RenderEvent.hpp>
#include <SDK/Minecraft/Actor/SerializedSkin.hpp>

#include <Features/Modules/Visual/Interface.hpp>
#include <Features/Modules/Visual/ClickGui.hpp>
#include <Hook/Hooks/RenderHooks/ActorRenderDispatcherHook.hpp>
#include <Hook/Hooks/RenderHooks/HoverTextRendererHook.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/mce.hpp>
#include <SDK/Minecraft/Options.hpp>
#include <SDK/Minecraft/Network/Packets/Packet.hpp>
#include <SDK/Minecraft/Network/Packets/PlayerAuthInputPacket.hpp>

#include <SDK/Minecraft/Actor/SyncedPlayerMovementSettings.hpp>
#include <SDK/Minecraft/Network/Packets/MovePlayerPacket.hpp>
#include <SDK/Minecraft/World/Level.hpp>
#include <shellapi.h>

#include <Hook/Hooks/RenderHooks/D3DHook.hpp>

#include <Features/Modules/Visual/LevelInfo.hpp>
#include <Features/Events/ConnectionRequestEvent.hpp>

#include <SDK/Minecraft/Actor/Actor.hpp>

std::vector<unsigned char> gFsBytes2 = { 0x0f, 0x85 };
DEFINE_PATCH_FUNC(patchFullStack, SigManager::ResourcePackManager_composeFullStackBp, gFsBytes2);

std::unordered_map<char, ImColor> mColorMap = {
    {'0', ImColor(0, 0, 0)},
    {'1', ImColor(0, 0, 170)},
    {'2', ImColor(0, 170, 0)},
    {'3', ImColor(0, 170, 170)},
    {'4', ImColor(170, 0, 0)},
    {'5', ImColor(170, 0, 170)},
    {'6', ImColor(255, 170, 0)},
    {'7', ImColor(170, 170, 170)},
    {'8', ImColor(85, 85, 85)},
    {'9', ImColor(85, 85, 255)},
    {'a', ImColor(85, 255, 85)},
    {'b', ImColor(85, 255, 255)},
    {'c', ImColor(255, 85, 85)},
    {'d', ImColor(255, 85, 255)},
    {'e', ImColor(255, 255, 85)},
    {'f', ImColor(255, 255, 255)},
    {'r', ImColor(255, 255, 255)}
};

template <typename T>
std::string combine(T t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template <typename T, typename... Args>
std::string combine(T t, Args... args)
{
    std::stringstream ss;
    ss << t << combine(args...);
    return ss.str();
}

float pYaw;
float pOldYaw;

float pHeadYaw;
float pOldHeadYaw;

float pPitch;
float pOldPitch;

float pBodyYaw;
float pOldBodyYaw;

float pLerpedYaw;
float pLerpedHeadYaw;
float pLerpedPitch;
float pLerpedBodyYaw;

bool usingPaip = false;

static bool lastPlayerState = false;
static std::string notificationText;
static float notificationTimer = 0.0f;
static float notificationAlpha = 0.0f;
static float menuAlpha = 0.0f;
static float menuScale = 0.0f;
static ImVec2 menuPosition;
static ImVec2 targetMenuPosition;
static std::map<std::string, float> loadAnimations;
static std::map<std::string, float> deleteAnimations;
static std::map<std::string, float> itemPositions;
static std::map<std::string, bool> itemVisible;
static std::map<std::string, float> hoverAnimations;
static bool wasValidScreen = false;
static float buttonYOffset = 0.0f;
static bool initialized = false;
static bool isConfigMenuOpen = false;
static bool isBindMenuOpen = false;
static std::map<std::string, ImVec2> targetPositions;
static std::map<std::string, ImVec2> currentPositions;

static float bindMenuScrollY = 0.0f;
static float targetBindMenuScrollY = 0.0f;
static std::map<std::shared_ptr<Module>, float> keyBoxHoverAnimations;
static std::map<std::shared_ptr<Module>, float> keyBoxRippleAnimations;
static std::map<std::shared_ptr<Module>, float> moduleVisibilityAnimations;
static std::map<std::shared_ptr<Module>, float> modulePositionOffsets;
static std::map<std::shared_ptr<Module>, ImVec2> keyBoxRipplePositions;
static std::shared_ptr<Module> currentlyBindingModule = nullptr;
static float bindingNotificationAlpha = 0.0f;

static float overlayAlpha = 0.0f;

bool isPregameTabMenu = false;

// --- Tab List Pagination ---
static int tabPage = 0;
static constexpr int TAB_PAGE_SIZE = 24;
static size_t tabListLoadIndex = 0;
static constexpr size_t TABLIST_CHUNK_SIZE = 25;

void Interface::onEnable()
{

}

void Interface::onDisable()
{
    patchFullStack(false);
}

void Interface::renderHoverText()
{
    static EasingUtil inEase;

    (HoverTextRender::mTimeDisplayed != 0 && gFeatureManager->mModuleManager->getModule<ClickGui>()->mEnabled != true) ?
        inEase.incrementPercentage(ImRenderUtils::getDeltaTime() * 2)
        : inEase.decrementPercentage(ImRenderUtils::getDeltaTime() * 4);

    float inScale = HoverTextRender::mTimeDisplayed != 0 && gFeatureManager->mModuleManager->getModule<ClickGui>()->mEnabled != true ? inEase.easeOutExpo() : inEase.easeOutBack();

    if (inEase.isPercentageMax())
        inScale = 1;

    if (inScale < 0.01)
        return;

    glm::vec2 mPos = HoverTextRender::mInfo.mPos;
    glm::vec2 mTextPos = glm::vec2(mPos.x + 6, mPos.y + 6);

    float mTextSize = 1.25 * inScale;

    std::string mMessage = HoverTextRender::mInfo.mText;
    std::string mNoneColoredText = ColorUtils::removeColorCodes(HoverTextRender::mInfo.mText);

    ImColor mCurrentColor = ImColor(255, 255, 255);

    float mMeasurementX = ImGui::GetFont()->CalcTextSizeA(mTextSize * 18, FLT_MAX, -1, mNoneColoredText.c_str()).x;
    float mMeasurementY = ImGui::GetFont()->CalcTextSizeA(mTextSize * 18, FLT_MAX, -1, mNoneColoredText.c_str()).y;

    ImVec4 mRect = ImVec4(mPos.x, mPos.y, mPos.x + mMeasurementX + 12, mPos.y + mMeasurementY + 12);

    ImRenderUtils::addBlur(mRect, 3 * inScale, 10);

    ImRenderUtils::fillRectangle(mRect, ImColor(0, 0, 0), 0.78f * inScale, 10);

    for (size_t j = 0; j < mMessage.length(); ++j) {
        char c = mMessage[j];

        if (c == '§' && j + 1 < mMessage.length()) {
            char colorCode = mMessage[j + 1];
            if (mColorMap.find(colorCode) != mColorMap.end()) {
                mCurrentColor = mColorMap[colorCode];
                j++;
            }
            continue;
        }

        if (c == '\n') {
            mTextPos.x = mPos.x + 6;
            mTextPos.y += ImGui::GetFont()->CalcTextSizeA(mTextSize * 18, FLT_MAX, 0, "\n").y;
        }

        if (!std::isprint(c)) {
            continue;
        }

        std::string mString = combine(c, "");

        ImRenderUtils::drawText(mTextPos, mString, mCurrentColor, mTextSize, inScale, false);

        mTextPos.x += ImGui::GetFont()->CalcTextSizeA(mTextSize * 18, FLT_MAX, -1, mString.c_str()).x;
    }

    HoverTextRender::mTimeDisplayed = 0;
}

void Interface::onModuleStateChange(ModuleStateChangeEvent& event)
{
    if (event.mModule == this)
    {
        event.setCancelled(true);
    }
}

void Interface::onPregameCheckEvent(PreGameCheckEvent& event)
{
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player || !mForcePackSwitching.mValue) {
        isPregameTabMenu = false;
        return;
    }
    std::string screenName = ClientInstance::get()->getScreenName();
    if (screenName.contains("screen_world_controls_and_settings") && !screenName.contains("global_texture_pack_tab")) {
        isPregameTabMenu = true;
        return;
    }
    event.setPreGame(true);
    isPregameTabMenu = true;
}

void Interface::updateTabPlayerList() {
    static std::vector<std::string> allNames;
    static std::vector<Actor*> allActors;
    if (tabListLoadIndex == 0) {
        allNames.clear();
        allActors.clear();
        tabPlayerNames.clear();
        tabPlayerActors.clear();
        tabMenuServerIP = lastConnectedServerIP;
        if (tabMenuServerIP.empty()) tabMenuServerIP = "Localhost";
        auto ci = ClientInstance::get();
        auto player = ci ? ci->getLocalPlayer() : nullptr;
        auto actors = ActorUtils::getActorList(true, true);
        for (auto actor : actors) {
            if (!actor || !actor->isPlayer()) continue;
            std::string name = actor->getRawName();
            if (name.empty()) continue;
            allNames.push_back(name);
            allActors.push_back(actor);
        }
        std::vector<std::pair<std::string, Actor*>> zipped;
        for (size_t i = 0; i < allNames.size(); ++i) {
            zipped.emplace_back(allNames[i], allActors[i]);
        }
        std::sort(zipped.begin(), zipped.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
        for (size_t i = 0; i < zipped.size(); ++i) {
            allNames[i] = zipped[i].first;
            allActors[i] = zipped[i].second;
        }
    }
    // Load next chunk
    size_t end = std::min(tabListLoadIndex + TABLIST_CHUNK_SIZE, allNames.size());
    for (size_t i = tabListLoadIndex; i < end; ++i) {
        tabPlayerNames.push_back(allNames[i]);
        tabPlayerActors.push_back(allActors[i]);
    }
    tabListLoadIndex = end;
    // Clamp tabPage to valid range
    int maxPage = std::max(0, (int)tabPlayerNames.size() / TAB_PAGE_SIZE);
    tabPage = std::clamp(tabPage, 0, maxPage);
}

ID3D11ShaderResourceView* Interface::getTabPlayerHeadTex(Actor* actor) {
    static std::map<Actor*, ID3D11ShaderResourceView*> tabHeadTextures;
    if (!actor) return nullptr;
    if (tabHeadTextures.contains(actor) && tabHeadTextures[actor])
        return tabHeadTextures[actor];
    ID3D11ShaderResourceView* texture = nullptr;
    auto skin = actor->getSkin();
    if (!skin || skin->impl->mObject.skinWidth <= 0 || skin->impl->mObject.skinHeight <= 0 || !skin->impl->mObject.skinData) return nullptr;
    int headSize = skin->impl->mObject.skinWidth / 8;
    int headOffsetX = skin->impl->mObject.skinWidth / 8;
    int headOffsetY = skin->impl->mObject.skinHeight / 8;
    if (headSize <= 0 || headOffsetX < 0 || headOffsetY < 0) return nullptr;
    if ((headOffsetY + headSize) * skin->impl->mObject.skinWidth * 4 > skin->impl->mObject.skinWidth * skin->impl->mObject.skinHeight * 4) return nullptr;
    std::vector<uint8_t> headData(headSize * headSize * 4);
    for (int y = 0; y < headSize; y++) {
        for (int x = 0; x < headSize; x++) {
            int srcIndex = ((y + headOffsetY) * skin->impl->mObject.skinWidth + (x + headOffsetX)) * 4;
            int dstIndex = (y * headSize + x) * 4;
            if (srcIndex < 0 || dstIndex < 0 || srcIndex + 3 >= skin->impl->mObject.skinWidth * skin->impl->mObject.skinHeight * 4 || dstIndex + 3 >= (int)headData.size())
                continue;
            std::copy_n(skin->impl->mObject.skinData + srcIndex, 4, headData.data() + dstIndex);
        }
    }
    int scalingFactor = 4;
    std::vector<uint8_t> scaledHeadData(headSize * scalingFactor * headSize * scalingFactor * 4);
    for (int y = 0; y < headSize * scalingFactor; y++) {
        for (int x = 0; x < headSize * scalingFactor; x++) {
            int srcX = x / scalingFactor;
            int srcY = y / scalingFactor;
            int srcIndex = (srcY * headSize + srcX) * 4;
            int dstIndex = (y * headSize * scalingFactor + x) * 4;
            if (srcIndex < 0 || dstIndex < 0 || srcIndex + 3 >= (int)headData.size() || dstIndex + 3 >= (int)scaledHeadData.size())
                continue;
            std::copy_n(headData.data() + srcIndex, 4, scaledHeadData.data() + dstIndex);
        }
    }
    headSize *= scalingFactor;
    headData = std::move(scaledHeadData);
    if (headSize > 0 && !headData.empty()) {
        D3DHook::createTextureFromData(headData.data(), headSize, headSize, &texture);
        tabHeadTextures[actor] = texture;
    }
    return texture;
}

void Interface::renderTabMenu() {
    ImVec2 screen = ImRenderUtils::getScreenSize();
    int playerCount = static_cast<int>(tabPlayerNames.size());
    int numColumns = 1;
    if (playerCount > 24) numColumns = 4;
    else if (playerCount > 12) numColumns = 3;
    else if (playerCount > 6) numColumns = 2;
    int rows = (playerCount + numColumns - 1) / numColumns;

    // --- Pagination logic ---
    int maxPage = std::max(0, (playerCount - 1) / TAB_PAGE_SIZE);
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) tabPage = std::min(tabPage + 1, maxPage);
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) tabPage = std::max(tabPage - 1, 0);
    int startIdx = tabPage * TAB_PAGE_SIZE;
    int endIdx = std::min(startIdx + TAB_PAGE_SIZE, playerCount);

    float menuWidth = 480.0f;
    float extraColSpacing = 18.0f;
    float menuHeight = 48.0f + rows * 28.0f + 36.0f;
    float colWidth = (menuWidth - 32.0f - (numColumns - 1) * extraColSpacing) / numColumns;
    float yTarget = 60.0f;
    if (isTabMenuOpen && !prevTabMenuOpen) tabMenuYLerped = -menuHeight;
    prevTabMenuOpen = isTabMenuOpen;
    tabMenuYLerped = MathUtils::lerp(tabMenuYLerped, yTarget, tabMenuAnim * 0.18f + (1.0f - tabMenuAnim) * 0.08f);
    float yOffset = tabMenuYLerped;

    ImVec2 menuPos((screen.x - menuWidth) * 0.5f, yOffset);
    ImVec2 menuEnd(menuPos.x + menuWidth, menuPos.y + menuHeight);

    ImRenderUtils::addBlur(ImVec4(menuPos.x, menuPos.y, menuEnd.x, menuEnd.y), mTabListBlur.mValue * tabMenuAnim, mTabListBlur.mValue);
    ImRenderUtils::fillRectangle(ImVec4(menuPos.x, menuPos.y, menuEnd.x, menuEnd.y), ImColor(0, 0, 0), mTabListOpacity.mValue * tabMenuAnim, 16.0f);

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    bool isPregame = isPregameTabMenu;
    auto ci = ClientInstance::get();
    if (ci) {
        std::string screenName = ci->getScreenName();
        if (screenName.find("start_screen") != std::string::npos) {
            isPregame = true;
        }
    }

    std::string connText;
    std::string ipText;
    ImColor connColor(255, 105, 180, 255);
    ImColor ipColor(255, 255, 80, 255);
    if (isPregame) {
        connText = "are you stupid???";
        ipText = "";
    }
    else {
        connText = "Connected to ";
        ipText = tabMenuServerIP;
    }
    ImVec2 connTextSize = ImGui::CalcTextSize(connText.c_str());
    ImVec2 ipTextSize = ImGui::CalcTextSize(ipText.c_str());
    float totalTextWidth = connTextSize.x + ipTextSize.x;
    float connTextY = menuPos.y + 10.0f;
    float startX = menuPos.x + (menuWidth - totalTextWidth) * 0.5f;
    drawList->AddText(ImVec2(startX, connTextY), connColor, connText.c_str());
    if (!ipText.empty())
        drawList->AddText(ImVec2(startX + connTextSize.x, connTextY), ipColor, ipText.c_str());

    // --- Page indicator ---
    if (maxPage > 0) {
        std::string pageStr = "Page " + std::to_string(tabPage + 1) + "/" + std::to_string(maxPage + 1);
        ImVec2 pageTextSize = ImGui::CalcTextSize(pageStr.c_str());
        drawList->AddText(ImVec2(menuEnd.x - pageTextSize.x - 16, menuPos.y + 10.0f), ImColor(200, 200, 200, 180), pageStr.c_str());
    }

    ImVec2 startPos(menuPos.x + 16, menuPos.y + 28 + connTextSize.y + 4.0f);
    float headSize = 18.0f;
    float rowHeight = 28.0f;
    float nameFontSize = 16.0f;
    int idx = startIdx;
    for (int col = 0; col < numColumns; ++col) {
        ImVec2 colPos = startPos;
        colPos.x += col * (colWidth + extraColSpacing);
        for (int row = 0; row < rows && idx < endIdx; ++row, ++idx) {
            if (idx >= (int)tabPlayerActors.size() || !tabPlayerActors[idx]) continue;
            ID3D11ShaderResourceView* headTex = getTabPlayerHeadTex(tabPlayerActors[idx]);
            ImVec2 headMin(colPos.x, colPos.y);
            ImVec2 headMax(colPos.x + headSize, colPos.y + headSize);
            if (headTex) {
                drawList->AddImageRounded(headTex, headMin, headMax, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), 3.0f);
            }
            else {
                drawList->AddRectFilled(headMin, headMax, ImColor(80, 80, 80, 200), 3.0f);
            }
            std::string name = tabPlayerNames[idx];
            if (name.empty()) name = "Unknown";
            ImVec2 namePos = ImVec2(colPos.x + headSize + 8, colPos.y + 2);
            drawList->AddText(ImGui::GetFont(), nameFontSize, namePos, ImColor(220, 220, 220, 230), name.c_str());
            colPos.y += rowHeight;
        }
    }
}

void Interface::onRenderEvent(RenderEvent& event)
{
    std::string screenName = ClientInstance::get()->getScreenName();

    auto player = ClientInstance::get()->getLocalPlayer();
    static float notificationTimer = 0.0f;
    static float notificationAlpha = 0.0f;
    static float menuAlpha = 0.0f;
    static float menuScale = 0.0f;
    static ImVec2 menuPosition;
    static ImVec2 targetMenuPosition;

    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        if (currentlyBindingModule) {

            currentlyBindingModule->mKey = 0;
            currentlyBindingModule = nullptr;
            bindingNotificationAlpha = 0.0f;
            ClientInstance::get()->playUi("random.break", 0.75f, 1.0f);
            return;
        }
        else if (isConfigMenuOpen || isBindMenuOpen) {

            isConfigMenuOpen = false;
            isBindMenuOpen = false;
            menuAlpha = 0.0f;
            menuScale = 0.0f;
            return;
        }
    }

    bool shouldShowButtons = (screenName == "start_screen") ||
        (screenName == "pause_screen") ||
        gFeatureManager->mModuleManager->getModule<ClickGui>()->mEnabled;

    if ((isConfigMenuOpen || isBindMenuOpen) && !shouldShowButtons) {
        isConfigMenuOpen = false;
        isBindMenuOpen = false;
        menuAlpha = 0.0f;
        menuScale = 0.0f;
    }

    static bool isMenuOpen = false;
    static float hoverAnimation = 0.0f;
    static float scrollY = 0.0f;
    static bool isAnimatingDown = false;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    bool inValidScreen = (screenName == "start_screen" || screenName == "pause_screen" || screenName == "world_loading_progress_screen - local_world_load");

    if (wasValidScreen != inValidScreen) {
        wasValidScreen = inValidScreen;
        if (!inValidScreen) {
            isAnimatingDown = true;
        }
    }

    if (!initialized) {
        buttonYOffset = displaySize.y + 100.0f;
        initialized = true;
    }

    float targetY = inValidScreen ?
        displaySize.y - 52.0f :
        displaySize.y + 100.0f;

    float deltaTime = ImGui::GetIO().DeltaTime;
    float lerpSpeed = inValidScreen ? 10.0f : 6.0f;
    buttonYOffset = MathUtils::lerp(buttonYOffset, targetY, deltaTime * lerpSpeed);

    if (isAnimatingDown && std::abs(buttonYOffset - targetY) < 0.1f) {
        isAnimatingDown = false;
    }

    if ((isConfigMenuOpen || isBindMenuOpen) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 menuMin(displaySize.x / 2 - 200, displaySize.y / 2 - 225);
        ImVec2 menuMax(displaySize.x / 2 + 200, displaySize.y / 2 + 225);

        if (mousePos.x < menuMin.x || mousePos.x > menuMax.x ||
            mousePos.y < menuMin.y || mousePos.y > menuMax.y) {

            ImVec2 buttonSize(160, 32);
            ImVec2 configButtonPos((displaySize.x - buttonSize.x * 2 - 10) * 0.5f, buttonYOffset);
            ImVec2 bindButtonPos(configButtonPos.x + buttonSize.x + 10, buttonYOffset);

            bool overConfigButton = mousePos.x >= configButtonPos.x && mousePos.x <= configButtonPos.x + buttonSize.x &&
                mousePos.y >= configButtonPos.y && mousePos.y <= configButtonPos.y + buttonSize.y;
            bool overBindButton = mousePos.x >= bindButtonPos.x && mousePos.x <= bindButtonPos.x + buttonSize.x &&
                mousePos.y >= bindButtonPos.y && mousePos.y <= bindButtonPos.y + buttonSize.y;

            if (!overConfigButton && !overBindButton) {
                isConfigMenuOpen = false;
                isBindMenuOpen = false;
                menuAlpha = 0.0f;
                menuScale = 0.0f;
            }
        }
    }

    if (shouldShowButtons || isAnimatingDown || buttonYOffset < displaySize.y + 90.0f) {
        ImVec2 buttonSize(160, 32);
        auto drawList = ImGui::GetForegroundDrawList();

        ImVec2 configButtonPos((displaySize.x - buttonSize.x * 2 - 10) * 0.5f, buttonYOffset);
        ImVec2 bindButtonPos(configButtonPos.x + buttonSize.x + 10, buttonYOffset);

        bool isConfigMouseOver = ImGui::IsMouseHoveringRect(
            configButtonPos,
            ImVec2(configButtonPos.x + buttonSize.x, configButtonPos.y + buttonSize.y),
            false
        );

        static float configHoverAnim = 0.0f;
        configHoverAnim = isConfigMouseOver ?
            std::min(1.0f, configHoverAnim + deltaTime * 4.0f) :
            std::max(0.0f, configHoverAnim - deltaTime * 4.0f);

        bool isBindMouseOver = ImGui::IsMouseHoveringRect(
            bindButtonPos,
            ImVec2(bindButtonPos.x + buttonSize.x, bindButtonPos.y + buttonSize.y),
            false
        );

        static float bindHoverAnim = 0.0f;
        bindHoverAnim = isBindMouseOver ?
            std::min(1.0f, bindHoverAnim + deltaTime * 4.0f) :
            std::max(0.0f, bindHoverAnim - deltaTime * 4.0f);

        ImColor configButtonColor(20, 20, 20, int(200 + 55 * configHoverAnim));
        ImColor configButtonOutline(255, 255, 255, int(40 + 40 * configHoverAnim));

        drawList->AddRectFilled(
            configButtonPos,
            ImVec2(configButtonPos.x + buttonSize.x, configButtonPos.y + buttonSize.y),
            configButtonColor,
            6.0f
        );

        drawList->AddRect(
            configButtonPos,
            ImVec2(configButtonPos.x + buttonSize.x, configButtonPos.y + buttonSize.y),
            configButtonOutline,
            6.0f,
            ImDrawFlags_RoundCornersAll,
            1.0f
        );

        ImColor bindButtonColor(20, 20, 20, int(200 + 55 * bindHoverAnim));
        ImColor bindButtonOutline(255, 255, 255, int(40 + 40 * bindHoverAnim));

        drawList->AddRectFilled(
            bindButtonPos,
            ImVec2(bindButtonPos.x + buttonSize.x, bindButtonPos.y + buttonSize.y),
            bindButtonColor,
            6.0f
        );

        drawList->AddRect(
            bindButtonPos,
            ImVec2(bindButtonPos.x + buttonSize.x, bindButtonPos.y + buttonSize.y),
            bindButtonOutline,
            6.0f,
            ImDrawFlags_RoundCornersAll,
            1.0f
        );

        ImVec2 configTextSize = ImGui::CalcTextSize("Config Menu");
        drawList->AddText(
            ImVec2(configButtonPos.x + (buttonSize.x - configTextSize.x) * 0.5f,
                configButtonPos.y + (buttonSize.y - configTextSize.y) * 0.5f),
            ImColor(255, 255, 255, int(200 + 55 * configHoverAnim)),
            "Config Menu"
        );

        ImVec2 bindTextSize = ImGui::CalcTextSize("Bind Menu");
        drawList->AddText(
            ImVec2(bindButtonPos.x + (buttonSize.x - bindTextSize.x) * 0.5f,
                bindButtonPos.y + (buttonSize.y - bindTextSize.y) * 0.5f),
            ImColor(255, 255, 255, int(200 + 55 * bindHoverAnim)),
            "Bind Menu"
        );

        if (isConfigMouseOver && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            isConfigMenuOpen = !isConfigMenuOpen;
            isBindMenuOpen = false;
            isMenuOpen = isConfigMenuOpen;
            if (!isConfigMenuOpen) {
                menuAlpha = 0.0f;
                menuScale = 0.0f;
            }
        }

        if (isBindMouseOver && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            isBindMenuOpen = !isBindMenuOpen;
            isConfigMenuOpen = false;
            isMenuOpen = isBindMenuOpen;
            if (!isBindMenuOpen) {
                menuAlpha = 0.0f;
                menuScale = 0.0f;
            }
        }

        if (isMenuOpen) {
            menuAlpha = std::min(1.0f, menuAlpha + deltaTime * 10.0f);
            menuScale = std::min(1.0f, menuScale + deltaTime * 8.0f);
        }
        else {
            menuAlpha = std::max(0.0f, menuAlpha - deltaTime * 15.0f);
            menuScale = std::max(0.0f, menuScale - deltaTime * 12.0f);
        }

        if (isConfigMenuOpen || isBindMenuOpen) {
            overlayAlpha = std::min(1.0f, overlayAlpha + deltaTime * 12.0f);
        }
        else {
            overlayAlpha = std::max(0.0f, overlayAlpha - deltaTime * 20.0f);
        }

        if (overlayAlpha > 0.01f) {
            drawList->AddRectFilled(
                ImVec2(0, 0),
                displaySize,
                ImColor(0, 0, 0, int(120 * overlayAlpha))
            );
        }

        if (isConfigMenuOpen && menuAlpha > 0.01f) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(displaySize);
            ImGui::Begin("##menuBlocker", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav
            );

            if (isConfigMenuOpen || isBindMenuOpen) {
                if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                    isConfigMenuOpen = false;
                    isBindMenuOpen = false;
                    menuAlpha = 0.0f;
                    menuScale = 0.0f;
                    ImGui::SetWindowFocus(nullptr);
                    ImGui::GetIO().WantCaptureKeyboard = true;
                }

                ImGui::GetIO().WantCaptureMouse = true;
                ImGui::GetIO().WantCaptureKeyboard = true;
            }

            drawList->AddRectFilled(
                ImVec2(0, 0),
                displaySize,
                ImColor(0, 0, 0, int(120 * menuAlpha))
            );

            const float CONFIG_WIDTH = 180.0f;
            const float CONFIG_HEIGHT = 40.0f;
            const float MENU_PADDING = 20.0f;
            const float ITEM_SPACING = 10.0f;
            const int ITEMS_PER_ROW = 3;

            std::vector<std::string> configs = FileUtils::listFiles(ConfigManager::getConfigPath());
            int configCount = 0;
            for (const auto& f : configs) {
                if (f.ends_with(".json")) configCount++;
            }

            int rows = (configCount + ITEMS_PER_ROW - 1) / ITEMS_PER_ROW;
            float menuWidth = (CONFIG_WIDTH * ITEMS_PER_ROW) + (ITEM_SPACING * (ITEMS_PER_ROW - 1)) + (MENU_PADDING * 2);
            float menuHeight = std::min(displaySize.y * 0.8f,
                (CONFIG_HEIGHT * rows) + ((rows - 1) * ITEM_SPACING) + 40.0f + MENU_PADDING * 2);

            ImVec2 menuSize(menuWidth, menuHeight);
            ImVec2 menuPos(
                (displaySize.x - menuSize.x * menuScale) * 0.5f,
                (displaySize.y - menuSize.y * menuScale) * 0.5f
            );

            drawList->AddRectFilled(
                menuPos,
                ImVec2(menuPos.x + menuSize.x * menuScale, menuPos.y + menuSize.y * menuScale),
                ImColor(20, 20, 20, int(230 * menuAlpha)),
                12.0f
            );

            drawList->AddRect(
                menuPos,
                ImVec2(menuPos.x + menuSize.x * menuScale, menuPos.y + menuSize.y * menuScale),
                ImColor(255, 255, 255, int(30 * menuAlpha)),
                12.0f,
                ImDrawFlags_RoundCornersAll,
                1.0f
            );

            drawList->AddRectFilled(
                menuPos,
                ImVec2(menuPos.x + menuSize.x * menuScale, menuPos.y + 30.0f),
                ImColor(30, 30, 30, int(250 * menuAlpha)),
                12.0f,
                ImDrawFlags_RoundCornersTop
            );

            ImVec2 headerTextSize = ImGui::CalcTextSize("Configurations");
            drawList->AddText(
                ImVec2(menuPos.x + (menuSize.x * menuScale - headerTextSize.x) * 0.5f,
                    menuPos.y + 8),
                ImColor(255, 255, 255, int(220 * menuAlpha)),
                "Configurations"
            );

            if (ImGui::IsMouseHoveringRect(
                ImVec2(menuPos.x, menuPos.y + 30.0f),
                ImVec2(menuPos.x + menuSize.x * menuScale, menuPos.y + menuSize.y * menuScale)
            )) {
                scrollY += ImGui::GetIO().MouseWheel * 30.0f;
            }

            float maxScroll = std::max(0.0f, (rows * (CONFIG_HEIGHT + ITEM_SPACING)) - (menuSize.y - 40.0f - MENU_PADDING * 2));
            scrollY = std::clamp(scrollY, -maxScroll, 0.0f);

            drawList->PushClipRect(
                ImVec2(menuPos.x, menuPos.y + 30.0f),
                ImVec2(menuPos.x + menuSize.x * menuScale, menuPos.y + menuSize.y * menuScale),
                true
            );

            int itemIndex = 0;
            float startY = menuPos.y + 40.0f + scrollY;

            {
                int row = 0;
                int col = 0;
                float itemX = menuPos.x + MENU_PADDING + (col * (CONFIG_WIDTH + ITEM_SPACING));
                float itemY = startY + (row * (CONFIG_HEIGHT + ITEM_SPACING));

                ImVec2 itemPos(itemX, itemY);
                bool itemHovered = ImGui::IsMouseHoveringRect(
                    itemPos,
                    ImVec2(itemPos.x + CONFIG_WIDTH, itemPos.y + CONFIG_HEIGHT),
                    false
                );

                static float importHoverAnim = 0.0f;
                if (itemHovered) {
                    importHoverAnim = std::min(1.0f, importHoverAnim + deltaTime * 4.0f);
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        std::string jsonStr = StringUtils::getClipboardText();
                        if (!jsonStr.empty()) {
                            try {
                                nlohmann::json json = nlohmann::json::parse(jsonStr);
                                gFeatureManager->mModuleManager->deserialize(json, false);
                                spdlog::info("Successfully imported config from clipboard");
                            }
                            catch (const std::exception& e) {
                                spdlog::error("Failed to import config: {}", e.what());
                            }
                        }
                    }
                    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                        ShellExecuteA(NULL, "explore", "%localappdata%\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\RoamingState\\Orphan\\Configs", NULL, NULL, SW_SHOWNORMAL);
                    }
                }
                else {
                    importHoverAnim = std::max(0.0f, importHoverAnim - deltaTime * 4.0f);
                }

                ImColor bgColor = ImColor(
                    ImVec4(
                        (30 + 15 * importHoverAnim) / 255.0f,
                        (30 + 15 * importHoverAnim) / 255.0f,
                        (30 + 15 * importHoverAnim) / 255.0f,
                        0.9f * menuAlpha
                    )
                );

                drawList->AddRectFilled(
                    itemPos,
                    ImVec2(itemPos.x + CONFIG_WIDTH, itemPos.y + CONFIG_HEIGHT),
                    bgColor,
                    8.0f
                );

                drawList->AddRect(
                    itemPos,
                    ImVec2(itemPos.x + CONFIG_WIDTH, itemPos.y + CONFIG_HEIGHT),
                    ImColor(ImVec4(1.0f, 1.0f, 1.0f, (0.12f + 0.12f * importHoverAnim) * menuAlpha)),
                    8.0f,
                    ImDrawFlags_RoundCornersAll,
                    1.0f
                );

                float iconSize = 20.0f;
                float iconThickness = 2.0f;
                ImVec2 center(
                    itemPos.x + CONFIG_WIDTH * 0.5f,
                    itemPos.y + CONFIG_HEIGHT * 0.5f
                );

                drawList->AddRectFilled(
                    ImVec2(center.x - iconSize * 0.5f, center.y - iconThickness * 0.5f),
                    ImVec2(center.x + iconSize * 0.5f, center.y + iconThickness * 0.5f),
                    ImColor(ImVec4(1.0f, 1.0f, 1.0f, (0.7f + 0.3f * importHoverAnim) * menuAlpha))
                );

                drawList->AddRectFilled(
                    ImVec2(center.x - iconThickness * 0.5f, center.y - iconSize * 0.5f),
                    ImVec2(center.x + iconThickness * 0.5f, center.y + iconSize * 0.5f),
                    ImColor(ImVec4(1.0f, 1.0f, 1.0f, (0.7f + 0.3f * importHoverAnim) * menuAlpha))
                );

                itemIndex++;
            }

            for (const auto& f : configs) {
                if (!f.ends_with(".json")) continue;
                std::string file = f.substr(0, f.size() - 5);

                if (itemVisible.find(file) == itemVisible.end()) {
                    itemVisible[file] = true;
                    int row = itemIndex / ITEMS_PER_ROW;
                    int col = itemIndex % ITEMS_PER_ROW;
                    float itemX = menuPos.x + MENU_PADDING + (col * (CONFIG_WIDTH + ITEM_SPACING));
                    float itemY = startY + (row * (CONFIG_HEIGHT + ITEM_SPACING));
                    currentPositions[file] = ImVec2(itemX, itemY);
                    targetPositions[file] = currentPositions[file];
                }

                if (itemVisible[file]) {
                    int row = itemIndex / ITEMS_PER_ROW;
                    int col = itemIndex % ITEMS_PER_ROW;
                    float itemX = menuPos.x + MENU_PADDING + (col * (CONFIG_WIDTH + ITEM_SPACING));
                    float itemY = startY + (row * (CONFIG_HEIGHT + ITEM_SPACING));
                    targetPositions[file] = ImVec2(itemX, itemY);
                    itemIndex++;
                }
            }

            for (auto& [file, currentPos] : currentPositions) {
                if (targetPositions.find(file) != targetPositions.end()) {
                    currentPos.x = MathUtils::lerp(currentPos.x, targetPositions[file].x, deltaTime * 10.0f);
                    currentPos.y = MathUtils::lerp(currentPos.y, targetPositions[file].y, deltaTime * 10.0f);
                }
            }

            for (const auto& f : configs) {
                if (!f.ends_with(".json")) continue;
                std::string file = f.substr(0, f.size() - 5);

                if (!itemVisible[file] && (!currentPositions.contains(file) ||
                    (std::abs(currentPositions[file].x - targetPositions[file].x) < 0.01f &&
                        std::abs(currentPositions[file].y - targetPositions[file].y) < 0.01f))) {
                    continue;
                }

                ImVec2 itemPos = currentPositions[file];

                bool itemHovered = ImGui::IsMouseHoveringRect(
                    itemPos,
                    ImVec2(itemPos.x + CONFIG_WIDTH, itemPos.y + CONFIG_HEIGHT),
                    false
                );

                bool isActive = (file == ConfigManager::LastLoadedConfig);

                ImColor bgColor = ImColor(
                    MathUtils::lerp(30.0f, 45.0f, hoverAnimations[file]) / 255.0f,
                    MathUtils::lerp(30.0f, 45.0f, hoverAnimations[file]) / 255.0f,
                    MathUtils::lerp(30.0f, 45.0f, hoverAnimations[file]) / 255.0f,
                    menuAlpha
                );

                drawList->AddRectFilled(
                    itemPos,
                    ImVec2(itemPos.x + CONFIG_WIDTH, itemPos.y + CONFIG_HEIGHT),
                    bgColor,
                    8.0f
                );

                ImColor outlineColor = isActive ?
                    ImColor(0, 255, 140, int(180 * menuAlpha)) :
                    ImColor(255, 255, 255, int(30 * menuAlpha));

                drawList->AddRect(
                    itemPos,
                    ImVec2(itemPos.x + CONFIG_WIDTH, itemPos.y + CONFIG_HEIGHT),
                    outlineColor,
                    8.0f,
                    ImDrawFlags_RoundCornersAll,
                    1.0f
                );

                float textWidth = ImGui::CalcTextSize(file.c_str()).x;
                float maxTextWidth = CONFIG_WIDTH - 24;
                std::string displayText = file;
                if (textWidth > maxTextWidth) {
                    while (textWidth > maxTextWidth - ImGui::CalcTextSize("...").x && !displayText.empty()) {
                        displayText.pop_back();
                        textWidth = ImGui::CalcTextSize(displayText.c_str()).x;
                    }
                    displayText += "...";
                }

                float textX = itemPos.x + 12;
                drawList->AddText(
                    ImVec2(textX, itemPos.y + (CONFIG_HEIGHT - ImGui::GetFontSize()) * 0.5f),
                    ImColor(255, 255, 255, int(220 * menuAlpha)),
                    displayText.c_str()
                );

                float buttonSize = 20.0f;
                float buttonSpacing = 8.0f;
                float buttonsStartX = itemPos.x + CONFIG_WIDTH - (buttonSize * 2 + buttonSpacing + 10);
                float buttonsY = itemPos.y + (CONFIG_HEIGHT - buttonSize) * 0.5f;

                ImVec2 loadPos(buttonsStartX, buttonsY);
                bool loadHovered = ImGui::IsMouseHoveringRect(
                    loadPos,
                    ImVec2(loadPos.x + buttonSize, loadPos.y + buttonSize),
                    false
                );

                if (loadAnimations.find(file) == loadAnimations.end()) {
                    loadAnimations[file] = 0.0f;
                }

                if (loadHovered) {
                    loadAnimations[file] = std::min(1.0f, loadAnimations[file] + deltaTime * 12.0f);
                }
                else {
                    loadAnimations[file] = std::max(0.0f, loadAnimations[file] - deltaTime * 12.0f);
                }

                ImColor loadOutlineColor = isActive ?
                    ImColor(0, 255, 140, int(180 * menuAlpha)) :
                    ImColor(0, 255, 140, int(120 * menuAlpha));

                drawList->AddRect(
                    loadPos,
                    ImVec2(loadPos.x + buttonSize, loadPos.y + buttonSize),
                    loadOutlineColor,
                    6.0f,
                    ImDrawFlags_RoundCornersAll,
                    1.5f
                );

                if (loadAnimations[file] > 0.01f) {
                    float fillSize = buttonSize * loadAnimations[file];
                    ImVec2 fillCenter(loadPos.x + buttonSize * 0.5f, loadPos.y + buttonSize * 0.5f);
                    drawList->AddRectFilled(
                        ImVec2(fillCenter.x - fillSize * 0.5f, fillCenter.y - fillSize * 0.5f),
                        ImVec2(fillCenter.x + fillSize * 0.5f, fillCenter.y + fillSize * 0.5f),
                        ImColor(0, 255, 140, int(150 * menuAlpha * loadAnimations[file])),
                        6.0f * loadAnimations[file]
                    );
                }

                ImVec2 deletePos(loadPos.x + buttonSize + buttonSpacing, buttonsY);
                bool deleteHovered = ImGui::IsMouseHoveringRect(
                    deletePos,
                    ImVec2(deletePos.x + buttonSize, deletePos.y + buttonSize),
                    false
                );

                if (deleteAnimations.find(file) == deleteAnimations.end()) {
                    deleteAnimations[file] = 0.0f;
                }

                if (deleteHovered) {
                    deleteAnimations[file] = std::min(1.0f, deleteAnimations[file] + deltaTime * 12.0f);
                }
                else {
                    deleteAnimations[file] = std::max(0.0f, deleteAnimations[file] - deltaTime * 12.0f);
                }

                drawList->AddRect(
                    deletePos,
                    ImVec2(deletePos.x + buttonSize, deletePos.y + buttonSize),
                    ImColor(255, 60, 60, int(120 * menuAlpha)),
                    6.0f,
                    ImDrawFlags_RoundCornersAll,
                    1.5f
                );

                if (deleteAnimations[file] > 0.01f) {
                    float fillSize = buttonSize * deleteAnimations[file];
                    ImVec2 fillCenter(deletePos.x + buttonSize * 0.5f, deletePos.y + buttonSize * 0.5f);
                    drawList->AddRectFilled(
                        ImVec2(fillCenter.x - fillSize * 0.5f, fillCenter.y - fillSize * 0.5f),
                        ImVec2(fillCenter.x + fillSize * 0.5f, fillCenter.y + fillSize * 0.5f),
                        ImColor(255, 60, 60, int(150 * menuAlpha * deleteAnimations[file])),
                        6.0f * deleteAnimations[file]
                    );
                }

                bool deleteButtonHovered = deleteHovered && ImGui::IsMouseHoveringRect(
                    deletePos,
                    ImVec2(deletePos.x + buttonSize, deletePos.y + buttonSize),
                    false
                );

                bool loadButtonHovered = loadHovered && ImGui::IsMouseHoveringRect(
                    loadPos,
                    ImVec2(loadPos.x + buttonSize, loadPos.y + buttonSize),
                    false
                );

                bool configRectHovered = ImGui::IsMouseHoveringRect(
                    itemPos,
                    ImVec2(itemPos.x + CONFIG_WIDTH, itemPos.y + CONFIG_HEIGHT),
                    false
                );

                if (deleteButtonHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    std::string configPath = ConfigManager::getConfigPath() + "/" + file + ".json";
                    if (std::filesystem::exists(configPath)) {
                        std::filesystem::remove(configPath);
                        itemVisible[file] = false;
                    }
                }
                else if ((loadButtonHovered || (configRectHovered && !deleteButtonHovered)) &&
                    ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    ConfigManager::loadConfig(file);
                }
            }

            for (auto it = currentPositions.begin(); it != currentPositions.end();) {
                if (!itemVisible[it->first]) {
                    it = currentPositions.erase(it);
                }
                else {
                    ++it;
                }
            }

            drawList->PopClipRect();
            ImGui::End();
        }

        if (isBindMenuOpen && menuAlpha > 0.01f) {

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(displaySize);
            ImGui::Begin("##bindMenuBlocker", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoBackground
            );

            ImVec2 menuSize(400, 450);
            ImVec2 menuPos(
                (displaySize.x - menuSize.x * menuScale) * 0.5f,
                (displaySize.y - menuSize.y * menuScale) * 0.5f
            );

            drawList->AddRectFilled(
                menuPos,
                ImVec2(menuPos.x + menuSize.x * menuScale, menuPos.y + menuSize.y * menuScale),
                ImColor(20, 20, 20, int(230 * menuAlpha)),
                12.0f
            );

            drawList->AddRect(
                menuPos,
                ImVec2(menuPos.x + menuSize.x * menuScale, menuPos.y + menuSize.y * menuScale),
                ImColor(255, 255, 255, int(30 * menuAlpha)),
                12.0f,
                ImDrawFlags_RoundCornersAll,
                1.0f
            );

            drawList->AddRectFilled(
                menuPos,
                ImVec2(menuPos.x + menuSize.x * menuScale, menuPos.y + 30.0f),
                ImColor(30, 30, 30, int(250 * menuAlpha)),
                12.0f,
                ImDrawFlags_RoundCornersTop
            );

            ImVec2 headerTextSize = ImGui::CalcTextSize("Key Binds");
            drawList->AddText(
                ImVec2(menuPos.x + (menuSize.x * menuScale - headerTextSize.x) * 0.5f,
                    menuPos.y + 8),
                ImColor(255, 255, 255, int(220 * menuAlpha)),
                "Key Binds"
            );

            ImVec4 contentArea(
                menuPos.x,
                menuPos.y + 40.0f,
                menuPos.x + menuSize.x * menuScale,
                menuPos.y + menuSize.y * menuScale
            );

            if (ImGui::IsMouseHoveringRect(
                ImVec2(contentArea.x, contentArea.y),
                ImVec2(contentArea.z, contentArea.w),
                false
            )) {
                targetBindMenuScrollY += ImGui::GetIO().MouseWheel * 40.0f;
            }

            float totalHeight = 0.0f;
            int boundModuleCount = 0;
            for (const auto& mod : gFeatureManager->mModuleManager->getModules()) {
                if (mod->mKey != 0 || moduleVisibilityAnimations[mod] > 0.001f) {
                    totalHeight += 45.0f;
                    boundModuleCount++;
                }
            }

            float maxScroll = std::max(0.0f, totalHeight - (menuSize.y - 40.0f));
            targetBindMenuScrollY = std::clamp(targetBindMenuScrollY, -maxScroll, 0.0f);

            bindMenuScrollY = MathUtils::lerp(bindMenuScrollY, targetBindMenuScrollY, deltaTime * 10.0f);

            drawList->PushClipRect(
                ImVec2(contentArea.x, contentArea.y),
                ImVec2(contentArea.z, contentArea.w),
                true
            );

            float yPos = menuPos.y + 40.0f + bindMenuScrollY;
            const float itemHeight = 40.0f;
            const float padding = 10.0f;

            for (const auto& mod : gFeatureManager->mModuleManager->getModules()) {
                if (moduleVisibilityAnimations.find(mod) == moduleVisibilityAnimations.end()) {
                    moduleVisibilityAnimations[mod] = mod->mKey != 0 ? 1.0f : 0.0f;
                    modulePositionOffsets[mod] = 0.0f;
                }

                float targetVisibility = mod->mKey != 0 ? 1.0f : 0.0f;
                moduleVisibilityAnimations[mod] = MathUtils::lerp(
                    moduleVisibilityAnimations[mod],
                    targetVisibility,
                    deltaTime * 12.0f
                );
            }

            float nextYOffset = 0.0f;
            for (const auto& mod : gFeatureManager->mModuleManager->getModules()) {
                if (mod->mKey != 0 || moduleVisibilityAnimations[mod] > 0.001f) {
                    float targetOffset = nextYOffset;
                    modulePositionOffsets[mod] = MathUtils::lerp(
                        modulePositionOffsets[mod],
                        targetOffset,
                        deltaTime * 8.0f
                    );

                    float currentYPos = yPos + modulePositionOffsets[mod];
                    float visibility = moduleVisibilityAnimations[mod];

                    ImVec4 itemRect(
                        menuPos.x + padding,
                        currentYPos,
                        menuPos.x + menuSize.x * menuScale - padding,
                        currentYPos + itemHeight
                    );

                    if (itemRect.w < contentArea.y || itemRect.y > contentArea.w) {
                        nextYOffset += (itemHeight + 5.0f) * visibility;
                        continue;
                    }

                    drawList->AddRectFilled(
                        ImVec2(itemRect.x, itemRect.y),
                        ImVec2(itemRect.z, itemRect.w),
                        ImColor(30, 30, 30, int(200 * menuAlpha * visibility)),
                        6.0f
                    );

                    drawList->AddText(
                        ImVec2(itemRect.x + 10, itemRect.y + (itemHeight - ImGui::GetFontSize()) * 0.5f),
                        ImColor(255, 255, 255, int(255 * menuAlpha * visibility)),
                        mod->getName().c_str()
                    );

                    if (keyBoxHoverAnimations.find(mod) == keyBoxHoverAnimations.end()) {
                        keyBoxHoverAnimations[mod] = 0.0f;
                        keyBoxRippleAnimations[mod] = 0.0f;
                    }

                    ImVec4 keyRect(
                        itemRect.z - 120,
                        itemRect.y + 5,
                        itemRect.z - 40,
                        itemRect.w - 5
                    );

                    ImVec4 unbindRect(
                        itemRect.z - 35,
                        itemRect.y + 5,
                        itemRect.z - 10,
                        itemRect.w - 5
                    );

                    bool isKeyBoxHovered = ImGui::IsMouseHoveringRect(
                        ImVec2(keyRect.x, keyRect.y),
                        ImVec2(keyRect.z, keyRect.w),
                        false
                    );

                    bool isUnbindHovered = ImGui::IsMouseHoveringRect(
                        ImVec2(unbindRect.x, unbindRect.y),
                        ImVec2(unbindRect.z, unbindRect.w),
                        false
                    );

                    if (isKeyBoxHovered) {
                        keyBoxHoverAnimations[mod] = std::min(1.0f, keyBoxHoverAnimations[mod] + deltaTime * 6.0f);
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            currentlyBindingModule = mod;
                            keyBoxRippleAnimations[mod] = 0.0f;
                            keyBoxRipplePositions[mod] = ImGui::GetMousePos();
                        }
                    }
                    else {
                        keyBoxHoverAnimations[mod] = std::max(0.0f, keyBoxHoverAnimations[mod] - deltaTime * 6.0f);
                    }

                    ImColor baseColor = ImColor(40, 40, 40, int(200 * menuAlpha * visibility));
                    ImColor hoverColor = ImColor(60, 60, 60, int(200 * menuAlpha * visibility));
                    ImColor finalColor = baseColor.Lerp(hoverColor, keyBoxHoverAnimations[mod]);

                    drawList->AddRectFilled(
                        ImVec2(keyRect.x, keyRect.y),
                        ImVec2(keyRect.z, keyRect.w),
                        finalColor,
                        4.0f
                    );

                    ImColor unbindBaseColor = ImColor(50, 50, 50, int(200 * menuAlpha * visibility));
                    ImColor unbindHoverColor = ImColor(70, 70, 70, int(200 * menuAlpha * visibility));
                    ImColor unbindFinalColor = isUnbindHovered ? unbindHoverColor : unbindBaseColor;

                    drawList->AddRectFilled(
                        ImVec2(unbindRect.x, unbindRect.y),
                        ImVec2(unbindRect.z, unbindRect.w),
                        unbindFinalColor,
                        4.0f
                    );

                    float centerX = (unbindRect.x + unbindRect.z) * 0.5f;
                    float centerY = (unbindRect.y + unbindRect.w) * 0.5f;
                    float iconSize = 6.0f;
                    float thickness = 1.5f;

                    ImColor xColor = ImColor(200, 200, 200, int(255 * menuAlpha * visibility));
                    drawList->AddLine(
                        ImVec2(centerX - iconSize, centerY - iconSize),
                        ImVec2(centerX + iconSize, centerY + iconSize),
                        xColor,
                        thickness
                    );
                    drawList->AddLine(
                        ImVec2(centerX + iconSize, centerY - iconSize),
                        ImVec2(centerX - iconSize, centerY + iconSize),
                        xColor,
                        thickness
                    );

                    if (isUnbindHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        mod->mKey = 0;
                        ClientInstance::get()->playUi("random.break", 0.75f, 1.0f);
                    }

                    if (keyBoxRippleAnimations[mod] > 0.0f) {
                        float rippleRadius = 50.0f * keyBoxRippleAnimations[mod];
                        ImVec2 rippleCenter = keyBoxRipplePositions[mod];
                        drawList->AddCircleFilled(
                            rippleCenter,
                            rippleRadius,
                            ImColor(255, 255, 255, int(50 * (1.0f - keyBoxRippleAnimations[mod]) * menuAlpha * visibility))
                        );
                        keyBoxRippleAnimations[mod] = std::min(1.0f, keyBoxRippleAnimations[mod] + deltaTime * 4.0f);
                    }

                    std::string keyName = currentlyBindingModule == mod ? "..." : Keyboard::getKey(mod->mKey);
                    ImVec2 keySize = ImGui::CalcTextSize(keyName.c_str());
                    drawList->AddText(
                        ImVec2(keyRect.x + (keyRect.z - keyRect.x - keySize.x) * 0.5f,
                            keyRect.y + (keyRect.w - keyRect.y - keySize.y) * 0.5f),
                        ImColor(170, 170, 170, int(255 * menuAlpha * visibility)),
                        keyName.c_str()
                    );

                    nextYOffset += (itemHeight + 5.0f) * visibility;
                }
            }

            drawList->PopClipRect();

            if (currentlyBindingModule) {
                bindingNotificationAlpha = std::min(1.0f, bindingNotificationAlpha + deltaTime * 12.0f);

                ImVec2 notifSize = ImGui::CalcTextSize("Press any key to bind...");
                float notifPadding = 20.0f;
                ImVec4 notifRect(
                    menuPos.x + (menuSize.x * menuScale - notifSize.x) * 0.5f - notifPadding,
                    menuPos.y + menuSize.y * menuScale - 40.0f,
                    menuPos.x + (menuSize.x * menuScale + notifSize.x) * 0.5f + notifPadding,
                    menuPos.y + menuSize.y * menuScale - 10.0f
                );

                drawList->AddRectFilled(
                    ImVec2(notifRect.x, notifRect.y),
                    ImVec2(notifRect.z, notifRect.w),
                    ImColor(40, 40, 40, int(200 * menuAlpha * bindingNotificationAlpha)),
                    6.0f
                );

                drawList->AddText(
                    ImVec2(notifRect.x + notifPadding, notifRect.y + (notifRect.w - notifRect.y - notifSize.y) * 0.5f),
                    ImColor(255, 255, 255, int(255 * menuAlpha * bindingNotificationAlpha)),
                    "Press any key to bind..."
                );

                for (const auto& key : Keyboard::mPressedKeys) {
                    if (key.second && currentlyBindingModule) {
                        if (key.first != VK_ESCAPE) {
                            currentlyBindingModule->mKey = key.first;
                            ClientInstance::get()->playUi("random.orb", 0.75f, 1.0f);
                        }
                        currentlyBindingModule = nullptr;
                        bindingNotificationAlpha = 0.0f;
                        break;
                    }
                }
            }
            else {
                bindingNotificationAlpha = std::max(0.0f, bindingNotificationAlpha - deltaTime * 15.0f);
            }

            ImGui::End();
        }
        else {
            currentlyBindingModule = nullptr;
            bindingNotificationAlpha = 0.0f;
        }
    }

    if (player && mForcePackSwitching.mValue)
    {
        patchFullStack(true);
    }
    else
    {
        patchFullStack(false);
    }

    if (player && !lastPlayerState)
    {
        usingPaip = false;
    }

    static constexpr float LERP_SPEED = 20.f;

    float yaw = MathUtils::wrap(pLerpedYaw, pYaw - 180, pYaw + 180);
    float headYaw = MathUtils::wrap(pLerpedHeadYaw, pHeadYaw - 180, pHeadYaw + 180);
    float pitch = pLerpedPitch;
    float bodyYaw = MathUtils::wrap(pLerpedBodyYaw, pBodyYaw - 180, pBodyYaw + 180);

    float preLerpedYaw = MathUtils::lerp(yaw, pYaw, deltaTime * LERP_SPEED);
    float preLerpedHeadYaw = MathUtils::lerp(headYaw, pHeadYaw, deltaTime * LERP_SPEED);
    float preLerpedPitch = MathUtils::lerp(pitch, pPitch, deltaTime * LERP_SPEED);
    float preLerpedBodyYaw = MathUtils::lerp(bodyYaw, pBodyYaw, deltaTime * LERP_SPEED);

    pLerpedYaw = MathUtils::wrap(pLerpedYaw, preLerpedYaw - 180, preLerpedYaw + 180);
    pLerpedHeadYaw = MathUtils::wrap(pLerpedHeadYaw, preLerpedHeadYaw - 180, preLerpedHeadYaw + 180);
    pLerpedBodyYaw = MathUtils::wrap(pLerpedBodyYaw, preLerpedBodyYaw - 180, preLerpedBodyYaw + 180);

    pLerpedYaw = MathUtils::lerp(preLerpedYaw, pLerpedYaw, deltaTime * LERP_SPEED);
    pLerpedHeadYaw = MathUtils::lerp(preLerpedHeadYaw, pLerpedHeadYaw, deltaTime * LERP_SPEED);
    pLerpedPitch = MathUtils::lerp(preLerpedPitch, pLerpedPitch, deltaTime * LERP_SPEED);
    pLerpedBodyYaw = MathUtils::lerp(preLerpedBodyYaw, pLerpedBodyYaw, deltaTime * LERP_SPEED);

    // Replace ImGuiKey_Tab logic with dynamic keybind
    static bool wasTabKeyDown = false;
    bool tabKeyDown = (mTabListKey.mKey != 0) && Keyboard::mPressedKeys[mTabListKey.mKey];
    if (tabKeyDown && !wasTabKeyDown) {
        isTabMenuOpen = true;
        tabListLoadIndex = 0;
        updateTabPlayerList();
    } else if (!tabKeyDown && wasTabKeyDown) {
        isTabMenuOpen = false;
    } else if (isTabMenuOpen && tabListLoadIndex < tabPlayerNames.size()) {
        // Continue loading chunks while open
        updateTabPlayerList();
    }
    wasTabKeyDown = tabKeyDown;
    float animSpeed = 8.0f;
    tabMenuAnim = isTabMenuOpen
        ? std::min(1.0f, tabMenuAnim + ImGui::GetIO().DeltaTime * animSpeed)
        : std::max(0.0f, tabMenuAnim - ImGui::GetIO().DeltaTime * animSpeed);
    if (tabMenuAnim > 0.01f)
        renderTabMenu();
}

void Interface::onActorRenderEvent(ActorRenderEvent& event)
{
    if (event.isCancelled()) return;
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    if (event.mEntity != player) return;
    if (*event.mPos == glm::vec3(0.f, 0.f, 0.f) && *event.mRot == glm::vec2(0.f, 0.f)) return;

    bool firstPerson = ClientInstance::get()->getOptions()->mThirdPerson->value == 0;
    if (firstPerson && !player->getFlag<RenderCameraComponent>()) return;

    const auto actorRotations = event.mEntity->getActorRotationComponent();
    const auto headRotations = event.mEntity->getActorHeadRotationComponent();
    const auto bodyRotations = event.mEntity->getMobBodyRotationComponent();
    if (!actorRotations || !headRotations || !bodyRotations) return;

    float realOldPitch = actorRotations->mOldPitch;
    float realPitch = actorRotations->mPitch;
    float realHeadRot = headRotations->mHeadRot;
    float realOldHeadRot = headRotations->mOldHeadRot;
    float realBodyYaw = bodyRotations->yBodyRot;
    float realOldBodyYaw = bodyRotations->yOldBodyRot;

    actorRotations->mOldPitch = pLerpedPitch;
    actorRotations->mPitch = pLerpedPitch;
    headRotations->mHeadRot = pLerpedHeadYaw;
    headRotations->mOldHeadRot = pLerpedHeadYaw;
    bodyRotations->yBodyRot = pLerpedBodyYaw;
    bodyRotations->yOldBodyRot = pLerpedBodyYaw;

    auto original = event.mDetour->getOriginal<&ActorRenderDispatcherHook::render>();
    original(event._this, event.mEntityRenderContext, event.mEntity, event.mCameraTargetPos, event.mPos, event.mRot, event.mIgnoreLighting);
    event.cancel();

    actorRotations->mOldPitch = realOldPitch;
    actorRotations->mPitch = realPitch;
    headRotations->mHeadRot = realHeadRot;
    headRotations->mOldHeadRot = realOldHeadRot;
    bodyRotations->yBodyRot = realBodyYaw;
    bodyRotations->yOldBodyRot = realOldBodyYaw;
}


void Interface::onDrawImageEvent(DrawImageEvent& event)
{
    /*if (event.mTexture && event.mTexture->mResourceLocation) {
        auto path = event.mTexture->getFilePath().c_str();
        if (strcmp(path, "textures/ui/selected_hotbar_slot") == 0) {
            float deltaTime = ImGui::GetIO().DeltaTime;
            if (hotbarPos.x == 0 || hotbarPos.y == 0) hotbarPos = *event.mPos;
            hotbarPos.x = MathUtils::lerp(hotbarPos.x, event.mPos->x, deltaTime * mSlotEasingSpeed.mValue);
            hotbarPos.y = event.mPos->y;
            *event.mPos = hotbarPos;
        }
    }*/
}

void Interface::onBaseTickEvent(BaseTickEvent& event)
{
    std::string screenName = ClientInstance::get()->getScreenName();

    auto player = ClientInstance::get()->getLocalPlayer();

    BodyYaw::updateRenderAngles(player, pYaw);
    pOldBodyYaw = pBodyYaw;
    pBodyYaw = BodyYaw::bodyYaw;
}

void Interface::onPacketOutEvent(PacketOutEvent& event)
{
    if (event.mPacket->getId() == PacketID::PlayerAuthInput) usingPaip = true;

    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;
    auto level = player->getLevel();
    if (!level) return;
    auto moveSettings = level->getPlayerMovementSettings();
    if (!moveSettings) return;
    bool isServerAuthoritative = usingPaip;

    if (event.mPacket->getId() == PacketID::PlayerAuthInput && isServerAuthoritative)
    {
        auto paip = event.getPacket<PlayerAuthInputPacket>();

        pOldYaw = pYaw;
        pOldPitch = pPitch;
        pOldBodyYaw = pBodyYaw;
        pYaw = paip->mRot.y;
        pPitch = paip->mRot.x;
        pHeadYaw = paip->mYHeadRot;
    }
    else if (event.mPacket->getId() == PacketID::MovePlayer && !isServerAuthoritative)
    {
        auto mpp = event.getPacket<MovePlayerPacket>();
        pOldYaw = pYaw;
        pOldPitch = pPitch;
        pOldBodyYaw = pBodyYaw;
        pYaw = mpp->mRot.y;
        pPitch = mpp->mRot.x;
        pHeadYaw = mpp->mYHeadRot;
    }
}

void Interface::onConnectionRequestEvent(ConnectionRequestEvent& event) {
    if (event.mServerAddress && !event.mServerAddress->empty()) {
        lastConnectedServerIP = *event.mServerAddress;
        tabMenuServerIP = lastConnectedServerIP;
    }
}