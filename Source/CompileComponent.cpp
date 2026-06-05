#include "CompileComponent.h"

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
    resized();   // cell count or size may have changed
    repaint();
}

//==============================================================================
// Geometry
//==============================================================================
juce::Rectangle<int> CompileComponent::cellRect (int col, int row) const
{
    return { col * cellSize.x, row * cellSize.y, cellSize.x, cellSize.y };
}

juce::Rectangle<int> CompileComponent::cellRectForIndex (int gridIndex) const
{
    auto& ps  = ProjectState::getInstance();
    int   col = gridIndex % ps.getNumCols();
    int   row = gridIndex / ps.getNumCols();
    return cellRect (col, row);
}

int CompileComponent::gridCellAtPoint (juce::Point<int> pt) const
{
    auto& ps = ProjectState::getInstance();
    if (cellSize.x <= 0 || cellSize.y <= 0) return -1;

    int col = pt.x / cellSize.x;
    int row = pt.y / cellSize.y;

    if (col < 0 || col >= ps.getNumCols()) return -1;
    if (row < 0 || row >= ps.getNumRows()) return -1;

    return row * ps.getNumCols() + col;
}

int CompileComponent::activeCellAtPoint (juce::Point<int> pt) const
{
    int grid = gridCellAtPoint (pt);
    if (grid < 0 || ProjectState::getInstance().isCellPadding (grid))
        return -1;
    return grid;
}

//==============================================================================
void CompileComponent::resized()
{
    auto& ps   = ProjectState::getInstance();
    int   cols = ps.getNumCols();
    int   rows = ps.getNumRows();

    if (cols > 0 && rows > 0)
    {
        cellSize.x = getWidth()  / cols;
        cellSize.y = getHeight() / rows;
    }
}

//==============================================================================
// Drawing
//==============================================================================
void CompileComponent::paintCell (juce::Graphics& g, int gridIndex)
{
    auto& ps = ProjectState::getInstance();

    int col      = gridIndex % ps.getNumCols();
    int row      = gridIndex / ps.getNumCols();
    auto r       = cellRect (col, row);
    bool padding = ps.isCellPadding (gridIndex);

    // ── Background ─────────────────────────────────────────────────────────
    if (padding)
    {
        // Greyed-out padding cell
        g.setColour (findColour (juce::ResizableWindow::backgroundColourId).darker (0.3f));
        g.fillRect (r);
        g.setColour (findColour (juce::Label::textColourId).withAlpha (0.15f));
        g.fillRect (r);
        return;
    }

    g.setColour (findColour (juce::ResizableWindow::backgroundColourId).darker (0.05f));
    g.fillRect (r);

    int imgIndex = ps.getCellImageIndex (gridIndex);

    // ── If this is the cell being dragged, show it dimmed ─────────────────
    if (isDragging && gridIndex == dragSourceCell)
    {
        g.setColour (findColour (juce::ResizableWindow::backgroundColourId).darker (0.2f));
        g.fillRect (r);
        g.setColour (juce::Colours::white.withAlpha (0.15f));
        g.fillRect (r);
        return;
    }

    // ── Thumbnail ─────────────────────────────────────────────────────────
    if (imgIndex >= 0 && imgIndex < ps.getNumImages())
    {
        const auto& entry = ps.getImage (imgIndex);
        if (entry.image.isValid())
            g.drawImage (entry.image, r.reduced (2).toFloat());
    }

    // ── Highlight: currently selected image ───────────────────────────────
    int currentImg = ps.getCurrentImageIndex();
    if (currentImg >= 0 && imgIndex == currentImg)
    {
        g.setColour (juce::Colours::dodgerblue.withAlpha (0.5f));
        g.drawRect (r, 3);
    }

    // ── Highlight: applied image in sync with current Refine entry ────────
    const auto* curEntry = ps.getCurrentEntry();
    if (curEntry && curEntry->isApplied && curEntry->cellIndex == gridIndex)
    {
        g.setColour (juce::Colours::gold.withAlpha (0.7f));
        g.drawRect (r.reduced (2), 2);
    }

    // ── Drag-over highlight ───────────────────────────────────────────────
    if (isDragging && dragSourceCell >= 0)
    {
        int hoverCell = activeCellAtPoint (dragCurrentPt);
        if (hoverCell == gridIndex && hoverCell != dragSourceCell)
        {
            g.setColour (juce::Colours::white.withAlpha (0.25f));
            g.fillRect (r);
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.drawRect (r, 2);
        }
    }
}

void CompileComponent::paintGrid (juce::Graphics& g)
{
    auto& ps   = ProjectState::getInstance();
    int   cols = ps.getNumCols();
    int   rows = ps.getNumRows();

    juce::Colour lineColor = findColour (juce::Label::textColourId).withAlpha (0.35f);
    g.setColour (lineColor);

    float dashArr[2] = { 4.0f, 4.0f };

    for (int col = 1; col < cols; ++col)
    {
        int x = col * cellSize.x;
        g.drawDashedLine (juce::Line<float> ((float) x, 0.0f,
                                             (float) x, (float) getHeight()),
                          dashArr, 2, 1.0f);
    }

    for (int row = 1; row < rows; ++row)
    {
        int y = row * cellSize.y;
        g.drawDashedLine (juce::Line<float> (0.0f, (float) y,
                                             (float) getWidth(), (float) y),
                          dashArr, 2, 1.0f);
    }

    g.drawRect (getLocalBounds(), 1);
}

void CompileComponent::paintDragGhost (juce::Graphics& g)
{
    if (! isDragging || dragSourceCell < 0)
        return;

    auto& ps       = ProjectState::getInstance();
    int   imgIndex = ps.getCellImageIndex (dragSourceCell);
    if (imgIndex < 0 || imgIndex >= ps.getNumImages())
        return;

    const auto& entry = ps.getImage (imgIndex);
    if (! entry.image.isValid())
        return;

    // Draw a semi-transparent ghost at the cursor position
    int ghostW = cellSize.x - 4;
    int ghostH = cellSize.y - 4;
    int ghostX = dragCurrentPt.x - ghostW / 2;
    int ghostY = dragCurrentPt.y - ghostH / 2;

    g.setOpacity (0.7f);
    g.drawImage (entry.image,
                 ghostX, ghostY, ghostW, ghostH,
                 0, 0, entry.image.getWidth(), entry.image.getHeight());
    g.setOpacity (1.0f);

    g.setColour (juce::Colours::white.withAlpha (0.6f));
    g.drawRect (ghostX, ghostY, ghostW, ghostH, 1);
}

void CompileComponent::paint (juce::Graphics& g)
{
    auto& ps = ProjectState::getInstance();

    g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));

    if (cellSize.x <= 0 || cellSize.y <= 0)
        return;

    int gridTotal = ps.getGridCellCount();
    for (int i = 0; i < gridTotal; ++i)
        paintCell (g, i);

    paintGrid (g);
    paintDragGhost (g);
}

//==============================================================================
// Mouse handling
//==============================================================================
void CompileComponent::mouseDown (const juce::MouseEvent& e)
{
    int cell = activeCellAtPoint (e.getPosition());
    dragSourceCell = cell;
    isDragging     = false;
    dragCurrentPt  = e.getPosition();
}

void CompileComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (dragSourceCell < 0)
        return;

    dragCurrentPt = e.getPosition();

    if (! isDragging)
    {
        // Begin drag only after threshold is exceeded
        int dx = e.getDistanceFromDragStartX();
        int dy = e.getDistanceFromDragStartY();
        if (dx * dx + dy * dy >= kDragThreshold * kDragThreshold)
            isDragging = true;
    }

    if (isDragging)
        repaint();
}

void CompileComponent::mouseUp (const juce::MouseEvent& e)
{
    if (isDragging && dragSourceCell >= 0)
    {
        int targetCell = activeCellAtPoint (e.getPosition());

        if (targetCell >= 0 && targetCell != dragSourceCell)
        {
            auto& ps = ProjectState::getInstance();
            int dstImg = ps.getCellImageIndex (targetCell);

            if (dstImg == -1)
                ps.moveCellContent (dragSourceCell, targetCell);   // move to empty
            else
                ps.swapCells (dragSourceCell, targetCell);         // swap
        }
    }
    else if (! isDragging && dragSourceCell >= 0)
    {
        // Plain click: select the image in Refine
        int imgIndex = ProjectState::getInstance().getCellImageIndex (dragSourceCell);
        if (imgIndex >= 0)
            ProjectState::getInstance().setCurrentImageIndex (imgIndex);
    }

    dragSourceCell = -1;
    isDragging     = false;
    repaint();
}
