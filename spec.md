Application Name: Sprite Helper
Details: Sprite Helper is a C++ (C++ 20) desktop application that utilizes the Juce framework for its user interface. It can run on Mac or Windows. Its purpose is to help game developers refine images for use as sprites and with creation of sprite sheets. The application window has a toolbar along the top, a workspace in the main pane and a status bar along the bottom the shows a summary of the current task being processed, or, if there is no task currently being processed it shows the last task that was processed. The application has two distinct phases, "Refine" and "Compile". Either phase can be shown full screen in the main pane or in a split view with Refine on the left and Compile on the right divided by a splitter that allows the relative scaling of the two sections in the available space.

The toolbar has the following options:

Sprite Helper
    Settings -> opens a pane to allow settings to be adjusted. Note that settings are global and not tied to the current project that is loaded.
        Font -> select the font for the application
        Dark Mode -> enables or disables dark mode theme

File
    Load Image -> Opens a file picker that allows the user to select an image file, supporting all popular formats, into the Refine section.

    Save Image -> Saves the image currently being shown in the Refine pane.

    Load Spritesheet -> Opens a file picker that allows the user to select a spritesheet file and loads it into the Compile section.

    Export -> Exports the spritesheet from the currently opened project.

Edit
    Scale (Fit) -> Scales the currently loaded image in the Refine pane to the Scale size set in the project, allowing the aspect ratio of the image to change if needed.

    Scale (Keep) -> Scales the currently loaded image in the Refine pane to the Scale size set in the project while keeping the aspect ratio of the image the same.

    Center -> Centers the subject in the image in the Refine pane.

    Set Margin -> Adjusts the margin between the subject in the image and the edge of the image, defined in percentage.
    

Project
    Open Project -> Opens a saved project, which is composed of an images being edited in the Refine section and a spritesheet being compiled in the Compose section.

    New Project -> Opens an empty default project, each project is stored as a folder in a Projects folder.
    Save Project -> Saves the current project serialized as a JSON file.

    Set Scale -> Shows and sets the scale of the image in the Refine pane to a standard "power of two" size, ranging from 16 pixels to 4096 pixels.  the width and the height can be set uniquely.  This also sets the cell size in the spritesheet and is saved as part of the project configuration.  The default setting for a new project is 128w x 256h.

    Set Spritesheet Size -> Shows and sets the current number of cells in the projects sprite sheet.  The default number of cells in 8x8.

View
    Refine
    Compile
    Split -> these three options are mutually exclusive and there is a check next to the currently selected option.  Refine will show just the image manipulation section, Compile will show only the spritesheet manipulation section and Split will show both as described above.

Help
    Documentation -> Opens a document with well formatted user facing documentation on how to use the application. 

The sections work as follows:

Refine:  Shows the loaded image. In the split view, this pane is on the left.  This section has its own toolbar section along the top separate from the application toolbar.  This is called the Refine toolbar.  The refine toolbar shows the dimensions of the loaded image with an icon next to it that shows if these dimensions align with the current Project Scale dimensions.  clicking this icon, if the image is not aligned to the project scale, will bring up a dialog to select between "Fit" and "keep" scaling options as defined in the toolbar section along with an Apply button and a Cancel button.  The Refine toolbar also contains a "+" and "-" button to zoom the view in and out on the image. There is also a "Apply" button that will first check of the image size is aligned to the project scale settings, and if it is, it will copy the image into the next open slot in the spritesheet.  The refine toolbar also contains a dropdown that allows the user to select between all images that have been loaded into the current project.  For images that have been applied to the sprite sheet, they will be highlighted in this dropdown.  If the currently shown image in the Refine pane has been applied to the sprite sheet, the cell in the spritesheet is outlined and highlighted.  Changes to a previously applied image are not automatically applied to the sprite sheet, instead there is a button "update" that will be highlighted if the image is out of sync with the spritesheet and if it is clicked, the current version of the image will be applied to the spritesheet in the appropriate cell. A button is shown in the Refine toolbar that toggles the visibility of a bullseye in the center of the image and a button that centers the image subject, invoking the same logic as Edit->Center

Compile:  Shows the projects spritesheet, presented as a grid as defined in "Set Spritesheet Size".  Each cell shows a thumbnail of the image it contains with a dotted line showing the division between the cells.  Clicking on a cell in the grid will select the image applied to this cell in the Refine window.


References:

Juce Framework:  
    https://github.com/juce-framework/JUCE
    https://juce.com/
    https://juce.com/blog/coding-standards/

Juce and CMake are installed in the development environment, a M1 based Mac.
    