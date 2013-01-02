/* $Id: main.cpp 83 2005-02-14 01:09:00Z marco $
 * 
 * ZarBee - main.cc
 * 
 */

// C
#include <stdlib.h>

// C++ STL
#include <list>
#include <vector>
#include <iostream>
#include <fstream>

// OpenGL
#include <GL/gl.h>
#include <GL/glu.h>

// SDL
#include <SDL.h>

// ZarBee
#include "world.hh"
#include "zar_parser.hh"


using namespace std;
using namespace ZarBee;


// Max framerate is 50fps
#define MIN_TICK_DELAY 20

// Update framerate every second
#define FRAMERATE_UPDATE 1000

#define MOUSE_SPEED 0.5

// Saved values used when returning from fullscreen mode
int windowWidth = 640;
int windowHeight = 480;

// Framerate global variable
double frameRate = 0.0;

// The SDL surface
SDL_Surface *surface;


list<World*>* worlds;     // List with all remaining world objects
ZarParser* parser;        // The parser
World* world;             // The current world
Music* music;             // The music object

//---------------------------------------------------------
// HELPER FUNCTIONS
//---------------------------------------------------------

/**
* Quit properly
*/
void quit(int statusCode) {
    SDL_Quit();
    exit(statusCode);
}

/**
* Load a new world object with a given filename
*/
World* loadNewWorld(char* fname) 
{
    // create a new parameter table with the time parameter as fist element
    ZarParamTable* paramTab = new ZarParamTable();
    
    // init the time object
    ZarBee::Time* time = new ZarBee::Time();
    paramTab->addParam("time", time);

//    ZarParser* pars = new ZarParser();

    // parse!
    World* w = parser->parseScene(fname, paramTab);
    
    if (w == NULL) {
        cerr << "Parser error." << endl;
        quit(EXIT_FAILURE);
    }
    return w;
}

/**
* Switch to the next scene
*/
void switchScene() {
    if (worlds->empty()) {
        cout << "No more scenes... Bye!" << endl;
        quit(EXIT_SUCCESS);
    }
    
    delete world;   // delete the old world first
    
    world = worlds->back();
    worlds->pop_back();
    
    //FIXME: backup the old time value
    world->getTime()->start();
    world->getCurrentCamera()->init(windowWidth, windowHeight);
}


/**
* Print usage
*/
void usage()
{
    cout << "ZarBee scene renderer" << endl
         << "Usage: zarbee [options] <scene file> ..." << endl
         << endl
         << "Available options:" << endl
         << "              -h  Show this help message" << endl
         << "              -f  Start in fullscreen mode" << endl
         << " -s <sound file>  Play this soundfile" << endl
//FIXME: maybe add options for resolution, mouse and keyboard speed
         << endl
         << "Key commands (during the execution of the program):" << endl
         << "       <space>  Load next scene (or quit)" << endl
         << "      <escape>  Quit program" << endl
         << "             H  Show help message" << endl
         << "             P  Pause animation" << endl
         << "          +, -  Speed up or slow down animation" << endl
         << "             R  Print the framerate" << endl
         << "             T  Print the current time" << endl
         << "             F  Toggle fullscreen" << endl
         << "  1, 2, ..., 9  Select a camera" << endl
         << "    A, D, W, S  Move the camera (if available)" << endl
         << "  <move mouse>  Turn the camera (if available)" << endl;
}


//---------------------------------------------------------
// "CALLBACK" FUNCTIONS (DISPLAY)
//---------------------------------------------------------


/*
* Display callback function
* - recalculate the parameters
* - call the renderer
*/
void displayScene()
{
    // clear the screen
    glClearColor (0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // render
    world->renderFrame();
    
    // swap
    SDL_GL_SwapBuffers();
}


//---------------------------------------------------------
// "CALLBACK" FUNCTIONS (USER INPUT)
//---------------------------------------------------------

/**
* Keyboard event callback function.
* - <space>: load next scene (or quit)
* - <escape>: quit program
* - H: Show help message
* - P: pause animation
* - +, -: faster, slower animation
* - F: switch fullscreen
* - 1, 2, ..., 9: select camera
* - A, D, W, S: move camera
* - R: print the framerate to the console
* - T: print the current time to the console
*/
void keyDown(SDL_keysym *keysym)
{
    int cam_num;
    
    switch (keysym->sym) {
    
    case SDLK_SPACE:
        cout << "Loading new scene model..." << endl;
        switchScene();
        break;
               
    case SDLK_ESCAPE:
        cout << "Bye!" << endl;
        quit(EXIT_SUCCESS);
        break;
              
    case SDLK_h:
        usage();
        break;
        
    case SDLK_p:
        if (world->getTime()->pause()) 
            cout << "Paused" << endl;
        else
            cout << "Unpaused" << endl;
        
        if(music != NULL)
            music->togglePlay();
        
        break;
    
    case SDLK_PLUS:
    case SDLK_KP_PLUS:
        world->getTime()->faster();
        cout << "Speed: " << world->getTime()->getSpeed() << "%" << endl;
        break;
        
    case SDLK_MINUS:
    case SDLK_KP_MINUS:
        world->getTime()->slower();
        cout << "Speed: " << world->getTime()->getSpeed() << "%" << endl;
        break;
    
    case SDLK_r:
        cout << "Framerate: " << frameRate <<" FPS" << endl;
        break;
        
    case SDLK_t:
        cout << "Current time: " << world->getTime()->getMillis() / 1000.0 
             << " s" << endl;
        break;
        
    case SDLK_f:
        cout << "Toggleling fullscreen" << endl;
        SDL_WM_ToggleFullScreen(surface);
        break;
        
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_5:
    case SDLK_6:
    case SDLK_7:
    case SDLK_8:
    case SDLK_9:
        // Set and initialize the camera
        cam_num = (int) keysym->sym - SDLK_1;
        if (world->setCurrentCamera(cam_num)) {
            cout << "Camera " << cam_num + 1 << " selected." << endl;
            world->getCurrentCamera()->init(surface->w, surface->h);
        } else {
            cout << "Camera " << cam_num + 1 << " is not available." << endl;
        }
        break;
        
    case SDLK_a:
        if (world->getCurrentCamera()->isMovable())
            world->getCurrentCamera()->startMoveLeft();
        else
            cout << "Camera is not movable." << endl;
        break;
        
    case SDLK_d:
        if (world->getCurrentCamera()->isMovable())
            world->getCurrentCamera()->startMoveRight();
        else
            cout << "Camera is not movable." << endl;
        break;
        
    case SDLK_w:
        if (world->getCurrentCamera()->isMovable())
            world->getCurrentCamera()->startMoveUp();
        else
            cout << "Camera is not movable." << endl;
        break;
        
    case SDLK_s:
        if (world->getCurrentCamera()->isMovable())
            world->getCurrentCamera()->startMoveDown();
        else
            cout << "Camera is not movable." << endl;
        break;
        
    default:
        break;
    }
}


/**
* Key-up-event callback function
*/
void keyUp(SDL_keysym *keysym)
{   
    switch (keysym->sym) {
    
    case SDLK_a:
        world->getCurrentCamera()->stopMoveLeft();
        break;
        
    case SDLK_d:
        world->getCurrentCamera()->stopMoveRight();
        break;
        
    case SDLK_w:
        world->getCurrentCamera()->stopMoveUp();
        break;
        
    case SDLK_s:
        world->getCurrentCamera()->stopMoveDown();
        break;
        
    default:
        break;
    }
}

/**
* Mouse motion callback function
*/
void mouseMoved(int dx, int dy)
{   
    Camera* cam = world->getCurrentCamera();
    cam->turnHorizontal(dx * MOUSE_SPEED);
    cam->turnVertical(dy * MOUSE_SPEED
                      * ((double) windowHeight / (double) windowWidth));
}

/**
* Handling user input events (Keyboard, Mouse, Quit)
*/
void handleEvents() 
{
    SDL_Event event;
    
    // handle all events in the queue
    while (SDL_PollEvent(&event)) {
        
        switch (event.type) {
            
        case SDL_KEYDOWN:
            // handle key presses
            keyDown(&event.key.keysym);
            break;
            
        case SDL_KEYUP:
            keyUp(&event.key.keysym);
            break;
            
        case SDL_MOUSEMOTION:
            mouseMoved(event.motion.xrel, event.motion.yrel);
            break;
            
        case SDL_QUIT:
            // handle quit requests
            quit(EXIT_SUCCESS);
            break;
            
        default:
            break;
        }
    }

}


//---------------------------------------------------------
// COMMAND LINE INTERFACE FUNCTIONS
//---------------------------------------------------------

/**
* Initialize everything concerning SDL
*/
void initSDL() {
    
    // Flags to pass to SDL_SetVideoMode
    int videoFlags;
    
    // this holds some info about our display
    const SDL_VideoInfo *videoInfo;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "Video initialization failed: " << SDL_GetError() << endl;
        quit(EXIT_FAILURE);
    }

    // Fetch the video info
    videoInfo = SDL_GetVideoInfo();

    if (!videoInfo) {
        cerr << "Video query failed: " << SDL_GetError() << endl;
        quit(EXIT_FAILURE);
    }

    // the flags to pass to SDL_SetVideoMode
    videoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL
    videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering
    videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware
    videoFlags |= SDL_RESIZABLE;       // Enable window resizing
    videoFlags |= SDL_OPENGLBLIT;
    
    // This checks to see if surfaces can be stored in memory
    if (videoInfo->hw_available)
       videoFlags |= SDL_HWSURFACE;
    else
       videoFlags |= SDL_SWSURFACE;

    // This checks if hardware blits can be done
    if (videoInfo->blit_hw)
       videoFlags |= SDL_HWACCEL;

    // Sets up OpenGL double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // get a SDL surface
    surface = SDL_SetVideoMode(windowWidth, windowHeight, 24, videoFlags);

    // Verify there is a surface
    if (!surface) {
        cerr << "Video mode set failed: " << SDL_GetError() << endl;
        quit(EXIT_FAILURE);
    }

    SDL_WM_SetCaption("ZarBee", NULL);
}


/**
* Main function:
* - parse the command line arguments
* - load a new scene (parse it)
* - render an empty frame
* - init a new window and the camera
* - register callback functions
* - start the time and the GLUT main loop.
*/
int main(int argc, char **argv) 
{   
    glutInit(&argc, argv);

    // init global objects
    worlds = new list<World*>();
    parser = new ZarParser();
    music = NULL;
    world = NULL;
    
    bool isFullScreen = false;
    
    initSDL();
    
    // parse arguments
    for (int i=1; i < argc; ++i) {
        // parse options
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; ++j) {
                if (argv[i][j] == 'h') {         // Help
                    usage();
                    quit(EXIT_SUCCESS);
                }
                else if (argv[i][j] == 'f') {    // FullScreen
                    isFullScreen = true;
                }
                else if (argv[i][j] == 's') {    // Sound file
                    ++i;
                    ifstream* in = new ifstream(argv[i]);
                    if ((*in) != NULL) {
                        // init Music
                        music = new Music(argv[i], false);  //do not loop
                        break;
                    }
                    else {
                        cerr << "Sound file does not exist: " << argv[i] 
                             << endl << endl;
                        usage();
                        quit(EXIT_FAILURE);
                    }
                        
                }
                else {
                    cerr << "Bad option '" << argv[i][j] << "'" << endl << endl;
                    usage();
                    quit(EXIT_FAILURE);
                }
            }
        }
        // Parse filenames
        else {
            ifstream in(argv[i]);
            if (in) {
                World* w = loadNewWorld(argv[i]);
                worlds->push_front(w);
                in.close();
            }
            else {
                cerr << "File does not exist: " << argv[i] << endl << endl;
                usage();
                quit(EXIT_FAILURE);
            }
        }
    }
    
    if (worlds->empty()) {
        cerr << "Please specify a filename" << endl << endl;
        usage();
        quit(EXIT_FAILURE);
    }
    
    
    // pre-render first frame for all scenes
    for (list<World*>::iterator it = worlds->begin(); 
         it != worlds->end(); ++it)
         (*it)->renderEmptyFrame();

    // get the first scene
    switchScene();
    
    if (music != NULL) {
        music->togglePlay();
    }
    
    // Enable fullscreen if needed
    if (isFullScreen) 
        SDL_WM_ToggleFullScreen(surface);
    
    
    // Main loop vars
    long int nextFrameTime = 0;
    long int nextFRUpdTime = FRAMERATE_UPDATE;
    int frameNbr = 0;
    
    
    ///////////////
    // MAIN LOOP //
    ///////////////
    
    while (true) {
    
        // should we switch to the next scene?
        int switchTime = world->getSceneSwitchTime();
        //Attention: use virtual time
        if ((switchTime != 0) && (switchTime < world->getTime()->getMillis()))
            switchScene(); 
        
        
        // Limit framerate
        long int now = SDL_GetTicks(); //Attention: use real time
        if (now < nextFrameTime)
            SDL_Delay(nextFrameTime - now);
        
        nextFrameTime = now + MIN_TICK_DELAY;
        
        // Calculate the framerate if needed
        if (now > nextFRUpdTime) {
            frameRate = frameNbr / (FRAMERATE_UPDATE / 1000.0);
            nextFRUpdTime = now + FRAMERATE_UPDATE;
            frameNbr = 0;
        }
        ++frameNbr;
        
        // Check for scheduled camera switches
        if (!world->getCameraSchedule()->empty()) {
            pair<int,int> p = world->getCameraSchedule()->front();
            //Attention: use virtual time
            if (p.first < world->getTime()->getMillis()) {
                world->setCurrentCamera(p.second);
                world->getCurrentCamera()->init(surface->w, surface->h);
                world->getCameraSchedule()->pop_front();
                cout << "Automatically switched to camera " << p.second + 1 
                     << endl;
            }
        }
        
        // Tick, tick, tick
        world->getTime()->tick();
        
        // Handle user input events
        handleEvents();
        
        // Draw!
        displayScene();
    }


    // Never reached
    return EXIT_FAILURE;
}

