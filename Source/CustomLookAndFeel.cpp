#include "CustomLookAndFeel.h"
#include "AppSettings.h"

//==============================================================================
const juce::Colour CustomLookAndFeel::darkBg     { 0xff1e1e2e };
const juce::Colour CustomLookAndFeel::darkPanel  { 0xff2a2a3a };
const juce::Colour CustomLookAndFeel::darkText   { 0xffcdd6f4 };
const juce::Colour CustomLookAndFeel::darkAccent { 0xff89b4fa };

const juce::Colour CustomLookAndFeel::lightBg    { 0xfff0f0f0 };
const juce::Colour CustomLookAndFeel::lightPanel { 0xffffffff };
const juce::Colour CustomLookAndFeel::lightText  { 0xff1e1e2e };
const juce::Colour CustomLookAndFeel::lightAccent{ 0xff1d6ae5 };

//==============================================================================
CustomLookAndFeel::CustomLookAndFeel()
{
    applyTheme();
}

void CustomLookAndFeel::applyTheme()
{
    const bool dark = AppSettings::getInstance().isDarkMode();

    juce::Colour bg      = dark ? darkBg     : lightBg;
    juce::Colour panel   = dark ? darkPanel  : lightPanel;
    juce::Colour text    = dark ? darkText   : lightText;
    juce::Colour accent  = dark ? darkAccent : lightAccent;
    juce::Colour textInv = dark ? lightText  : darkText;

    // Window / component backgrounds
    setColour (juce::ResizableWindow::backgroundColourId,         bg);
    setColour (juce::DocumentWindow::backgroundColourId,          bg);

    // Labels
    setColour (juce::Label::backgroundColourId,                   juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,                         text);

    // Text buttons
    setColour (juce::TextButton::buttonColourId,                  panel);
    setColour (juce::TextButton::buttonOnColourId,                accent);
    setColour (juce::TextButton::textColourOffId,                 text);
    setColour (juce::TextButton::textColourOnId,                  textInv);

    // Toggle buttons
    setColour (juce::ToggleButton::textColourId,                  text);
    setColour (juce::ToggleButton::tickColourId,                  accent);
    setColour (juce::ToggleButton::tickDisabledColourId,          text.withAlpha (0.4f));

    // ComboBox
    setColour (juce::ComboBox::backgroundColourId,                panel);
    setColour (juce::ComboBox::textColourId,                      text);
    setColour (juce::ComboBox::outlineColourId,                   text.withAlpha (0.3f));
    setColour (juce::ComboBox::arrowColourId,                     text);
    setColour (juce::ComboBox::buttonColourId,                    panel);
    setColour (juce::ComboBox::focusedOutlineColourId,            accent);

    // PopupMenu
    setColour (juce::PopupMenu::backgroundColourId,               panel);
    setColour (juce::PopupMenu::textColourId,                     text);
    setColour (juce::PopupMenu::headerTextColourId,               text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId,    accent.withAlpha (0.3f));
    setColour (juce::PopupMenu::highlightedTextColourId,          text);

    // ScrollBar
    setColour (juce::ScrollBar::backgroundColourId,               bg);
    setColour (juce::ScrollBar::thumbColourId,                    accent.withAlpha (0.6f));
    setColour (juce::ScrollBar::trackColourId,                    panel);

    // MenuBar (colour IDs removed in JUCE 8; rendering handled by LookAndFeel overrides)

    // ListBox
    setColour (juce::ListBox::backgroundColourId,                 bg);
    setColour (juce::ListBox::textColourId,                       text);
    setColour (juce::ListBox::outlineColourId,                    text.withAlpha (0.2f));

    // Slider
    setColour (juce::Slider::backgroundColourId,                  panel);
    setColour (juce::Slider::trackColourId,                       accent.withAlpha (0.4f));
    setColour (juce::Slider::thumbColourId,                       accent);
    setColour (juce::Slider::textBoxTextColourId,                 text);
    setColour (juce::Slider::textBoxBackgroundColourId,           panel);
    setColour (juce::Slider::textBoxOutlineColourId,              juce::Colours::transparentBlack);

    // TextEditor
    setColour (juce::TextEditor::backgroundColourId,              panel);
    setColour (juce::TextEditor::textColourId,                    text);
    setColour (juce::TextEditor::outlineColourId,                 text.withAlpha (0.3f));
    setColour (juce::TextEditor::focusedOutlineColourId,          accent);
    setColour (juce::CaretComponent::caretColourId,               accent);

    // AlertWindow
    setColour (juce::AlertWindow::backgroundColourId,             panel);
    setColour (juce::AlertWindow::textColourId,                   text);
    setColour (juce::AlertWindow::outlineColourId,                accent);
}
