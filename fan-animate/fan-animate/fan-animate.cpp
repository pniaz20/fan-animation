/***************************************************************
 * Fan Animation
 * Author: Pouya Pourakbarian Niaz
 * email: pniaz20@ku.edu.tr
 * Last update: April 15 2021
 ***************************************************************/

 //File Input Headers for reading from VRML file
#include <Inventor/SoInput.h>


 // SoWin headers for the Scene Viewer:
#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>
#include <Inventor/Win/SoWinRenderArea.h>

// Node headers for creating and manipulating nodes:
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>

// Action headers for creating and manipulating actions such as writing to a file:
#include <Inventor/actions/SoWriteAction.h>

// Event headers for creating and manipulating events such as pressing a keybord button:
#include <Inventor/events/SoEvent.h>
#include <Inventor/events/SoKeyboardEvent.h>

// Sensor headers for craeting and manipulating sensors such as a timer sensor for animating objects:
#include <Inventor/sensors/SoSensor.h>
#include <Inventor/sensors/SoTimerSensor.h>

// Basic C++ headers:
#include <iostream>
#include <stdlib.h>

// Defining the namspace to be used throughout the program, e.g. for 'cout' commands:
using namespace std;

// Global definitions:
float rate = 0.0;                                           // Angular velocity coefficient of the animation
float targetRate = 0.0;                                     // Target Angular Velocity Coefficient
int accel = 0;                                              // Angular acceleration determiner of the animation
bool killSwitch = FALSE;                                    // Key to determine the sudden stop
float accelRate = 0.0;                                      // Angular Acceleration Coefficient
float diff = 0.0;                                           /*Difference between the current angular velocity
                                                            and the desired angular velocity */

                                                            // User-Defined Global Parameters:
float slow = 0.05;                                          // Slow Acceleration Coefficient
float fast = 0.5;                                           // Fast Acceleration Coefficient
int fps = 30;                                               // Animation frame rate (Frames Per Second)
float rateIncr = 2.0;                                       // Target Angular Velocity Increment at Key Press


/**********************************  Describing the Key Press Event Callback Function:  **************************************/

void myKeyPressCB(void* userData, SoEventCallback* eventCB)
{
    void rotSensorCB(void* data, SoSensor*);                // Function prototype for the timer sensor callback function
    SoTimerSensor* tSensor = (SoTimerSensor*)userData;      // Treat userData like a timer sensor
    const SoEvent* event = eventCB->getEvent();             // Find out how the event is defined

    tSensor->setInterval(1.0 / fps);                        // Maintain FPS

    if (SO_KEY_PRESS_EVENT(event, UP_ARROW))                // If the up arrow is pressed
    {
        tSensor->unschedule();                              // Reset the timer
        killSwitch = FALSE;                                 // Continue animation
        targetRate += rateIncr;                             // Increase turn rate by one increment
        cout << "The Speed is set to: " << (targetRate * fps / 6) << " RPM." << endl;
        accelRate = slow;                                   // Set the acceleration rate to slow
        tSensor->setFunction(rotSensorCB);                  // Set the timer sensor's callback function
        tSensor->schedule();                                // Schedule the timer
        eventCB->setHandled();                              // Event was handled
    }
    else if (SO_KEY_PRESS_EVENT(event, DOWN_ARROW))
    {
        tSensor->unschedule();
        killSwitch = FALSE;
        targetRate -= rateIncr;                             // Decrease turn rate by one increment
        cout << "The Speed is set to: " << (targetRate * fps / 6) << " RPM." << endl;
        accelRate = slow;
        tSensor->setFunction(rotSensorCB);
        tSensor->schedule();
        eventCB->setHandled();
    }
    else if (SO_KEY_PRESS_EVENT(event, LEFT_ARROW))
    {
        tSensor->unschedule();
        killSwitch = FALSE;
        targetRate = 0.0;                                   // Stop the fan
        cout << "The Speed is set to: " << (targetRate * fps / 6) << " RPM." << endl;
        accelRate = fast;                                   // Set the acceleration rate to fast
        tSensor->setFunction(rotSensorCB);
        tSensor->schedule();
        eventCB->setHandled();
    }
    else if (SO_KEY_PRESS_EVENT(event, RIGHT_ARROW))
    {
        tSensor->unschedule();
        killSwitch = TRUE;                                  // Kill the animation
        targetRate = 0;                                     // Reset targetRate to zero
        cout << "The animation is stopped and the fan position is reset." << endl;
        tSensor->setFunction(rotSensorCB);
        tSensor->schedule();
        eventCB->setHandled();
    }
    /* Note that you cannot set the function for the timer sensor
    and set the event to 'handled' here. It has to be done within each case of the IF ELSE block.*/
}

/**********************************  Describing the Rotation Timer Sensor Callback Function:  ********************************/

void rotSensorCB(void* data, SoSensor*)
{
    SoRotation* rot = (SoRotation*)data;                    // Treat data like a rotation matrix.
    SbRotation currentRot = rot->rotation.getValue();       // Get the current angular positoin.

    diff = targetRate - rate;                               // Evaluate the velocity deficit
    accel = (diff > 0.01) ? 1 : (diff < -0.01) ? -1 : 0;    // Decide if acceleration or decelaration should happen
    rate += accel * accelRate;                              // Change the velocity value according to the acceleration rate

    //Check to see if killSwitch (Key to reset) is TRUE or FALSE:
    if (killSwitch == FALSE) //keep going
    {
        currentRot = SbRotation(SbVec3f(0, 1, 0),          // Rotate the model 1 degree times the velocity coefficient
            rate * M_PI / 180.0) * currentRot;
        rot->rotation.setValue(currentRot);                // Replace the rotation matrix for the model in order to turn it.
    }
    else //stop and reset fan
    {
        rot->rotation.setValue(SbVec3f(0, 1, 0), 0.0);     // Return to base.
        rate = 0.0;                                        // Reset the velocity coefficient
    }

}

/***************************************************  The Main Function:  ****************************************************/
int main(int, char** argv)
{
    // Initializing the Window:
    HWND window = SoWin::init(argv[0]);                     /*'HWND' is the SoWin equivalent of
                                                            'Widget' in SoXt-using programs.*/
    if (window == NULL)
        exit(1);                                            // If the windows is not initialized, exit the console.
    else
        cout << ">>> Viewer Window is Initialized." << endl;

    // Initializing the Viewer:
    SoWinExaminerViewer* viewer =                           /*The scene viewer object is what will be used for
                                                            rendering the scene graph. */
        new SoWinExaminerViewer(window);

    // Generate and Refer to the Root Node:
    SoSeparator* root = new SoSeparator;
    root->setName("Root Node");
    root->ref();                                            /* The root node had better be manually referenced in order
                                                            not to leave it unreferenced, since it has no parent node
                                                            referring to it. */
    cout << ">>> The root node is referenced." << endl;

    // Construct the active rotation matrix:
    SoRotation* myRot = new SoRotation;                     /* This is the active rotation matrix of the entire
                                                            scene graph, dynamically changing every moment in order to
                                                            animate the model. */
    myRot->setName("Active Rotation Matrix");

    // Declare the Timer Sensor:
    SoTimerSensor* rotatingSensor = new SoTimerSensor();    /* Note that the callback function and the user data to be used
                                                            could also have been declared inline here along with the timer
                                                            sensor itself, but that would not help us with our goals. */

    rotatingSensor->setData(myRot);                         /* Set the user data that will be used/manipulated/affected
                                                            by this particular timer sensor's callback function. */

                                                            // Declare the Key Press Event Callback:
    SoEventCallback* myEventCB = new SoEventCallback;
    myEventCB->setName("My Event Callback");
    myEventCB->                                             // Add the event callback function, object, and user data.
        addEventCallback(
            SoKeyboardEvent::getClassTypeId(),              // Automatically get the class type of the keyboard event.
            myKeyPressCB,                                   // The event Callback function to be used for this event node.
            rotatingSensor);                                /* User data that will be used/affected/manipulated by this event's
                                                            callback function. */


    // NOTE: Time sensors are not actually nodes, and therefore cannot be added to the scene graph.

    /***************************************  Separator and Group Declarations  **********************************************/

    SoSeparator* bladeSep = new SoSeparator;            bladeSep->setName("Blade Separator Node");
    SoSeparator* frameSep = new SoSeparator;            frameSep->setName("Frame Separator Node");
    SoGroup* viewGroup = new SoGroup;                   viewGroup->setName("Viewing Group Node");


    /********************************************************  NODES  *******************************************************/
    /* Note that all CAD applications and  Graphics software are capable of opening and writing VRML (.wrl)
    files. The files in the current directory were downloaded from www.grabcad.com as .sldprt files,
    opened in SolidWorks, and simply saved as .wrl file.
    Open Inventor and Coin3D have VRML 1.0 and VRML 2.0 support. */

    //VRML Geometries
    SoSeparator* bladeBody = new SoSeparator;
    bladeBody->setName("Blade VRML Geometry Node");
    SoInput bladeInput;
    if (!bladeInput.openFile("noctua nf-a15 blade.wrl")) {  // Open the VRML file 
        cout << "\n>>> Cannot open Blade input file, replacing it with empty scene." << endl;
        bladeBody = new SoSeparator;
    }
    else
    {
        bladeBody = SoDB::readAll(&bladeInput);
        cout << "\n>>> Blade successfully imported from VRML file." << endl;
    }
    bladeInput.closeFile();

    SoSeparator* frameBody = new SoSeparator;
    frameBody->setName("Frame VRML Geometry Node");
    SoInput frameInput;
    if (!frameInput.openFile("noctua nf-a15 frame.wrl")) {
        cout << "\n>>> Cannot open Frame input file, replacing it with empty scene." << endl;
        frameBody = new SoSeparator;
    }
    else
    {
        frameBody = SoDB::readAll(&frameInput);
        cout << ">>> Frame successfully imported from VRML file.\n" << endl;
    }
    frameInput.closeFile();

    //Window Camera
    SoPerspectiveCamera* cam = new SoPerspectiveCamera;     /* This camera is used for getting a better, slightly
                                                            zoomed-out view of the model. */

                                                            // Transformations:
    SoTransform* globalTrans = new SoTransform;             // For Turning the whole model upwards to be able to see it
    globalTrans->setName("Global Transformation");
    globalTrans->translation.setValue(0.0, 0.0, 0.0);
    globalTrans->rotation.setValue(SbVec3f(1, 0, 0), M_PI / 2.0);

    SoTransform* bladeTrans = new SoTransform;              // To place the blade correctly in the frame
    bladeTrans->setName("Blade Transformation");
    bladeTrans->translation.setValue(0.0, 0.005, 0.0);

    cout << ">>> The model components are built." << endl;

    /*********************************************  Setting up the Scene Graph  *********************************************/

    root->addChild(viewGroup);
    root->addChild(frameSep);
    root->addChild(bladeSep);

    viewGroup->addChild(cam);
    viewGroup->addChild(globalTrans);

    frameSep->addChild(frameBody);

    bladeSep->addChild(bladeTrans);
    bladeSep->addChild(myEventCB);                          // Add the Event node to the scene graph
    bladeSep->addChild(myRot);                              // Add the animated rotation matrix to the scene graph
    bladeSep->addChild(bladeBody);

    cout << ">>> The scene graph is generated." << endl;

    /************************************************  Setting up the viewer ************************************************/

    cout << ">>> Setting up the viewer ..." << endl;

    viewer->setSize(SbVec2s(750, 750));                     // Set the size of the viewer window in pixels
    SbViewportRegion myRegion(viewer->getSize());           // Get the camera viewport region.
    cam->viewAll(root, myRegion);                           // Set the camera to view the whole region.
    SbVec3f initialPos;
    initialPos = cam->position.getValue();                  // Get the current view point position.
    float x, y, z;
    initialPos.getValue(x, y, z);                           // Decompose the vector to its compoennts
    cam->position.setValue(0.0, 0.0, z + z / 5.);           // Zoom back a little to see the whole model better

    viewer->setSceneGraph(root);                            // Set the (part of the) scene graph to be viewed
    viewer->setTitle("Pouya Pourakbarian Niaz - Fan Animation");
    float a, b, c;
    a = 207.0 / 255.0;
    b = 231.0 / 255.0;
    c = 253.0 / 255.0;
    viewer->setBackgroundColor(SbColor(a, b, c));           // Set the viewer background color to light blue

    cout << ">>> The viewer is set up.\n" << endl;
    cout << ">>> Wait for a few seconds, then press Enter to launch the viewer: ";
    cin.get(); // press enter

    viewer->show();                                         // Show the selected scene graph in the viewer

    SoWin::show(window);                                    // Open the window
    SoWin::mainLoop();

    // Once the viewer's window is manually closed by the user...
    cout << "\n>>> The viewer is closed." << endl;

    /*******************************************  Writing to an IV file  *****************************************************/

    cout << ">>> Generating the IV file..." << endl;
    SoWriteAction myAction;
    myAction.getOutput()->openFile("AnimateFan.iv");
    myAction.getOutput()->setBinary(FALSE);
    myAction.apply(root);
    myAction.getOutput()->closeFile();
    cout << endl << ">>> The output file is generated as AnimateFan.iv\n" << endl;

    cout << ">>> The Program is terminated. GOOD BYE!\n" << endl;
    return 0;
}
