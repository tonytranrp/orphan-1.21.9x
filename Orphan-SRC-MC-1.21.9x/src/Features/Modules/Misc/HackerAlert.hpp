#pragma once
//
// Created by rekitrelt (2/2/2025)
//


class HackerAlert : public ModuleBase<HackerAlert>
{
public:
    BoolSetting mShowNotifications = BoolSetting("Show Notifications", "Show notifications when a hacker/enemy player joins or leaves", true);
    BoolSetting mPlaySound = BoolSetting("Play Sound", "Plays a sound when a hacker/enemy player joins", true);

    HackerAlert() : ModuleBase("HackerAlert", "Automatically detects hackers/enemies from a list", ModuleCategory::Misc, 0, false)
    {
        addSettings(
            &mShowNotifications,
            &mPlaySound
        );

        mNames = {
            {Lowercase, "hackeralert"},
            {LowercaseSpaced, "hacker alert"},
            {Normal, "HackerAlert"},
            {NormalSpaced, "Hacker Alert"}
        };
    }

    class PlayerInfo : public DataObject
    {
    public:
        std::string name = "";
        std::string rank = "";
        int64_t first_played;
        uint64_t storedAt = 0;

        PlayerInfo(const std::string& name, const std::string& rank, int64_t firstPlayed)
        : name(name), rank(rank), first_played(firstPlayed) {}

        int64_t getFirstJoined() const { return first_played; }

        nlohmann::json toJson() override
        {
            nlohmann::json json;
            json["name"] = name;
            json["rank"] = rank;
            json["storedAt"] = storedAt;
            return json;
        }

        void fromJson(nlohmann::json json) override
        {
            name = json["name"];
            rank = json["rank"];
            storedAt = json["storedAt"];
        }

        PlayerInfo() = default;
    };

    struct PlayerEvent
    {
        enum class Type {
            JOIN,
            LEAVE
        };

        Type type;
        std::string name{};

        PlayerEvent(const Type type, const std::string& name) : type(type), name(name) {}
    };

    std::vector<std::string> cachedHackerList = {};
    std::unordered_set<std::string> warnedPlayers;

    static bool isHackerName(const std::string& playerName, const std::string& hackerName);
    void checkForHackers(const std::vector<std::string>& playerNames);
    void updateHackerList();
    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketInEvent(class PacketInEvent& event);
};