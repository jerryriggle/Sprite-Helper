#include "ProjectState.h"

//==============================================================================
ProjectState& ProjectState::getInstance()
{
    static ProjectState instance;
    return instance;
}

void ProjectState::reset()
{
    images.clear();
    currentImageIndex = -1;
    cells.clear();
    cells.resize (totalCells);
    std::fill (cells.begin(), cells.end(), -1);
    projectFile = juce::File();
    dirty = false;
    statusText = "Ready";
}

int ProjectState::getNumRows() const
{
    return (totalCells + sheetCols - 1) / sheetCols;
}

int ProjectState::getGridCellCount() const
{
    return getNumRows() * sheetCols;
}

//==============================================================================
// Images
//==============================================================================
void ProjectState::addImage (const juce::File& file)
{
    juce::Image img = juce::ImageFileFormat::loadFrom (file);
    if (! img.isValid())
        return;

    pushSnapshot();

    ImageEntry entry;
    entry.file  = file;
    entry.image = img;
    images.add (entry);

    currentImageIndex = images.size() - 1;
    dirty = true;
    setStatusText ("Loaded: " + file.getFileName());
    sendChangeMessage();
}

void ProjectState::replaceCurrentImage (const juce::Image& newImage)
{
    if (currentImageIndex < 0 || currentImageIndex >= images.size())
        return;

    pushSnapshot();

    auto& entry = images.getReference (currentImageIndex);
    entry.image = newImage;

    if (entry.isApplied)
        entry.isModifiedSinceApply = true;

    dirty = true;
    sendChangeMessage();
}

void ProjectState::removeImage (int index)
{
    if (index < 0 || index >= images.size())
        return;

    pushSnapshot();

    auto& entry = images.getReference (index);
    if (entry.isApplied && entry.cellIndex >= 0 && entry.cellIndex < cells.size())
        cells.set (entry.cellIndex, -1);

    images.remove (index);

    // Adjust indices stored in cells
    for (int i = 0; i < cells.size(); ++i)
        if (cells[i] > index)
            cells.set (i, cells[i] - 1);

    currentImageIndex = juce::jlimit (-1, images.size() - 1, currentImageIndex);
    dirty = true;
    sendChangeMessage();
}

int ProjectState::getNumImages() const { return images.size(); }

const ImageEntry& ProjectState::getImage (int i) const { return images.getReference (i); }
ImageEntry&       ProjectState::getImage (int i)       { return images.getReference (i); }

int ProjectState::getCurrentImageIndex() const { return currentImageIndex; }

void ProjectState::setCurrentImageIndex (int index)
{
    if (index != currentImageIndex && index >= -1 && index < images.size())
    {
        currentImageIndex = index;
        sendChangeMessage();
    }
}

const ImageEntry* ProjectState::getCurrentEntry() const
{
    if (currentImageIndex < 0 || currentImageIndex >= images.size())
        return nullptr;
    return &images.getReference (currentImageIndex);
}

ImageEntry* ProjectState::getCurrentEntry()
{
    if (currentImageIndex < 0 || currentImageIndex >= images.size())
        return nullptr;
    return &images.getReference (currentImageIndex);
}

//==============================================================================
// Image editing
//==============================================================================
void ProjectState::rotateCurrentImage (float deltaDegrees, bool snapTo45)
{
    auto* entry = getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid())
        return;

    if (snapTo45)
    {
        float target = std::round ((entry->cumulativeRotation + deltaDegrees) / 45.0f) * 45.0f;
        deltaDegrees = target - entry->cumulativeRotation;
    }

    if (std::abs (deltaDegrees) < 0.001f)
        return;

    pushSnapshot();

    entry->image = ImageOps::rotateImage (entry->image, deltaDegrees);
    entry->cumulativeRotation += deltaDegrees;
    entry->isRotatedSinceApply = (std::abs (entry->cumulativeRotation) > 0.001f);

    if (entry->isApplied)
        entry->isModifiedSinceApply = true;

    dirty = true;
    sendChangeMessage();
}

void ProjectState::removeCurrentImageBackground (float tolerance)
{
    auto* entry = getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid())
        return;

    pushSnapshot();

    entry->image = ImageOps::removeBackground (entry->image, tolerance);

    if (entry->isApplied)
        entry->isModifiedSinceApply = true;

    dirty = true;
    setStatusText ("Background removed");
    sendChangeMessage();
}

//==============================================================================
// Spritesheet cells
//==============================================================================
int ProjectState::findFreeCell() const
{
    for (int i = 0; i < cells.size(); ++i)
        if (cells[i] == -1)
            return i;
    return -1;
}

int ProjectState::applyCurrentImageToSheet()
{
    auto* entry = getCurrentEntry();
    if (entry == nullptr)
        return -1;

    pushSnapshot();

    // Rotated images always go to a new free cell, releasing the old one
    if (entry->isRotatedSinceApply && entry->isApplied && entry->cellIndex >= 0)
    {
        cells.set (entry->cellIndex, -1);
        entry->isApplied  = false;
        entry->cellIndex  = -1;
    }

    int cell = (entry->isApplied && entry->cellIndex >= 0)
               ? entry->cellIndex
               : findFreeCell();

    if (cell < 0)
        return -1;

    // Release previous cell if moving to a different one
    if (entry->isApplied && entry->cellIndex >= 0 && entry->cellIndex != cell)
        cells.set (entry->cellIndex, -1);

    cells.set (cell, currentImageIndex);
    entry->cellIndex              = cell;
    entry->appliedImage           = entry->image;
    entry->isApplied              = true;
    entry->isModifiedSinceApply   = false;
    entry->isRotatedSinceApply    = false;
    entry->cumulativeRotation     = 0.0f;

    dirty = true;
    setStatusText ("Applied to cell " + juce::String (cell));
    sendChangeMessage();
    return cell;
}

void ProjectState::updateCurrentImageInSheet()
{
    auto* entry = getCurrentEntry();
    if (entry == nullptr || !entry->isApplied || entry->cellIndex < 0)
        return;

    pushSnapshot();

    cells.set (entry->cellIndex, currentImageIndex);
    entry->appliedImage           = entry->image;
    entry->isModifiedSinceApply   = false;
    entry->isRotatedSinceApply    = false;
    entry->cumulativeRotation     = 0.0f;

    dirty = true;
    setStatusText ("Updated cell " + juce::String (entry->cellIndex));
    sendChangeMessage();
}

int ProjectState::getCellImageIndex (int cellIndex) const
{
    if (cellIndex < 0 || cellIndex >= cells.size())
        return -1;
    return cells[cellIndex];
}

void ProjectState::clearCell (int cellIndex)
{
    if (cellIndex < 0 || cellIndex >= cells.size())
        return;

    pushSnapshot();

    int imgIdx = cells[cellIndex];
    if (imgIdx >= 0 && imgIdx < images.size())
    {
        auto& e = images.getReference (imgIdx);
        if (e.cellIndex == cellIndex)
        {
            e.isApplied            = false;
            e.isModifiedSinceApply = false;
            e.cellIndex            = -1;
        }
    }
    cells.set (cellIndex, -1);
    dirty = true;
    sendChangeMessage();
}

void ProjectState::swapCells (int cellA, int cellB)
{
    if (cellA < 0 || cellB < 0 || cellA >= cells.size() || cellB >= cells.size())
        return;
    if (cellA == cellB)
        return;

    pushSnapshot();

    int imgA = cells[cellA];
    int imgB = cells[cellB];

    cells.set (cellA, imgB);
    cells.set (cellB, imgA);

    if (imgA >= 0 && imgA < images.size())
        images.getReference (imgA).cellIndex = cellB;
    if (imgB >= 0 && imgB < images.size())
        images.getReference (imgB).cellIndex = cellA;

    dirty = true;
    sendChangeMessage();
}

void ProjectState::moveCellContent (int fromCell, int toCell)
{
    if (fromCell < 0 || toCell < 0
        || fromCell >= cells.size() || toCell >= cells.size())
        return;

    pushSnapshot();

    int imgIdx = cells[fromCell];
    cells.set (toCell,   imgIdx);
    cells.set (fromCell, -1);

    if (imgIdx >= 0 && imgIdx < images.size())
        images.getReference (imgIdx).cellIndex = toCell;

    dirty = true;
    sendChangeMessage();
}

//==============================================================================
// Scale / sheet size
//==============================================================================
void ProjectState::setScale (int w, int h)
{
    pushSnapshot();

    scaleW = w;
    scaleH = h;
    dirty = true;
    sendChangeMessage();
}

void ProjectState::setSheetSize (int cols, int total)
{
    pushSnapshot();

    sheetCols  = juce::jlimit (1, 32, cols);
    totalCells = juce::jlimit (1, 1024, total);
    cells.clear();
    cells.resize (totalCells);
    std::fill (cells.begin(), cells.end(), -1);
    dirty = true;
    sendChangeMessage();
}

//==============================================================================
// Project I/O
//==============================================================================
void ProjectState::newProject()
{
    sheetCols  = 8;
    totalCells = 64;
    scaleW     = 128;
    scaleH     = 256;
    reset();
    clearUndoHistory();
    setStatusText ("New project");
    sendChangeMessage();
}

bool ProjectState::saveProject (const juce::File& file)
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty ("scaleW",      scaleW);
    root->setProperty ("scaleH",      scaleH);
    root->setProperty ("sheetCols",   sheetCols);
    root->setProperty ("totalCells",  totalCells);

    juce::Array<juce::var> imageArray;
    for (int i = 0; i < images.size(); ++i)
    {
        const auto& e = images.getReference (i);
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty ("file",      e.file.getFullPathName());
        obj->setProperty ("isApplied", e.isApplied);
        obj->setProperty ("cellIndex", e.cellIndex);

        // Embed the actual (possibly edited) pixel data so that cells loaded
        // from a spritesheet round-trip correctly (the file path alone points
        // to the full sheet, not the individual cell).
        if (e.image.isValid())
        {
            juce::MemoryOutputStream ms;
            juce::PNGImageFormat png;
            if (png.writeImageToStream (e.image, ms))
                obj->setProperty ("imageData",
                    juce::Base64::toBase64 (ms.getData(), ms.getDataSize()));
        }

        imageArray.add (juce::var (obj.get()));
    }
    root->setProperty ("images", imageArray);

    juce::Array<juce::var> cellArray;
    for (int c : cells)
        cellArray.add (c);
    root->setProperty ("cells", cellArray);

    juce::String json = juce::JSON::toString (juce::var (root.get()), true);

    if (! file.replaceWithText (json))
        return false;

    projectFile = file;
    dirty = false;
    setStatusText ("Saved: " + file.getFileName());
    sendChangeMessage();
    return true;
}

bool ProjectState::loadProject (const juce::File& file)
{
    juce::var root = juce::JSON::parse (file.loadFileAsString());
    if (! root.isObject())
        return false;

    reset();
    clearUndoHistory();

    scaleW     = (int) root.getProperty ("scaleW",     128);
    scaleH     = (int) root.getProperty ("scaleH",     256);
    sheetCols  = (int) root.getProperty ("sheetCols",  8);

    // Support old sheetRows-based files as well as new totalCells
    if (root.hasProperty ("totalCells"))
        totalCells = (int) root.getProperty ("totalCells", 64);
    else
        totalCells = sheetCols * (int) root.getProperty ("sheetRows", 8);

    cells.clear();
    cells.resize (totalCells);
    std::fill (cells.begin(), cells.end(), -1);

    if (auto* arr = root.getProperty ("images", {}).getArray())
    {
        for (const auto& item : *arr)
        {
            ImageEntry entry;
            entry.file      = juce::File (item.getProperty ("file", "").toString());
            entry.isApplied = (bool) item.getProperty ("isApplied", false);
            entry.cellIndex = (int)  item.getProperty ("cellIndex", -1);

            // Prefer the embedded pixel data (written by saveProject so that
            // cells sliced from a spritesheet load back as individual tiles,
            // not as the full sheet).  Fall back to the source file for
            // project files saved by older versions.
            juce::String b64 = item.getProperty ("imageData", "").toString();
            if (b64.isNotEmpty())
            {
                juce::MemoryBlock mb;
                juce::MemoryOutputStream mos (mb, false);
                juce::Base64::convertFromBase64 (mos, b64);
                juce::MemoryInputStream mis (mb.getData(), mb.getSize(), false);
                entry.image = juce::ImageFileFormat::loadFrom (mis);
            }
            else if (entry.file.existsAsFile())
            {
                entry.image = juce::ImageFileFormat::loadFrom (entry.file);
            }

            if (entry.image.isValid())
            {
                if (entry.isApplied)
                    entry.appliedImage = entry.image;
                images.add (entry);
            }
        }
    }

    if (auto* arr = root.getProperty ("cells", {}).getArray())
    {
        for (int i = 0; i < std::min (arr->size(), cells.size()); ++i)
            cells.set (i, (int) (*arr)[i]);
    }

    currentImageIndex = images.isEmpty() ? -1 : 0;
    projectFile = file;
    dirty = false;
    setStatusText ("Loaded project: " + file.getFileName());
    sendChangeMessage();
    return true;
}

//==============================================================================
// Spritesheet image
//==============================================================================
juce::Image ProjectState::buildSpritesheetImage() const
{
    int W = scaleW * sheetCols;
    int H = scaleH * getNumRows();
    juce::Image sheet (juce::Image::ARGB, W, H, true);
    juce::Graphics g (sheet);

    for (int i = 0; i < totalCells; ++i)
    {
        int col    = i % sheetCols;
        int row    = i / sheetCols;
        int imgIdx = cells[i];
        if (imgIdx >= 0 && imgIdx < images.size())
        {
            const auto& e = images.getReference (imgIdx);
            if (e.image.isValid())
            {
                int x = col * scaleW;
                int y = row * scaleH;
                g.drawImage (e.image, x, y, scaleW, scaleH,
                             0, 0, e.image.getWidth(), e.image.getHeight());
            }
        }
    }
    return sheet;
}

bool ProjectState::exportSpritesheet (const juce::File& destFile)
{
    auto sheet = buildSpritesheetImage();
    juce::FileOutputStream stream (destFile);
    if (! stream.openedOk())
        return false;

    juce::PNGImageFormat png;
    bool ok = png.writeImageToStream (sheet, stream);
    if (ok)
        setStatusText ("Exported: " + destFile.getFileName());
    return ok;
}

bool ProjectState::loadSpritesheetFile (const juce::File& file)
{
    juce::Image sheet = juce::ImageFileFormat::loadFrom (file);
    if (! sheet.isValid())
        return false;

    return loadSpritesheetImage (sheet, file);
}

bool ProjectState::loadSpritesheetImage (const juce::Image& sheet,
                                         const juce::File& sourceFile)
{
    if (! sheet.isValid())
        return false;

    pushSnapshot();

    images.clear();
    cells.clear();
    cells.resize (totalCells);
    std::fill (cells.begin(), cells.end(), -1);

    for (int i = 0; i < totalCells; ++i)
    {
        int col = i % sheetCols;
        int row = i / sheetCols;
        int x   = col * scaleW;
        int y   = row * scaleH;

        if (x + scaleW > sheet.getWidth() || y + scaleH > sheet.getHeight())
            continue;

        juce::Image cellImg = sheet.getClippedImage (
            juce::Rectangle<int> (x, y, scaleW, scaleH));

        ImageEntry entry;
        entry.file         = sourceFile;
        entry.image        = cellImg.createCopy();
        entry.isApplied    = true;
        entry.cellIndex    = i;
        entry.appliedImage = entry.image;

        int imgIdx = images.size();
        images.add (entry);
        cells.set (i, imgIdx);
    }

    currentImageIndex = images.isEmpty() ? -1 : 0;
    dirty = true;
    setStatusText ("Loaded spritesheet: " + sourceFile.getFileName());
    sendChangeMessage();
    return true;
}

//==============================================================================
void ProjectState::setStatusText (const juce::String& text)
{
    statusText = text;
    sendChangeMessage();
}

//==============================================================================
// Undo / Redo
//==============================================================================
void ProjectState::pushSnapshot()
{
    Snapshot snap;
    snap.images            = images;
    snap.currentImageIndex = currentImageIndex;
    snap.cells             = cells;
    snap.scaleW            = scaleW;
    snap.scaleH            = scaleH;
    snap.sheetCols         = sheetCols;
    snap.totalCells        = totalCells;

    if ((int) undoStack.size() >= kMaxUndoLevels)
        undoStack.erase (undoStack.begin());

    undoStack.push_back (std::move (snap));
    redoStack.clear();
}

void ProjectState::clearUndoHistory()
{
    undoStack.clear();
    redoStack.clear();
}

bool ProjectState::canUndo() const { return ! undoStack.empty(); }
bool ProjectState::canRedo() const { return ! redoStack.empty(); }

void ProjectState::undo()
{
    if (undoStack.empty())
        return;

    // Save current state to redo stack
    Snapshot current;
    current.images            = images;
    current.currentImageIndex = currentImageIndex;
    current.cells             = cells;
    current.scaleW            = scaleW;
    current.scaleH            = scaleH;
    current.sheetCols         = sheetCols;
    current.totalCells        = totalCells;
    redoStack.push_back (std::move (current));

    // Restore previous state
    const Snapshot& prev = undoStack.back();
    images            = prev.images;
    currentImageIndex = prev.currentImageIndex;
    cells             = prev.cells;
    scaleW            = prev.scaleW;
    scaleH            = prev.scaleH;
    sheetCols         = prev.sheetCols;
    totalCells        = prev.totalCells;
    undoStack.pop_back();

    dirty = true;
    setStatusText ("Undo  (" + juce::String (undoStack.size()) + " left)");
}

void ProjectState::redo()
{
    if (redoStack.empty())
        return;

    // Save current state back onto the undo stack
    Snapshot current;
    current.images            = images;
    current.currentImageIndex = currentImageIndex;
    current.cells             = cells;
    current.scaleW            = scaleW;
    current.scaleH            = scaleH;
    current.sheetCols         = sheetCols;
    current.totalCells        = totalCells;

    if ((int) undoStack.size() >= kMaxUndoLevels)
        undoStack.erase (undoStack.begin());
    undoStack.push_back (std::move (current));

    // Restore next state
    const Snapshot& next = redoStack.back();
    images            = next.images;
    currentImageIndex = next.currentImageIndex;
    cells             = next.cells;
    scaleW            = next.scaleW;
    scaleH            = next.scaleH;
    sheetCols         = next.sheetCols;
    totalCells        = next.totalCells;
    redoStack.pop_back();

    dirty = true;
    setStatusText ("Redo  (" + juce::String (redoStack.size()) + " left)");
}

//==============================================================================
// ImageOps
//==============================================================================
namespace ImageOps
{
    juce::Image scaleFit (const juce::Image& src, int w, int h)
    {
        juce::Image result (juce::Image::ARGB, w, h, true);
        juce::Graphics g (result);
        g.drawImage (src, 0, 0, w, h, 0, 0, src.getWidth(), src.getHeight());
        return result;
    }

    juce::Image scaleKeep (const juce::Image& src, int w, int h)
    {
        float scaleX = (float) w / (float) src.getWidth();
        float scaleY = (float) h / (float) src.getHeight();
        float scale  = std::min (scaleX, scaleY);

        int newW = juce::roundToInt ((float) src.getWidth()  * scale);
        int newH = juce::roundToInt ((float) src.getHeight() * scale);
        int offX = (w - newW) / 2;
        int offY = (h - newH) / 2;

        juce::Image result (juce::Image::ARGB, w, h, true);
        juce::Graphics g (result);
        g.drawImage (src, offX, offY, newW, newH,
                     0, 0, src.getWidth(), src.getHeight());
        return result;
    }

    static juce::Rectangle<int> findSubjectBounds (const juce::Image& src,
                                                    int alphaThreshold = 10)
    {
        int minX = src.getWidth(), maxX = 0;
        int minY = src.getHeight(), maxY = 0;
        bool found = false;

        for (int y = 0; y < src.getHeight(); ++y)
        {
            for (int x = 0; x < src.getWidth(); ++x)
            {
                if (src.getPixelAt (x, y).getAlpha() > alphaThreshold)
                {
                    minX = std::min (minX, x);
                    maxX = std::max (maxX, x);
                    minY = std::min (minY, y);
                    maxY = std::max (maxY, y);
                    found = true;
                }
            }
        }

        if (! found)
            return {};

        return { minX, minY, maxX - minX + 1, maxY - minY + 1 };
    }

    juce::Image centerSubject (const juce::Image& src)
    {
        auto bounds = findSubjectBounds (src);
        if (bounds.isEmpty())
            return src;

        int destX = (src.getWidth()  - bounds.getWidth())  / 2;
        int destY = (src.getHeight() - bounds.getHeight()) / 2;

        juce::Image result (juce::Image::ARGB, src.getWidth(), src.getHeight(), true);
        juce::Graphics g (result);
        g.drawImage (src,
                     destX, destY, bounds.getWidth(), bounds.getHeight(),
                     bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
        return result;
    }

    juce::Image applyMargin (const juce::Image& src, float marginPercent)
    {
        auto bounds = findSubjectBounds (src);
        if (bounds.isEmpty())
            return src;

        marginPercent = juce::jlimit (0.0f, 49.0f, marginPercent);
        float margin  = marginPercent / 100.0f;

        float availableW = (float) src.getWidth()  * (1.0f - 2.0f * margin);
        float availableH = (float) src.getHeight() * (1.0f - 2.0f * margin);

        float scaleX = availableW / (float) bounds.getWidth();
        float scaleY = availableH / (float) bounds.getHeight();
        float scale  = std::min (scaleX, scaleY);

        int drawW = juce::roundToInt ((float) bounds.getWidth()  * scale);
        int drawH = juce::roundToInt ((float) bounds.getHeight() * scale);
        int drawX = (src.getWidth()  - drawW) / 2;
        int drawY = (src.getHeight() - drawH) / 2;

        juce::Image result (juce::Image::ARGB, src.getWidth(), src.getHeight(), true);
        juce::Graphics g (result);
        g.drawImage (src,
                     drawX, drawY, drawW, drawH,
                     bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
        return result;
    }

    juce::Image rotateImage (const juce::Image& src, float angleDegrees)
    {
        if (std::abs (angleDegrees) < 0.001f)
            return src;

        float rad = angleDegrees * juce::MathConstants<float>::pi / 180.0f;

        juce::Image result (juce::Image::ARGB, src.getWidth(), src.getHeight(), true);
        juce::Graphics g (result);

        float cx = (float) src.getWidth()  * 0.5f;
        float cy = (float) src.getHeight() * 0.5f;

        g.addTransform (juce::AffineTransform::rotation (rad, cx, cy));
        g.drawImageAt (src, 0, 0);

        return result;
    }

    juce::Image removeBackground (const juce::Image& src, float tolerance)
    {
        if (! src.isValid())
            return src;

        int W = src.getWidth();
        int H = src.getHeight();

        // Sample the four corner pixels and pick the most common colour
        juce::Colour corners[4] =
        {
            src.getPixelAt (0,     0),
            src.getPixelAt (W - 1, 0),
            src.getPixelAt (0,     H - 1),
            src.getPixelAt (W - 1, H - 1)
        };

        // Use top-left corner as representative background colour
        juce::Colour bgColor = corners[0];

        // Create a writable copy
        juce::Image result (juce::Image::ARGB, W, H, true);
        {
            juce::Graphics g (result);
            g.drawImageAt (src, 0, 0);
        }

        // Colour distance in 0–255 Euclidean RGB space
        auto colorDist = [] (juce::Colour a, juce::Colour b) -> float
        {
            float dr = (float) a.getRed()   - (float) b.getRed();
            float dg = (float) a.getGreen() - (float) b.getGreen();
            float db = (float) a.getBlue()  - (float) b.getBlue();
            return std::sqrt (dr * dr + dg * dg + db * db);
        };

        // Stack-based flood fill from all four corners
        std::vector<bool> visited ((size_t) (W * H), false);
        std::vector<juce::Point<int>> stack;
        stack.reserve ((size_t) (W * H / 4));

        auto push = [&] (int x, int y)
        {
            if (x >= 0 && x < W && y >= 0 && y < H)
                stack.push_back ({ x, y });
        };

        push (0,     0);
        push (W - 1, 0);
        push (0,     H - 1);
        push (W - 1, H - 1);

        while (! stack.empty())
        {
            auto pt = stack.back();
            stack.pop_back();

            int x   = pt.x;
            int y   = pt.y;
            int idx = y * W + x;

            if (visited[(size_t) idx])
                continue;
            visited[(size_t) idx] = true;

            juce::Colour px = src.getPixelAt (x, y);
            if (colorDist (px, bgColor) <= tolerance)
            {
                result.setPixelAt (x, y, juce::Colours::transparentBlack);
                push (x + 1, y);
                push (x - 1, y);
                push (x,     y + 1);
                push (x,     y - 1);
            }
        }

        return result;
    }

} // namespace ImageOps
