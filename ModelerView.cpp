#include "ModelerView.h"
#include "camera.h"
#include "modelerapp.h"

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <GL/glu.h>
#include <cstdio>

ModelerView::ModelerView(int x, int y, int w, int h,
             const char *label):Fl_Gl_Window(x, y, w, h, label)
{
    m_camera = new Camera();

    m_camera->SetDimensions( w, h );
    m_camera->SetDistance( 2 );
    m_camera->SetCenter( Vector3f( 0.5, 0.5, 0.5 ) );

    m_drawAxes = true;
    m_drawSkeleton = false;
}

// If you want to load files, etc, do that here.
void ModelerView::loadModels(int argc, char* argv[])
{
    glutInit( &argc, argv );

    // Load models based on the command-line arguments
    for (int i = 1; i < argc; ++i) {
        string prefix = argv[i];
        string skeletonFile = prefix + ".skel";
        string meshFile = prefix + ".obj";
        string attachmentsFile = prefix + ".attach";

        SkeletalModel model = SkeletalModel();
        model.load(skeletonFile.c_str(), meshFile.c_str(), attachmentsFile.c_str());
        models.push_back(model);
    }
}

vector<vector<Joint*>> ModelerView::getJointsPerModel()
{
    auto nums = vector<vector<Joint*>>();
    for (auto &model : models)
        nums.push_back(model.getJoints());
    return nums;
}

ModelerView::~ModelerView()
{
    delete m_camera;
}

int ModelerView::handle( int event )
{
    unsigned eventCoordX = Fl::event_x();
    unsigned eventCoordY = Fl::event_y();
    unsigned eventButton = Fl::event_button();
    unsigned eventState  = Fl::event_state();

    switch( event )
    {
    case FL_PUSH:
        {
            switch (eventButton)
            {
                case FL_LEFT_MOUSE:
                    m_camera->MouseClick( Camera::LEFT, eventCoordX, eventCoordY );
                    break;

                case FL_MIDDLE_MOUSE:
                    m_camera->MouseClick( Camera::MIDDLE, eventCoordX, eventCoordY );
                    break;

                case FL_RIGHT_MOUSE:
                    m_camera->MouseClick( Camera::RIGHT, eventCoordX, eventCoordY );
                    break;
            }
        }
        break;

    case FL_DRAG:
        {
            m_camera->MouseDrag(eventCoordX, eventCoordY);
        }
        break;

    case FL_RELEASE:
        {
            m_camera->MouseRelease(eventCoordX, eventCoordY);
        }
        break;

    case FL_KEYUP:
        {
            unsigned key = Fl::event_key();

            if( key == 'a' )
            {
                m_drawAxes = !m_drawAxes;
                cout << "drawAxes is now: " << m_drawAxes << endl;
            }
            // added the color trigger
            else if (key == 'c')
            {
                if (m_drawSkeleton == 0) {
                    m_drawColor = !m_drawColor;
                    if (m_drawColor) {
                        glEnable(GL_COLOR_MATERIAL);
                    }
                    else {
                        glDisable(GL_COLOR_MATERIAL);
                    }
                }
                else {
                    glDisable(GL_COLOR_MATERIAL);
                }
            }
            else if (key == 's')
            {
                m_drawSkeleton = !m_drawSkeleton;
                cout << "drawSkeleton is now: " << m_drawSkeleton << endl;
                if (m_drawSkeleton == 0) {
                    if (m_drawColor) {
                        glEnable(GL_COLOR_MATERIAL);
                    }
                    else {
                        glDisable(GL_COLOR_MATERIAL);
                    }
                }
                else {
                    glDisable(GL_COLOR_MATERIAL);
                }
            }
        }
        break;

    default:
        return 0;
    }

    redraw();

    return 1;
}

void ModelerView::update()
{
    // update the skeleton from sliders
    updateJoints();

    for (auto &model : models) {
        // Update the bone to world transforms for SSD.
        model.updateCurrentJointToWorldTransforms();

        // update the mesh given the new skeleton
        model.updateMesh();
    }
}

void ModelerView::updateJoints()
{
    auto app = ModelerApplication::Instance();

    // Iterate each model
    for (int modelIndex = 0, numModels = models.size(); modelIndex < numModels; ++modelIndex) {
        auto &model = models[modelIndex];
        // Set translation of root joint
        Vector3f ret = app->getJointToControlValues(modelIndex, 0, true);
        model.setRootTranslation(ret[0], ret[1], ret[2]);

        // Set transform (currently rotation only) of all joints
        for (int jointIndex = 0, numJoints = model.getJoints().size(); jointIndex < numJoints; jointIndex++)
        {
            ret = app->getJointToControlValues(modelIndex, jointIndex, false);
            model.setJointTransform(jointIndex, ret[0], ret[1], ret[2]);
        }
    }
}

// Call the draw function of the parent.  This sets up the
// viewport, projection, and camera position, as well as some
// default lighting parameters.
void ModelerView::draw()
{
    // Window is !valid() upon resize
    // FLTK convention has you initializing rendering here.
    if( !valid() )
    {
        // Setup opengl
        glShadeModel( GL_SMOOTH );
        glEnable( GL_DEPTH_TEST );
        glEnable( GL_LIGHTING );
        glEnable( GL_LIGHT0 );
        glEnable( GL_NORMALIZE );

        m_camera->SetDimensions(w(),h());
        m_camera->SetViewport(0,0,w(),h());
        m_camera->ApplyViewport();

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        m_camera->SetPerspective( 50.0f );
        glLoadMatrixf( m_camera->projectionMatrix() );
    }

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Note that the lighting is applied *before* applying the camera
    // transform.  This is so the light appeared fixed on the camera.
    GLfloat Lt0diff[] = {1.0,1.0,1.0,1.0};
    GLfloat Lt0pos[] = {3.0,3.0,5.0,1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, Lt0diff);
    glLightfv(GL_LIGHT0, GL_POSITION, Lt0pos);

    // These are just some default material colors
    GLfloat diffColor[] = {0.4f, 0.4f, 0.4f, 1.f};
    GLfloat specColor[] = {0.6f, 0.6f, 0.6f, 1.f};
    GLfloat shininess[] = {50.0f};

    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffColor );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specColor );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, shininess );

    // Load the camera view matrix
    glLoadMatrixf( m_camera->viewMatrix() );

    if(m_drawAxes)
    {
        drawAxes();
    }

    for (auto &model : models)
        model.draw( m_camera->viewMatrix(), m_drawSkeleton );
}

void ModelerView::drawAxes()
{
    glDisable( GL_LIGHTING );
    glBegin( GL_LINES );

    glColor3f( 1, 0, 0 );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 1, 0, 0 );

    glColor3f( 0, 1, 0 );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 1, 0 );

    glColor3f( 0, 0, 1 );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 0, 1 );

    glEnd();
    glEnable( GL_LIGHTING );
}
