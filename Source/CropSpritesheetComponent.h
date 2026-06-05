#pragma once
#include <JuceHeader.h>

//==============================================================================
/** Popup tool for cropping and aligning a spritesheet to the project grid.

    Shows the current composite spritesheet image with an adjustable crop
    rectangle and a dotted-line grid overlay matching the project's configured
    cell dimensions.  The user can drag any of the eight grab handles (four
    edges + four corners) to resize the crop region.

    Clicking "Accept" scales the cropped area to the project's configured
    sheet dimensions (scaleW*cols × scaleH*rows) and replaces the current
    spritesheet data via ProjectState::loadSpritesheetImage().
    Clicking "Cancel" discards any changes and closes the window.           */
class CropSpritesheetComponent : public juce::Component
{
public:
    explicit CropSpritesheetComponent (juce::Component* parentWindow);

    void paint   (juce::Graphics& g) override;
    void resized () override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

private:
    //==========================================================================
    static constexpr int kToolbarH   = 44;
    static constexpr int kHandleSize = 8;   ///< visual square side length (px)
    static constexpr int kHitRadius  = 10;  ///< hit-test half-size (px)
    static constexpr int kDialogW    = 720;
    static constexpr int kDialogH    = 580;

    //==========================================================================
    enum class Handle
    {
        None,
        NW, N, NE,
        W,       E,
        SW, S, SE
    };

    static constexpr Handle kAllHandles[] = {
        Handle::NW, Handle::N, Handle::NE,
        Handle::E,  Handle::SE, Handle::S,
        Handle::SW, Handle::W
    };

    //==========================================================================
    juce::Image            spritesheetImage; ///< snapshot built at construction
    juce::Rectangle<float> cropRect;         ///< in image-pixel coordinates

    Handle                 activeHandle     { Handle::None };
    juce::Point<float>     dragStartDisplay;
    juce::Rectangle<float> cropAtDragStart;

    juce::TextButton cancelButton { "Cancel" };
    juce::TextButton acceptButton { "Accept" };

    juce::Component* parentWindow { nullptr };

    //==========================================================================
    // Coordinate helpers

    /** Letterboxed rectangle (within the canvas area) where the image is drawn,
        expressed in component coordinates.                                      */
    juce::Rectangle<float> imageDisplayRect() const;

    /** Uniform scale factor: image pixels → display pixels. */
    float displayScale() const;

    juce::Point<float>     imageToDisplay (juce::Point<float> pt) const;
    juce::Point<float>     displayToImage (juce::Point<float> pt) const;

    /** cropRect expressed in display coordinates.                               */
    juce::Rectangle<float> cropDisplayRect() const;

    //==========================================================================
    // Handle helpers

    juce::Point<float>   handleCentre     (Handle h) const; ///< display coords
    Handle               hitTestHandle    (juce::Point<float> displayPt) const;
    juce::MouseCursor    cursorForHandle  (Handle h) const;

    //==========================================================================
    void drawGrid    (juce::Graphics& g, juce::Rectangle<float> cropDisp) const;
    void applyAccept ();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CropSpritesheetComponent)
};
