#include "CropSpritesheetComponent.h"
#include "ProjectState.h"

//==============================================================================
CropSpritesheetComponent::CropSpritesheetComponent (juce::Component* parent)
    : parentWindow (parent)
{
    // Build a snapshot of the current composite spritesheet
    spritesheetImage = ProjectState::getInstance().buildSpritesheetImage();

    // Initial crop = full image
    if (spritesheetImage.isValid())
        cropRect = { 0.0f, 0.0f,
                     (float) spritesheetImage.getWidth(),
                     (float) spritesheetImage.getHeight() };

    addAndMakeVisible (cancelButton);
    addAndMakeVisible (acceptButton);

    cancelButton.onClick = [this]
    {
        if (parentWindow != nullptr)
            delete parentWindow;
    };

    acceptButton.onClick = [this] { applyAccept(); };

    setSize (kDialogW, kDialogH);
}

//==============================================================================
// Coordinate helpers
//==============================================================================
juce::Rectangle<float> CropSpritesheetComponent::imageDisplayRect() const
{
    if (! spritesheetImage.isValid())
        return {};

    // Canvas = full component minus toolbar, with padding
    auto canvas = getLocalBounds()
                      .withTrimmedBottom (kToolbarH)
                      .reduced (12)
                      .toFloat();

    float imgW = (float) spritesheetImage.getWidth();
    float imgH = (float) spritesheetImage.getHeight();
    if (imgW <= 0.0f || imgH <= 0.0f)
        return canvas;

    float scale = juce::jmin (canvas.getWidth()  / imgW,
                               canvas.getHeight() / imgH);

    float drawW = imgW * scale;
    float drawH = imgH * scale;

    return { canvas.getCentreX() - drawW * 0.5f,
             canvas.getCentreY() - drawH * 0.5f,
             drawW, drawH };
}

float CropSpritesheetComponent::displayScale() const
{
    if (! spritesheetImage.isValid() || spritesheetImage.getWidth() == 0)
        return 1.0f;

    return imageDisplayRect().getWidth() / (float) spritesheetImage.getWidth();
}

juce::Point<float>
CropSpritesheetComponent::imageToDisplay (juce::Point<float> pt) const
{
    auto r = imageDisplayRect();
    float s = displayScale();
    return { r.getX() + pt.x * s, r.getY() + pt.y * s };
}

juce::Point<float>
CropSpritesheetComponent::displayToImage (juce::Point<float> pt) const
{
    auto  r = imageDisplayRect();
    float s = displayScale();
    if (s == 0.0f) return {};
    return { (pt.x - r.getX()) / s, (pt.y - r.getY()) / s };
}

juce::Rectangle<float> CropSpritesheetComponent::cropDisplayRect() const
{
    auto tl = imageToDisplay (cropRect.getTopLeft());
    auto br = imageToDisplay (cropRect.getBottomRight());
    return juce::Rectangle<float>::leftTopRightBottom (tl.x, tl.y, br.x, br.y);
}

//==============================================================================
// Handle helpers
//==============================================================================
juce::Point<float>
CropSpritesheetComponent::handleCentre (Handle h) const
{
    auto cr = cropDisplayRect();
    float l  = cr.getX(),       r  = cr.getRight();
    float t  = cr.getY(),       b  = cr.getBottom();
    float mx = cr.getCentreX(), my = cr.getCentreY();

    switch (h)
    {
        case Handle::NW:  return { l,  t  };
        case Handle::N:   return { mx, t  };
        case Handle::NE:  return { r,  t  };
        case Handle::E:   return { r,  my };
        case Handle::SE:  return { r,  b  };
        case Handle::S:   return { mx, b  };
        case Handle::SW:  return { l,  b  };
        case Handle::W:   return { l,  my };
        default:          return {};
    }
}

CropSpritesheetComponent::Handle
CropSpritesheetComponent::hitTest (juce::Point<float> pt) const
{
    for (auto h : kAllHandles)
    {
        auto c = handleCentre (h);
        if (std::abs (pt.x - c.x) <= (float) kHitRadius &&
            std::abs (pt.y - c.y) <= (float) kHitRadius)
            return h;
    }
    return Handle::None;
}

juce::MouseCursor
CropSpritesheetComponent::cursorForHandle (Handle h) const
{
    using MC = juce::MouseCursor;
    switch (h)
    {
        case Handle::NW: case Handle::SE: return { MC::TopLeftCornerResizeCursor  };
        case Handle::NE: case Handle::SW: return { MC::TopRightCornerResizeCursor };
        case Handle::N:  case Handle::S:  return { MC::UpDownResizeCursor         };
        case Handle::E:  case Handle::W:  return { MC::LeftRightResizeCursor      };
        default:                          return { MC::NormalCursor               };
    }
}

//==============================================================================
// paint / resized
//==============================================================================
void CropSpritesheetComponent::paint (juce::Graphics& g)
{
    // Background
    g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));

    // Toolbar separator + background
    {
        auto toolbarRect = getLocalBounds().removeFromBottom (kToolbarH).toFloat();
        g.setColour (findColour (juce::TextButton::buttonColourId).darker (0.15f));
        g.fillRect (toolbarRect);
        g.setColour (findColour (juce::Label::textColourId).withAlpha (0.25f));
        g.drawLine (toolbarRect.getX(), toolbarRect.getY(),
                    toolbarRect.getRight(), toolbarRect.getY(), 1.0f);
    }

    if (! spritesheetImage.isValid())
    {
        g.setColour (findColour (juce::Label::textColourId).withAlpha (0.5f));
        g.drawText ("No spritesheet loaded",
                    getLocalBounds().withTrimmedBottom (kToolbarH),
                    juce::Justification::centred);
        return;
    }

    auto imgRect  = imageDisplayRect();
    auto cropDisp = cropDisplayRect();

    // ── Spritesheet image ──────────────────────────────────────────────────────
    g.drawImage (spritesheetImage, imgRect);

    // ── Dark overlay on the four regions outside the crop rect ─────────────────
    juce::Colour overlay = juce::Colours::black.withAlpha (0.50f);
    g.setColour (overlay);

    // Top band
    g.fillRect (juce::Rectangle<float> (imgRect.getX(), imgRect.getY(),
                                        imgRect.getWidth(),
                                        cropDisp.getY() - imgRect.getY()));
    // Bottom band
    g.fillRect (juce::Rectangle<float> (imgRect.getX(), cropDisp.getBottom(),
                                        imgRect.getWidth(),
                                        imgRect.getBottom() - cropDisp.getBottom()));
    // Left band (between top/bottom bands)
    g.fillRect (juce::Rectangle<float> (imgRect.getX(), cropDisp.getY(),
                                        cropDisp.getX() - imgRect.getX(),
                                        cropDisp.getHeight()));
    // Right band
    g.fillRect (juce::Rectangle<float> (cropDisp.getRight(), cropDisp.getY(),
                                        imgRect.getRight() - cropDisp.getRight(),
                                        cropDisp.getHeight()));

    // ── Dotted grid within crop rect ───────────────────────────────────────────
    drawGrid (g, cropDisp);

    // ── Crop rect border ───────────────────────────────────────────────────────
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.drawRect (cropDisp, 2.0f);

    // ── Handles ───────────────────────────────────────────────────────────────
    const float hs = (float) kHandleSize;

    for (auto h : kAllHandles)
    {
        auto centre = handleCentre (h);
        juce::Rectangle<float> hr { centre.x - hs * 0.5f,
                                    centre.y - hs * 0.5f,
                                    hs, hs };
        g.setColour (juce::Colours::white);
        g.fillRect (hr);
        g.setColour (juce::Colours::darkgrey.withAlpha (0.8f));
        g.drawRect (hr, 1.0f);
    }
}

void CropSpritesheetComponent::resized()
{
    auto toolbar = getLocalBounds().removeFromBottom (kToolbarH);
    toolbar.reduce (10, 7);

    acceptButton.setBounds (toolbar.removeFromRight (90)
                                   .withSizeKeepingCentre (90, 30));
    toolbar.removeFromRight (8);
    cancelButton.setBounds (toolbar.removeFromRight (90)
                                   .withSizeKeepingCentre (90, 30));
}

//==============================================================================
// Mouse handling
//==============================================================================
void CropSpritesheetComponent::mouseDown (const juce::MouseEvent& e)
{
    auto pt = e.getPosition().toFloat();
    activeHandle     = hitTest (pt);
    dragStartDisplay = pt;
    cropAtDragStart  = cropRect;
}

void CropSpritesheetComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (activeHandle == Handle::None)
        return;

    float s = displayScale();
    if (s == 0.0f) return;

    auto pt = e.getPosition().toFloat();

    // Delta in image-pixel space
    float dx = (pt.x - dragStartDisplay.x) / s;
    float dy = (pt.y - dragStartDisplay.y) / s;

    float imgW = (float) spritesheetImage.getWidth();
    float imgH = (float) spritesheetImage.getHeight();

    float l = cropAtDragStart.getX();
    float t = cropAtDragStart.getY();
    float r = cropAtDragStart.getRight();
    float b = cropAtDragStart.getBottom();

    switch (activeHandle)
    {
        case Handle::NW: l += dx; t += dy; break;
        case Handle::N:            t += dy; break;
        case Handle::NE: r += dx; t += dy; break;
        case Handle::E:  r += dx;           break;
        case Handle::SE: r += dx; b += dy; break;
        case Handle::S:            b += dy; break;
        case Handle::SW: l += dx; b += dy; break;
        case Handle::W:  l += dx;           break;
        default: break;
    }

    // Clamp to image bounds
    l = juce::jlimit (0.0f, imgW, l);
    t = juce::jlimit (0.0f, imgH, t);
    r = juce::jlimit (0.0f, imgW, r);
    b = juce::jlimit (0.0f, imgH, b);

    // Enforce minimum crop size (10 px in image space)
    static constexpr float kMinCrop = 10.0f;

    if (r - l < kMinCrop)
    {
        bool leftEdge = (activeHandle == Handle::W  ||
                         activeHandle == Handle::NW ||
                         activeHandle == Handle::SW);
        if (leftEdge) l = r - kMinCrop;
        else          r = l + kMinCrop;
    }
    if (b - t < kMinCrop)
    {
        bool topEdge = (activeHandle == Handle::N  ||
                        activeHandle == Handle::NW ||
                        activeHandle == Handle::NE);
        if (topEdge) t = b - kMinCrop;
        else         b = t + kMinCrop;
    }

    cropRect = { l, t, r - l, b - t };
    repaint();
}

void CropSpritesheetComponent::mouseMove (const juce::MouseEvent& e)
{
    setMouseCursor (cursorForHandle (hitTest (e.getPosition().toFloat())));
}

void CropSpritesheetComponent::mouseUp (const juce::MouseEvent&)
{
    activeHandle = Handle::None;
    setMouseCursor (juce::MouseCursor::NormalCursor);
}

//==============================================================================
// Grid drawing
//==============================================================================
void CropSpritesheetComponent::drawGrid (juce::Graphics& g,
                                          juce::Rectangle<float> cropDisp) const
{
    auto& ps = ProjectState::getInstance();
    int cols = ps.getNumCols();
    int rows = ps.getNumRows();

    if (cols <= 0 || rows <= 0) return;

    float cellW = cropDisp.getWidth()  / (float) cols;
    float cellH = cropDisp.getHeight() / (float) rows;

    float dashes[] = { 4.0f, 4.0f };

    g.setColour (juce::Colours::white.withAlpha (0.65f));

    // Internal vertical lines
    for (int c = 1; c < cols; ++c)
    {
        float x = cropDisp.getX() + cellW * (float) c;
        g.drawDashedLine (
            juce::Line<float> (x, cropDisp.getY(), x, cropDisp.getBottom()),
            dashes, 2, 1.0f);
    }

    // Internal horizontal lines
    for (int r = 1; r < rows; ++r)
    {
        float y = cropDisp.getY() + cellH * (float) r;
        g.drawDashedLine (
            juce::Line<float> (cropDisp.getX(), y, cropDisp.getRight(), y),
            dashes, 2, 1.0f);
    }
}

//==============================================================================
// Apply / Accept
//==============================================================================
void CropSpritesheetComponent::applyAccept()
{
    if (! spritesheetImage.isValid())
        return;

    auto& ps = ProjectState::getInstance();

    // Convert cropRect (image-pixel coords) to integer bounds, clamped
    int x = juce::roundToInt (cropRect.getX());
    int y = juce::roundToInt (cropRect.getY());
    int w = juce::roundToInt (cropRect.getWidth());
    int h = juce::roundToInt (cropRect.getHeight());

    x = juce::jlimit (0, spritesheetImage.getWidth(),  x);
    y = juce::jlimit (0, spritesheetImage.getHeight(), y);
    w = juce::jlimit (1, spritesheetImage.getWidth()  - x, w);
    h = juce::jlimit (1, spritesheetImage.getHeight() - y, h);

    // Extract the cropped sub-image
    juce::Image cropped = spritesheetImage.getClippedImage ({ x, y, w, h });

    // Scale to the project's expected full sheet size
    int expectedW = ps.getScaleW() * ps.getNumCols();
    int expectedH = ps.getScaleH() * ps.getNumRows();

    juce::Image scaled = ImageOps::scaleFit (cropped, expectedW, expectedH);

    // Replace the spritesheet
    ps.loadSpritesheetImage (scaled, juce::File());

    // Close the dialog
    if (parentWindow != nullptr)
        delete parentWindow;
}
