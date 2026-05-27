#include "SplitComponent.h"

//==============================================================================
// Divider
//==============================================================================
SplitComponent::Divider::Divider (SplitComponent& owner_) : owner (owner_)
{
    setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
    setRepaintsOnMouseActivity (true);
}

void SplitComponent::Divider::paint (juce::Graphics& g)
{
    auto col = findColour (juce::Label::textColourId).withAlpha (hovering ? 0.5f : 0.2f);
    g.fillAll (col);

    // Draw three horizontal dots as a drag indicator
    g.setColour (findColour (juce::Label::textColourId).withAlpha (0.6f));
    float cx = (float) getWidth() / 2.0f;
    float cy = (float) getHeight() / 2.0f;
    float r  = 2.0f;
    for (int i = -1; i <= 1; ++i)
        g.fillEllipse (cx - r, cy + (float) i * 6.0f - r, r * 2.0f, r * 2.0f);
}

void SplitComponent::Divider::mouseDown (const juce::MouseEvent& e)
{
    dragStartX = e.getScreenX();
    dragStartR = owner.splitRatio;
}

void SplitComponent::Divider::mouseDrag (const juce::MouseEvent& e)
{
    int   deltaX  = e.getScreenX() - dragStartX;
    float fraction = (float) deltaX / (float) owner.getWidth();
    owner.splitRatio = juce::jlimit (0.1f, 0.9f, dragStartR + fraction);
    owner.resized();
}

void SplitComponent::Divider::mouseEnter (const juce::MouseEvent&) { hovering = true;  repaint(); }
void SplitComponent::Divider::mouseExit  (const juce::MouseEvent&) { hovering = false; repaint(); }

//==============================================================================
// SplitComponent
//==============================================================================
SplitComponent::SplitComponent()
{
    addAndMakeVisible (refine);
    addAndMakeVisible (compile);
    addAndMakeVisible (divider);
}

void SplitComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));
}

void SplitComponent::resized()
{
    int total    = getWidth();
    int leftW    = juce::roundToInt ((float) total * splitRatio);
    int divW     = kDividerW;
    int rightW   = total - leftW - divW;
    int h        = getHeight();

    refine .setBounds (0,              0, leftW, h);
    divider.setBounds (leftW,          0, divW,  h);
    compile.setBounds (leftW + divW,   0, rightW, h);
}
