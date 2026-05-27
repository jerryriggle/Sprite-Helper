#pragma once
#include <JuceHeader.h>

//==============================================================================
/** Represents one image loaded into the Refine section. */
struct ImageEntry
{
    juce::File   file;
    juce::Image  image;          ///< Current (possibly modified) image
    juce::Image  appliedImage;   ///< Image as it was when last applied to sheet
    bool         isApplied        { false };
    bool         isModifiedSinceApply { false };
    int          cellIndex        { -1 };  ///< Which spritesheet cell, or -1
};

//==============================================================================
/** Holds all mutable state for the current project.
    Broadcasts changes via ChangeListener whenever the state is modified.     */
class ProjectState : public juce::ChangeBroadcaster
{
public:
    static ProjectState& getInstance();

    // ── Images ────────────────────────────────────────────────────────────────
    void addImage (const juce::File& file);
    void replaceCurrentImage (const juce::Image& newImage);
    void removeImage (int index);

    int  getNumImages() const;
    const ImageEntry& getImage (int index) const;
    ImageEntry&       getImage (int index);

    int  getCurrentImageIndex() const;
    void setCurrentImageIndex (int index);
    const ImageEntry* getCurrentEntry() const;
    ImageEntry*       getCurrentEntry();

    // ── Spritesheet cells ─────────────────────────────────────────────────────
    /** Apply the current image to the next free cell. Returns cell index, or -1. */
    int  applyCurrentImageToSheet();

    /** Re-apply current image to its existing cell. */
    void updateCurrentImageInSheet();

    /** Returns the image index stored in a cell, or -1 if empty. */
    int  getCellImageIndex (int cellIndex) const;

    /** Clears a cell. */
    void clearCell (int cellIndex);

    int  getNumCols() const { return sheetCols; }
    int  getNumRows() const { return sheetRows; }
    int  getNumCells() const { return sheetCols * sheetRows; }

    // ── Scale settings ────────────────────────────────────────────────────────
    int  getScaleW() const { return scaleW; }
    int  getScaleH() const { return scaleH; }
    void setScale (int w, int h);

    // ── Sheet size ────────────────────────────────────────────────────────────
    void setSheetSize (int cols, int rows);

    // ── Project file I/O ──────────────────────────────────────────────────────
    void newProject();
    bool saveProject (const juce::File& file);
    bool loadProject (const juce::File& file);

    juce::File getProjectFile() const { return projectFile; }
    bool       isDirty() const        { return dirty; }

    // ── Spritesheet export/import ─────────────────────────────────────────────
    bool exportSpritesheet (const juce::File& destFile);
    bool loadSpritesheetFile (const juce::File& file);

    /** Returns a composite spritesheet image (used in export and compile view). */
    juce::Image buildSpritesheetImage() const;

    // ── Status text ───────────────────────────────────────────────────────────
    juce::String getStatusText() const { return statusText; }
    void         setStatusText (const juce::String& text);

private:
    ProjectState() { reset(); }

    void reset();
    int  findFreeCell() const;

    juce::Array<ImageEntry> images;
    int currentImageIndex { -1 };

    juce::Array<int> cells;  ///< size = sheetCols * sheetRows; value = image index or -1

    int scaleW { 128 };
    int scaleH { 256 };
    int sheetCols { 8 };
    int sheetRows { 8 };

    juce::File   projectFile;
    bool         dirty { false };
    juce::String statusText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectState)
};

//==============================================================================
/** Image-processing helpers used by edit operations. */
namespace ImageOps
{
    /** Scale image to (w, h), allowing aspect ratio change. */
    juce::Image scaleFit (const juce::Image& src, int w, int h);

    /** Scale image to fit within (w, h) while keeping aspect ratio. */
    juce::Image scaleKeep (const juce::Image& src, int w, int h);

    /** Center the non-transparent subject within the image bounds. */
    juce::Image centerSubject (const juce::Image& src);

    /** Apply a margin (0–50 %) around the subject, scaling it down as needed. */
    juce::Image applyMargin (const juce::Image& src, float marginPercent);
}
