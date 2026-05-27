#pragma once
#include <JuceHeader.h>

//==============================================================================
/** LookAndFeel subclass that supports dark and light mode, honouring the
    AppSettings::isDarkMode() flag.  Apply to the top-level component so that
    all children inherit the colour scheme.                                    */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    /** Re-applies colour scheme from AppSettings (call after toggling dark mode). */
    void applyTheme();

    // ── Custom colours ────────────────────────────────────────────────────────
    static const juce::Colour darkBg;
    static const juce::Colour darkPanel;
    static const juce::Colour darkText;
    static const juce::Colour darkAccent;

    static const juce::Colour lightBg;
    static const juce::Colour lightPanel;
    static const juce::Colour lightText;
    static const juce::Colour lightAccent;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomLookAndFeel)
};
