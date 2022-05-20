// DO NOT mess with this file.  If you do, animator will not work with
// your model, and you'll have to write a new one.  If you really really
// really need to do something here (unlikely) then don't complain if and
// when animator doesn't work.  -- Eugene
#ifndef MODELERAPP_H
#define MODELERAPP_H

#include "ModelerView.h"

// Forward declarations for ModelerApplication
class ModelerView;
class ModelerUserInterface;
class Fl_Box;
class Fl_Slider;
class Fl_Value_Slider;

// The ModelerApplication is implemented as a "singleton" design pattern,
// the purpose of which is to only allow one instance of it.
class ModelerApplication
{
public:
    ~ModelerApplication();

    // Fetch the global ModelerApplication instance
    static ModelerApplication *Instance();

    // Initialize the application; see sample models for usage
    void Init(int argc, char* argv[], string jointNames[]);

    // Starts the application, returns when application is closed
    int Run();

    // Get and set slider values.
    double GetControlValue(int controlNumber);
    void SetControlValue(int controlNumber, double value);
    unsigned GetNumControls();

    // Extra: Get the mapping from control to selector
    int getControlToSelector(int controlIndex);
    // Extra: Get the mapping from joint to control values
    Vector3f getJointToControlValues(int modelIndex, int jointIndex, bool isTranslation);
    // Extra: Check whether a control type is translation
    bool getControlIsTranslation(int controlIndex);

    // Redraw trigger
    void redrawControlsWindow();

private:
    // Private for singleton
    ModelerApplication() : m_numControls(-1) { }
    ModelerApplication(const ModelerApplication &) { }

    // The instance
    static ModelerApplication *m_instance;

    friend class ModelerUserInterface;

    void ShowControl(int controlNumber);
    void HideControl(int controlNumber);

    ModelerUserInterface * m_ui;

    int m_numControls;

    // Mapping between controls, selectors and joints
    vector<pair<int, int>> m_controlToJoint;
    vector<int> m_controlToSelector;

    // Control type (translation / rotation)
    vector<bool> m_controlIsTranslation;

    Fl_Box ** m_controlLabelBoxes;
    Fl_Value_Slider ** m_controlValueSliders;

    static void SliderCallback(Fl_Slider *, void *);
    static void RedrawLoop(void *);
};

#endif
