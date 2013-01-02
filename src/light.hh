/* $Id: light.hh 247 2005-04-25 15:24:39Z marco $$
 *
 * ZarBee - light.hh
 * 
 */
 
#include <GL/gl.h>

#include "bee_primitives.hh"


#ifndef LIGHT_HH
#define LIGHT_HH

namespace ZarBee {

/**
* Abstract light base class.
*/
class Light {
public:
    /** Default destructor. */
    virtual ~Light();
    
    /** Abstract activate function.
    * Called before rendering.
    * 
    * @param light  The OpenGL light (ex. GL_LIGHT0) used.
    */
    virtual void activate(GLenum light) = 0;
    
protected:
    /** Constructor for white light sources. */
    Light(ZarNode* intensity);
    
    /** Constructor for RGB colored light sources. */
    Light(ZarNode** intensity);
    
    
    /** The RGBA intensity of the light source. */
    ZarNode* intensity[3];
    
    /** RGBA black. */
    static const GLfloat darkIntensity[4];
};


/** 
* Ambient light class.
* Enables ambient light for the whole scene. By default all ambient light is
* disabled.
*/
class AmbientLight : public Light {
public:
    /** White ambient light. */
    AmbientLight(ZarNode* intensity);
    
    /** Colored ambient light. */
    AmbientLight(ZarNode** intensity);
    
    virtual ~AmbientLight();
    
    virtual void activate(GLenum light);
};


/**
* Positional light source (Lamp).
* The light intensity is attenuated quadratically with the distance.
*/
class Lamp : public Light {
public:
    /** White lamp. */
    Lamp(BeePoint* position, ZarNode* intensity);
    
    /** Colored lamp. */
    Lamp(BeePoint* position, ZarNode** intensity);
    
    virtual ~Lamp();
    
    virtual void activate(GLenum light);
    
private:
    /** Reference to the BeePoint holding the location for the lamp. */
    BeePoint* position;
};


/*
* Positional and directed light source (Spot).
* The light intensity is attenuated linearly with the distance.
*/
class Spot : public Light {
public:

    /** White spot.
    * @param cutOff   CutOff angle. Valid values are within [0,90]. Values out
    *                 of this intervall will be clamped.
    * @param exponent Focus of the spot. Valid values are within [0,128]. Values 
    *                 out of this intervall will be clamped. Higher the value
    *                 the more focused the light source is.
    */
    Spot(BeePoint* position, 
         BeePoint* lookAt, 
         ZarNode*  cutOff, 
         ZarNode*  exponent, 
         ZarNode*  intensity);
    
    /** Colored spot.
    * @see Spot(BeePoint*, BeePoint*, GLfloat)
    */
    Spot(BeePoint* position, 
         BeePoint* lookAt, 
         ZarNode*  cutOff, 
         ZarNode*  exponent, 
         ZarNode** intensity);
         
    virtual ~Spot();
    
    virtual void activate(GLenum light);
    
private:
    /** Reference to the BeePoint holding the location for the spot. */
    BeePoint* position;
    
    /** Reference to the BeePoint holding the look-at point for the spot. */
    BeePoint* lookAt;
    
    /** Cut-off angel [0,90] */
    ZarNode*  cutOff;
    
    /** Exponent [0,128]. Defines the focussing of the spot. */
    ZarNode*  exponent;
    
    /** Check and if necessary clamp cutOff and exponent. */
    void checkValues();
};


/**
* Directional light source (Sun)
*/
class Sun : public Light {
public:
    /** White sun.
    * @param direction  Point which defines the direction of the light source.
    */
    Sun(BeePoint* direction, ZarNode* intensity);
    
    /** Colored sun. */
    Sun(BeePoint* direction, ZarNode** intensity);
        
    virtual ~Sun();
    
    virtual void activate(GLenum light);
    
private:
    /** Reference to the BeePoint holding the direction vector for the Sun. */
    BeePoint* direction;
};

}  // Namespace

#endif  /* LIGHT_HH */
