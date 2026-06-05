#include "AnimationPreviewComponent.h"
#include "ProjectState.h"

static constexpr int kPreviewAreaH = 260;   ///< Height reserved for the sprite canvas
static constexpr int kDialogW      = 500;
static constexpr int kDialogH      = 560;

//==============================================================================
AnimationPreviewComponent::AnimationPreviewComponent()
{
    // ── Selection label ───────────────────────────────────────────────────────
    addAndMakeVisible (selectionLabel);

    // ── Radio-style toggle group ──────────────────────────────────────────────
    allCellsButton.setRadioGroupId (1);
    rangeButton   .setRadioGroupId (1);
    allCellsButton.setToggleState (true, juce::dontSendNotification);
    addAndMakeVisible (allCellsButton);
    addAndMakeVisible (rangeButton);

    allCellsButton.onClick = [this] { updateRangeEnabled(); };
    rangeButton   .onClick = [this] { updateRangeEnabled(); };

    // ── Range editors ─────────────────────────────────────────────────────────
    addAndMakeVisible (fromLabel);
    addAndMakeVisible (fromEditor);
    addAndMakeVisible (toLabel);
    addAndMakeVisible (toEditor);

    fromEditor.setInputRestrictions (5, "0123456789");
    toEditor  .setInputRestrictions (5, "0123456789");

    // Pre-fill with current sheet bounds
    {
        auto& ps = ProjectState::getInstance();
        fromEditor.setText ("0",  juce::dontSendNotification);
        toEditor  .setText (juce::String (juce::jmax (0, ps.getNumCells() - 1)),
                            juce::dontSendNotification);
    }

    // ── Interval ──────────────────────────────────────────────────────────────
    addAndMakeVisible (intervalLabel);
    addAndMakeVisible (intervalEditor);
    intervalEditor.setInputRestrictions (4, "0123456789.");
    intervalEditor.setText ("0.2", juce::dontSendNotification);

    // ── Frame count label ─────────────────────────────────────────────────────
    addAndMakeVisible (frameCountLabel);
    frameCountLabel.setJustificationType (juce::Justification::centred);
    frameCountLabel.setText ("No cells to preview", juce::dontSendNotification);

    // ── Slider ────────────────────────────────────────────────────────────────
    addAndMakeVisible (frameSlider);
    frameSlider.setRange (0.0, 0.0, 1.0);   // updated when animation starts
    frameSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    frameSlider.onValueChange = [this]
    {
        if (! isPlaying)
        {
            int idx = static_cast<int> (std::round (frameSlider.getValue()));
            showFrame (idx);
        }
    };

    // ── Preview / Stop button ─────────────────────────────────────────────────
    addAndMakeVisible (previewStopButton);
    previewStopButton.onClick = [this]
    {
        if (isPlaying)
            stopAnimation();
        else
            startAnimation();
    };

    updateRangeEnabled();
    setSize (kDialogW, kDialogH);
}

AnimationPreviewComponent::~AnimationPreviewComponent()
{
    stopTimer();
}

//==============================================================================
void AnimationPreviewComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));

    // ── Preview canvas ────────────────────────────────────────────────────────
    auto canvasBounds = getLocalBounds()
                            .reduced (16)
                            .withTrimmedTop (140)   // below controls
                            .removeFromTop (kPreviewAreaH);

    // Background checkerboard (transparency indicator)
    {
        const int sq = 8;
        for (int y = canvasBounds.getY(); y < canvasBounds.getBottom(); y += sq)
        {
            for (int x = canvasBounds.getX(); x < canvasBounds.getRight(); x += sq)
            {
                bool light = ((x / sq + y / sq) % 2 == 0);
                g.setColour (light ? juce::Colour (0xffcccccc) : juce::Colour (0xff999999));
                g.fillRect (x, y,
                            juce::jmin (sq, canvasBounds.getRight()  - x),
                            juce::jmin (sq, canvasBounds.getBottom() - y));
            }
        }
    }

    // Draw current frame
    if (! activeCells.isEmpty() && frameIndex >= 0 && frameIndex < activeCells.size())
    {
        juce::Image img = getFrameImage (activeCells[frameIndex]);
        if (img.isValid())
        {
            // Scale to fit canvas while keeping aspect ratio
            auto destF = canvasBounds.toFloat();
            float scaleX = destF.getWidth()  / (float) img.getWidth();
            float scaleY = destF.getHeight() / (float) img.getHeight();
            float scale  = juce::jmin (scaleX, scaleY);

            float dw = (float) img.getWidth()  * scale;
            float dh = (float) img.getHeight() * scale;
            auto dest = juce::Rectangle<float> (
                destF.getCentreX() - dw * 0.5f,
                destF.getCentreY() - dh * 0.5f,
                dw, dh);

            g.drawImage (img, dest);
        }
    }

    // Border around canvas
    g.setColour (findColour (juce::Label::textColourId).withAlpha (0.3f));
    g.drawRect (canvasBounds, 1);
}

void AnimationPreviewComponent::resized()
{
    auto area  = getLocalBounds().reduced (16);
    const int rowH   = 28;
    const int gap    = 8;
    const int labelW = 110;

    // ── Selection label + toggles ─────────────────────────────────────────────
    selectionLabel.setBounds (area.removeFromTop (rowH));
    area.removeFromTop (4);

    auto selRow = area.removeFromTop (rowH);
    allCellsButton.setBounds (selRow.removeFromLeft (110));
    selRow.removeFromLeft (12);
    rangeButton.setBounds (selRow.removeFromLeft (100));
    area.removeFromTop (gap);

    // ── Range row ─────────────────────────────────────────────────────────────
    auto rangeRow = area.removeFromTop (rowH);
    fromLabel .setBounds (rangeRow.removeFromLeft (labelW));
    fromEditor.setBounds (rangeRow.removeFromLeft (56));
    rangeRow.removeFromLeft (12);
    toLabel   .setBounds (rangeRow.removeFromLeft (30));
    toEditor  .setBounds (rangeRow.removeFromLeft (56));
    area.removeFromTop (gap);

    // ── Interval row ──────────────────────────────────────────────────────────
    auto intRow = area.removeFromTop (rowH);
    intervalLabel .setBounds (intRow.removeFromLeft (labelW));
    intervalEditor.setBounds (intRow.removeFromLeft (56));
    area.removeFromTop (gap);

    // ── Preview canvas (painted directly) ─────────────────────────────────────
    area.removeFromTop (kPreviewAreaH + gap);

    // ── Frame label ───────────────────────────────────────────────────────────
    frameCountLabel.setBounds (area.removeFromTop (20));
    area.removeFromTop (4);

    // ── Slider ────────────────────────────────────────────────────────────────
    frameSlider.setBounds (area.removeFromTop (36));
    area.removeFromTop (gap);

    // ── Button ────────────────────────────────────────────────────────────────
    previewStopButton.setBounds (
        area.removeFromTop (rowH).withSizeKeepingCentre (100, rowH - 2));
}

//==============================================================================
// Timer
//==============================================================================
void AnimationPreviewComponent::timerCallback()
{
    if (activeCells.isEmpty())
    {
        stopAnimation();
        return;
    }

    frameIndex = (frameIndex + 1) % activeCells.size();
    showFrame (frameIndex);
}

//==============================================================================
// Helpers
//==============================================================================
void AnimationPreviewComponent::buildCellList()
{
    activeCells.clear();
    auto& ps = ProjectState::getInstance();

    if (allCellsButton.getToggleState())
    {
        // All occupied cells
        for (int i = 0; i < ps.getNumCells(); ++i)
            if (ps.getCellImageIndex (i) >= 0)
                activeCells.add (i);
    }
    else
    {
        int from = fromEditor.getText().getIntValue();
        int to   = toEditor  .getText().getIntValue();
        if (from > to) std::swap (from, to);
        from = juce::jlimit (0, ps.getNumCells() - 1, from);
        to   = juce::jlimit (0, ps.getNumCells() - 1, to);

        for (int i = from; i <= to; ++i)
            if (ps.getCellImageIndex (i) >= 0)
                activeCells.add (i);
    }
}

void AnimationPreviewComponent::startAnimation()
{
    buildCellList();

    if (activeCells.isEmpty())
    {
        frameCountLabel.setText ("No occupied cells in selection",
                                 juce::sendNotification);
        return;
    }

    float intervalSec = intervalEditor.getText().getFloatValue();
    intervalSec = juce::jlimit (0.1f, 60.0f, intervalSec);
    // Round to nearest tenth
    intervalSec = std::round (intervalSec * 10.0f) / 10.0f;

    frameSlider.setRange (0.0, static_cast<double> (activeCells.size() - 1), 1.0);
    frameIndex = 0;
    isPlaying  = true;

    showFrame (0);
    previewStopButton.setButtonText ("Stop");

    startTimer (static_cast<int> (intervalSec * 1000.0f));
}

void AnimationPreviewComponent::stopAnimation()
{
    stopTimer();
    isPlaying = false;
    previewStopButton.setButtonText ("Preview");
}

void AnimationPreviewComponent::showFrame (int index)
{
    if (activeCells.isEmpty()) return;

    frameIndex = juce::jlimit (0, activeCells.size() - 1, index);

    // Sync slider without triggering onValueChange re-entrancy
    frameSlider.setValue (static_cast<double> (frameIndex),
                          juce::dontSendNotification);

    // Update label
    frameCountLabel.setText (
        "Cell " + juce::String (activeCells[frameIndex] + 1)
        + "  (frame " + juce::String (frameIndex + 1)
        + " / " + juce::String (activeCells.size()) + ")",
        juce::dontSendNotification);

    repaint();
}

void AnimationPreviewComponent::updateRangeEnabled()
{
    bool rangeMode = rangeButton.getToggleState();
    fromLabel .setEnabled (rangeMode);
    fromEditor.setEnabled (rangeMode);
    toLabel   .setEnabled (rangeMode);
    toEditor  .setEnabled (rangeMode);
}

juce::Image AnimationPreviewComponent::getFrameImage (int cellIndex) const
{
    auto& ps = ProjectState::getInstance();
    int imgIdx = ps.getCellImageIndex (cellIndex);
    if (imgIdx < 0 || imgIdx >= ps.getNumImages())
        return {};
    return ps.getImage (imgIdx).image;
}
