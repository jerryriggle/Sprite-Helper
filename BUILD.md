# Building Sprite Helper

## Requirements

- **macOS** (M1 / Apple Silicon or Intel) with Xcode Command Line Tools installed
- **CMake** ≥ 3.22  (`brew install cmake` if missing)
- **Git** (for FetchContent to download JUCE automatically)
- Internet access on first build (JUCE is fetched from GitHub)

## Quick Start

```bash
# 1. Clone / enter the project folder
cd /path/to/sprite-helper-juce

# 2. Configure (downloads JUCE ~50 MB on first run)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build build --config Release -j$(sysctl -n hw.logicalcpu)

# 4. Run
open build/SpriteHelper_artefacts/Release/Sprite\ Helper.app
```

For a Debug build replace `Release` with `Debug` in steps 2–4.

## Xcode project (optional)

```bash
cmake -B build-xcode -G Xcode
open build-xcode/SpriteHelper.xcodeproj
```

## File layout

```
sprite-helper-juce/
├── CMakeLists.txt          # Build configuration (JUCE fetched automatically)
├── spec.md                 # Application specification
├── BUILD.md                # This file
└── Source/
    ├── Main.cpp            # JUCEApplication entry point + MainWindow
    ├── Commands.h          # CommandID constants
    ├── AppSettings.h/.cpp  # Font + dark-mode settings (persistent)
    ├── ProjectState.h/.cpp # Project data model + ImageOps utilities
    ├── CustomLookAndFeel.h/.cpp  # Dark / light colour theme
    ├── RefineComponent.h/.cpp    # Image-editor panel + toolbar + canvas
    ├── CompileComponent.h/.cpp   # Spritesheet grid panel
    ├── SplitComponent.h/.cpp     # Resizable split view
    └── MainComponent.h/.cpp      # Menu bar, commands, status bar
```

## Troubleshooting

| Problem | Fix |
|---|---|
| CMake not found | `brew install cmake` |
| Xcode tools missing | `xcode-select --install` |
| FetchContent fails | Check internet / proxy; or clone JUCE manually and set `FETCHCONTENT_SOURCE_DIR_JUCE` |
| `JUCE_MODAL_LOOPS_PERMITTED` warning | Already set in CMakeLists.txt — safe to ignore |
