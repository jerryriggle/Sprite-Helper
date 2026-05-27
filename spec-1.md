Add the following additional functions:

Edit
    Rotate (Left) -> Rotates the image loaded into the Refine section 5 degrees (configurable) to the left. This feature is also added to the Refine toolbar with a "left twist" arrow button.  If this button is clicked while holding the shift button, the rotation is locked to one of 0 degrees, 45 degrees, 90 degrees, etc..

    Rotate (Right) -> Rotates the image loaded into the Refine section 5 degrees (configurable) to the right. This feature is also added to the Refine toolbar with a "right twist" arrow button.  If this button is clicked while holding the shift button, the rotation is locked to one of 0 degrees, 45 degrees, 90 degrees, etc..

    Remove Background -> Removes the solid background from the image loaded in the Refine section and replaces it with transparency

Refine section

    In the refine section, if the image loaded in the Refine section is rotated and then the Apply button is clicked, the rotated image will be added to the next open cell in the spritesheet instead of updating the existing assigned cell.

Compile section

    In the compile section, cells can be re-arranged by dragging and dropping.  If an image is dropped on an empty cell, the image is moved to that cell.  If an image is dropped on a cell with an image assigned, then images in the cells will be swapped.  The drag and drop operation works with the typical mechanism where the image is clicked and held, which enables the drag, then released to initiate the drop action.

    In the compile section, if the number of cells is changed in the project settings, the pane is redrawn to fill the area only with the configured number of cells.  Padding with extra cells is allowed to complete a row if the number of cells does not fit evenly. Extra cells that are padding are greyed out and cannot be targeted by the drag and drop operation.