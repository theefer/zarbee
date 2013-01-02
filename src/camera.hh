/* $Id: camera.hh 223 2005-04-23 15:01:26Z marco $
 *
 * ZarBee - camera.hh
 * 
 */
 
#include <GL/gl.h>

#include "bee_primitives.hh"


#ifndef CAMERA_HH
#define CAMERA_HH

namespace ZarBee {

/**
* Abstract camera base class
*/
class Camera {

public:
    virtual ~Camera();
    
    /** 
    * Initialize the perspective matrix (sets the frustum). Needs to be called 
    * whenever the viewport size changes.
    */
    virtual void init(int viewportWidth, int viewportHeight);
    
    /** Called upon camera-move-up user event */
    virtual void startMoveUp();
    virtual void stopMoveUp();
    /** Called upon camera-move-down user event */
    virtual void startMoveDown();
    virtual void stopMoveDown();
    /** Called upon camera-move-left user event */
    virtual void startMoveLeft();
    virtual void stopMoveLeft();
    /** Called upon camera-move-right user event */
    virtual void startMoveRight();
    virtual void stopMoveRight();
    /** Called upon camera-turn-horizontal user event */
    virtual void turnHorizontal(GLfloat angle);
    /** Called upon camera-turn-vertical user event */
    virtual void turnVertical(GLfloat angle);
    
    /** Returns if the camera is movable */
    bool isMovable();
    
    /** Returns if the camera is turnable */
    bool isTurnable();

    /** Activat the camera.
    * Called before rendering. Applies the necessary changes on the perspetive
    * matrix stack. The camera's inclination is always zero (up-vector=(0,1,0)).
    */
    virtual void activate() = 0;
    
protected:
    /** Generic camera constructor.
    * @param position  BeePoint holding the position.
    * @param lookAt    BeePoint holding the look-at or reference point.
    * @param openingCoef  Opening coefficient. 1.0 corresponds to 45° (1-to-1).
    */
    Camera(BeePoint* position, BeePoint* lookAt, GLfloat openingCoef);

    BeePoint* position;
    BeePoint* lookAt;
    
    bool movable;
    bool turnable;
    
private:
    GLfloat openingCoef;
};


/**
* Fixed camera.
* This camera is fixed by two (not necessarily static) points in the scene.
* It does not support any user interaction.
*/
class FixedCamera: public Camera {
public:
    FixedCamera(BeePoint* position, BeePoint* lookAt, GLfloat openingCoef);
    virtual ~FixedCamera();  
    virtual void activate();
};


/**
* Free look camera.
* The position is fixed by a BeePoint. The look-at direction is relative to
* the reference point and can be changed by user-interaction (mouse-movement).
*/
class FreeLookCamera: public Camera {
public:
    FreeLookCamera(BeePoint* position, 
                   BeePoint* reference, 
                   GLfloat openingCoef);
    ~FreeLookCamera();
    virtual void activate();
    virtual void turnHorizontal(GLfloat angle);
    virtual void turnVertical(GLfloat angle);
private:
    GLfloat horizontalAngle;
    GLfloat verticalAngle;
};

class PanCamera: public Camera {
public:
    PanCamera(BeePoint* initalPosition, 
              BeePoint* initialLookAt, 
              GLfloat openingCoef);
    ~PanCamera();
    virtual void activate();
    virtual void startMoveUp();
    virtual void startMoveDown();
    virtual void startMoveLeft();
    virtual void startMoveRight();
    virtual void stopMoveUp();
    virtual void stopMoveDown();
    virtual void stopMoveLeft();
    virtual void stopMoveRight();
protected:
    GLfloat hPanAngle;
    GLfloat vPanAngle;
    bool movingUp;
    bool movingDown;
    bool movingLeft;
    bool movingRight;
};

class FreeLookPanCamera: public PanCamera {
public:
    FreeLookPanCamera(BeePoint* initalPosition, 
                      BeePoint* initialLookAt, 
                      GLfloat openingCoef);
    ~FreeLookPanCamera();
    virtual void activate();
    virtual void turnHorizontal(GLfloat angle);
    virtual void turnVertical(GLfloat angle);
private:
    GLfloat horizontalAngle;
    GLfloat verticalAngle;
};

}  // namespace

#endif  /* CAMERA_HH */
