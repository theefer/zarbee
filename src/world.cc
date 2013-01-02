/* $Id: world.cc 292 2005-04-28 21:53:19Z marco $
 *
 * ZarBee - world.cc
 * 
 */
 
#include "world.hh"

using namespace ZarBee;

/**
* World Constructor.
* It sets the current camera to the first camera in the list.
* Get's the time object out of the parameter table.
*
* parameters:
* @param sceneModel    Full scene model tree
* @param cameras       List with all the available cameras
* @param light         List with all the lights
* @param animparams    List with all the animation parameters (fnode trees) 
*/
World::World(BeeThing*         sceneModel,
             vector<Camera*>*  cameras, 
             list<Light*>*     lights,
             ZarParamTable*    animparams,
             Music*            music,
             schedule_t*       cameraSchedule) 
    : sceneModel(sceneModel),
      cameras(cameras), 
      lights(lights),
      animParams(animparams),
      cameraSchedule(cameraSchedule),
      sceneSwitchTime(0) 
{
    for (int i=0; i<10; ++i) {
        if (setCurrentCamera(i))
            break;
    }
    
    // Get the time object out of the parameterlist
    time = dynamic_cast<Time*>(animparams->getParam("time"));

    // try to cache as much static parts of the scene as possible in display
    // lists
    sceneModel->cache();

    // That's it for now     
}
      
World::World(BeeThing*         sceneModel,
             vector<Camera*>*  cameras, 
             list<Light*>*     lights,
             ZarParamTable*    animparams,
             schedule_t*       cameraSchedule,
             int               sceneSwitchTime) 
    : sceneModel(sceneModel),
      cameras(cameras), 
      lights(lights),
      animParams(animparams),
      cameraSchedule(cameraSchedule),
      sceneSwitchTime(sceneSwitchTime)
{
    for (int i=0; i<10; ++i) {
        if (setCurrentCamera(i))
            break;
    }
    
    // Get the time object out of the parameterlist
    time = dynamic_cast<Time*>(animparams->getParam("time"));

    // try to cache as much static parts of the scene as possible in display
    // lists
    sceneModel->cache();

    // That's it for now     
}


/**
* World destructor
* 
* Cleans up everything: 
* - the scene model tree
* - the pamameter table
* - the lights
* - the cameras
* - the time object
*/
World::~World() {
    // FIXME. clean up more thoroughly
    delete sceneModel;
    delete animParams;
    delete lights;
    delete cameras;
    delete cameraSchedule;
}



//---------------------------------------------------------
// ACTIONS
//---------------------------------------------------------


/**
* Render one Frame
*/
void World::renderFrame() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // FIXME: eventually use glHint() for tuning.
    
    // Update the parameters
    animParams->updateCache();
    
    // activate the camera
    getCurrentCamera()->activate();
    
    // disable ambient light by default
    GLfloat ambLight[] = { 0.0, 0.0, 0.0, 1.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambLight);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    
    // activate lights after activating the camera
    list<Light*>::iterator lightIt = lights->begin();
    int i = 0;
    while ((lightIt != lights->end()) && (i < GL_MAX_LIGHTS)) {
        (*lightIt)->activate(GL_LIGHT0 + i);
        ++i;
        ++lightIt;
    }

    // draw recursively
    sceneModel->flush();
}

/**
* Render the Scene without drawing anything on the screen.
*/
void World::renderEmptyFrame() {
    // disable ambient light
    glEnable(GL_LIGHTING);
    GLfloat ambLight[] = { 0.0, 0.0, 0.0, 1.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambLight);

    // not activating any other light source

    glLoadIdentity();    
    // draw recursively
    sceneModel->flush();
}


//---------------------------------------------------------
// CURRENT ACTIVE CAMERA HANDLING
//---------------------------------------------------------

/**
* Set the current camera.
* @param  index      Camera's index
* @return true if the camera existed.
*/
bool World::setCurrentCamera(int index) {
    try {
        cameras->at(index);
    } catch (...) {
        return false;
    }
    currentCamera = index;
    return true;
}

/**
* Return the currently used camera object
*/
Camera* World::getCurrentCamera() {
    return cameras->at(currentCamera);
}


//---------------------------------------------------------
// GENERIC GETTERS
//---------------------------------------------------------

BeeThing* World::getSceneModel() {
    return sceneModel;
}

vector<Camera*>* World::getCameras() {
    return cameras;
}

list<Light*>* World::getLights() {
    return lights;
}

ZarParamTable* World::getAnimationParameters() {
    return animParams;
}

Time* World::getTime() {
    return time;
}
  
schedule_t* World::getCameraSchedule() {
    return cameraSchedule;
}
 
int World::getSceneSwitchTime() {
    return sceneSwitchTime;
}

