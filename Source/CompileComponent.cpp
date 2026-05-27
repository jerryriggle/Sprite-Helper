#include "CompileComponent.h"

static const float kDividerDash[] = { 4.0f, 4.0f };

//==============================================================================
CompileComponent::CompileComponent()
{
    setOpaque (true);
    ProjectState::getInstance().addChangeListener (this);
}

CompileComponent::~CompileComponent()
{
    ProjectState::getInstance().removeChangeListener (this);
}

void CompileComponent::changeListenerCallback (juce::ChangeBroadcaster*)
{
    repaint();
}

//==============================================================================
juce::Rectangle<int> CompileComponent::cellRect (int col, int row) const
{
    return { col * cellSize.x, row * cellSize.y, cellSize.x, cellSize.y };
}

int CompileComponent::cellAtPoint (juce::Point<int> pt) const
{
    auto& ps = ProjectState::getInstance();
    if (cellSize.x <= 0 || cellSize.y <= 0) return -1;

    int col = pt.x / cellSize.x;
    int row = pt.y / cellSize.y;

    if (col < 0 || col >= ps.getNumCols()) return -1;
    if (row < 0 || row >= ps.getNumRows()) return -1;

    return row * ps.getNumCols() + col;
}

//==============================================================================
void CompileComponent::resized()
{
    auto& ps = ProjectState::getInstance();
    int cols = ps.getNumCols();
    int rows = ps.getNumRows();

    if (cols > 0 && rows > 0)
    {
        cellSize.x = getWidth()  / cols;
        cellSize.y = getHeight() / rows;
    }
}

//==============================================================================
void CompileComponent::paintCell (juce::Graphics& g, int col, int row,
                                  int cellIndex, int imageIndex)
{
    auto& ps = ProjectState::getInstance();
    auto  r  = cellRect (col, row);

    // Background
    g.setColour (findColour (juce::ResizableWindow::backgroundColourId).darker (0.05f));
    g.fillRect (r);

    // Thumbnail
    if (imageIndex >= 0 && imageIndex < ps.getNumImages())
    {
        const auto& entry = ps.getImage (imageIndex);
        if (entry.image.isValid())
        {
            auto inner = r.reduced (2);
            g.drawImage (entry.image, inner.toFloat());
        }
    }

    // Highlight if this cell corresponds to the currently selected image
    int currentImg = ps.getCurrentImageIndex();
    if (currentImg >= 0 && imageIndex == currentImg)
    {
        g.setColour (juce::Colours::dodgerblue.withAlpha (0.5f));
        g.drawRect (r, 3);
    }

    // Highlight if applied image is currently selected in Refine
    const auto* curEntry = ps.getCurrentEntry();
    if (curEntry && curEntry->isApplied && curEntry->cellIndex == cellIndex)
    {
        g.setColour (juce::Colours::gold.withAlpha (0.7f));
        g.drawRect (r.reduced (2), 2);
    }
}

void CompileComponent::paintGrid (juce::Graphics& g)
{
    auto& ps   = ProjectState::getInstance();
    int   cols = ps.getNumCols();
    int   rows = ps.getNumRows();

    juce::Colour lineColor = findColour (juce::Label::textColourId).withAlpha (0.4f);
    g.setColour (lineColor);

    // Vertical lines
    for (int col = 1; col < cols; ++col)
    {
        int x = col * cellSize.x;
        float dashArr[2] = { 4.0f, 4.0f };
        g.drawDashedLine (juce::Line<float> ((float) x, 0.0f,
                                             (float) x, (float) getHeight()),
                          dashArr, 2, 1.0f);
    }

    // Horizontal lines
    for (int row = 1; row < rows; ++row)
    {
        int y = row * cellSize.y;
        float dashArr[2] = { 4.0f, 4.0f };
        g.drawDashedLine (juce::Line<float> (0.0f, (float) y,
                                             (float) getWidth(), (float) y),
                          dashArr, 2, 1.0f);
    }

    // Outer border
    g.drawRect (getLocalBounds(), 1);
}

void CompileComponent::paint (juce::Graphics& g)
{
    auto& ps   = ProjectState::getInstance();
    int   cols = ps.getNumCols();
    int   rows = ps.getNumRows();

    g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));

    if (cellSize.x <= 0 || cellSize.y <= 0)
        return;

    // Draw each cell
    for (int row = 0; row < rows; ++row)
        for (int col = 0; col < cols; ++col)
        {
            int cell     = row * cols + col;
            int imgIndex = ps.getCellImageIndex (cell);
            paintCell (g, col, row, cell, imgIndex);
        }

    // Draw grid lines on top
    paintGrid (g);
}

//==============================================================================
void CompileComponent::mouseDown (const juce::MouseEvent& e)
{
    int cell = cellAtPoint (e.getPosition());
    if (cell < 0)
        return;

    int imgIndex = ProjectState::getInstance().getCellImageIndex (cell);
    if (imgIndex >= 0)
        ProjectState::getInstance().setCurrentImageIndex (imgIndex);
}
