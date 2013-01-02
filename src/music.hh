/*
 * ZarBee - music.hh
 *
 * Author: seb
 * Created on: dim avr 17 18:13:52 CEST 2005 *
 */

#ifndef _music_hh_
#define _music_hh_

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_mixer.h"


class Music {
protected:
  Mix_Music* zic;
  bool loop;
  bool valid;

public:
  Music(char* filename, bool looping);
  ~Music();

  void togglePlay();
};


#endif // _music_hh_
