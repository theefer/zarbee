/* $Id: light.cc 250 2005-04-25 15:35:59Z marco $
 *
 * ZarBee - light.cc
 * 
 */
 
#include "light.hh"

using namespace ZarBee;

//---------------------------------------------------------
// GENERIC LIGHT CLASS
//---------------------------------------------------------

Light::Light(ZarNode* intens) {
    for (int i = 0; i < 3; ++i) 
        intensity[i] = intens;
}

Light::Light(ZarNode** intens) {             
    for (int i = 0; i < 3; ++i)
        intensity[i] = intens[i];
}

Light::~Light() {}

const GLfloat Light::darkIntensity[] = { 0.0, 0.0, 0.0, 1.0 };

void Light::activate(GLenum light) {
    GLfloat in[4];
    for (int i = 0; i < 3; ++i)
        in[i] = intensity[i]->getValue();
    
    in[3] = 1.0;   //alpha channel
    
    // using the same values for diffuse and specular light
    glLightfv(light, GL_DIFFUSE, in);   // enable diffuse light
    glLightfv(light, GL_SPECULAR, in);  // enable specular light
    
    // disable ambient light
    glLightfv(light, GL_AMBIENT, darkIntensity);  
}



//---------------------------------------------------------
// THE AMBIENT LIGHT
//---------------------------------------------------------

AmbientLight::AmbientLight(ZarNode* intensity)
    : Light(intensity) {}
    
AmbientLight::AmbientLight(ZarNode** intensity)
    : Light(intensity) {}
                 
AmbientLight::~AmbientLight() {}
    
void AmbientLight::activate(GLenum light) {
    GLfloat in[4];
    for (int i = 0; i < 3; ++i)
        in[i] = intensity[i]->getValue();
    
    in[3] = 1.0;   //alpha channel
    
    glLightfv(light, GL_AMBIENT, in); 
    glLightfv(light, GL_DIFFUSE, darkIntensity);
    glLightfv(light, GL_SPECULAR, darkIntensity); 
    
    glEnable(light);
}



//---------------------------------------------------------
// THE LAMP
//---------------------------------------------------------
     
Lamp::Lamp(BeePoint* position, ZarNode* intensity)
     : Light(intensity), position(position) {}

Lamp::Lamp(BeePoint* position, 
           ZarNode** intensity)
    : Light(intensity), position(position) {}

Lamp::~Lamp() {}

void Lamp::activate(GLenum light) {

    Light::activate(light);    //set the intensity
    
    // Position
    GLfloat pos[4];
    
    for (int i=0; i<3; ++i)
        pos[i] = position->getCoords()[i];
    
    pos[3] = 1.0;   // a lamp is positional!
    
    glLightfv(light, GL_POSITION, pos);
    
    // Attenuation
    glLighti(light, GL_CONSTANT_ATTENUATION, 0);
    glLighti(light, GL_LINEAR_ATTENUATION, 0);
    glLighti(light, GL_QUADRATIC_ATTENUATION, 1);   //quadratic attenuation
    
    // Disable all spot settings
    glLighti(light, GL_SPOT_CUTOFF, 180);  // no cutoff
    glLighti(light, GL_SPOT_EXPONENT, 0);  // no focussing
    
    // And finally enable the light
    glEnable(light);
}



//---------------------------------------------------------
// THE SPOT
//---------------------------------------------------------

Spot::Spot(BeePoint* position, 
           BeePoint* lookAt,
           ZarNode*  cutOff,
           ZarNode*  exponent,
           ZarNode*  intensity)
    : Light(intensity), 
      position(position), 
      lookAt(lookAt), 
      cutOff(cutOff),
      exponent(exponent)
{
    checkValues();
}

Spot::Spot(BeePoint* position, 
           BeePoint* lookAt,
           ZarNode*  cutOff,
           ZarNode*  exponent,
           ZarNode** intensity)
    : Light(intensity), 
      position(position), 
      lookAt(lookAt), 
      cutOff(cutOff),
      exponent(exponent) 
{
    checkValues();
}

Spot::~Spot() {}

void Spot::checkValues() {
    // cut-off
    if (cutOff->getValue() < 0)
        cerr << "Spot cut-off value too small: " << cutOff->getValue() << endl;
    if (cutOff->getValue() > 90)
        cerr << "Spot cut-off value too big: " << cutOff->getValue() << endl;
        
    // exponent
    if (exponent->getValue() < 0)
        cerr << "Spot exponent value too small: " << exponent->getValue()<<endl;
    if (exponent->getValue() > 128)
        cerr << "Spot exponent value too big: " << exponent->getValue() << endl;
}

void Spot::activate(GLenum light) {

    Light::activate(light);    //set the intensity

    // Position
    GLfloat pos[4];

    for (int i=0; i<3; ++i)
        pos[i] = position->getCoords()[i];

    pos[3] = 1.0;      // a spot is positional!
    
    glLightfv(light, GL_POSITION, pos);
    
    // Direction
    GLfloat direction[3];
    
    for (int i=0; i<3; ++i)
        direction[i] = lookAt->getCoords()[i] - position->getCoords()[i];
        
    glLightfv(light, GL_SPOT_DIRECTION, direction);
    
    // Attenuation
    glLighti(light, GL_CONSTANT_ATTENUATION, 0);
    glLighti(light, GL_LINEAR_ATTENUATION, 1);     // linear attenuation
    glLighti(light, GL_QUADRATIC_ATTENUATION, 0);
    
    // Spot settings
    glLighti(light, GL_SPOT_CUTOFF, (int) cutOff->getValue());
    glLighti(light, GL_SPOT_EXPONENT, (int) exponent->getValue());
    
    // And finally enable the light
    glEnable(light);
}



//---------------------------------------------------------
// THE SUN
//---------------------------------------------------------

Sun::Sun(BeePoint* direction, ZarNode* intensity)
    : Light(intensity), direction(direction) {}

Sun::Sun(BeePoint* direction, ZarNode** intensity)
    : Light(intensity), direction(direction){}

Sun::~Sun() {}

void Sun::activate(GLenum light) {

    Light::activate(light);    //set the intensity
    
    // Position
    GLfloat pos[4];
    for (int i=0; i<3; ++i) 
        pos[i] = - direction->getCoords()[i];
    
    pos[3] = 0.0;      // a sun *IS* directional!
    
    glLightfv(light, GL_POSITION, pos);
    
    // Attenuation
    glLighti(light, GL_CONSTANT_ATTENUATION, 1);  // no attenuation
    glLighti(light, GL_LINEAR_ATTENUATION, 0);
    glLighti(light, GL_QUADRATIC_ATTENUATION, 0);
    
    // Disable all spot settings
    glLighti(light, GL_SPOT_CUTOFF, 180);  // no cutoff
    glLighti(light, GL_SPOT_EXPONENT, 0);  // no focussing
    
    // And finally enable the light
    glEnable(light);
}



