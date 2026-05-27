#pragma once
#include <JuceHeader.h>

//==============================================================================
/** Dialog content component for Project -> Preview.
 *
 *  Lets the user choose:
 *    • All cells  –or–  a range of cell indices
 *    • Time interval between frames (0.1 s granularity)
 *
 *  A "Preview" button starts a looping animation; a slider with one detent
 *  per frame tracks (and, while stopped, scrubs) the current frame.
 *  The active sprite is rendered in a preview pane in the centre of the dialog.
 */
class AnimationPreviewComponent : public juce::Component,
                                  private juce::Timer
{
public:
    AnimationPreviewComponent();
    ~AnimationPreviewComponent() override;

    void paint   (juce::Graphics& g) override;
    void resized () override;

private:
    //==========================================================================
    // Selection controls
    juce::Label        selectionLabel { {}, "Cells to preview:" };
    juce::ToggleButton allCellsButton { "All Cells" };
    juce::ToggleButton rangeButton    { "Range" };

    juce::Label      fromLabel { {}, "From:" };
    juce::TextEditor fromEditor;
    juce::Label      toLabel   { {}, "To:" };
    juce::TextEditor toEditor;

    // Interval
    juce::Label      intervalLabel  { {}, "Interval (s):" };
    juce::TextEditor intervalEditor;

    // Preview canvas
    juce::Label frameCountLabel;   ///< "Frame N / M"

    // Slider – one integer step per frame
    juce::Slider frameSlider { juce::Slider::LinearHorizontal,
                               juce::Slider::TextBoxBelow };

    // Buttons
    juce::TextButton previewStopButton { "Preview" };

    //==========================================================================
    // Animation state
    juce::Array<int> activeCells;   ///< spritesheet cell indices to animate
    int  frameIndex { 0 };
    bool isPlaying  { false };

    //==========================================================================
    // juce::Timer
    void timerCallback() override;

    //==========================================================================
    // Helpers
    void buildCellList();
    void startAnimation();
    void stopAnimation();
    void showFrame (int index);
    void updateRangeEnabled();
    juce::Image getFrameImage (int cellIndex) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimationPreviewComponent)
};
