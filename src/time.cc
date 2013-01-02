/* $Id: time.cc 243 2005-04-25 11:39:05Z marco $
 *
 * ZarBee - time.cc
 * 
 */
 
#include "time.hh"

#include <SDL.h>

using namespace ZarBee;

Time::Time() : isStarted(false) {}

Time::~Time() {}


void Time::start() {
    isStarted = true;
    isPaused = false;
    speed = 100;
    time = 0;
    value = 0.0;
    oldSystemTime = SDL_GetTicks();
}
    
void Time::tick() {
    long int newSystemTime = SDL_GetTicks();

    if (!isStarted || isPaused) {
        oldSystemTime = newSystemTime;
        return;
    }
    
    // overflows after ~49 days. ok maybe not :)
    time +=  static_cast<long int>((newSystemTime - oldSystemTime) 
                                   * speed / 100.0); 
    value = time;
    
    oldSystemTime = newSystemTime;
}

bool Time::pause() {
    isPaused = !isPaused;
    return isPaused;
}

int Time::getSpeed() {
    return speed;
}

int Time::slower() {
    speed -= 10;
    return speed;
}

int Time::faster() {
    speed += 10;
    return speed;
}

long int Time::getMillis() {
    return time;
}

