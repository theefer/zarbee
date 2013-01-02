/* $Id: world.hh 292 2005-04-28 21:53:19Z marco $
 *
 * ZarBee - world.hh
 * 
 */

#include <vector>
#include <list>

#include "bee_base.hh"
#include "zar_param_table.hh"
#include "light.hh"
#include "camera.hh"
#include "time.hh"
#include "music.hh"

#ifndef WORLD_HH
#define WORLD_HH

using namespace std;

namespace ZarBee {

typedef list<pair<int,int> > schedule_t;

class World {

public:
    World(BeeThing*         sceneModel,
          vector<Camera*>*  cameras, 
          list<Light*>*     lights,
          ZarParamTable*    animparams,
          Music*            music,
          schedule_t*       cameraSchedule);
          
    World(BeeThing*         sceneModel,
          vector<Camera*>*  cameras, 
          list<Light*>*     lights,
          ZarParamTable*    animparams,
          schedule_t*       cameraSchedule,
          int               sceneSwitchTime);
                
    ~World();
    
    // Actions
    void renderFrame();
    void renderEmptyFrame();
    
    // Camera methods
    bool setCurrentCamera(int index);
    Camera* getCurrentCamera();
    
    // Generic getters
    BeeThing*         getSceneModel();
    vector<Camera*>*  getCameras();
    list<Light*>*     getLights();
    ZarParamTable*    getAnimationParameters();
    Time*             getTime();
    schedule_t*       getCameraSchedule();
    int               getSceneSwitchTime();
    
private:
    BeeThing*         sceneModel;
    vector<Camera*>*  cameras;
    list<Light*>*     lights;
    ZarParamTable*    animParams;
    int               currentCamera;
    Time*             time;
    schedule_t*       cameraSchedule;
    int               sceneSwitchTime;
    
    void clearScreen();
    void updateAnimParams();
    void disableCamera();
    void placeCamera();
    void placeLights();
};

}  // Namespace

#endif  /* WORLD_HH */
