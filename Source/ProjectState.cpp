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
    cells.resize (sheetCols * sheetRows);
    std::fill (cells.begin(), cells.end(), -1);
    projectFile = juce::File();
    dirty = false;
    statusText = "Ready";
}

//==============================================================================
// Images
//==============================================================================
void ProjectState::addImage (const juce::File& file)
{
    juce::Image img = juce::ImageFileFormat::loadFrom (file);
    if (! img.isValid())
        return;

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

    int cell = (entry->isApplied && entry->cellIndex >= 0)
               ? entry->cellIndex
               : findFreeCell();

    if (cell < 0)
        return -1;

    // If the entry was previously in a different cell, clear the old cell
    if (entry->isApplied && entry->cellIndex != cell && entry->cellIndex >= 0)
        cells.set (entry->cellIndex, -1);

    cells.set (cell, currentImageIndex);
    entry->cellIndex            = cell;
    entry->appliedImage         = entry->image;
    entry->isApplied            = true;
    entry->isModifiedSinceApply = false;

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

    cells.set (entry->cellIndex, currentImageIndex);
    entry->appliedImage         = entry->image;
    entry->isModifiedSinceApply = false;

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

//==============================================================================
// Scale / sheet size
//==============================================================================
void ProjectState::setScale (int w, int h)
{
    scaleW = w;
    scaleH = h;
    dirty = true;
    sendChangeMessage();
}

void ProjectState::setSheetSize (int cols, int rows)
{
    sheetCols = cols;
    sheetRows = rows;
    cells.clear();
    cells.resize (cols * rows);
    std::fill (cells.begin(), cells.end(), -1);
    dirty = true;
    sendChangeMessage();
}

//==============================================================================
// Project I/O
//==============================================================================
void ProjectState::newProject()
{
    sheetCols = 8;
    sheetRows = 8;
    scaleW    = 128;
    scaleH    = 256;
    reset();
    setStatusText ("New project");
    sendChangeMessage();
}

bool ProjectState::saveProject (const juce::File& file)
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty ("scaleW",    scaleW);
    root->setProperty ("scaleH",    scaleH);
    root->setProperty ("sheetCols", sheetCols);
    root->setProperty ("sheetRows", sheetRows);

    juce::Array<juce::var> imageArray;
    for (int i = 0; i < images.size(); ++i)
    {
        const auto& e = images.getReference (i);
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty ("file",      e.file.getFullPathName());
        obj->setProperty ("isApplied", e.isApplied);
        obj->setProperty ("cellIndex", e.cellIndex);
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

    scaleW    = (int) root.getProperty ("scaleW",    128);
    scaleH    = (int) root.getProperty ("scaleH",    256);
    sheetCols = (int) root.getProperty ("sheetCols", 8);
    sheetRows = (int) root.getProperty ("sheetRows", 8);

    cells.clear();
    cells.resize (sheetCols * sheetRows);
    std::fill (cells.begin(), cells.end(), -1);

    if (auto* arr = root.getProperty ("images", {}).getArray())
    {
        for (const auto& item : *arr)
        {
            ImageEntry entry;
            juce::File f (item.getProperty ("file", "").toString());
            if (f.existsAsFile())
            {
                entry.file      = f;
                entry.image     = juce::ImageFileFormat::loadFrom (f);
                entry.isApplied = (bool) item.getProperty ("isApplied", false);
                entry.cellIndex = (int)  item.getProperty ("cellIndex", -1);
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
    int H = scaleH * sheetRows;
    juce::Image sheet (juce::Image::ARGB, W, H, true);
    juce::Graphics g (sheet);

    for (int row = 0; row < sheetRows; ++row)
    {
        for (int col = 0; col < sheetCols; ++col)
        {
            int cell = row * sheetCols + col;
            int imgIdx = cells[cell];
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

    // Partition the sheet into cells
    images.clear();
    cells.clear();
    cells.resize (sheetCols * sheetRows);
    std::fill (cells.begin(), cells.end(), -1);

    for (int row = 0; row < sheetRows; ++row)
    {
        for (int col = 0; col < sheetCols; ++col)
        {
            int cell = row * sheetCols + col;
            int x = col * scaleW;
            int y = row * scaleH;

            if (x + scaleW > sheet.getWidth() || y + scaleH > sheet.getHeight())
                continue;

            juce::Image cellImg = sheet.getClippedImage (
                juce::Rectangle<int> (x, y, scaleW, scaleH));

            ImageEntry entry;
            entry.file       = file;
            entry.image      = cellImg.createCopy();
            entry.isApplied  = true;
            entry.cellIndex  = cell;
            entry.appliedImage = entry.image;

            int imgIdx = images.size();
            images.add (entry);
            cells.set (cell, imgIdx);
        }
    }

    currentImageIndex = images.isEmpty() ? -1 : 0;
    dirty = true;
    setStatusText ("Loaded spritesheet: " + file.getFileName());
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
}
