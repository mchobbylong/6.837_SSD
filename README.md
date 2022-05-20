# Hierarchical Model and SSD

This is an enhanced version of housework "Hierarchical Model and SSD" from MIT 6.837 course.

In addition to the basic functions (described in the homework requirement), some extra functions (as follows) are also implemented.

## Compiling & Running Environment

The code is mostly written in Windows. Tested on Visual Studio 2019.

## Extra Functions

### Multiple Models Support

Support loading multiple models.

**Usage:** Append filenames of multiple models as command line parameters:

`a3 data/Model1 data/Model2`

### Model Translation

Implemented by controling translation offsets of the root joint.

**Usage:** Drag the sliders corresponding the browser item `Root (Translation)`.

### Colored Vertices

Triangle faces are colored according to the numbers of binding joints of the 3 vertices.

**Usage:** Press "c" to toggle vertex coloring when in "mesh view" mode.

**Configuration:**

Modify [L26 of `Mesh.cpp`](Mesh.cpp#L26) to use different color schemes. Possible strings after `#define`:
- `COLOR_SCHEME_1`
- `COLOR_SCHEME_2`

### Model Animation

Users can create and display simple animations.

**Usage:**

1. Use the slider controls provided in the user interface to pose the model for the first keyframe.
2. Click the `Save Position File` option on the menu and save it as a `.pos` file.
3. Pose the model for the next keyframe. Make sure that the model does not change too much compared to the previous keyframe pose.
4. Click the `Save Position File` option on the menu and save it as another `.pos` file.
5. Repeat steps 3~4 until all keyframe poses have been saved to the corresponding `.pos` files.
6. Write an `.anim` file in plain text format and store it in the same directory with all `.pos` files.

   In the ".anim" file, the first line is the filename of the first keyframe pose, and each subsequent line has a decimal number and a filename (separated by a space) indicating the interpolation time (in seconds) between the current keyframe and the previous keyframe, and the filename of the current keyframe pose, respectively.

   See [animation/Model1.anim](animation/Model1.anim) for example.

7. Load the `.anim` file in our program using the `Load Animation File` option on the menu.
8. Click the `Play Animation Once` or `Play Animation Repeatedly` option on the menu to play the animation.

**Configuration:**

The number of frames per second (FPS) can be changed by modifying [L311 of `modelerui.cpp`](modelerui.cpp#L311), which is the initial value of `m_animateFps` variable.
