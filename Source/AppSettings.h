#pragma once
#include <JuceHeader.h>

//==============================================================================
/** Manages persistent application-level settings (font, dark mode).
    Single instance accessed via getInstance(). Call initialise() once at
    startup before using any other method.                                    */
class AppSettings : public juce::ChangeBroadcaster
{
public:
    static AppSettings& getInstance();

    /** Must be called once during application startup. */
    void initialise();

    /** Must be called once during application shutdown. */
    void shutdown();

    // ── Font ─────────────────────────────────────────────────────────────────
    juce::Font getFont() const;
    void       setFont (const juce::Font& font);

    juce::String getFontName() const;
    void         setFontByName (const juce::String& name);

    // ── Dark mode ─────────────────────────────────────────────────────────────
    bool isDarkMode() const;
    void setDarkMode (bool dark);

private:
    AppSettings() = default;

    std::unique_ptr<juce::ApplicationProperties> appProperties;

    juce::String fontName;
    float        fontSize { 14.0f };
    bool         darkMode { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppSettings)
};
