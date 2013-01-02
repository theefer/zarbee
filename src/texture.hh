/*
 * ZarBee - textures.hh
 *
 * Author: sast
 * Created on: Thu Mar  3 13:15:21 CET 2005
 *
 */

#ifndef _texture_hh_
#define _texture_hh_

#include <GL/gl.h>
#include <png.h>

class Texture {
public:
    // create a texture from a file
    Texture (char* fileName);
    ~Texture();

    // make this the current texture
    void select();

    // getters for the texture dimensions
    GLsizei getWidth();
    GLsizei getHeight();

    // getter for the texture reference number
    GLuint getTexNum();

    bool isRGBA();
private:
    // the dimensions of the texture
    GLsizei width;
    GLsizei height;

    // the OpenGL texture reference number
    GLuint texNum;

    // whether the texture has an alpha channel or not
    bool rgba;
};

#endif // _texture_hh_
