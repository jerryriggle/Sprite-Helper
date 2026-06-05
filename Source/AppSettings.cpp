#include "AppSettings.h"

static const char* kFontNameKey      = "fontName";
static const char* kFontSizeKey      = "fontSize";
static const char* kDarkModeKey      = "darkMode";
static const char* kRotationStepKey  = "rotationStep";

//==============================================================================
AppSettings& AppSettings::getInstance()
{
    static AppSettings instance;
    return instance;
}

void AppSettings::initialise()
{
    juce::PropertiesFile::Options opts;
    opts.applicationName     = "SpriteHelper";
    opts.filenameSuffix      = ".settings";
    opts.osxLibrarySubFolder = "Application Support";

    appProperties = std::make_unique<juce::ApplicationProperties>();
    appProperties->setStorageParameters (opts);

    auto* props = appProperties->getUserSettings();

    fontName     = props->getValue (kFontNameKey, juce::Font (juce::FontOptions{}).getTypefaceName());
    fontSize     = (float) props->getDoubleValue (kFontSizeKey, 14.0);
    darkMode     = props->getBoolValue (kDarkModeKey, false);
    rotationStep = (float) props->getDoubleValue (kRotationStepKey, 5.0);
}

void AppSettings::shutdown()
{
    appProperties.reset();
}

//==============================================================================
juce::Font AppSettings::getFont() const
{
    return juce::Font (juce::FontOptions{}
                           .withName (fontName)
                           .withHeight (fontSize));
}

void AppSettings::setFont (const juce::Font& font)
{
    fontName = font.getTypefaceName();
    fontSize = font.getHeight();

    if (auto* props = appProperties->getUserSettings())
    {
        props->setValue (kFontNameKey, fontName);
        props->setValue (kFontSizeKey, (double) fontSize);
        props->saveIfNeeded();
    }
    sendChangeMessage();
}

juce::String AppSettings::getFontName() const  { return fontName; }

void AppSettings::setFontByName (const juce::String& name)
{
    setFont (juce::Font (juce::FontOptions{}.withName (name).withHeight (fontSize)));
}

//==============================================================================
float AppSettings::getRotationStep() const { return rotationStep; }

void AppSettings::setRotationStep (float degrees)
{
    rotationStep = juce::jlimit (1.0f, 90.0f, degrees);

    if (auto* props = appProperties->getUserSettings())
    {
        props->setValue (kRotationStepKey, (double) rotationStep);
        props->saveIfNeeded();
    }
    sendChangeMessage();
}

//==============================================================================
bool AppSettings::isDarkMode() const { return darkMode; }

void AppSettings::setDarkMode (bool dark)
{
    darkMode = dark;

    if (auto* props = appProperties->getUserSettings())
    {
        props->setValue (kDarkModeKey, dark);
        props->saveIfNeeded();
    }
    sendChangeMessage();
}
