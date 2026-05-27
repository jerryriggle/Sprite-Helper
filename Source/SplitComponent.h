#pragma once
#include <JuceHeader.h>
#include "RefineComponent.h"
#include "CompileComponent.h"

//==============================================================================
/** Holds RefineComponent and CompileComponent side-by-side with a draggable
    splitter divider between them.                                             */
class SplitComponent : public juce::Component
{
public:
    SplitComponent();

    RefineComponent&  getRefine()  { return refine; }
    CompileComponent& getCompile() { return compile; }

    void paint   (juce::Graphics& g) override;
    void resized () override;

private:
    //==========================================================================
    /** Thin draggable bar between the two panels. */
    class Divider : public juce::Component
    {
    public:
        explicit Divider (SplitComponent& owner_);
        void paint     (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;
        void mouseDrag (const juce::MouseEvent& e) override;
        void mouseEnter (const juce::MouseEvent& e) override;
        void mouseExit  (const juce::MouseEvent& e) override;

    private:
        SplitComponent& owner;
        int dragStartX   { 0 };
        float dragStartR { 0.0f };
        bool  hovering   { false };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Divider)
    };

    RefineComponent  refine;
    CompileComponent compile;
    Divider          divider { *this };

    /** Fraction of total width given to the left (Refine) pane [0.1, 0.9]. */
    float splitRatio { 0.5f };

    static constexpr int kDividerW = 6;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SplitComponent)
};
