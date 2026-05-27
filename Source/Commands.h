#pragma once
#include <JuceHeader.h>

namespace CommandIDs
{
    // Sprite Helper menu
    inline constexpr juce::CommandID openSettings      = 0x2001;

    // File menu
    inline constexpr juce::CommandID loadImage         = 0x2010;
    inline constexpr juce::CommandID saveImage         = 0x2011;
    inline constexpr juce::CommandID loadSpritesheet   = 0x2012;
    inline constexpr juce::CommandID exportSpritesheet = 0x2013;

    // Edit menu
    inline constexpr juce::CommandID scaleFit          = 0x2020;
    inline constexpr juce::CommandID scaleKeep         = 0x2021;
    inline constexpr juce::CommandID centerImage       = 0x2022;
    inline constexpr juce::CommandID setMargin         = 0x2023;
    inline constexpr juce::CommandID rotateLeft        = 0x2024;
    inline constexpr juce::CommandID rotateRight       = 0x2025;
    inline constexpr juce::CommandID removeBackground  = 0x2026;

    // Project menu
    inline constexpr juce::CommandID openProject       = 0x2030;
    inline constexpr juce::CommandID newProject        = 0x2031;
    inline constexpr juce::CommandID saveProject       = 0x2032;
    inline constexpr juce::CommandID setScale          = 0x2033;
    inline constexpr juce::CommandID setSpritesheetSize= 0x2034;
    inline constexpr juce::CommandID previewAnimation  = 0x2035;

    // View menu
    inline constexpr juce::CommandID viewRefine        = 0x2040;
    inline constexpr juce::CommandID viewCompile       = 0x2041;
    inline constexpr juce::CommandID viewSplit         = 0x2042;

    // Help menu
    inline constexpr juce::CommandID openDocumentation = 0x2050;
}
