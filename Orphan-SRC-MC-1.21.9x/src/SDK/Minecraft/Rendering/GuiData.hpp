#pragma once
#include <string>
#include <Utils/MemUtils.hpp>


//
// Created by vastrakai on 6/25/2024.
//

class GuiData {
public:
    // unlikely to change
    CLASS_FIELD(glm::vec2, mResolution, 0x40);
    CLASS_FIELD(glm::vec2, mResolutionRounded, 0x48);
    CLASS_FIELD(glm::vec2, mResolutionScaled, 0x50);

    CLASS_FIELD(float, mGuiScale, 0x5C);
    CLASS_FIELD(float, mScalingMultiplier, 0x60);

    void displayClientMessageQueued(const std::string& msg);
    void displayClientMessage(const std::string& msg);
};
//I am going to be doing a walkthrough on how to get the offsets for the latest version of the game and versions to come.
//First off open up your ida with the 1.16 China version and the ida latest version of minecraft
//In this example my version is 1.21.8x for the latest version
//We are going to be looking for GuiData which is a really easy offset to find
//1. On the left hand side panel search for "GuiData" in the china version
//NOTE: It's really common for your ida to freeze on you so just be patient.
//2. Double click on the method and hit f5
//NOTE: You should be able to see all the function names for what you are looking for.
//3. So now you need to look for the offset your trying to make the offset for, for us its the mResolution so go into the 1.16 china
//NOTE: You should see totalscreensize so what you need to do to find it in the latest version is look for a string in the GuiData class in the china
//version you can scan for in the latest version
//For us its the "ui_invert_overlay" or you can scan for "ui_textured"
//4. So now go to the latest version in ida
//5. Hit View>Open Subviews>Strings then search for the string you found in the china version "ui_invert_overlay"
//NOTE: I hit x on that thing I highlighted to view the x references
//6. Scroll up to the general section of where those offsets we were looking for in the latest version from the china version.
//7. What you need to do now is compare the psuedocode to each other and look for similarities of where the thing we are looking for is
//8. As you saw I found the similarities between the two meaning the line "*(_QWORD *)(a1 + 64) = 0i64;" is the totalscreensize.
//9. Right click on the integer in the function "64" and click Hexadecimal
//10. this will equal your new offset for the latest version of the game which for me 1.21.8x is "0x40"
//11. You can repeat this for the other two mResolutionRounded, mResolutionScaled which are right below mResolution, mGuiScale etc.
// Thats the full tutorial :o enjoy.