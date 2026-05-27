#pragma once
#include <JuceHeader.h>
#include "ProjectState.h"

//==============================================================================
/** Shows the spritesheet as a grid of thumbnail cells.
    - Clicking a cell selects that image in the Refine view.
    - Cells can be rearranged by drag-and-drop: drop on empty = move,
      drop on occupied = swap.
    - Padding cells (beyond totalCells, filling the last row) are greyed out
      and cannot be drag targets.
    - Listens to ProjectState for live updates.                               */
class CompileComponent : public juce::Component,
                         public juce::ChangeListener
{
public:
    CompileComponent();
    ~CompileComponent() override;

    void paint     (juce::Graphics& g) override;
    void resized   () override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    // ── Geometry helpers ──────────────────────────────────────────────────────
    /** Returns the active cell index (0..totalCells-1) under pt, or -1. */
    int  activeCellAtPoint (juce::Point<int> pt) const;

    /** Returns the grid cell index (including padding) under pt, or -1. */
    int  gridCellAtPoint   (juce::Point<int> pt) const;

    /** Cell pixel rectangle for a given (col, row). */
    juce::Rectangle<int> cellRect (int col, int row) const;

    /** Cell pixel rectangle for a flat grid index. */
    juce::Rectangle<int> cellRectForIndex (int gridIndex) const;

    // ── Drawing helpers ───────────────────────────────────────────────────────
    void paintCell   (juce::Graphics& g, int gridIndex);
    void paintGrid   (juce::Graphics& g);
    void paintDragGhost (juce::Graphics& g);

    // ── Drag state ────────────────────────────────────────────────────────────
    static constexpr int kDragThreshold = 6;  ///< pixels before drag begins

    int              dragSourceCell { -1 };    ///< active cell being dragged
    bool             isDragging     { false };
    juce::Point<int> dragCurrentPt;            ///< current cursor position

    // ── Layout ────────────────────────────────────────────────────────────────
    juce::Point<int> cellSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompileComponent)
};
