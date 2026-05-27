cmake_minimum_required(VERSION 3.22)
project(SpriteHelper VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Fetch JUCE ────────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ── Application target ────────────────────────────────────────────────────────
juce_add_gui_app(SpriteHelper
    PRODUCT_NAME        "Sprite Helper"
    COMPANY_NAME        "Sprite Helper"
    BUNDLE_ID           "com.spritehelper.app"
    MICROPHONE_PERMISSION_ENABLED FALSE
    CAMERA_PERMISSION_ENABLED     FALSE
)

juce_generate_juce_header(SpriteHelper)

target_sources(SpriteHelper
    PRIVATE
        Source/Main.cpp
        Source/AppSettings.cpp
        Source/ProjectState.cpp
        Source/CustomLookAndFeel.cpp
        Source/RefineComponent.cpp
        Source/CompileComponent.cpp
        Source/SplitComponent.cpp
        Source/MainComponent.cpp
        Source/AnimationPreviewComponent.cpp
)

target_compile_definitions(SpriteHelper
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SpriteHelper,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SpriteHelper,JUCE_VERSION>"
)

target_link_libraries(SpriteHelper
    PRIVATE
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
