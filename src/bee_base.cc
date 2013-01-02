/*
 * ZarBee - bee_base.cc
 * Author: sast
 * Created on: Mon Mar 21 16:03:04 CET 2005
 *
 */

#include "bee_base.hh"

// **********************************
// *** The general BeeThing class ***
// **********************************

BeeThing::BeeThing()
    : listName(0), instances(1) { }

BeeThing::~BeeThing() {
    // free the cache
    unCache();
}

BeeThing* BeeThing::addInstance() {
    instances++;
    return this;
}

BeeThing* BeeThing::removeInstance() {
    if (--instances == 0) {
        delete this;
        return NULL;
    } else {
        return this;
    }
}

void BeeThing::flush() {
    // do some drawing
    if (listName != 0) {
        glCallLists(1, GL_UNSIGNED_INT, &listName);
    } else {
        draw();
    }
}

#include <iostream>
using namespace std;

void BeeThing::cache() {
    if (isCachable() && listName == 0) {
        // get an OpenGL display list name
        listName = glGenLists(1);
        if (listName != 0) {
            // compile the list
            glNewList(listName, GL_COMPILE);
            draw();
            glEndList();
        }
    }
}

void BeeThing::unCache() {
    if (listName != 0) {
        // free the display list used to cache the BeeThing
        glDeleteLists(listName, 1);
        listName = 0;
    }
}

// *********************************
// *** The basic Primitive class ***
// *********************************

BeePrimitive::BeePrimitive()
    : BeeThing() { }

// ****************************
// *** The Modifier classes ***
// ****************************

// *** The basic modifier class

BeeModifier::BeeModifier(BeeThing* base)
    : BeeThing(), base(base) { }

BeeModifier::~BeeModifier() {
    base->removeInstance();
}

int BeeModifier::getVertexCount() {
    // simple forward
    return base->getVertexCount();
}

Vertex* BeeModifier::getVertices() {
    // simple forward
    return base->getVertices();
}

int BeeModifier::getFaceCount() {
    // simple forward
    return base->getFaceCount();
}

GLuint* BeeModifier::getFaces() {
    // simple forward
    return base->getFaces();
}

int BeeModifier::getBorderVertexCount() {
    // simple forward
    return base->getBorderVertexCount();
}

GLuint* BeeModifier::getBorderVertices() {
    // simple forward
    return base->getBorderVertices();
}

bool BeeModifier::isCachable() {
    return base->isCachable();
}

void BeeModifier::cache() {
    base->cache();
    BeeThing::cache();
}

// *** A special class for linear modifiers

BeeLinearModifier::BeeLinearModifier(BeeThing* base)
    : BeeModifier(base) { }

void BeeLinearModifier::draw() {
    // save the current modelview matrix
    glPushMatrix();

    // apply the transformation matrix and draw the base
    applyMatrix();    
    //base->draw();
    base->flush();
    
    // restore the original modelview matrix
    glPopMatrix();
}

Vertex* BeeLinearModifier::getVertices() {

    // check if there is anything to do...
    int vertexCount = base->getVertexCount();
    if (vertexCount <= 0) return NULL;

    // get the bases vertices
    Vertex* vertices = base->getVertices();

    // get the initial matrix mode
    GLint mat_mode;
    glGetIntegerv(GL_MATRIX_MODE, &mat_mode);
    
    // change to modleview mode
    glMatrixMode(GL_MODELVIEW);

    // set the modelview matrix to our transformation matrix
    glPushMatrix();
    glLoadIdentity();
    applyMatrix();

    // get the matrix from the top of the stack
    GLfloat t[4][4];
    glGetFloatv(GL_MODELVIEW_MATRIX, t[0]);

    // restore the original modelview matrix
    glPopMatrix();

    // restore the original matrix mode
    glMatrixMode(mat_mode);

    // create some buffers
    GLfloat p[3], n[3];

    // loop through the vertex buffer
    for (Vertex *vp = vertices, *ve = &vertices[vertexCount]; vp < ve; ++vp) {
        
        // calculate the new vertex data
        for (int i = 0; i < 3; ++i) {
            p[i] = t[3][i];
            n[i] = 0.0;
            for (int j = 0; j < 3; ++j) {
                p[i] += t[j][i] * vp->pos[j];
                n[i] += t[j][i] * vp->norm[j];
            }
        }

        // save the new vertex data
        for (int i = 0; i < 3; ++i) {
            vp->pos[i] = p[i];
            vp->norm[i] = n[i];
        }
    }
    
    // return the modified vertices
    return vertices;
}
