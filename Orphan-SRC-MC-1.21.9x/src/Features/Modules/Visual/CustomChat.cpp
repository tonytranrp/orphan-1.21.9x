#include "CustomChat.hpp"

#include <Features/Events/ChatEvent.hpp>
#include <Features/Events/PacketInEvent.hpp>
#include <SDK/Minecraft/ClientInstance.hpp>
#include <SDK/Minecraft/Network/Packets/TextPacket.hpp>
#include <Utils/MiscUtils/MathUtils.hpp>
#include <iostream>

void CustomChat::onEnable()
{
    gFeatureManager->mDispatcher->listen<PacketInEvent, &CustomChat::onPacketInEvent, nes::event_priority::ABSOLUTE_LAST>(this);
    gFeatureManager->mDispatcher->listen<RenderEvent, &CustomChat::onRenderEvent>(this);
}

void CustomChat::onDisable()
{
    gFeatureManager->mDispatcher->deafen<PacketInEvent, &CustomChat::onPacketInEvent>(this);
    gFeatureManager->mDispatcher->deafen<RenderEvent, &CustomChat::onRenderEvent>(this);
}

struct ParsedText {
    std::string text;
    ImU32 color;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikethrough = false;
};

enum class ChatColor : char {
    BLACK = '0',
    DARK_BLUE = '1',
    DARK_GREEN = '2',
    DARK_AQUA = '3',
    DARK_RED = '4',
    DARK_PURPLE = '5',
    GOLD = '6',
    GRAY = '7',
    DARK_GRAY = '8',
    BLUE = '9',
    GREEN = 'a',
    AQUA = 'b',
    RED = 'c',
    LIGHT_PURPLE = 'd',
    YELLOW = 'e',
    WHITE = 'f'
};

static const std::unordered_map<char, ImU32> colorMap = {
    {'0', IM_COL32(0, 0, 0, 255)},
    {'1', IM_COL32(0, 0, 170, 255)},
    {'2', IM_COL32(0, 170, 0, 255)},
    {'3', IM_COL32(0, 170, 170, 255)},
    {'4', IM_COL32(170, 0, 0, 255)},
    {'5', IM_COL32(170, 0, 170, 255)},
    {'6', IM_COL32(255, 170, 0, 255)},
    {'7', IM_COL32(170, 170, 170, 255)},
    {'8', IM_COL32(85, 85, 85, 255)},
    {'9', IM_COL32(85, 85, 255, 255)},
    {'a', IM_COL32(85, 255, 85, 255)},
    {'b', IM_COL32(85, 255, 255, 255)},
    {'c', IM_COL32(255, 85, 85, 255)},
    {'d', IM_COL32(255, 85, 255, 255)},
    {'e', IM_COL32(255, 255, 85, 255)},
    {'f', IM_COL32(255, 255, 255, 255)},
    {'r', IM_COL32(255, 255, 255, 255)},
};

static const std::unordered_map<char, std::string> formatMap = {
    {'l', "bold"},
    {'o', "italic"},
    {'n', "underline"},
    {'m', "strikethrough"},
    {'r', "reset"},
};

template <typename T>
ImU32 getColorValue(const std::unordered_map<char, ImU32>& map, char code) {
    auto it = map.find(code);
    if (it != map.end()) {
        return it->second;
    }
    return IM_COL32(255, 255, 255, 255);
}

struct ParsedMessage {

};

std::vector<ParsedText> parseMessage(const std::string& message) {
    std::vector<ParsedText> parsedText;
    ImU32 currentColor = IM_COL32(255, 255, 255, 255);
    std::string currentSegment;
    bool bold = false, italic = false, underline = false, strikethrough = false;

    std::cout << "[CustomChat] Parsing message: " << message << std::endl;
    std::cout << "[CustomChat] Raw bytes: ";
    for (unsigned char c : message) {
        std::cout << std::hex << (int)c << " ";
    }
    std::cout << std::dec << std::endl;

    for (size_t i = 0; i < message.length(); ) {

        if (i + 1 < message.length() &&
            static_cast<unsigned char>(message[i]) == 0xC2 &&
            static_cast<unsigned char>(message[i + 1]) == 0xA7) {

            if (!currentSegment.empty()) {
                ParsedText p;
                p.text = currentSegment;
                p.color = currentColor;
                p.bold = bold;
                p.italic = italic;
                p.underline = underline;
                p.strikethrough = strikethrough;
                parsedText.emplace_back(p);
                std::cout << "  Segment: '" << p.text << "' color: " << std::hex << p.color << std::dec << std::endl;
                currentSegment.clear();
            }

            if (i + 2 < message.length()) {
                char codeChar = std::tolower(static_cast<unsigned char>(message[i + 2]));
                if (colorMap.find(codeChar) != colorMap.end()) {
                    currentColor = colorMap.at(codeChar);
                    bold = italic = underline = strikethrough = false;
                    i += 3;
                }
                else if (formatMap.find(codeChar) != formatMap.end()) {
                    if (codeChar == 'l') bold = true;
                    else if (codeChar == 'o') italic = true;
                    else if (codeChar == 'n') underline = true;
                    else if (codeChar == 'm') strikethrough = true;
                    else if (codeChar == 'r') {
                        currentColor = IM_COL32(255, 255, 255, 255);
                        bold = italic = underline = strikethrough = false;
                    }
                    i += 3;
                }
                else {

                    currentSegment += '\xC2';
                    currentSegment += '\xA7';
                    currentSegment += message[i + 2];
                    i += 3;
                }
            }
            else {

                currentSegment += '\xC2';
                currentSegment += '\xA7';
                i += 2;
            }
        }
        else {
            currentSegment += message[i];
            i++;
        }
    }
    return parsedText;
}

void CustomChat::addMessage(std::string message) {
    ChatMessage chatMessage;
    chatMessage.mText = message;
    chatMessage.mLifeTime = mMaxLifeTime.as<float>();
    chatMessage.mTime = std::chrono::system_clock::now();
    chatMessage.mPercent = 0.f;
    mMessages.push_back(chatMessage);
}

void CustomChat::onRenderEvent(RenderEvent& event)
{
    FontHelper::pushPrefFont();
    auto drawList = ImGui::GetBackgroundDrawList();
    const auto delta = ImGui::GetIO().DeltaTime;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImVec2 windowSize = { 600, 450 };
    ImVec2 windowPos = { 10, displaySize.y - 85.f };
    float rounding = 15.f;
    float totalHeight = 0.f;
    float fontSize = 20.0f;

    static float easedHeight = 0.f;
    static float maxHeight = 0.f;
    ImRect rect = ImVec4(windowPos.x, windowPos.y - 10.f, windowPos.x + windowSize.x, windowPos.y - 10 - easedHeight);
    ImRect flipped = ImVec4(rect.Min.x, rect.Max.y, rect.Max.x, rect.Min.y);

    if (rect.Min.y - rect.Max.y >= 1)
    {
        ImRenderUtils::addBlur(rect.ToVec4(), 3, rounding);
        drawList->AddRectFilled(flipped.Min, flipped.Max, IM_COL32(0, 0, 0, 200), rounding);
        drawList->PushClipRect({ rect.Min.x, rect.Max.y }, { rect.Max.x, rect.Min.y });

        auto fontHeight = ImGui::GetFont()->CalcTextSizeA(
            fontSize,
            FLT_MAX,
            0,
            ""
        ).y;

        ImVec2 cursorPos = { windowPos.x + 10, windowPos.y - 10.f };
        auto now = std::chrono::system_clock::now();
        auto isInGyat = ClientInstance::get()->getScreenName() == "chat_screen";

        for (auto it = mMessages.rbegin(); it != mMessages.rend(); ) {
            float elapsed = std::chrono::duration<float>(now - it->mTime).count();
            bool hasElapsed = elapsed >= it->mLifeTime;

            if (hasElapsed) {
                it->mPercent -= delta * 2.5f;
                if (easedHeight > cursorPos.y) {
                    it = std::reverse_iterator(mMessages.erase((++it).base()));
                    continue;
                }
            }
            else {
                it->mPercent = MathUtils::lerp(it->mPercent, 1.f, delta * 8.f);
            }
            it->mPercent = std::clamp(it->mPercent, 0.f, 1.f);

            if (it->mPercent > 0.0f) {
                cursorPos.y = MathUtils::lerp(cursorPos.y, cursorPos.y - fontHeight - 5.0f, hasElapsed ? 1.f : it->mPercent);
                int alpha = static_cast<int>(255 * (!hasElapsed ? it->mPercent : 1.f));

                auto segments = parseMessage(it->mText);
                ImVec2 segCursor = cursorPos;
                for (const auto& seg : segments) {
                    ImU32 segColor = (seg.color & 0x00FFFFFF) | (alpha << 24);

                    drawList->AddText(
                        ImGui::GetFont(),
                        fontSize,
                        segCursor,
                        segColor,
                        seg.text.c_str()
                    );

                    if (seg.underline) {
                        ImVec2 textSize = ImGui::CalcTextSize(seg.text.c_str());
                        drawList->AddLine(
                            { segCursor.x, segCursor.y + fontSize + 1 },
                            { segCursor.x + textSize.x, segCursor.y + fontSize + 1 },
                            segColor, 1.5f
                        );
                    }

                    if (seg.strikethrough) {
                        ImVec2 textSize = ImGui::CalcTextSize(seg.text.c_str());
                        drawList->AddLine(
                            { segCursor.x, segCursor.y + fontSize / 2 },
                            { segCursor.x + textSize.x, segCursor.y + fontSize / 2 },
                            segColor, 1.5f
                        );
                    }
                    segCursor.x += ImGui::CalcTextSize(seg.text.c_str()).x;
                }

                if (it->mCount > 1) {
                    std::string countText = " x" + std::to_string(it->mCount);
                    drawList->AddText(
                        ImGui::GetFont(),
                        fontSize,
                        { cursorPos.x + ImGui::CalcTextSize(it->mText.c_str()).x + 5.0f, cursorPos.y },
                        IM_COL32(170, 170, 170, alpha),
                        countText.c_str()
                    );
                }
            }
            if (!hasElapsed) {
                if (mMessages.size() < 12) {
                    totalHeight += (fontHeight + 5.0f);
                    maxHeight = totalHeight;
                }
                else {
                    totalHeight = maxHeight;
                }
            }
            ++it;
        }

        easedHeight = MathUtils::lerp(easedHeight, isInGyat ? (fontHeight + 5.0f) * 12 : totalHeight, delta * 8.f);
        drawList->PopClipRect();
        FontHelper::popPrefFont();
    }
    else {
        bool isInChatScreen = ClientInstance::get()->getScreenName() == "chat_screen";
        auto fontHeight = ImGui::GetFont()->CalcTextSizeA(
            fontSize,
            FLT_MAX,
            0,
            ""
        ).y;

        easedHeight = MathUtils::lerp(easedHeight, isInChatScreen ? (fontHeight + 5.0f) * 12 : totalHeight, delta * 8.f);
        FontHelper::popPrefFont();
    }
}

void CustomChat::onPacketInEvent(PacketInEvent& event)
{
    if (event.isCancelled()) return;
    if (event.mPacket->getId() != PacketID::Text) return;
    auto player = ClientInstance::get()->getLocalPlayer();
    if (!player) return;

    auto textPacket = event.getPacket<TextPacket>();
    std::string message = textPacket->mMessage;

    if (textPacket->mType == TextPacketType::Chat) {

        message = textPacket->mAuthor.empty() ? message : "<" + textPacket->mAuthor + "> " + message;
    }

    if (!mMessages.empty() && mMessages.back().mText == message) {
        mMessages.back().mCount++;
        mMessages.back().mCount++;
        mMessages.back().mTime = std::chrono::system_clock::now();
        mMessages.back().mLifeTime = mMaxLifeTime.as<float>();
        mMessages.back().mPercent = 0.f;
        mCachedMessages.push_back(mMessages.back());
        return;
    }

    ChatMessage chatMessage;
    chatMessage.mText = message;
    chatMessage.mLifeTime = mMaxLifeTime.as<float>();
    chatMessage.mTime = std::chrono::system_clock::now();
    chatMessage.mPercent = 0.f;
    chatMessage.mCount = 1;

    mMessages.push_back(chatMessage);
    mCachedMessages.push_back(chatMessage);
}