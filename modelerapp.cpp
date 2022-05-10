#include "modelerapp.h"
#include "ModelerView.h"
#include "modelerui.h"

#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Box.H>

#include <cstring>
#include <cstdio>
#include <cstdlib>



// Set the singleton initially to a NULL instance
ModelerApplication *ModelerApplication::m_instance = NULL;

// CLASS ModelerApplication METHODS

ModelerApplication *ModelerApplication::Instance()
{
    // Make a new instance if none exists, otherwise, return
    // the existing instance of the ModelerApplication
    return (m_instance) ? (m_instance) : (m_instance =
                      new ModelerApplication());
}

void ModelerApplication::Init( int argc, char* argv[], string jointNames[] )
{
    m_ui = new ModelerUserInterface();

    // Make sure that we remove the view from the
    // Fl_Group, otherwise, it'll blow up
    // THIS BUG FIXED 04-18-01 ehsu
    m_ui->m_modelerWindow->remove(*(m_ui->m_modelerView));
    delete m_ui->m_modelerView;

    // Load all models, then determine the number of controls
    m_ui->m_modelerWindow->begin();

    m_ui->m_modelerView = new ModelerView(0, 0, m_ui->m_modelerWindow->w(), m_ui->m_modelerWindow->h(), NULL);
    m_ui->m_modelerView->loadModels(argc, argv);

    Fl_Group::current()->resizable(m_ui->m_modelerView);
    m_ui->m_modelerWindow->end();

    // Initial state of animation
    m_animating = false;

    // ********************************************************
    // Create the FLTK user interface
    // ********************************************************


    // Constants for user interface setup
    const int textHeight = 20;
    const int sliderHeight = 20;
    const int packWidth = m_ui->m_controlsPack->w();

    // Determine the total number of controls
    vector<int> numJoints = m_ui->m_modelerView->getJointsPerModel();
    m_numControls = 0;
    for (int num : numJoints)
        m_numControls += (num + 1) * 3; // Extra control for root joint translation

    // Store pointers to the controls for manipulation
    m_controlLabelBoxes = new Fl_Box *[m_numControls];
    m_controlValueSliders = new Fl_Value_Slider *[m_numControls];

    // Initialize controls for every model
    int controlIndex = 0, selectorIndex = 1;
    m_ui->m_controlsPack->begin();
    for (int modelIndex = 0, numModels = numJoints.size(); modelIndex < numModels; ++modelIndex) {
        // Note that we have an extra control for adjusting translation of the root joint, modelNumJoints is actually + 1
        int modelNumJoints = numJoints[modelIndex] + 1;

        // Add "root" selector (as a label only) for every model
        m_ui->m_controlsBrowser->add(argv[modelIndex + 1]);
        ++selectorIndex;

        // Then for each joint, add appropriate objects to the user interface
        char selectorLabel[256];
        for (int jointIndex = 0; jointIndex < modelNumJoints; ++jointIndex) {
            // Add a corresponding selector for the joint, label indented by 4 spaces
            sprintf(selectorLabel, "    %s", jointNames[jointIndex].c_str());
            m_ui->m_controlsBrowser->add(selectorLabel);

            // Then add 3 sliders (each wrapped in a label box) for this joint
            for (int k = 0; k < 3; ++k) {
                // Determine name of the slider
                char* buf = new char[256];
                sprintf(buf, "%s %c", jointNames[jointIndex].c_str(), 'X' + k);

                // Add a label box (as a wrapper for the slider), make it hidden for now
                Fl_Box *box = new Fl_Box(0, 0, packWidth, textHeight, buf);
                box->labelsize(10);
                box->hide();
                box->box(FL_FLAT_BOX);	// otherwise, Fl_Scroll messes up (ehsu)
                m_controlLabelBoxes[controlIndex] = box;

                // Add a slider, make it hidden for now
                Fl_Value_Slider *slider = new Fl_Value_Slider(0, 0, packWidth, sliderHeight, 0);
                slider->type(1);
                // Determine params of the slider, according to the control target of this control (translation / rotation)
                if (jointIndex == 0) {
                    // Root joint translation
                    slider->range(-1, 1);
                    slider->step(0.05f);
                } else {
                    // Normal joint rotation
                    slider->range(-M_PI, M_PI);
                    slider->step(0.1f);
                }
                slider->value(0);
                slider->hide();
                m_controlValueSliders[controlIndex] = slider;
                // Set slider callback
                slider->callback((Fl_Callback *) ModelerApplication::SliderCallback);

                // Finally, specify the mapping from this slider to its corresponding selector / joint
                m_controlToSelector.push_back(selectorIndex);
                m_controlToJoint.push_back(pair<int, int>(modelIndex, jointIndex == 0 ? 0 : jointIndex - 1));
                m_controlIsTranslation.push_back(jointIndex == 0);

                // Index for next control
                ++controlIndex;
            }

            // Index for next selector
            ++selectorIndex;
        }
    }
    m_ui->m_controlsPack->end();
}

ModelerApplication::~ModelerApplication()
{
    // FLTK handles widget deletion
    delete m_ui;
    delete [] m_controlLabelBoxes;
    delete [] m_controlValueSliders;
}

int ModelerApplication::Run()
{
    if (m_numControls == -1) {
    fprintf(stderr,
        "ERROR: ModelerApplication must be initialized before Run()!\n");
    return -1;
    }

    // Just tell FLTK to go for it.
    Fl::visual(FL_RGB | FL_DOUBLE);
    m_ui->show();

    return Fl::run();
}

double ModelerApplication::GetControlValue(int controlNumber)
{
    return m_controlValueSliders[controlNumber]->value();
}

void ModelerApplication::SetControlValue(int controlNumber, double value)
{
    m_controlValueSliders[controlNumber]->value(value);
}

unsigned ModelerApplication::GetNumControls()
{
    return m_numControls;
}

bool ModelerApplication::GetAnimating()
{
    return m_animating;
}

void ModelerApplication::ShowControl(int controlNumber)
{
    m_controlLabelBoxes[controlNumber]->show();
    m_controlValueSliders[controlNumber]->show();
}

void ModelerApplication::HideControl(int controlNumber)
{
    m_controlLabelBoxes[controlNumber]->hide();
    m_controlValueSliders[controlNumber]->hide();
}

void ModelerApplication::SliderCallback(Fl_Slider *, void *)
{
    auto app = ModelerApplication::Instance();
    app->m_ui->m_modelerView->update();
    app->m_ui->m_modelerView->redraw();
}

int ModelerApplication::getControlToSelector(int controlIndex) {
    return m_controlToSelector[controlIndex];
}

Vector3f ModelerApplication::getJointToControlValues(int modelIndex, int jointIndex, bool isTranslation) {
    for (int i = 0; i < m_numControls; ++i) {
        auto p = m_controlToJoint[i];
        if (p.first == modelIndex && p.second == jointIndex && m_controlIsTranslation[i] == isTranslation)
            return Vector3f(GetControlValue(i), GetControlValue(i + 1), GetControlValue(i + 2));
    }
    return NULL;
}

void ModelerApplication::redrawControlsWindow() {
    m_ui->m_controlsWindow->redraw();
}
