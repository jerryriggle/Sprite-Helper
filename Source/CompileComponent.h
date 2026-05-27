#pragma once
#include <JuceHeader.h>
#include "ProjectState.h"

//==============================================================================
/** Shows the spritesheet as a grid of thumbnail cells.
    Clicking a cell selects that image in the Refine view.
    Listens to ProjectState for live updates.                                  */
class CompileComponent : public juce::Component,
                         public juce::ChangeListener
{
public:
    CompileComponent();
    ~CompileComponent() override;

    void paint    (juce::Graphics& g) override;
    void resized  () override;
    void mouseDown (const juce::MouseEvent& e) override;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    /** Returns the cell index under the given point, or -1. */
    int  cellAtPoint (juce::Point<int> pt) const;

    /** Cell pixel rectangle for a given (col, row). */
    juce::Rectangle<int> cellRect (int col, int row) const;

    // Drawing helpers
    void paintCell (juce::Graphics& g, int col, int row,
                    int cellIndex, int imageIndex);
    void paintGrid (juce::Graphics& g);

    juce::Point<int> cellSize;  ///< pixel size of each cell (computed in resized)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompileComponent)
};
