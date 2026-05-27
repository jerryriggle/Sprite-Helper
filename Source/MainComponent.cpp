#include "MainComponent.h"

static constexpr int kMenuBarH  = 24;
static constexpr int kStatusBarH = 22;

//==============================================================================
// StatusBar
//==============================================================================
StatusBar::StatusBar()
{
    addAndMakeVisible (label);
    label.setFont (juce::Font (juce::FontOptions{}.withHeight (12.0f)));
    label.setJustificationType (juce::Justification::centredLeft);
    label.setText (ProjectState::getInstance().getStatusText(),
                   juce::dontSendNotification);

    ProjectState::getInstance().addChangeListener (this);
}

StatusBar::~StatusBar()
{
    ProjectState::getInstance().removeChangeListener (this);
}

void StatusBar::changeListenerCallback (juce::ChangeBroadcaster*)
{
    label.setText (ProjectState::getInstance().getStatusText(),
                   juce::dontSendNotification);
}

void StatusBar::paint (juce::Graphics& g)
{
    g.fillAll (findColour (juce::TextButton::buttonColourId).darker (0.2f));
    g.setColour (findColour (juce::Label::textColourId).withAlpha (0.3f));
    g.drawLine (0.0f, 0.0f, (float) getWidth(), 0.0f, 1.0f);
}

//==============================================================================
// SettingsComponent
//==============================================================================
SettingsComponent::SettingsComponent (juce::Component* parent)
    : parentWindow (parent)
{
    addAndMakeVisible (fontLabel);
    addAndMakeVisible (fontPicker);
    addAndMakeVisible (darkLabel);
    addAndMakeVisible (darkToggle);
    addAndMakeVisible (rotStepLabel);
    addAndMakeVisible (rotStepEditor);
    addAndMakeVisible (closeButton);

    // Populate font list
    juce::StringArray fonts = juce::Font::findAllTypefaceNames();
    for (int i = 0; i < fonts.size(); ++i)
        fontPicker.addItem (fonts[i], i + 1);

    // Set current font selection
    {
        juce::String cur = AppSettings::getInstance().getFontName();
        int idx = fonts.indexOf (cur);
        fontPicker.setSelectedId (idx >= 0 ? idx + 1 : 1, juce::dontSendNotification);
    }

    fontPicker.onChange = [this]
    {
        juce::String selected = fontPicker.getText();
        if (selected.isNotEmpty())
            AppSettings::getInstance().setFontByName (selected);
    };

    darkToggle.setToggleState (AppSettings::getInstance().isDarkMode(),
                               juce::dontSendNotification);
    darkToggle.onStateChange = [this]
    {
        AppSettings::getInstance().setDarkMode (darkToggle.getToggleState());
    };

    // Rotation step editor
    rotStepEditor.setText (juce::String (AppSettings::getInstance().getRotationStep(), 1));
    rotStepEditor.setInputRestrictions (5, "0123456789.");
    rotStepEditor.onReturnKey = [this]
    {
        float v = rotStepEditor.getText().getFloatValue();
        AppSettings::getInstance().setRotationStep (v);
    };
    rotStepEditor.onFocusLost = [this]
    {
        float v = rotStepEditor.getText().getFloatValue();
        AppSettings::getInstance().setRotationStep (v);
    };

    closeButton.onClick = [this]
    {
        if (parentWindow != nullptr)
            delete parentWindow;
    };

    setSize (360, 224);
}

void SettingsComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));
}

void SettingsComponent::resized()
{
    auto area = getLocalBounds().reduced (16);
    const int rowH   = 32;
    const int gap    = 8;
    const int labelW = 130;

    auto row1 = area.removeFromTop (rowH);
    fontLabel.setBounds (row1.removeFromLeft (labelW));
    fontPicker.setBounds (row1);

    area.removeFromTop (gap);

    auto row2 = area.removeFromTop (rowH);
    darkLabel.setBounds (row2.removeFromLeft (labelW));
    darkToggle.setBounds (row2);

    area.removeFromTop (gap);

    auto row3 = area.removeFromTop (rowH);
    rotStepLabel.setBounds (row3.removeFromLeft (labelW));
    rotStepEditor.setBounds (row3.removeFromLeft (60));

    area.removeFromTop (gap * 2);
    closeButton.setBounds (area.removeFromTop (rowH).withSizeKeepingCentre (80, rowH - 4));
}

//==============================================================================
// MainComponent
//==============================================================================
MainComponent::MainComponent()
    : menuBar (this)
{
    setLookAndFeel (&laf);

    AppSettings::getInstance().addChangeListener (this);
    ProjectState::getInstance().addChangeListener (this);

    commandManager.registerAllCommandsForTarget (this);
    commandManager.setFirstCommandTarget (this);

    addAndMakeVisible (menuBar);
    menuBar.setModel (this);

    addChildComponent (refineView);
    addChildComponent (compileView);
    addChildComponent (splitView);
    addAndMakeVisible (statusBar);

    setViewMode (ViewMode::Split);

    setSize (1280, 800);
}

MainComponent::~MainComponent()
{
    AppSettings::getInstance().removeChangeListener (this);
    ProjectState::getInstance().removeChangeListener (this);
    setLookAndFeel (nullptr);
    menuBar.setModel (nullptr);
}

//==============================================================================
void MainComponent::changeListenerCallback (juce::ChangeBroadcaster* src)
{
    if (src == &AppSettings::getInstance())
    {
        laf.applyTheme();
        repaint();
    }
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    menuBar  .setBounds (area.removeFromTop    (kMenuBarH));
    statusBar.setBounds (area.removeFromBottom (kStatusBarH));

    refineView .setBounds (area);
    compileView.setBounds (area);
    splitView  .setBounds (area);
}

//==============================================================================
// MenuBarModel
//==============================================================================
juce::StringArray MainComponent::getMenuBarNames()
{
    return { "Sprite Helper", "File", "Edit", "Project", "View", "Help" };
}

juce::PopupMenu MainComponent::getMenuForIndex (int index, const juce::String&)
{
    juce::PopupMenu menu;

    switch (index)
    {
        case 0:  // Sprite Helper
            menu.addCommandItem (&commandManager, CommandIDs::openSettings, "Settings...");
            break;

        case 1:  // File
            menu.addCommandItem (&commandManager, CommandIDs::loadImage,         "Load Image...");
            menu.addCommandItem (&commandManager, CommandIDs::saveImage,         "Save Image...");
            menu.addSeparator();
            menu.addCommandItem (&commandManager, CommandIDs::loadSpritesheet,   "Load Spritesheet...");
            menu.addCommandItem (&commandManager, CommandIDs::exportSpritesheet, "Export...");
            break;

        case 2:  // Edit
            menu.addCommandItem (&commandManager, CommandIDs::scaleFit,          "Scale (Fit)");
            menu.addCommandItem (&commandManager, CommandIDs::scaleKeep,         "Scale (Keep)");
            menu.addSeparator();
            menu.addCommandItem (&commandManager, CommandIDs::centerImage,       "Center");
            menu.addCommandItem (&commandManager, CommandIDs::setMargin,         "Set Margin...");
            menu.addSeparator();
            menu.addCommandItem (&commandManager, CommandIDs::rotateLeft,        "Rotate Left");
            menu.addCommandItem (&commandManager, CommandIDs::rotateRight,       "Rotate Right");
            menu.addSeparator();
            menu.addCommandItem (&commandManager, CommandIDs::removeBackground,  "Remove Background");
            break;

        case 3:  // Project
            menu.addCommandItem (&commandManager, CommandIDs::openProject,        "Open Project...");
            menu.addCommandItem (&commandManager, CommandIDs::newProject,         "New Project");
            menu.addCommandItem (&commandManager, CommandIDs::saveProject,        "Save Project...");
            menu.addSeparator();
            menu.addCommandItem (&commandManager, CommandIDs::setScale,           "Set Scale...");
            menu.addCommandItem (&commandManager, CommandIDs::setSpritesheetSize, "Set Spritesheet Size...");
            break;

        case 4:  // View
        {
            bool isRefine  = (currentView == ViewMode::Refine);
            bool isCompile = (currentView == ViewMode::Compile);
            bool isSplit   = (currentView == ViewMode::Split);

            menu.addCommandItem (&commandManager, CommandIDs::viewRefine,  "Refine");
            menu.addCommandItem (&commandManager, CommandIDs::viewCompile, "Compile");
            menu.addCommandItem (&commandManager, CommandIDs::viewSplit,   "Split");
            break;
        }

        case 5:  // Help
            menu.addCommandItem (&commandManager, CommandIDs::openDocumentation, "Documentation...");
            break;

        default:
            break;
    }

    return menu;
}

//==============================================================================
// ApplicationCommandTarget
//==============================================================================
juce::ApplicationCommandTarget* MainComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void MainComponent::getAllCommands (juce::Array<juce::CommandID>& commands)
{
    commands.addArray ({
        CommandIDs::openSettings,
        CommandIDs::loadImage,
        CommandIDs::saveImage,
        CommandIDs::loadSpritesheet,
        CommandIDs::exportSpritesheet,
        CommandIDs::scaleFit,
        CommandIDs::scaleKeep,
        CommandIDs::centerImage,
        CommandIDs::setMargin,
        CommandIDs::rotateLeft,
        CommandIDs::rotateRight,
        CommandIDs::removeBackground,
        CommandIDs::openProject,
        CommandIDs::newProject,
        CommandIDs::saveProject,
        CommandIDs::setScale,
        CommandIDs::setSpritesheetSize,
        CommandIDs::viewRefine,
        CommandIDs::viewCompile,
        CommandIDs::viewSplit,
        CommandIDs::openDocumentation,
    });
}

void MainComponent::getCommandInfo (juce::CommandID id,
                                    juce::ApplicationCommandInfo& result)
{
    auto& ps      = ProjectState::getInstance();
    bool  hasImage = ps.getCurrentEntry() != nullptr
                     && ps.getCurrentEntry()->image.isValid();

    switch (id)
    {
        case CommandIDs::openSettings:
            result.setInfo ("Settings...", "Open application settings", "App", 0);
            break;
        case CommandIDs::loadImage:
            result.setInfo ("Load Image...", "Load an image into the Refine panel", "File", 0);
            result.addDefaultKeypress ('O', juce::ModifierKeys::commandModifier);
            break;
        case CommandIDs::saveImage:
            result.setInfo ("Save Image...", "Save the current Refine image", "File", 0);
            result.addDefaultKeypress ('S', juce::ModifierKeys::commandModifier);
            result.setActive (hasImage);
            break;
        case CommandIDs::loadSpritesheet:
            result.setInfo ("Load Spritesheet...", "Load a spritesheet file", "File", 0);
            break;
        case CommandIDs::exportSpritesheet:
            result.setInfo ("Export...", "Export the spritesheet as PNG", "File", 0);
            result.addDefaultKeypress ('E', juce::ModifierKeys::commandModifier);
            break;
        case CommandIDs::scaleFit:
            result.setInfo ("Scale (Fit)", "Scale image to project dimensions (may distort)", "Edit", 0);
            result.setActive (hasImage);
            break;
        case CommandIDs::scaleKeep:
            result.setInfo ("Scale (Keep)", "Scale image keeping aspect ratio", "Edit", 0);
            result.setActive (hasImage);
            break;
        case CommandIDs::centerImage:
            result.setInfo ("Center", "Center the subject within the image", "Edit", 0);
            result.setActive (hasImage);
            break;
        case CommandIDs::setMargin:
            result.setInfo ("Set Margin...", "Apply a margin around the subject", "Edit", 0);
            result.setActive (hasImage);
            break;
        case CommandIDs::rotateLeft:
            result.setInfo ("Rotate Left", "Rotate image counter-clockwise by rotation step", "Edit", 0);
            result.addDefaultKeypress ('[', juce::ModifierKeys::commandModifier);
            result.setActive (hasImage);
            break;
        case CommandIDs::rotateRight:
            result.setInfo ("Rotate Right", "Rotate image clockwise by rotation step", "Edit", 0);
            result.addDefaultKeypress (']', juce::ModifierKeys::commandModifier);
            result.setActive (hasImage);
            break;
        case CommandIDs::removeBackground:
            result.setInfo ("Remove Background", "Flood-fill remove the solid background colour", "Edit", 0);
            result.addDefaultKeypress ('B', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            result.setActive (hasImage);
            break;
        case CommandIDs::openProject:
            result.setInfo ("Open Project...", "Open a saved project", "Project", 0);
            break;
        case CommandIDs::newProject:
            result.setInfo ("New Project", "Create a new empty project", "Project", 0);
            result.addDefaultKeypress ('N', juce::ModifierKeys::commandModifier);
            break;
        case CommandIDs::saveProject:
            result.setInfo ("Save Project...", "Save the current project as JSON", "Project", 0);
            result.addDefaultKeypress ('S', juce::ModifierKeys::commandModifier
                                            | juce::ModifierKeys::shiftModifier);
            break;
        case CommandIDs::setScale:
            result.setInfo ("Set Scale...", "Set the project image scale", "Project", 0);
            break;
        case CommandIDs::setSpritesheetSize:
            result.setInfo ("Set Spritesheet Size...", "Set the number of cells in the spritesheet", "Project", 0);
            break;
        case CommandIDs::viewRefine:
            result.setInfo ("Refine", "Show only the Refine panel", "View", 0);
            result.setTicked (currentView == ViewMode::Refine);
            result.addDefaultKeypress ('1', juce::ModifierKeys::commandModifier);
            break;
        case CommandIDs::viewCompile:
            result.setInfo ("Compile", "Show only the Compile panel", "View", 0);
            result.setTicked (currentView == ViewMode::Compile);
            result.addDefaultKeypress ('2', juce::ModifierKeys::commandModifier);
            break;
        case CommandIDs::viewSplit:
            result.setInfo ("Split", "Show both panels", "View", 0);
            result.setTicked (currentView == ViewMode::Split);
            result.addDefaultKeypress ('3', juce::ModifierKeys::commandModifier);
            break;
        case CommandIDs::openDocumentation:
            result.setInfo ("Documentation...", "Open user documentation", "Help", 0);
            break;
        default:
            break;
    }
}

bool MainComponent::perform (const juce::ApplicationCommandTarget::InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::openSettings:      handleOpenSettings();      return true;
        case CommandIDs::loadImage:         handleLoadImage();         return true;
        case CommandIDs::saveImage:         handleSaveImage();         return true;
        case CommandIDs::loadSpritesheet:   handleLoadSpritesheet();   return true;
        case CommandIDs::exportSpritesheet: handleExport();            return true;
        case CommandIDs::scaleFit:          handleScaleFit();          return true;
        case CommandIDs::scaleKeep:         handleScaleKeep();         return true;
        case CommandIDs::centerImage:       handleCenterImage();       return true;
        case CommandIDs::setMargin:         handleSetMargin();         return true;
        case CommandIDs::rotateLeft:        handleRotateLeft();        return true;
        case CommandIDs::rotateRight:       handleRotateRight();       return true;
        case CommandIDs::removeBackground:  handleRemoveBackground();  return true;
        case CommandIDs::openProject:       handleOpenProject();       return true;
        case CommandIDs::newProject:        handleNewProject();        return true;
        case CommandIDs::saveProject:       handleSaveProject();       return true;
        case CommandIDs::setScale:          handleSetScale();          return true;
        case CommandIDs::setSpritesheetSize:handleSetSpritesheetSize();return true;
        case CommandIDs::viewRefine:        setViewMode (ViewMode::Refine);   return true;
        case CommandIDs::viewCompile:       setViewMode (ViewMode::Compile);  return true;
        case CommandIDs::viewSplit:         setViewMode (ViewMode::Split);    return true;
        case CommandIDs::openDocumentation: handleOpenDocumentation(); return true;
        default: return false;
    }
}

//==============================================================================
// View switching
//==============================================================================
void MainComponent::setViewMode (ViewMode mode)
{
    currentView = mode;

    refineView .setVisible (mode == ViewMode::Refine);
    compileView.setVisible (mode == ViewMode::Compile);
    splitView  .setVisible (mode == ViewMode::Split);

    commandManager.commandStatusChanged();
}

//==============================================================================
// File commands
//==============================================================================
void MainComponent::handleLoadImage()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Load Image",
        juce::File::getSpecialLocation (juce::File::userPicturesDirectory),
        "*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.tif;*.webp");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result.existsAsFile())
            {
                ProjectState::getInstance().addImage (result);
                setViewMode (ViewMode::Refine);
            }
        });
}

void MainComponent::handleSaveImage()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid())
        return;

    fileChooser = std::make_unique<juce::FileChooser> (
        "Save Image",
        juce::File::getSpecialLocation (juce::File::userPicturesDirectory)
              .getChildFile (entry->file.getFileNameWithoutExtension() + "_refined.png"),
        "*.png");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result != juce::File{})
            {
                auto dest = result.withFileExtension (".png");
                juce::FileOutputStream stream (dest);
                if (stream.openedOk())
                {
                    juce::PNGImageFormat png;
                    const auto* e = ProjectState::getInstance().getCurrentEntry();
                    if (e)
                        png.writeImageToStream (e->image, stream);
                }
                ProjectState::getInstance().setStatusText ("Saved: " + dest.getFileName());
            }
        });
}

void MainComponent::handleLoadSpritesheet()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Load Spritesheet",
        juce::File::getSpecialLocation (juce::File::userPicturesDirectory),
        "*.png;*.jpg;*.jpeg;*.bmp");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result.existsAsFile())
            {
                ProjectState::getInstance().loadSpritesheetFile (result);
                setViewMode (ViewMode::Compile);
            }
        });
}

void MainComponent::handleExport()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Export Spritesheet",
        juce::File::getSpecialLocation (juce::File::userPicturesDirectory)
              .getChildFile ("spritesheet.png"),
        "*.png");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result != juce::File{})
            {
                auto dest = result.withFileExtension (".png");
                ProjectState::getInstance().exportSpritesheet (dest);
            }
        });
}

//==============================================================================
// Edit commands
//==============================================================================
void MainComponent::handleScaleFit()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid()) return;
    ps.replaceCurrentImage (ImageOps::scaleFit (entry->image, ps.getScaleW(), ps.getScaleH()));
    ps.setStatusText ("Scaled (Fit) to " + juce::String (ps.getScaleW())
                      + " x " + juce::String (ps.getScaleH()));
}

void MainComponent::handleScaleKeep()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid()) return;
    ps.replaceCurrentImage (ImageOps::scaleKeep (entry->image, ps.getScaleW(), ps.getScaleH()));
    ps.setStatusText ("Scaled (Keep) to fit " + juce::String (ps.getScaleW())
                      + " x " + juce::String (ps.getScaleH()));
}

void MainComponent::handleCenterImage()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid()) return;
    ps.replaceCurrentImage (ImageOps::centerSubject (entry->image));
    ps.setStatusText ("Centered subject");
}

void MainComponent::handleSetMargin()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid()) return;

    juce::AlertWindow dlg ("Set Margin",
                           "Enter margin percentage (0–49):",
                           juce::MessageBoxIconType::QuestionIcon);
    dlg.addTextEditor ("margin", "10", "Margin %:");
    dlg.addButton ("Apply",  1);
    dlg.addButton ("Cancel", 0);

    if (dlg.runModalLoop() == 1)
    {
        float pct = dlg.getTextEditorContents ("margin").getFloatValue();
        ps.replaceCurrentImage (ImageOps::applyMargin (entry->image, pct));
        ps.setStatusText ("Applied " + juce::String (pct, 1) + "% margin");
    }
}

//==============================================================================
// Rotate / Remove Background commands
//==============================================================================
void MainComponent::handleRotateLeft()
{
    auto& ps = ProjectState::getInstance();
    if (ps.getCurrentEntry() == nullptr) return;

    bool snap = juce::ModifierKeys::getCurrentModifiers().isShiftDown();
    float step = AppSettings::getInstance().getRotationStep();
    ps.rotateCurrentImage (-step, snap);
    ps.setStatusText (juce::String ("Rotated left ") + juce::String (step, 1) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")));
}

void MainComponent::handleRotateRight()
{
    auto& ps = ProjectState::getInstance();
    if (ps.getCurrentEntry() == nullptr) return;

    bool snap = juce::ModifierKeys::getCurrentModifiers().isShiftDown();
    float step = AppSettings::getInstance().getRotationStep();
    ps.rotateCurrentImage (+step, snap);
    ps.setStatusText (juce::String ("Rotated right ") + juce::String (step, 1) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")));
}

void MainComponent::handleRemoveBackground()
{
    auto& ps = ProjectState::getInstance();
    const auto* entry = ps.getCurrentEntry();
    if (entry == nullptr || ! entry->image.isValid()) return;

    juce::AlertWindow dlg ("Remove Background",
                           "Set colour tolerance (0 = exact match, 100 = very broad):",
                           juce::MessageBoxIconType::QuestionIcon);
    dlg.addTextEditor ("tol", "30", "Tolerance:");
    dlg.addButton ("Remove", 1);
    dlg.addButton ("Cancel", 0);

    if (dlg.runModalLoop() == 1)
    {
        float tol = dlg.getTextEditorContents ("tol").getFloatValue();
        // Scale tolerance from 0–100 user-friendly range to 0–441 Euclidean space
        float euclidean = tol * 4.41f;
        ps.removeCurrentImageBackground (euclidean);
    }
}

//==============================================================================
// Project commands
//==============================================================================
void MainComponent::handleNewProject()
{
    if (ProjectState::getInstance().isDirty())
    {
        auto result = juce::AlertWindow::showYesNoCancelBox (
            juce::MessageBoxIconType::QuestionIcon,
            "New Project",
            "Save the current project before creating a new one?",
            "Save", "Don't Save", "Cancel");

        if (result == 1) handleSaveProject();
        else if (result == 0) return;
    }
    ProjectState::getInstance().newProject();
}

void MainComponent::handleOpenProject()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Open Project",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
        "*.spritehelper");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result.existsAsFile())
                ProjectState::getInstance().loadProject (result);
        });
}

void MainComponent::handleSaveProject()
{
    auto existingFile = ProjectState::getInstance().getProjectFile();

    if (existingFile != juce::File{} && existingFile.getParentDirectory().isDirectory())
    {
        ProjectState::getInstance().saveProject (existingFile);
        return;
    }

    fileChooser = std::make_unique<juce::FileChooser> (
        "Save Project",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
              .getChildFile ("MyProject.spritehelper"),
        "*.spritehelper");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result != juce::File{})
            {
                auto dest = result.withFileExtension (".spritehelper");
                ProjectState::getInstance().saveProject (dest);
            }
        });
}

//==============================================================================
// Set Scale dialog
//==============================================================================
void MainComponent::handleSetScale()
{
    auto& ps = ProjectState::getInstance();

    // Power-of-two sizes 16..4096
    juce::StringArray sizes;
    for (int s = 16; s <= 4096; s *= 2)
        sizes.add (juce::String (s));

    juce::AlertWindow dlg ("Set Scale",
                           "Set the image scale (pixels). Width and height can differ.",
                           juce::MessageBoxIconType::QuestionIcon);

    dlg.addComboBox ("width",  sizes, "Width:");
    dlg.addComboBox ("height", sizes, "Height:");

    // Pre-select current values
    auto* wBox = dlg.getComboBoxComponent ("width");
    auto* hBox = dlg.getComboBoxComponent ("height");

    if (wBox)
    {
        int curW = ps.getScaleW();
        for (int i = 0; i < sizes.size(); ++i)
            if (sizes[i].getIntValue() == curW) { wBox->setSelectedId (i + 1); break; }
    }
    if (hBox)
    {
        int curH = ps.getScaleH();
        for (int i = 0; i < sizes.size(); ++i)
            if (sizes[i].getIntValue() == curH) { hBox->setSelectedId (i + 1); break; }
    }

    dlg.addButton ("OK",     1);
    dlg.addButton ("Cancel", 0);

    if (dlg.runModalLoop() == 1)
    {
        int newW = wBox ? wBox->getText().getIntValue() : ps.getScaleW();
        int newH = hBox ? hBox->getText().getIntValue() : ps.getScaleH();
        ps.setScale (newW, newH);
        ps.setStatusText ("Scale set to " + juce::String (newW) + " x " + juce::String (newH));
    }
}

//==============================================================================
// Set Spritesheet Size dialog
//==============================================================================
void MainComponent::handleSetSpritesheetSize()
{
    auto& ps = ProjectState::getInstance();

    juce::AlertWindow dlg ("Set Spritesheet Size",
                           "Set the total number of cells and the column count.\n"
                           "Incomplete last rows are padded with greyed-out cells.",
                           juce::MessageBoxIconType::QuestionIcon);

    dlg.addTextEditor ("cells", juce::String (ps.getNumCells()),  "Total Cells:");
    dlg.addTextEditor ("cols",  juce::String (ps.getNumCols()),   "Columns:");
    dlg.addButton ("OK",     1);
    dlg.addButton ("Cancel", 0);

    if (dlg.runModalLoop() == 1)
    {
        int cells = dlg.getTextEditorContents ("cells").getIntValue();
        int cols  = dlg.getTextEditorContents ("cols").getIntValue();
        cells = juce::jlimit (1, 1024, cells);
        cols  = juce::jlimit (1, 32,   cols);
        ps.setSheetSize (cols, cells);
        int rows = ps.getNumRows();
        ps.setStatusText ("Spritesheet: " + juce::String (cells) + " cells, "
                          + juce::String (cols) + " cols, " + juce::String (rows) + " rows");
    }
}

//==============================================================================
// Settings
//==============================================================================
void MainComponent::handleOpenSettings()
{
    auto* win = new juce::DocumentWindow ("Settings",
                                          findColour (juce::ResizableWindow::backgroundColourId),
                                          juce::DocumentWindow::closeButton);
    auto* content = new SettingsComponent (win);
    win->setContentOwned (content, true);
    win->setResizable (false, false);
    win->centreAroundComponent (this, win->getWidth(), win->getHeight());
    win->setVisible (true);
}

//==============================================================================
// Documentation
//==============================================================================
void MainComponent::handleOpenDocumentation()
{
    const juce::String docText = R"doc(
SPRITE HELPER — User Documentation
====================================

OVERVIEW
Sprite Helper assists game developers in refining images and composing sprite sheets.
The application has two main sections: Refine and Compile.

VIEWS
• Refine  (Cmd+1) — Edit a single image.
• Compile (Cmd+2) — View and manage the sprite sheet grid.
• Split   (Cmd+3) — Show both panels side-by-side (default).

REFINE PANEL
The Refine toolbar shows:
  • Image dimensions  — current pixel size of the loaded image.
  • ✓ / ⚠ icon       — green if dimensions match the project scale; orange if not.
                        Click to open a scale dialog.
  • + / −            — Zoom the canvas in or out.
  • Image selector   — Dropdown listing all loaded images. Applied images are noted.
  • Center           — Centers the visible subject within the image bounds.
  • Bullseye         — Toggle a crosshair overlay at the image centre.
  • Apply            — Copy the current image into the next free spritesheet cell.
  • Update           — Re-apply a modified image to its existing cell (highlighted
                        in orange when the image is out of sync).

COMPILE PANEL
Displays the sprite sheet as a grid. Each cell shows a thumbnail.
Click a cell to select that image in the Refine view.
The currently selected image's cell is outlined in blue; the last applied cell in gold.

MENUS

File
  Load Image          Open an image file (PNG, JPG, BMP, GIF, TIFF, WebP).
  Save Image          Save the current Refine image as PNG.
  Load Spritesheet    Import an existing sprite sheet — partitions it into cells.
  Export              Export the entire sprite sheet as a single PNG.

Edit
  Scale (Fit)         Stretch/squash image to exactly match the project scale.
  Scale (Keep)        Scale image to fit within the project scale, preserving ratio.
  Center              Move subject to the centre of the image canvas.
  Set Margin...       Scale the subject down to create a margin (enter 0–49 %).

Project
  New Project         Create a new blank project (8×8 sheet, 128×256 px scale).
  Open Project...     Load a saved .spritehelper project file.
  Save Project...     Save the current project as a .spritehelper JSON file.
  Set Scale...        Choose the power-of-two width and height for each sprite cell.
  Set Spritesheet Size... Choose the number of columns and rows in the sheet.

Sprite Helper
  Settings            Change the application font and toggle dark mode.

KEYBOARD SHORTCUTS
  Cmd+O    Load Image
  Cmd+S    Save Image
  Cmd+E    Export Spritesheet
  Cmd+N    New Project
  Cmd+⇧+S  Save Project
  Cmd+1    Refine view
  Cmd+2    Compile view
  Cmd+3    Split view
)doc";

    auto* win = new juce::DocumentWindow ("Documentation",
                                          findColour (juce::ResizableWindow::backgroundColourId),
                                          juce::DocumentWindow::closeButton);
    win->setUsingNativeTitleBar (true);

    auto* editor = new juce::TextEditor();
    editor->setMultiLine (true);
    editor->setReadOnly (true);
    editor->setFont (juce::Font (juce::FontOptions{}.withName ("Courier New").withHeight (13.0f)));
    editor->setText (docText.trim());
    editor->setSize (600, 500);

    win->setContentOwned (editor, true);
    win->setResizable (true, false);
    win->centreAroundComponent (this, 620, 540);
    win->setVisible (true);
}
