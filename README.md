# Sprite Helper

A C++20 desktop application built with the [JUCE framework](https://juce.com/) that helps game developers refine individual sprite images and compose sprite sheets. Runs on **macOS** and **Windows**.

---

## Features

### Two-Panel Workflow

**Refine** — Load and edit individual sprite images before they go into the sheet.

- Zoom in/out on the canvas with a checkerboard transparency background
- Bullseye overlay to verify subject centering
- Dimensions display with a live alignment indicator (✓ green / ⚠ orange) relative to the project scale
- Scale to project dimensions — *Fit* (stretch/squash) or *Keep* (maintain aspect ratio)
- Center the non-transparent subject automatically within the image bounds
- Apply a margin percentage around the subject
- Per-image dropdown selector; applied images are visually distinguished
- **Rotate Left / Right** (↺ / ↻ toolbar buttons, or `Cmd+[` / `Cmd+]`) — rotate the current image by the configured step (default 5°); hold **Shift** to snap to the nearest 45°
- **Apply** — copies the current image into the next free spritesheet cell; if the image has been rotated since the last apply, it always occupies a **new** cell so the original orientation is preserved
- **Update** — highlighted in orange when an applied image has been modified (non-rotation changes); click to sync
- **Remove Background** (`Cmd+Shift+B`) — flood-fill from all four corners using a configurable tolerance (0–100), replacing the background colour with transparency

**Compile** — Compose and inspect the sprite sheet.

- Grid view with dotted cell dividers matching the project scale and sheet dimensions
- Thumbnail preview in every cell
- Click any cell to select its image back in the Refine view
- Currently-selected cell outlined in blue; last-applied cell outlined in gold
- **Drag-and-drop rearrangement** — drag a cell onto an empty cell to move it, or onto an occupied cell to swap the two
- Padding cells (greyed-out) fill any incomplete last row and cannot be dragged to

**Split view** — Both panels side-by-side with a draggable divider (default).

---

### Project Management

| Setting | Default |
|---|---|
| Sprite scale (W × H) | 128 × 256 px |
| Total cells | 64 |
| Columns | 8 |

- **New / Open / Save project** — persisted as a `.spritehelper` JSON file that records image paths, applied cells, scale, and sheet dimensions
- **Set Scale** — choose any power-of-two size from 16 to 4096 px; width and height set independently
- **Set Spritesheet Size** — specify **Total Cells** and **Columns**; the number of rows is computed automatically (`ceil(totalCells / columns)`); any incomplete final row is padded with greyed-out cells
- **Export** — renders the full composite sprite sheet to a PNG file
- **Load Spritesheet** — imports an existing sheet PNG, partitions it into cells by the current scale settings
- **Preview Animation** (`Cmd+Shift+P`) — opens a dedicated animation preview dialog (see below)

---

### Animation Preview

**Project → Preview…** (`Cmd+Shift+P`) opens a looping animation player directly from the spritesheet.

| Control | Description |
|---|---|
| **All Cells** | Animates every occupied cell in order |
| **Range** | Animates a contiguous subset — enter the start and end cell index (0-based) |
| **Interval (s)** | Time between frames, in tenths of a second (e.g. `0.1` = 100 ms, `0.5` = 500 ms) |
| **Preview / Stop** | Starts or stops the looping animation |
| **Frame slider** | Tracks the active frame during playback; drag it manually while stopped to scrub through frames |

The preview pane renders each sprite at its natural aspect ratio against a checkerboard background (so transparency is visible). Only occupied cells are included in the animation — empty cells are skipped even when using Range mode.

---

### Application Settings

Accessed via **Sprite Helper → Settings**:

- **Font** — choose any installed system font
- **Dark Mode** — toggle between a dark (Catppuccin-inspired) and light theme; settings persist across sessions
- **Rotation Step (°)** — angle increment used by the Rotate Left / Right buttons (1–90°, default 5°)

---

## Building

### Requirements

| Tool | Minimum version |
|---|---|
| CMake | 3.22 |
| Xcode Command Line Tools (macOS) | latest |
| MSVC / Clang (Windows) | C++20 capable |
| Git | any recent |
| Internet access | first build only (JUCE is fetched automatically) |

### macOS

```bash
# Configure — downloads JUCE ~50 MB on first run
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release -j$(sysctl -n hw.logicalcpu)

# Run
open build/SpriteHelper_artefacts/Release/Sprite\ Helper.app
```

### Windows

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\SpriteHelper_artefacts\Release\Sprite Helper.exe
```

### Xcode project (optional)

```bash
cmake -B build-xcode -G Xcode
open build-xcode/SpriteHelper.xcodeproj
```

---

## Keyboard Shortcuts

| Action | Shortcut |
|---|---|
| Load Image | Cmd/Ctrl+O |
| Save Image | Cmd/Ctrl+S |
| Export Spritesheet | Cmd/Ctrl+E |
| New Project | Cmd/Ctrl+N |
| Save Project | Cmd/Ctrl+Shift+S |
| Rotate Left | Cmd/Ctrl+[ |
| Rotate Right | Cmd/Ctrl+] |
| Remove Background | Cmd/Ctrl+Shift+B |
| Preview Animation | Cmd/Ctrl+Shift+P |
| Refine view | Cmd/Ctrl+1 |
| Compile view | Cmd/Ctrl+2 |
| Split view | Cmd/Ctrl+3 |

---

## Project Structure

```
sprite-helper-juce/
├── CMakeLists.txt              # Build config; JUCE fetched via FetchContent
├── Source/
│   ├── Main.cpp                         # JUCEApplication entry point + MainWindow
│   ├── Commands.h                       # CommandID constants
│   ├── AppSettings.h/.cpp               # Font + dark-mode (PropertiesFile persistence)
│   ├── ProjectState.h/.cpp              # Data model + ImageOps utilities
│   ├── CustomLookAndFeel.h/.cpp         # Dark / light colour theme (LookAndFeel_V4)
│   ├── RefineComponent.h/.cpp           # Image editor: canvas, zoom, toolbar
│   ├── CompileComponent.h/.cpp          # Spritesheet grid view
│   ├── SplitComponent.h/.cpp            # Resizable split view with drag divider
│   ├── AnimationPreviewComponent.h/.cpp # Animation preview dialog
│   └── MainComponent.h/.cpp             # Menu bar, commands, status bar
├── spec.md                     # Original application specification
└── BUILD.md                    # Detailed build notes and troubleshooting
```

---

## Supported Image Formats

PNG, JPEG, BMP, GIF, TIFF, WebP — anything JUCE's `ImageFileFormat` can decode. Export is always PNG.

---

## References

- [JUCE Framework](https://github.com/juce-framework/JUCE)
- [JUCE Coding Standards](https://juce.com/blog/coding-standards/)
- [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md)
