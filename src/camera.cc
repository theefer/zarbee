/* $Id: camera.cc 292 2005-04-28 21:53:19Z marco $
 *
 * ZarBee - camera.cc
 * 
 */
 
#include "camera.hh"

#include <GL/glu.h>

#include <SDL.h>

#define FRUSTUM_NEAR 0.2
#define FRUSTUM_FAR 20
#define KEYBOARD_SPEED 1

#include <iostream>
using namespace std;
using namespace ZarBee;

//---------------------------------------------------------
// GENERIC CAMERA CLASS
//---------------------------------------------------------

Camera::Camera(BeePoint* position, BeePoint* lookAt, GLfloat openingCoef) : 
    position(position),
    lookAt(lookAt),
    openingCoef(openingCoef) {}

Camera::~Camera() {}

void Camera::init(int viewportWidth, int viewportHeight) {
    // set Viewport
    glViewport(0, 0, viewportWidth, viewportHeight);

    // prepare the projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glLoadIdentity();

    // adjust the aspect ratio
    GLfloat nearWidth = 1.0;
    GLfloat nearHeight = 1.0;

    if (viewportWidth > viewportHeight)
        nearWidth = (GLfloat)viewportWidth / (GLfloat)viewportHeight; 
    else
        nearHeight = (GLfloat)viewportHeight / (GLfloat)viewportWidth;
    
    // set the frustum
    glFrustum(-nearWidth * FRUSTUM_NEAR * openingCoef,    // left
              nearWidth * FRUSTUM_NEAR * openingCoef,     // right
              -nearHeight * FRUSTUM_NEAR * openingCoef,   // bottom
              nearHeight * FRUSTUM_NEAR * openingCoef,    // top
              FRUSTUM_NEAR,                               // near
              FRUSTUM_FAR);                               // far
    
    // save a copy on the stack
    glPushMatrix();
    
    // go back to modelview
    glMatrixMode(GL_MODELVIEW);
    
    // show/hide mouse cursor
    if (isTurnable()) {
        SDL_WM_GrabInput(SDL_GRAB_ON);
        SDL_ShowCursor(SDL_DISABLE);
    }
    else {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        SDL_ShowCursor(SDL_ENABLE);
    }
    
}

void Camera::startMoveUp() {}
void Camera::stopMoveUp() {}
void Camera::startMoveDown() {}
void Camera::stopMoveDown() {}
void Camera::startMoveLeft() {}
void Camera::stopMoveLeft() {}
void Camera::startMoveRight() {}
void Camera::stopMoveRight() {}
void Camera::turnHorizontal(GLfloat angle) {}
void Camera::turnVertical(GLfloat angle) {}

bool Camera::isMovable() { return movable; }
bool Camera::isTurnable() { return turnable; }


//---------------------------------------------------------
// FIXED CAMERA CLASS
//---------------------------------------------------------

FixedCamera::FixedCamera(BeePoint* position, 
                         BeePoint* lookAt, 
                         GLfloat openingCoef)
    : Camera(position, lookAt, openingCoef)
{
    movable = false;
    turnable = false;
}

FixedCamera::~FixedCamera() {}

void FixedCamera::activate() {
    // Get the saved projection matrix and store back a copy immediatly
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPushMatrix();
    
    GLfloat* pos = position->getCoords();
    GLfloat* look = lookAt->getCoords();
    
    gluLookAt(pos[0], pos[1], pos[2],     // camera position
              look[0], look[1], look[2],  // camera look-at point
              0.0, 1.0, 0.0);             // camera inclination
    
    // init the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



//---------------------------------------------------------
// FREE-LOOK CAMERA CLASS
//---------------------------------------------------------

FreeLookCamera::FreeLookCamera(BeePoint* position, 
                               BeePoint* reference, 
                               GLfloat openingCoef)
    : Camera(position, reference, openingCoef), 
      horizontalAngle(0.0), verticalAngle(0.0) 
{
    movable = false;
    turnable = true;
}
    
FreeLookCamera::~FreeLookCamera() {}

void FreeLookCamera::activate() {
    // Get the saved projection matrix and store back a copy immediatly
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPushMatrix();
    
    // Turn my head
    glRotatef(verticalAngle, 1.0, 0.0, 0.0);
    glRotatef(horizontalAngle, 0.0, 1.0, 0.0);
    
    GLfloat* pos = position->getCoords();
    GLfloat* look = lookAt->getCoords();
    
    gluLookAt(pos[0], pos[1], pos[2],     // camera position
              look[0], look[1], look[2],  // camera look-at point
              0.0, 1.0, 0.0);             // camera inclination
    
    // init the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void FreeLookCamera::turnHorizontal(GLfloat angle) { 
    horizontalAngle += angle;
}

void FreeLookCamera::turnVertical(GLfloat angle) { 
    verticalAngle += angle;
}


//---------------------------------------------------------
// PAN CAMERA CLASS
//---------------------------------------------------------

PanCamera::PanCamera(BeePoint* initalPosition, 
                     BeePoint* initialLookAt, 
                     GLfloat openingCoef)
    : Camera(initalPosition, initialLookAt, openingCoef),
      hPanAngle(0.0), vPanAngle(0.0), 
      movingUp(false), movingDown(false), movingLeft(false), movingRight(false)
{
    movable = true;
    turnable = false;
}

PanCamera::~PanCamera() {}


void PanCamera::activate() {
    // Move the camera if necessary
    if (movingUp)    vPanAngle += KEYBOARD_SPEED;
    if (movingDown)  vPanAngle -= KEYBOARD_SPEED;
    if (movingLeft)  hPanAngle += KEYBOARD_SPEED;
    if (movingRight) hPanAngle -= KEYBOARD_SPEED;

    // Get the saved projection matrix and store back a copy immediatly
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPushMatrix();
    
    GLfloat* pos = position->getCoords();
    GLfloat* look = lookAt->getCoords();
    
    gluLookAt(pos[0], pos[1], pos[2],     // camera position
              look[0], look[1], look[2],  // camera look-at point
              0.0, 1.0, 0.0);             // camera inclination
    
    glRotatef(vPanAngle, 1.0, 0.0, 0.0);
    glRotatef(hPanAngle, 0.0, 1.0, 0.0);
    
    // init the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void PanCamera::startMoveUp()    { movingUp    = true; }
void PanCamera::startMoveDown()  { movingDown  = true; }
void PanCamera::startMoveLeft()  { movingLeft  = true; }
void PanCamera::startMoveRight() { movingRight = true; }

void PanCamera::stopMoveUp()     { movingUp    = false; }
void PanCamera::stopMoveDown()   { movingDown  = false; }
void PanCamera::stopMoveLeft()   { movingLeft  = false; }
void PanCamera::stopMoveRight()  { movingRight = false; }


//---------------------------------------------------------
// FREE-LOOK PAN CAMERA CLASS
//---------------------------------------------------------

FreeLookPanCamera::FreeLookPanCamera(BeePoint* initialPosition, 
                                     BeePoint* initialLookAt,
                                     GLfloat openingCoef)
    : PanCamera(initialPosition, initialLookAt, openingCoef),
      horizontalAngle(0.0), verticalAngle(0.0) 
{
    movable = true;
    turnable = true;
}

FreeLookPanCamera::~FreeLookPanCamera() {}

void FreeLookPanCamera::activate() {
    // Move the camera if necessary
    if (movingUp)    vPanAngle -= KEYBOARD_SPEED;
    if (movingDown)  vPanAngle += KEYBOARD_SPEED;
    if (movingLeft)  hPanAngle += KEYBOARD_SPEED;
    if (movingRight) hPanAngle -= KEYBOARD_SPEED;

    // Get the saved projection matrix and store back a copy immediatly
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPushMatrix();
    
    // Turn the head
    glRotatef(verticalAngle, 1.0, 0.0, 0.0);
    glRotatef(horizontalAngle, 0.0, 1.0, 0.0);
    
    GLfloat* pos = position->getCoords();
    GLfloat* look = lookAt->getCoords();
    
    gluLookAt(pos[0], pos[1], pos[2],     // camera position
              look[0], look[1], look[2],  // camera look-at point
              0.0, 1.0, 0.0);             // camera inclination
    
    // Pan the camera
    glRotatef(vPanAngle, 1.0, 0.0, 0.0);
    glRotatef(hPanAngle, 0.0, 1.0, 0.0);
    
    // init the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}
void FreeLookPanCamera::turnHorizontal(GLfloat angle) {
    horizontalAngle += angle;
}
void FreeLookPanCamera::turnVertical(GLfloat angle) {
    verticalAngle += angle;
}
