//
// Created by vastrakai on 7/12/2024.
//

#pragma once
#include <SDK/SigManager.hpp>
#include <string>
#include <unordered_map>
#include <vector>
class AttributeCollection;
struct AttributeData;

enum AttributeId {
    ZombieSpawnReinforcementsChange = -1, // Add commentMore actions
    PlayerHunger = 1,
    PlayerSaturation = 2,
    PlayerExhaustion = 3,
    PlayerLevel = 4,
    PlayerExperience = 5,
    Health = 7,
    FollowRange = 8,
    KnockbackResistance = 9,
    MovementSpeed = 10,
    UnderwaterMovementSpeed = 11,
    LavaMovementSpeed = 12,
    AttackDamage = 13,
    Absorption = 14,
    Luck = 15,
    JumpStrength = 16, // For horses and mobs
};

class AttributeInstance {
PAD(0x60);

    union {
        float mDefaultValues[3];
        struct {
            float mDefaultMinValue;
            float mDefaultMaxValue;
            float mDefaultValue;
        };
    };

    union {
        float mCurrentValues[3];
        struct {
            float mCurrentMinValue;
            float mCurrentMaxValue;
            float mCurrentValue;
        };
    };

    virtual ~AttributeInstance();
    virtual void tick();
};

static_assert(sizeof(AttributeInstance) == 0x80, "AttributeInstance size is not correct");

class Attribute {
public:
    __int64 hash;
    __int64 hashedStringHash;
    std::string attributeName;
private:
    char __padding[0x32];
public:

    Attribute() {
        memset(this, 0x0, sizeof(Attribute));
    }

    Attribute(__int64 hash) {
        memset(this, 0x0, sizeof(Attribute));
        this->hash = hash;
    }
};

enum AttributeHashes : unsigned __int64 {
    HEALTH = 30064771328 - 4294967296,
    HUNGER = 8589934848 - 4294967296,
    MOVEMENT = 42949673217 - 4294967296,
    ABSORPTION = 60129542401 - 4294967296,
    SATURATION = 12884902144 - 4294967296,
    FOLLOW_RANGE = 34359738369 - 4294967296,
    LEVEL = 21474836736 - 4294967296,
    EXPERIENCE = 25769804032 - 4294967296
};

class HealthAttribute : public Attribute
{
public:
    HealthAttribute() { this->hash = AttributeHashes::HEALTH; }
};

class PlayerHungerAttribute : public Attribute
{
public:
    PlayerHungerAttribute() { this->hash = AttributeHashes::HUNGER; }
};

class MovementAttribute : public Attribute
{
public:
    MovementAttribute() { this->hash = AttributeHashes::MOVEMENT; }
};

class AbsorptionAttribute : public Attribute
{
public:
    AbsorptionAttribute() { this->hash = AttributeHashes::ABSORPTION; }
};

class PlayerSaturationAttribute : public Attribute
{
public:
    PlayerSaturationAttribute() { this->hash = AttributeHashes::SATURATION; }
};

class FollowRangeAttribute : public Attribute
{
public:
    FollowRangeAttribute() { this->hash = AttributeHashes::FOLLOW_RANGE; }
};

class PlayerLevelAttribute : public Attribute {
public:
    PlayerLevelAttribute() { this->hash = AttributeHashes::LEVEL; }
};

class PlayerExperienceAttribute : public Attribute
{
public:
    PlayerExperienceAttribute() { this->hash = AttributeHashes::EXPERIENCE; }
};

class BaseAttributeMap
{
public:
    std::unordered_map<int, AttributeInstance> mAttributes;
    //std::vector<uint64_t> mDirtyAttributes;
private:
PAD(0x20);
public:

    AttributeInstance* getInstance(unsigned int id)
    {
        return MemUtils::callFastcall<AttributeInstance*>(SigManager::BaseAttributeMap_getInstance, this, id);
    }
};

static_assert(sizeof(BaseAttributeMap) == 0x60);


struct AttributesComponent
{
    BaseAttributeMap mBaseAttributeMap;
};

static_assert(sizeof(AttributesComponent) == 0x60);