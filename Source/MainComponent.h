#pragma once
#include <JuceHeader.h>
#include "Commands.h"
#include "AppSettings.h"
#include "ProjectState.h"
#include "CustomLookAndFeel.h"
#include "RefineComponent.h"
#include "CompileComponent.h"
#include "SplitComponent.h"
#include "AnimationPreviewComponent.h"

//==============================================================================
/** Simple status bar component shown at the bottom of the window. */
class StatusBar : public juce::Component,
                  public juce::ChangeListener
{
public:
    StatusBar();
    ~StatusBar() override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void paint (juce::Graphics& g) override;

private:
    juce::Label label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusBar)
};

//==============================================================================
/** Settings panel shown in a popup window. */
class SettingsComponent : public juce::Component
{
public:
    explicit SettingsComponent (juce::Component* parentWindow);

    void paint   (juce::Graphics& g) override;
    void resized () override;

private:
    juce::Label    fontLabel   { {}, "Font:" };
    juce::ComboBox fontPicker;
    juce::Label    darkLabel   { {}, "Dark Mode:" };
    juce::ToggleButton darkToggle { "Enabled" };
    juce::Label    rotStepLabel { {}, "Rotation Step (°):" };
    juce::TextEditor rotStepEditor;
    juce::TextButton   closeButton { "Close" };

    juce::Component* parentWindow { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
};

//==============================================================================
/** Enum for the current view mode. */
enum class ViewMode { Refine, Compile, Split };

//==============================================================================
/** Top-level application component.
    - Owns the ApplicationCommandManager and MenuBarComponent.
    - Switches between Refine, Compile, and Split views.
    - Handles all menu commands.
    - Shows a status bar at the bottom.                                        */
class MainComponent : public juce::Component,
                      public juce::MenuBarModel,
                      public juce::ApplicationCommandTarget,
                      public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent() override;

    // ── Component ─────────────────────────────────────────────────────────────
    void paint   (juce::Graphics& g) override;
    void resized () override;

    // ── ChangeListener ────────────────────────────────────────────────────────
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    // ── MenuBarModel ──────────────────────────────────────────────────────────
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu   getMenuForIndex (int index, const juce::String& name) override;
    void              menuItemSelected (int, int) override {}

    // ── ApplicationCommandTarget ──────────────────────────────────────────────
    juce::ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (juce::Array<juce::CommandID>& commands) override;
    void getCommandInfo (juce::CommandID commandID,
                         juce::ApplicationCommandInfo& result) override;
    bool perform (const juce::ApplicationCommandTarget::InvocationInfo& info) override;

    juce::ApplicationCommandManager& getCommandManager() { return commandManager; }

private:
    // ── Command handlers ──────────────────────────────────────────────────────
    void handleLoadImage();
    void handleSaveImage();
    void handleLoadSpritesheet();
    void handleExport();
    void handleUndo();
    void handleRedo();
    void handleScaleFit();
    void handleScaleKeep();
    void handleCenterImage();
    void handleSetMargin();
    void handleRotateLeft();
    void handleRotateRight();
    void handleRemoveBackground();
    void handleOpenProject();
    void handleNewProject();
    void handleSaveProject();
    void handleSetScale();
    void handleSetSpritesheetSize();
    void handlePreviewAnimation();
    void handleOpenSettings();
    void handleOpenDocumentation();
    void setViewMode (ViewMode mode);

    // ── Sub-components ────────────────────────────────────────────────────────
    juce::ApplicationCommandManager commandManager;
    juce::MenuBarComponent          menuBar;

    RefineComponent  refineView;
    CompileComponent compileView;
    SplitComponent   splitView;

    StatusBar statusBar;

    ViewMode currentView { ViewMode::Split };

    CustomLookAndFeel laf;

    // We hold the file chooser alive across async operations
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
