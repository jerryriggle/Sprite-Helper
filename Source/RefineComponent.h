#pragma once
#include <JuceHeader.h>
#include "ProjectState.h"

//==============================================================================
/** Canvas that renders the current image with zoom and optional bullseye. */
class RefineCanvas : public juce::Component
{
public:
    RefineCanvas();

    void setImage (const juce::Image& img);
    void setZoom (float level);   ///< 1.0 = fit-to-canvas
    void zoomIn();
    void zoomOut();
    void setBullseyeVisible (bool v);
    bool isBullseyeVisible() const { return bullseyeVisible; }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::Image image;
    float       zoomLevel       { 1.0f };
    bool        bullseyeVisible { false };

    juce::Rectangle<float> computeImageRect() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefineCanvas)
};

//==============================================================================
/** Toolbar strip at the top of the Refine section. */
class RefineToolbar : public juce::Component
{
public:
    explicit RefineToolbar (class RefineComponent& owner);

    void refresh();   ///< Sync controls to current ProjectState
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    class RefineComponent& owner;

    juce::Label      dimensionsLabel;
    juce::TextButton alignmentButton { "" };   ///< Shows alignment icon
    juce::TextButton zoomInButton    { "+" };
    juce::TextButton zoomOutButton   { "-" };
    juce::ComboBox   imageSelector;
    juce::TextButton applyButton     { "Apply" };
    juce::TextButton updateButton    { "Update" };
    juce::ToggleButton bullseyeButton { "Bullseye" };
    juce::TextButton centerButton    { "Center" };

    void populateImageSelector();
    void onImageSelectorChanged();
    void onApply();
    void onUpdate();
    void onAlignmentClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefineToolbar)
};

//==============================================================================
/** Full Refine panel: toolbar + canvas.
    Listens to ProjectState changes and redraws accordingly.                  */
class RefineComponent : public juce::Component,
                        public juce::ChangeListener
{
public:
    RefineComponent();
    ~RefineComponent() override;

    // Called by toolbar actions and menu commands
    void applyToSheet();
    void updateInSheet();
    void zoomIn();
    void zoomOut();
    void toggleBullseye();
    void centerSubject();

    RefineCanvas& getCanvas() { return canvas; }

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    RefineToolbar toolbar { *this };
    RefineCanvas  canvas;

    void refreshAll();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefineComponent)
};
