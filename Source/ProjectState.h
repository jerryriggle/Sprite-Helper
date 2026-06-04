#pragma once
#include <JuceHeader.h>

//==============================================================================
/** Represents one image loaded into the Refine section. */
struct ImageEntry
{
    juce::File   file;
    juce::Image  image;               ///< Current (possibly modified) image
    juce::Image  appliedImage;        ///< Image as it was when last applied to sheet
    bool         isApplied            { false };
    bool         isModifiedSinceApply { false };
    int          cellIndex            { -1 };   ///< Spritesheet cell, or -1

    // ── Rotation tracking ─────────────────────────────────────────────────────
    float cumulativeRotation  { 0.0f };  ///< Total rotation applied since last apply
    bool  isRotatedSinceApply { false }; ///< True if rotated after the last Apply
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

    // ── Image editing ─────────────────────────────────────────────────────────
    /** Rotate the current image by delta degrees (positive = clockwise).
        If snapTo45 is true, the cumulative rotation is rounded to the nearest
        multiple of 45° and only the remaining delta is applied.               */
    void rotateCurrentImage (float deltaDegrees, bool snapTo45 = false);

    /** Flood-fill remove the solid background from the current image,
        replacing matching pixels with transparency.
        @param tolerance  per-channel Euclidean distance in 0–255 space.      */
    void removeCurrentImageBackground (float tolerance = 30.0f);

    // ── Spritesheet cells ─────────────────────────────────────────────────────
    /** Apply the current image to the appropriate cell.
        If the image has been rotated since last apply, it always goes to the
        next free cell even if it was already applied.
        Returns the cell index used, or -1 if the sheet is full.               */
    int  applyCurrentImageToSheet();

    /** Re-apply current image to its existing cell (Update button). */
    void updateCurrentImageInSheet();

    /** Returns the image index stored in a cell, or -1 if empty. */
    int  getCellImageIndex (int cellIndex) const;

    /** Clears a cell. */
    void clearCell (int cellIndex);

    /** Swap the contents of two active cells. */
    void swapCells (int cellA, int cellB);

    /** Move the content of fromCell to toCell (toCell must be empty). */
    void moveCellContent (int fromCell, int toCell);

    // ── Sheet dimensions ──────────────────────────────────────────────────────
    int  getNumCols()  const { return sheetCols; }

    /** Number of display rows = ceil(totalCells / sheetCols). */
    int  getNumRows()  const;

    /** Total number of active (non-padding) cells. */
    int  getNumCells() const { return totalCells; }

    /** Total cells in the visual grid including padding = getNumRows()*getNumCols(). */
    int  getGridCellCount() const;

    /** True if the grid-cell at grid index i is a padding (greyed-out) cell. */
    bool isCellPadding (int gridIndex) const { return gridIndex >= totalCells; }

    // ── Scale settings ────────────────────────────────────────────────────────
    int  getScaleW() const { return scaleW; }
    int  getScaleH() const { return scaleH; }
    void setScale (int w, int h);

    // ── Sheet size ────────────────────────────────────────────────────────────
    /** @param cols       number of columns in the grid.
        @param total      total number of active cells (may not be a multiple of cols). */
    void setSheetSize (int cols, int total);

    // ── Project file I/O ──────────────────────────────────────────────────────
    void newProject();
    bool saveProject (const juce::File& file);
    bool loadProject (const juce::File& file);

    juce::File getProjectFile() const { return projectFile; }
    bool       isDirty() const        { return dirty; }

    // ── Spritesheet export/import ─────────────────────────────────────────────
    bool exportSpritesheet (const juce::File& destFile);
    bool loadSpritesheetFile (const juce::File& file);

    /** Load a spritesheet from a pre-loaded (possibly scaled) image.
        @param sheet       The image to slice into cells.
        @param sourceFile  Recorded as the source file in each ImageEntry. */
    bool loadSpritesheetImage (const juce::Image& sheet, const juce::File& sourceFile);

    /** Returns a composite spritesheet image (used in export and compile view). */
    juce::Image buildSpritesheetImage() const;

    // ── Undo / Redo ───────────────────────────────────────────────────────────
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

    // ── Status text ───────────────────────────────────────────────────────────
    juce::String getStatusText() const { return statusText; }
    void         setStatusText (const juce::String& text);

private:
    ProjectState() { reset(); }

    void reset();
    int  findFreeCell() const;

    juce::Array<ImageEntry> images;
    int currentImageIndex { -1 };

    juce::Array<int> cells;  ///< size = totalCells; value = image index or -1

    int scaleW      { 128 };
    int scaleH      { 256 };
    int sheetCols   { 8 };
    int totalCells  { 64 };  ///< Active cell count (replaces sheetCols * sheetRows)

    juce::File   projectFile;
    bool         dirty { false };
    juce::String statusText;

    // ── Undo / Redo internals ─────────────────────────────────────────────────
    /** Snapshot of all mutable project data (images, cells, dimensions).
        juce::Image is ref-counted so copying is cheap: the snapshot holds its
        own reference to each image's pixel buffer, which is only duplicated if
        the live image is later replaced. */
    struct Snapshot
    {
        juce::Array<ImageEntry> images;
        int                     currentImageIndex { -1 };
        juce::Array<int>        cells;
        int scaleW     { 128 };
        int scaleH     { 256 };
        int sheetCols  { 8 };
        int totalCells { 64 };
    };

    /** Capture current state onto the undo stack and clear the redo stack.
        Call this at the start of every mutating operation. */
    void pushSnapshot();

    /** Discard both stacks — called on new-project and project-load. */
    void clearUndoHistory();

    static constexpr int kMaxUndoLevels = 64;

    std::vector<Snapshot> undoStack;   ///< past states; back() is the most recent
    std::vector<Snapshot> redoStack;   ///< future states (populated by undo)

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

    /** Rotate the image by angleDegrees clockwise, keeping the same canvas size. */
    juce::Image rotateImage (const juce::Image& src, float angleDegrees);

    /** Flood-fill remove the solid background, replacing it with transparency.
        Samples the background colour from the four corner pixels.
        @param tolerance  Euclidean distance threshold in 0–255 RGB space.    */
    juce::Image removeBackground (const juce::Image& src, float tolerance = 30.0f);
}
