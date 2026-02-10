//
// Created by alteik on 01/09/2024.
//
#pragma once
#include <Features/Modules/Module.hpp>

#include <Features/Events/RenderEvent.hpp>
#include <SDK/Minecraft/Actor/Actor.hpp>

class Encaser : public ModuleBase<Encaser> {
public:
    enum class Mode {
        //Encase,
        Penis,
        Threeline,
        Text
    };

    enum class SwitchMode {
        None,
        Full,
        Spoof
    };

    NumberSetting mBlocksPerTick = NumberSetting("Places Per Tick", "Maximum blocks to place per tick", 1.f, 1.f, 10.f, 1.f);
    NumberSetting mRange = NumberSetting("Range", "Maximum range to target players", 5.f, 1.f, 10.f, 0.5f);
    BoolSetting mRotate = BoolSetting("Rotate", "Rotate to place positions", true);
    BoolSetting mToggle = BoolSetting("Toggle", "Automatically disable when done", true);
    //BoolSetting mDebug = BoolSetting("Debug", "Show debug information", false);
    EnumSettingT<Mode> mMode = EnumSettingT<Mode>("Mode", "The mode to use", Mode::Penis, "Penis", "Threeline", "Text");
    EnumSettingT<SwitchMode> mSwitchMode = EnumSettingT<SwitchMode>("Switch Mode", "The mode for block switching", SwitchMode::Full, "None", "Full", "Spoof");

    Encaser() : ModuleBase("Builder", "Builds stuff really fast", ModuleCategory::Player, 0, false) {
        addSettings(&mBlocksPerTick);
        addSettings(&mRange);
        addSettings(&mRotate);
        addSettings(&mToggle);
        //addSettings(&mDebug);
        addSettings(&mMode);
        addSettings(&mSwitchMode);

        mNames = {
                {Lowercase, "builder"},
                {LowercaseSpaced, "builder"},
                {Normal, "Builder"},
                {NormalSpaced, "Builder"},
        };
    }

    static AABB mTargetedAABB;
    static bool mRotating;
    static bool isTargetTrapped;
    static glm::vec3 mBuildPos;
    static bool mShouldRotate;
    static glm::vec3 mLastBlock;
    static int mLastFace;
    static float mPlaceTimer;
    static int64_t mLastSwitchTime;
    static int placeIndex;
    static std::string mTextToBuild;
    static glm::vec3 mBuildStartPos; 
    static bool mBuildInProgress;     
    static int mLastSlot;  

    void onEnable() override;
    void onDisable() override;
    void onBaseTickEvent(class BaseTickEvent& event);
    void onPacketOutEvent(class PacketOutEvent& event);
    void onPacketInEvent(class PacketInEvent& event);
    void onRenderEvent(RenderEvent& event);

private:
    Actor* getClosestActor();
    std::vector<glm::vec3> getCollidingBlocks();
    std::vector<glm::vec3> getSurroundingBlocks(bool airOnly = true);
    glm::vec3 getNextPlacePos();
    std::vector<glm::vec3> pattern;
    glm::vec3 mStoredBasePos = { INT_MAX, INT_MAX, INT_MAX };

    // Character patterns for text building
    struct CharacterPattern {
        std::vector<glm::vec3> blocks;  // Block positions relative to base position
        int width;  // Width of the character
    };

    static std::unordered_map<char, CharacterPattern> characterPatterns;
    void initializeCharacterPatterns() {
        // Lowercase a-m
        characterPatterns['a'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0},  // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Sides
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0}, {3, 2, 0},  // Middle
                {0, 3, 0}, {3, 3, 0},                         // Sides
            },
            4
        };

        characterPatterns['b'] = {
            {
                {0, 0, 0}, {0, 1, 0}, {0, 2, 0}, {0, 3, 0},  // Left side
                {1, 0, 0}, {2, 0, 0},                         // Bottom
                {1, 2, 0}, {2, 2, 0},                         // Middle
                {3, 1, 0}, {3, 3, 0},                         // Right side curves
            },
            4
        };

        characterPatterns['c'] = {
            {
                {1, 0, 0}, {2, 0, 0},                         // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Sides
                {0, 2, 0},                                    // Left side
                {1, 3, 0}, {2, 3, 0},                         // Top
            },
            4
        };

        characterPatterns['d'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0},             // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Sides
                {0, 2, 0}, {3, 2, 0},                         // Sides
                {0, 3, 0}, {1, 3, 0}, {2, 3, 0},             // Top
            },
            4
        };

        characterPatterns['e'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0},             // Bottom base
                {0, 1, 0},                                    // Left side
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},             // Middle
                {0, 3, 0},                                    // Left side
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0},             // Top
            },
            3
        };

        characterPatterns['f'] = {
            {
                {0, 0, 0},                                    // Bottom
                {0, 1, 0},                                    // Left side
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},             // Middle
                {0, 3, 0},                                    // Left side
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0},             // Top
            },
            3
        };

        characterPatterns['g'] = {
            {
                {1, 0, 0}, {2, 0, 0},                         // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Sides
                {0, 2, 0}, {2, 2, 0}, {3, 2, 0},             // Middle with tail
                {0, 3, 0}, {3, 3, 0},                         // Sides
                {1, 4, 0}, {2, 4, 0},                         // Top
            },
            4
        };

        characterPatterns['h'] = {
            {
                {0, 0, 0}, {3, 0, 0},                         // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Sides
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0}, {3, 2, 0},  // Middle
                {0, 3, 0}, {3, 3, 0},                         // Sides
                {0, 4, 0}, {3, 4, 0},                         // Top
            },
            4
        };

        characterPatterns['i'] = {
            {
                {1, 0, 0},                                    // Bottom
                {1, 1, 0},                                    // Middle
                {1, 2, 0},                                    // Middle
                {1, 3, 0},                                    // Middle
                {1, 4, 0},                                    // Top
            },
            3
        };

        characterPatterns['j'] = {
            {
                {0, 0, 0}, {1, 0, 0},                         // Bottom
                {2, 1, 0},                                    // Right side
                {2, 2, 0},                                    // Right side
                {2, 3, 0},                                    // Right side
                {2, 4, 0},                                    // Top
            },
            3
        };

        characterPatterns['k'] = {
            {
                {0, 0, 0}, {3, 0, 0},                        // Bottom
                {0, 1, 0}, {2, 1, 0}, {3, 1, 0},            // Connected from left
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},            // Connected from left
                {0, 3, 0}, {2, 3, 0}, {3, 3, 0},            // Connected from left
                {0, 4, 0}, {3, 4, 0},                        // Top
            },
            4
        };

        characterPatterns['l'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0},             // Bottom
                {0, 1, 0},                                    // Left side
                {0, 2, 0},                                    // Left side
                {0, 3, 0},                                    // Left side
                {0, 4, 0},                                    // Top
            },
            3
        };

        characterPatterns['m'] = {
            {
                {0, 0, 0}, {4, 0, 0},                         // Bottom
                {0, 1, 0}, {2, 1, 0}, {4, 1, 0},             // Posts
                {0, 2, 0}, {2, 2, 0}, {4, 2, 0},             // Posts
                {0, 3, 0}, {1, 3, 0}, {2, 3, 0}, {3, 3, 0}, {4, 3, 0},  // Top
            },
            5
        };

        // Uppercase A-M
        characterPatterns['A'] = {
            {
                {0, 0, 0}, {4, 0, 0},                         // Bottom
                {0, 1, 0}, {4, 1, 0},                         // Posts
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0}, {3, 2, 0}, {4, 2, 0},  // Middle
                {0, 3, 0}, {4, 3, 0},                         // Posts
                {0, 4, 0}, {4, 4, 0},                         // Top
            },
            5
        };

        characterPatterns['B'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0},             // Bottom
                {0, 1, 0}, {3, 1, 0},                                    // Posts
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},                        // Middle
                {0, 3, 0}, {3, 3, 0},                                    // Posts
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0}, {3, 4, 0},             // Top
            },
            4
        };

        // Lowercase n-z
        characterPatterns['n'] = {
            {
                {0, 0, 0}, {3, 0, 0},                         // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Posts
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0}, {3, 2, 0},  // Middle
                {0, 3, 0}, {3, 3, 0},                         // Posts
            },
            4
        };

        characterPatterns['o'] = {
            {
                {1, 0, 0}, {2, 0, 0},                         // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Sides
                {0, 2, 0}, {3, 2, 0},                         // Sides
                {1, 3, 0}, {2, 3, 0},                         // Top
            },
            4
        };

        characterPatterns['p'] = {
            {
                {0, 0, 0},                                    // Bottom
                {0, 1, 0},                                    // Left side
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},             // Middle
                {0, 3, 0}, {3, 3, 0},                         // Sides
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0},             // Top
            },
            4
        };

        characterPatterns['q'] = {
            {
                {1, 0, 0}, {2, 0, 0}, {3, 0, 0},             // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Sides
                {0, 2, 0}, {3, 2, 0},                         // Sides
                {1, 3, 0}, {2, 3, 0}, {3, 3, 0},             // Top
            },
            4
        };

        characterPatterns['r'] = {
            {
                {0, 0, 0}, {3, 0, 0},                         // Bottom
                {0, 1, 0}, {2, 1, 0},                         // Diagonal
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},             // Middle
                {0, 3, 0}, {3, 3, 0},                         // Sides
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0},             // Top
            },
            4
        };

        characterPatterns['s'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0},             // Bottom base
                {2, 1, 0},                                    // Right side
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},             // Middle (connected)
                {0, 3, 0},                                    // Left side
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0},             // Top
            },
            3
        };

        characterPatterns['t'] = {
            {
                {1, 0, 0},                                    // Bottom
                {1, 1, 0},                                    // Middle
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},             // Cross
                {1, 3, 0},                                    // Middle
                {1, 4, 0},                                    // Top
            },
            3
        };

        characterPatterns['u'] = {
            {
                {0, 0, 0}, {3, 0, 0},                         // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Sides
                {0, 2, 0}, {3, 2, 0},                         // Sides
                {1, 3, 0}, {2, 3, 0},                         // Top curve
            },
            4
        };

        characterPatterns['v'] = {
            {
                {2, 0, 0},                                    // Bottom point
                {1, 1, 0}, {3, 1, 0},                         // Diagonal
                {1, 2, 0}, {3, 2, 0},                         // Diagonal
                {0, 3, 0}, {4, 3, 0},                         // Top spread
            },
            5
        };

        characterPatterns['w'] = {
            {
                {0, 0, 0}, {4, 0, 0},                         // Bottom
                {0, 1, 0}, {2, 1, 0}, {4, 1, 0},             // Triple posts
                {0, 2, 0}, {2, 2, 0}, {4, 2, 0},             // Triple posts
                {1, 3, 0}, {3, 3, 0},                         // Top curves
            },
            5
        };

        characterPatterns['x'] = {
            {
                {0, 0, 0}, {3, 0, 0},                         // Bottom spread
                {1, 1, 0}, {2, 1, 0},                         // Cross
                {1, 2, 0}, {2, 2, 0},                         // Cross
                {0, 3, 0}, {3, 3, 0},                         // Top spread
            },
            4
        };

        characterPatterns['y'] = {
            {
                {0, 3, 0}, {2, 3, 0},                        // Top spread
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},            // Connected middle layer
                {1, 1, 0},                                   // Lower center
                {1, 0, 0},                                   // Bottom center
            },
            3
        };

        characterPatterns['z'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0},             // Bottom
                {2, 1, 0},                                    // Diagonal
                {1, 2, 0},                                    // Middle
                {0, 3, 0},                                    // Diagonal
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0},             // Top
            },
            3
        };

        // Uppercase N-Z
        characterPatterns['N'] = {
            {
                {0, 0, 0}, {4, 0, 0},                         // Bottom
                {0, 1, 0}, {1, 1, 0}, {4, 1, 0},             // Diagonal
                {0, 2, 0}, {2, 2, 0}, {4, 2, 0},             // Middle
                {0, 3, 0}, {3, 3, 0}, {4, 3, 0},             // Diagonal
                {0, 4, 0}, {4, 4, 0},                         // Top
            },
            5
        };

        characterPatterns['O'] = {
            {
                {1, 0, 0}, {2, 0, 0}, {3, 0, 0},             // Bottom
                {0, 1, 0}, {4, 1, 0},                         // Sides
                {0, 2, 0}, {4, 2, 0},                         // Sides
                {0, 3, 0}, {4, 3, 0},                         // Sides
                {1, 4, 0}, {2, 4, 0}, {3, 4, 0},             // Top
            },
            5
        };

        characterPatterns['P'] = {
            {
                {0, 0, 0},                                    // Bottom
                {0, 1, 0},                                    // Left side
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0}, {3, 2, 0},  // Middle
                {0, 3, 0}, {4, 3, 0},                         // Posts
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0}, {3, 4, 0},  // Top
            },
            5
        };

        characterPatterns['Q'] = {
            {
                {1, 0, 0}, {2, 0, 0}, {3, 0, 0},             // Bottom with tail
                {0, 1, 0}, {4, 1, 0},                         // Sides
                {0, 2, 0}, {2, 2, 0}, {4, 2, 0},             // Middle with dot
                {0, 3, 0}, {4, 3, 0},                         // Sides
                {1, 4, 0}, {2, 4, 0}, {3, 4, 0},             // Top
            },
            5
        };

        characterPatterns['R'] = {
            {
                {0, 0, 0}, {4, 0, 0},                         // Bottom
                {0, 1, 0}, {3, 1, 0},                         // Diagonal
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},             // Middle
                {0, 3, 0}, {4, 3, 0},                         // Posts
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0}, {3, 4, 0},  // Top
            },
            5
        };

        characterPatterns['S'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}, // Bottom base
                {0, 1, 0},                                    // Left side
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},            // Middle
                {3, 3, 0},                                    // Right side
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0}, {3, 4, 0}, // Top
            },
            4
        };

        characterPatterns['T'] = {
            {
                {2, 0, 0},                                    // Bottom
                {2, 1, 0},                                    // Middle
                {2, 2, 0},                                    // Middle
                {2, 3, 0},                                    // Middle
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0}, {3, 4, 0}, {4, 4, 0},  // Top
            },
            5
        };

        characterPatterns['U'] = {
            {
                {1, 0, 0}, {2, 0, 0}, {3, 0, 0},             // Bottom
                {0, 1, 0}, {4, 1, 0},                         // Sides
                {0, 2, 0}, {4, 2, 0},                         // Sides
                {0, 3, 0}, {4, 3, 0},                         // Sides
                {0, 4, 0}, {4, 4, 0},                         // Top
            },
            5
        };

        characterPatterns['V'] = {
            {
                {2, 0, 0},                                    // Bottom point
                {1, 1, 0}, {3, 1, 0},                         // Diagonal
                {1, 2, 0}, {3, 2, 0},                         // Diagonal
                {0, 3, 0}, {4, 3, 0},                         // Spread
                {0, 4, 0}, {4, 4, 0},                         // Top
            },
            5
        };

        characterPatterns['W'] = {
            {
                {0, 0, 0}, {2, 0, 0}, {4, 0, 0},             // Bottom posts
                {0, 1, 0}, {2, 1, 0}, {4, 1, 0},             // Posts
                {0, 2, 0}, {2, 2, 0}, {4, 2, 0},             // Posts
                {1, 3, 0}, {3, 3, 0},                         // Inner peaks
                {0, 4, 0}, {4, 4, 0},                         // Outer posts
            },
            5
        };

        characterPatterns['X'] = {
            {
                {0, 0, 0}, {4, 0, 0},                         // Bottom
                {1, 1, 0}, {3, 1, 0},                         // Diagonal in
                {2, 2, 0},                                    // Center
                {1, 3, 0}, {3, 3, 0},                         // Diagonal out
                {0, 4, 0}, {4, 4, 0},                         // Top
            },
            5
        };

        characterPatterns['Y'] = {
            {
                {2, 0, 0},                                    // Bottom
                {2, 1, 0},                                    // Middle
                {2, 2, 0},                                    // Middle
                {1, 3, 0}, {3, 3, 0},                         // Fork
                {0, 4, 0}, {4, 4, 0},                         // Top
            },
            5
        };

        characterPatterns['Z'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}, {4, 0, 0},  // Bottom
                {3, 1, 0},                                    // Diagonal
                {2, 2, 0},                                    // Middle
                {1, 3, 0},                                    // Diagonal
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0}, {3, 4, 0}, {4, 4, 0},  // Top
            },
            5
        };

        // Special characters
        characterPatterns[' '] = {
            {
                // Empty pattern for space
            },
            3  // Width of space
        };

        // Fix uppercase K for consistency
        characterPatterns['K'] = {
            {
                {0, 0, 0},                                    // Base
                {0, 1, 0},                                    // Vertical stem
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},            // Middle connection
                {0, 3, 0}, {2, 3, 0},                        // Upper spread
                {0, 4, 0}, {3, 4, 0},                        // Top spread
            },
            4
        };

        // Fix uppercase S for consistency
        characterPatterns['S'] = {
            {
                {0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}, // Bottom base
                {0, 1, 0},                                    // Left side
                {0, 2, 0}, {1, 2, 0}, {2, 2, 0},            // Middle
                {3, 3, 0},                                    // Right side
                {0, 4, 0}, {1, 4, 0}, {2, 4, 0}, {3, 4, 0}, // Top
            },
            4
        };
    }

    // Helper function to get character pattern
    const CharacterPattern& getCharacterPattern(char c) {
        static bool initialized = false;
        if (!initialized) {
            initializeCharacterPatterns();
            initialized = true;
        }
        
        auto it = characterPatterns.find(c);
        if (it != characterPatterns.end()) {
            return it->second;
        }
        
        // Return a default empty pattern if character not found
        static const CharacterPattern emptyPattern = {{}, 3};
        return emptyPattern;
    }

    // Helper function to convert text to block pattern
    void textToPattern(const std::string& text, const glm::vec3& startPos) {
        pattern.clear();
        int currentX = 0;
        
        for (char c : text) {
            const auto& charPattern = getCharacterPattern(c);
            for (const auto& offset : charPattern.blocks) {
                pattern.push_back({
                    startPos.x + offset.x + currentX,
                    startPos.y + offset.y,
                    startPos.z + offset.z
                });
            }
            currentX += charPattern.width + 1;  // Add 1 block gap between characters
        }
    }

    bool isInRange(const glm::vec3& pos) {
        auto player = ClientInstance::get()->getLocalPlayer();
        if (!player) return false;
        
        glm::vec3 playerPos = *player->getPos();
        float dx = playerPos.x - pos.x;
        float dy = playerPos.y - pos.y;
        float dz = playerPos.z - pos.z;
        
        return (dx * dx + dy * dy + dz * dz) <= (mRange.mValue * mRange.mValue);
    }
};