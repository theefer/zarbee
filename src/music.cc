/*
 * ZarBee - music.cc
 *
 * Author: seb
 * Created on: dim avr 17 18:13:52 CEST 2005 *
 */

#include "music.hh"

#include <iostream>
#include "SDL.h"

Music::Music(char* filename, bool looping) : loop(looping) {
  // Make it a dummy object if no filename is provided
  valid = false;
  if(filename == NULL) {
    return;
  }

  // Connect the audio and load the music
  if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) < 0) {
    std::cerr << "[Music] Warning: could not open the audio device!" << std::endl;
    return;   
  }

  zic = Mix_LoadMUS(filename);
  if(zic == NULL) {
    std::cerr << "[Music] Warning: could not open the music file '"
              << filename << "'!" << std::endl;
    return;
  }

  // We got so far, we have a valid music object
  valid = true;
}


Music::~Music() {
  // Only continue if there is a valid file to free
  if(!valid) {
    return;
  }

  Mix_FreeMusic(zic);
  zic = NULL;
}


void
Music::togglePlay() {
  // Only continue if there is a valid file to play
  if(!valid) {
    return;
  }

  if(Mix_PausedMusic()) {
    Mix_ResumeMusic();
  }
  else if(Mix_PlayingMusic()) {
    Mix_PauseMusic();
  }
  else {
    Mix_PlayMusic(zic, (loop ? -1 : 1));
    Mix_PauseMusic();
    SDL_Delay(2000);
    Mix_ResumeMusic();
  }
}
