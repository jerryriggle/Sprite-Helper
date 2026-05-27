#include <JuceHeader.h>
#include "AppSettings.h"
#include "ProjectState.h"
#include "MainComponent.h"

//==============================================================================
class SpriteHelperApplication : public juce::JUCEApplication
{
public:
    SpriteHelperApplication() = default;

    const juce::String getApplicationName()    override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool               moreThanOneInstanceAllowed() override { return false; }

    //==========================================================================
    void initialise (const juce::String&) override
    {
        AppSettings::getInstance().initialise();
        ProjectState::getInstance().newProject();

        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow.reset();
        AppSettings::getInstance().shutdown();
    }

    //==========================================================================
    void systemRequestedQuit() override
    {
        if (ProjectState::getInstance().isDirty())
        {
            auto result = juce::AlertWindow::showYesNoCancelBox (
                juce::MessageBoxIconType::QuestionIcon,
                "Quit",
                "Save the current project before quitting?",
                "Save", "Don't Save", "Cancel");

            if (result == 0)   // Cancel
                return;

            if (result == 1)   // Save
            {
                auto file = ProjectState::getInstance().getProjectFile();
                if (file == juce::File{})
                {
                    // No saved file yet — ask where to save
                    juce::FileChooser chooser ("Save Project Before Quit",
                                              juce::File::getSpecialLocation (
                                                  juce::File::userDocumentsDirectory)
                                                  .getChildFile ("MyProject.spritehelper"),
                                              "*.spritehelper");
                    if (chooser.browseForFileToSave (true))
                        ProjectState::getInstance().saveProject (
                            chooser.getResult().withFileExtension (".spritehelper"));
                }
                else
                {
                    ProjectState::getInstance().saveProject (file);
                }
            }
        }

        quit();
    }

    void anotherInstanceStarted (const juce::String&) override {}

    //==========================================================================
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& name)
            : juce::DocumentWindow (name,
                                    juce::Desktop::getInstance().getDefaultLookAndFeel()
                                        .findColour (juce::ResizableWindow::backgroundColourId),
                                    allButtons)
        {
            setUsingNativeTitleBar (true);

            auto* content = new MainComponent();
            setContentOwned (content, true);

            // Register keyboard shortcut handler
            content->getCommandManager().getKeyMappings()->resetToDefaultMappings();
            addKeyListener (content->getCommandManager().getKeyMappings());

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
#else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
#endif
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (SpriteHelperApplication)
