/*
 * ZarBee - bee_base.hh
 * Author: sast
 * Created on: Sun Feb 13 19:41:37 2005
 *
 */

#ifndef _bee_base_hh_
#define _bee_base_hh_

#include <GL/glu.h>
#include "zar_nodes.hh"

// ***********************************
// *** Typedef struct for vertices ***
// ***********************************

typedef struct {
    GLfloat norm[3];    // the normal assigned to the vertex
    GLfloat pos[3];     // the position of the vertex
} Vertex;

// **********************************
// *** The general BeeThing class ***
// **********************************

class BeeThing {
public:
    // *** constructor, destructor and consecutive destruction handling
    BeeThing();
    virtual ~BeeThing();

    BeeThing* addInstance();
    BeeThing* removeInstance();

    // *** rendering
    virtual void draw() = 0;
    void flush();
    
    // *** vertex/face buffer generation
    // Stubs
    virtual int getVertexCount() { return 0; }
    virtual Vertex* getVertices() { return NULL; }

    // Stubs
    virtual int getFaceCount() { return 0; }
    virtual GLuint* getFaces() { return NULL; }

    // Stubs
    virtual int getBorderVertexCount() { return 0; }
    virtual GLuint* getBorderVertices() { return NULL; }

    // *** caching
    virtual bool isCachable() { return false; } // Stub
    virtual void cache();
    void unCache();

protected:
    GLuint listName;

private:
    int instances;
};

// *********************************
// *** The basic Primitive class ***
// *********************************

class BeePrimitive : public BeeThing {
public:
    BeePrimitive();
    virtual ~BeePrimitive() { }

    virtual void draw() = 0;
};

// ********************************
// *** The Modifier classes ***
// ********************************

// *** The basic modifier class

class BeeModifier : public BeeThing {
public:
    BeeModifier(BeeThing* base);
    virtual ~BeeModifier();
  
    virtual void draw() = 0;
	
    virtual int getVertexCount();
    virtual Vertex* getVertices();

    virtual int getFaceCount();
    virtual GLuint* getFaces();

    virtual int getBorderVertexCount();
    virtual GLuint* getBorderVertices();

    virtual bool isCachable();
    virtual void cache();

protected:
    BeeThing* base;
};

// *** A special class for linear modifiers

class BeeLinearModifier : public BeeModifier {
public:
    BeeLinearModifier(BeeThing* base);
    virtual ~BeeLinearModifier() { }
  
    virtual void draw();

    virtual Vertex* getVertices();

protected:
    virtual void applyMatrix() = 0;
};

#endif // _bee_base_hh_
