#include "RefineComponent.h"
#include "AppSettings.h"

static constexpr float kZoomStep = 0.25f;
static constexpr float kZoomMin  = 0.25f;
static constexpr float kZoomMax  = 8.0f;
static constexpr int   kToolbarH = 40;

//==============================================================================
// RefineCanvas
//==============================================================================
RefineCanvas::RefineCanvas()
{
    setOpaque (false);
}

void RefineCanvas::setImage (const juce::Image& img)
{
    image = img;
    repaint();
}

void RefineCanvas::setZoom (float level)
{
    zoomLevel = juce::jlimit (kZoomMin, kZoomMax, level);
    repaint();
}

void RefineCanvas::zoomIn()  { setZoom (zoomLevel + kZoomStep); }
void RefineCanvas::zoomOut() { setZoom (zoomLevel - kZoomStep); }

void RefineCanvas::setBullseyeVisible (bool v)
{
    bullseyeVisible = v;
    repaint();
}

juce::Rectangle<float> RefineCanvas::computeImageRect() const
{
    if (! image.isValid())
        return {};

    float canvasW = (float) getWidth();
    float canvasH = (float) getHeight();
    float imgW    = (float) image.getWidth();
    float imgH    = (float) image.getHeight();

    // Base scale: fit image to canvas
    float fitScale = std::min (canvasW / imgW, canvasH / imgH);
    float scale    = fitScale * zoomLevel;

    float drawW = imgW * scale;
    float drawH = imgH * scale;
    float drawX = (canvasW - drawW) / 2.0f;
    float drawY = (canvasH - drawH) / 2.0f;

    return { drawX, drawY, drawW, drawH };
}

void RefineCanvas::paint (juce::Graphics& g)
{
    // Checker-board background to show transparency
    const int tileSize = 16;
    const juce::Colour colA (0xffaaaaaa);
    const juce::Colour colB (0xff888888);

    for (int y = 0; y < getHeight(); y += tileSize)
        for (int x = 0; x < getWidth(); x += tileSize)
        {
            g.setColour (((x / tileSize + y / tileSize) % 2 == 0) ? colA : colB);
            g.fillRect (juce::Rectangle<int> (x, y, tileSize, tileSize)
                            .getIntersection (getLocalBounds()));
        }

    if (image.isValid())
    {
        auto r = computeImageRect();
        g.drawImage (image, r);
    }

    // Bullseye overlay
    if (bullseyeVisible)
    {
        float cx = (float) getWidth()  / 2.0f;
        float cy = (float) getHeight() / 2.0f;
        float r1 = 6.0f;
        float r2 = 16.0f;
        float lineLen = 24.0f;

        g.setColour (juce::Colours::red.withAlpha (0.85f));
        g.drawEllipse (cx - r1, cy - r1, r1 * 2.0f, r1 * 2.0f, 1.5f);
        g.drawEllipse (cx - r2, cy - r2, r2 * 2.0f, r2 * 2.0f, 1.5f);
        g.drawLine (cx - lineLen, cy,         cx + lineLen, cy,         1.5f);
        g.drawLine (cx,           cy - lineLen, cx,         cy + lineLen, 1.5f);
    }
}

void RefineCanvas::resized() { repaint(); }

//==============================================================================
// RefineToolbar
//==============================================================================
RefineToolbar::RefineToolbar (RefineComponent& owner_) : owner (owner_)
{
    addAndMakeVisible (dimensionsLabel);
    dimensionsLabel.setFont (juce::Font (juce::FontOptions{}.withHeight (13.0f)));
    dimensionsLabel.setJustificationType (juce::Justification::centredLeft);

    addAndMakeVisible (alignmentButton);
    alignmentButton.setTooltip ("Click to scale image to project dimensions");
    alignmentButton.onClick = [this] { onAlignmentClicked(); };

    addAndMakeVisible (zoomInButton);
    zoomInButton.onClick = [this] { owner.zoomIn(); };

    addAndMakeVisible (zoomOutButton);
    zoomOutButton.onClick = [this] { owner.zoomOut(); };

    // Rotate left/right — Unicode arrow characters
    rotateLeftButton .setButtonText (juce::String (juce::CharPointer_UTF8 ("\xe2\x86\xba")));
    rotateRightButton.setButtonText (juce::String (juce::CharPointer_UTF8 ("\xe2\x86\xbb")));
    rotateLeftButton .setTooltip ("Rotate left  (hold Shift to snap to 45°)");
    rotateRightButton.setTooltip ("Rotate right (hold Shift to snap to 45°)");
    rotateLeftButton .onClick = [this] { onRotateLeft();  };
    rotateRightButton.onClick = [this] { onRotateRight(); };
    addAndMakeVisible (rotateLeftButton);
    addAndMakeVisible (rotateRightButton);

    addAndMakeVisible (imageSelector);
    imageSelector.onChange = [this] { onImageSelectorChanged(); };

    addAndMakeVisible (applyButton);
    applyButton.onClick = [this] { onApply(); };

    addAndMakeVisible (updateButton);
    updateButton.setEnabled (false);
    updateButton.onClick = [this] { onUpdate(); };

    addAndMakeVisible (bullseyeButton);
    bullseyeButton.setClickingTogglesState (true);
    bullseyeButton.onStateChange = [this]
    {
        owner.getCanvas().setBullseyeVisible (bullseyeButton.getToggleState());
    };

    addAndMakeVisible (centerButton);
    centerButton.onClick = [this] { owner.centerSubject(); };
}

void RefineToolbar::paint (juce::Graphics& g)
{
    g.fillAll (findColour (juce::TextButton::buttonColourId).darker (0.1f));
}

void RefineToolbar::resized()
{
    auto area = getLocalBounds().reduced (4, 4);
    const int btnW   = 36;
    const int btnH   = area.getHeight();
    const int gap    = 4;
    const int wideW  = 80;
    const int comboW = 180;

    dimensionsLabel  .setBounds (area.removeFromLeft (100));
    area.removeFromLeft (gap);

    alignmentButton  .setBounds (area.removeFromLeft (24).withHeight (24)
                                     .withY (area.getY() + (btnH - 24) / 2));
    area.removeFromLeft (gap);

    zoomOutButton    .setBounds (area.removeFromLeft (btnW));
    area.removeFromLeft (2);
    zoomInButton     .setBounds (area.removeFromLeft (btnW));
    area.removeFromLeft (gap);

    rotateLeftButton .setBounds (area.removeFromLeft (btnW));
    area.removeFromLeft (2);
    rotateRightButton.setBounds (area.removeFromLeft (btnW));
    area.removeFromLeft (gap * 2);

    imageSelector    .setBounds (area.removeFromLeft (comboW));
    area.removeFromLeft (gap * 2);

    centerButton     .setBounds (area.removeFromLeft (wideW));
    area.removeFromLeft (gap);

    bullseyeButton   .setBounds (area.removeFromLeft (wideW + 10));
    area.removeFromLeft (gap * 2);

    applyButton      .setBounds (area.removeFromLeft (wideW));
    area.removeFromLeft (gap);

    updateButton     .setBounds (area.removeFromLeft (wideW));
}

void RefineToolbar::populateImageSelector()
{
    auto& ps = ProjectState::getInstance();

    imageSelector.clear (juce::dontSendNotification);

    for (int i = 0; i < ps.getNumImages(); ++i)
    {
        const auto& entry = ps.getImage (i);
        juce::String name = entry.file.getFileNameWithoutExtension();
        if (name.isEmpty())
            name = "Image " + juce::String (i + 1);

        imageSelector.addItem (name, i + 1);  // item IDs are 1-based
    }

    int cur = ps.getCurrentImageIndex();
    if (cur >= 0)
        imageSelector.setSelectedId (cur + 1, juce::dontSendNotification);
}

void RefineToolbar::onImageSelectorChanged()
{
    int id = imageSelector.getSelectedId();
    if (id > 0)
        ProjectState::getInstance().setCurrentImageIndex (id - 1);
}

void RefineToolbar::onApply()
{
    owner.applyToSheet();
}

void RefineToolbar::onUpdate()
{
    owner.updateInSheet();
}

void RefineToolbar::onRotateLeft()
{
    bool snap = juce::ModifierKeys::getCurrentModifiers().isShiftDown();
    float step = AppSettings::getInstance().getRotationStep();
    owner.rotateImage (-step, snap);   // negative = counter-clockwise
}

void RefineToolbar::onRotateRight()
{
    bool snap = juce::ModifierKeys::getCurrentModifiers().isShiftDown();
    float step = AppSettings::getInstance().getRotationStep();
    owner.rotateImage (+step, snap);   // positive = clockwise
}

void RefineToolbar::onAlignmentClicked()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid())
        return;

    if (entry->image.getWidth()  == ps.getScaleW() &&
        entry->image.getHeight() == ps.getScaleH())
        return;  // Already aligned

    juce::AlertWindow dlg ("Scale Image",
                           "Choose how to scale the image to "
                           + juce::String (ps.getScaleW()) + " x "
                           + juce::String (ps.getScaleH()) + ":",
                           juce::MessageBoxIconType::QuestionIcon);
    dlg.addButton ("Fit",    1);
    dlg.addButton ("Keep",   2);
    dlg.addButton ("Cancel", 0);

    int result = dlg.runModalLoop();

    if (result == 1)
        ps.replaceCurrentImage (ImageOps::scaleFit  (entry->image, ps.getScaleW(), ps.getScaleH()));
    else if (result == 2)
        ps.replaceCurrentImage (ImageOps::scaleKeep (entry->image, ps.getScaleW(), ps.getScaleH()));
}

void RefineToolbar::refresh()
{
    auto& ps  = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();

    // Dimensions label
    if (entry && entry->image.isValid())
    {
        dimensionsLabel.setText (
            juce::String (entry->image.getWidth()) + " x "
            + juce::String (entry->image.getHeight()),
            juce::dontSendNotification);
    }
    else
    {
        dimensionsLabel.setText ("No image", juce::dontSendNotification);
    }

    // Alignment indicator
    bool aligned = entry && entry->image.isValid()
                   && entry->image.getWidth()  == ps.getScaleW()
                   && entry->image.getHeight() == ps.getScaleH();

    alignmentButton.setButtonText (aligned ? juce::String (juce::CharPointer_UTF8 ("\xe2\x9c\x93"))
                                           : juce::String (juce::CharPointer_UTF8 ("\xe2\x9a\xa0")));
    alignmentButton.setColour (juce::TextButton::textColourOffId,
                               aligned ? juce::Colours::green : juce::Colours::orange);

    // Image selector
    populateImageSelector();

    // Update button — orange when modified (but NOT when only rotated, since
    // rotation always goes to a new cell rather than updating in place)
    bool needsUpdate = entry && entry->isApplied
                       && entry->isModifiedSinceApply
                       && ! entry->isRotatedSinceApply;
    updateButton.setEnabled (needsUpdate);
    updateButton.setColour (juce::TextButton::buttonColourId,
                            needsUpdate ? juce::Colours::orange
                                        : findColour (juce::TextButton::buttonColourId));

    // Rotate buttons — tint when image has been rotated since last apply
    bool isRotated = entry && entry->isRotatedSinceApply;
    rotateLeftButton .setColour (juce::TextButton::buttonColourId,
                                 isRotated ? juce::Colours::cornflowerblue.withAlpha (0.6f)
                                           : findColour (juce::TextButton::buttonColourId));
    rotateRightButton.setColour (juce::TextButton::buttonColourId,
                                 isRotated ? juce::Colours::cornflowerblue.withAlpha (0.6f)
                                           : findColour (juce::TextButton::buttonColourId));
    rotateLeftButton .setEnabled (entry != nullptr && entry->image.isValid());
    rotateRightButton.setEnabled (entry != nullptr && entry->image.isValid());

    // Apply button
    applyButton.setEnabled (entry != nullptr && entry->image.isValid());

    // Show "New Cell" on apply when rotated (to hint that it will go to a new slot)
    if (entry && entry->isApplied && entry->isRotatedSinceApply)
        applyButton.setButtonText ("New Cell");
    else if (entry && entry->isApplied && ! entry->isModifiedSinceApply)
        applyButton.setButtonText ("Re-Apply");
    else
        applyButton.setButtonText ("Apply");
}

//==============================================================================
// RefineComponent
//==============================================================================
RefineComponent::RefineComponent()
{
    addAndMakeVisible (toolbar);
    addAndMakeVisible (canvas);

    ProjectState::getInstance().addChangeListener (this);
}

RefineComponent::~RefineComponent()
{
    ProjectState::getInstance().removeChangeListener (this);
}

void RefineComponent::changeListenerCallback (juce::ChangeBroadcaster*)
{
    refreshAll();
}

void RefineComponent::refreshAll()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();

    if (entry && entry->image.isValid())
        canvas.setImage (entry->image);
    else
        canvas.setImage ({});

    toolbar.refresh();
    repaint();
}

void RefineComponent::applyToSheet()
{
    auto& ps    = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();

    if (entry == nullptr || ! entry->image.isValid())
        return;

    // Warn if not aligned
    if (entry->image.getWidth()  != ps.getScaleW() ||
        entry->image.getHeight() != ps.getScaleH())
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::WarningIcon,
            "Image not scaled",
            "The image dimensions (" +
            juce::String (entry->image.getWidth()) + " x " +
            juce::String (entry->image.getHeight()) +
            ") do not match the project scale (" +
            juce::String (ps.getScaleW()) + " x " +
            juce::String (ps.getScaleH()) +
            "). Please scale the image first.");
        return;
    }

    int cell = ps.applyCurrentImageToSheet();
    if (cell < 0)
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::MessageBoxIconType::InfoIcon,
            "Spritesheet full",
            "There are no empty cells remaining in the spritesheet.");
    }
}

void RefineComponent::updateInSheet()
{
    ProjectState::getInstance().updateCurrentImageInSheet();
}

void RefineComponent::zoomIn()  { canvas.zoomIn(); }
void RefineComponent::zoomOut() { canvas.zoomOut(); }

void RefineComponent::toggleBullseye()
{
    canvas.setBullseyeVisible (! canvas.isBullseyeVisible());
}

void RefineComponent::rotateImage (float deltaDegrees, bool snapTo45)
{
    ProjectState::getInstance().rotateCurrentImage (deltaDegrees, snapTo45);
}

void RefineComponent::centerSubject()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid())
        return;

    ps.replaceCurrentImage (ImageOps::centerSubject (entry->image));
}

void RefineComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));
}

void RefineComponent::resized()
{
    auto area = getLocalBounds();
    toolbar.setBounds (area.removeFromTop (kToolbarH));
    canvas .setBounds (area);
}
