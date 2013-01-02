/* $Id: time.hh 235 2005-04-24 17:47:39Z marco $
 *
 * ZarBee - time.hh
 * 
 */

#include "zar_nodes.hh"


#ifndef TIME_HH
#define TIME_HH

namespace ZarBee {

class Time : public ZarVar {
public:
    Time();
    virtual ~Time();
    
    void tick();
    
    void start();
    bool pause();

    int getSpeed();
    int slower();
    int faster();
    
    long int getMillis();
    
private:
    bool isStarted;
    bool isPaused;
    int speed;
    long int time;
    long int oldSystemTime;
};

}  // namespace

#endif  /* TIME_HH */
