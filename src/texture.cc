/*
 * ZarBee - textures.cc
 *
 * Author: sast
 * Created on: Thu Mar  3 13:15:21 CET 2005
 *
 */

#include "texture.hh"

#include <iostream>
#include <png.h>

using namespace std;

// png header size constant
#define HEADER_SIZE 8

Texture::Texture (char* fileName) {
    // set some default values
    width = 0;
    height = 0;
    texNum = 0;

    // try to open the file and assign it to "fp"
    // note: this is oldstyle c, but it's required!
    FILE *fp = fopen(fileName, "rb");
    if (!fp)
    {
        cerr << "[Texture] Error: Could not open the file " << fileName << endl;
        return;
    }
  
    // read some header bytes and check wheter the file is PNG
    png_byte header[HEADER_SIZE];
    fread(header, 1, HEADER_SIZE, fp);
    if (png_sig_cmp(header, 0, HEADER_SIZE)) {
        cerr << "[Texture] Error: The file " << fileName
             << " is not a valid PNG file." << endl;;

        fclose(fp);
        return;
    }
  
    // create a new png_struct to store the data in
    png_structp imgData = png_create_read_struct (PNG_LIBPNG_VER_STRING,
        NULL, NULL, NULL);
    if (!imgData) {
        cerr << "[Texture] Error creating png_struct." << endl;
        fclose(fp);
        return;
    }

    // create a new png_info structure to store the image information in. */
    png_infop imgInfo = png_create_info_struct(imgData);
    if (!imgInfo) {
        cerr << "[Texture] Error creating png_info." << endl;
        png_destroy_read_struct(&imgData, NULL, NULL);
        return;
    }

    // prepare the file to be read and reset the filepointer to the correct
    // position
    png_init_io(imgData, fp);
    png_set_sig_bytes(imgData, HEADER_SIZE);


    // read the image information into the png_info structure and get some of
    // the image properties
    png_read_info(imgData, imgInfo);

    png_uint_32 imgWidth;
    png_uint_32 imgHeight;
    int imgBitDepth;
    int imgColorType;
  
    png_get_IHDR(imgData, imgInfo, &imgWidth, &imgHeight, &imgBitDepth,
        &imgColorType, NULL, NULL, NULL);

    // set some transformations to obtain a RGB (3byte per pixel) or
    // RGBA (4byte per pixel) image:

    // convert palette and gray scale images to RGB
    if (imgColorType == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(imgData);
    }

    if (imgColorType == PNG_COLOR_TYPE_GRAY ||
        imgColorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(imgData);
    }

    // add a full alpha channel if there is transparency information in a tRNS
    // chunk
    if (png_get_valid(imgData, imgInfo, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(imgData);
    }
    
    // set the bit-depth to 8bit per channel
    if (imgBitDepth == 16) {
        png_set_strip_16(imgData);
    } else if (imgBitDepth < 8) {
        png_set_packing(imgData);
    }

    // set the correct number of color components
    GLint colComp;
    if (imgColorType == PNG_COLOR_TYPE_RGB_ALPHA) {
        colComp = 4; // RGBA (4bytes per pixel)
    } else {
        colComp = 3; // RGB  (3bytes per pixel)
    }

    // update the png_info structure to reflect the transformations
    png_read_update_info(imgData, imgInfo);

    // Allocate some memory for the raw image data and create an array of 
    // row-pointers, ...
    png_byte* imgBuffer = new png_byte[imgHeight * imgWidth * colComp];
    png_bytep* imgRows = new png_bytep[imgHeight];

    // ...init the row pointers...
    for (int i = 0, j = 0; i < (int)imgHeight; ++i, j+= imgWidth * colComp) {
        imgRows[i] = &imgBuffer[j];
    }
   
    // ...and read the whole image at once. (let's hope noone will use this for 
    // huge images...)
    png_read_image(imgData, imgRows);

     // get an OpenGL texture reference number and bind it
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // set the texture properties
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // register our png image as a texture object
    // FIXME: what happens if imgWidth and imgHeight are not 2^n?
    GLenum format = (colComp == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, colComp, (GLsizei)imgWidth,
        (GLsizei)imgHeight, 0, format, GL_UNSIGNED_BYTE, imgBuffer);


    // properly destroy the libpng-stuff
    png_read_end(imgData, NULL);
    png_destroy_read_struct(&imgData, &imgInfo, NULL);

    // we don't need the image buffer anymore, so destroy it as well
    delete imgBuffer;
    delete imgRows;

    // as all went fine, finally set our attributes
    width = (GLsizei)imgWidth;
    height = (GLsizei)imgHeight;
    texNum = tex;
    rgba = (colComp == 4);

    cout << "[Texture] Successfully bound " << fileName << " to texture ";
    cout << texNum << "." << endl;
}

Texture::~Texture() {
    glDeleteTextures(1, &texNum);
}

void Texture::select() {
    // simply bind our texture number
    glBindTexture(GL_TEXTURE_2D, texNum);
}

GLsizei Texture::getWidth() {
    return width;
}

GLsizei Texture::getHeight() {
    return height;
}

GLuint Texture::getTexNum() {
    return texNum;
}

bool Texture::isRGBA() {
    return rgba;
}
