/*
 * ZarBee - bee_primitives.hh
 *
 * Author: sast
 * Created on: Sun Feb 13 20:54:24 CET 2005 *
 */

#ifndef _bee_primitives_hh_
#define _bee_primitives_hh_

#include "bee_base.hh"
#include <vector>

using namespace std;

// *************************
// *** The 2D Primitives ***
// *************************

// *** BeePoly

class BeePoly : public BeePrimitive {
public:
    BeePoly(ZarNode* vertices[], int vertexCount);
    virtual ~BeePoly();

    virtual void draw();

    virtual int getVertexCount();
    virtual Vertex* getVertices();

    virtual int getFaceCount();
    virtual GLuint* getFaces();

    virtual int getBorderVertexCount();
    virtual GLuint* getBorderVertices();

    virtual bool isCachable();

protected:
    ZarNode** vertices;
    int vertexCount;

    GLuint* faces;
    int faceCount;

    // FIXME: this may move to BeePrimitive later...
    void getNormal(GLfloat result[]);
};

// *** BeeCircle

class BeeCircle : public BeePrimitive {
public:
    BeeCircle(ZarNode* radius, ZarNode* vertexCount);
    ~BeeCircle();

    virtual void draw();

    virtual int getVertexCount();
    virtual Vertex* getVertices();

    virtual int getFaceCount();
    virtual GLuint* getFaces();

    virtual int getBorderVertexCount();
    virtual GLuint* getBorderVertices();

    virtual bool isCachable();

protected:
    GLUquadricObj *quadName;

    ZarNode* radius;
    ZarNode* vertexCount;
};

// *************************
// *** The 3D Primitives ***
// *************************

// *** BeeCube

class BeeCube : public BeePrimitive {
public:
    BeeCube(ZarNode* size);
    ~BeeCube();
            
    virtual void draw();
            
    virtual bool isCachable();

protected:
    ZarNode* size;
};

// *** BeeSphere

class BeeSphere : public BeePrimitive {
public:
    BeeSphere (ZarNode* radius, ZarNode* slices, ZarNode* stacks);
    ~BeeSphere();
            
    virtual void draw();
            
    virtual bool isCachable();

protected:
    GLUquadricObj *quadName;

    ZarNode* radius;
    ZarNode* slices;
    ZarNode* stacks;
};

// *** BeeCylinder

class BeeCylinder : public BeePrimitive {
public:
    BeeCylinder (ZarNode* bottom, ZarNode* top, ZarNode* height,
        ZarNode* slices, ZarNode* stacks);
    ~BeeCylinder();
            
    virtual void draw();
            
    virtual bool isCachable();

protected:
    GLUquadricObj *quadName;

    ZarNode* bottom;
    ZarNode* top;
    ZarNode* height;
    ZarNode* slices;
    ZarNode* stacks;
};

// *** BeeTerrain

class BeeTerrain : public BeePrimitive {
public:
    BeeTerrain(ZarNode* width, ZarNode* height, ZarNode* levels,
        ZarNode* seed);
    ~BeeTerrain();

    virtual void draw();

    virtual int getVertexCount();
    virtual Vertex* getVertices();

    virtual int getFaceCount();
    virtual GLuint* getFaces();

    virtual bool isCachable();

private:
    ZarNode* width;
    ZarNode* height;
    ZarNode* levels;
    ZarNode* seed;

    unsigned int getNextRand(unsigned int seed);

    void center(Vertex* dest, Vertex* a, Vertex* b);

    unsigned int subdivideVertices2D(Vertex* start, Vertex* end,
        GLfloat heightFac, unsigned int level, unsigned int seed);
    unsigned int subdivideVertices3D(Vertex* e0, Vertex* e1, Vertex* e2,
        Vertex* e3, GLfloat heightFac, unsigned int step, unsigned int seed);
};

// **************************
// *** Special Primitives ***
// **************************

// *** BeePoint

class BeePoint : public BeeThing {
public:
    BeePoint();
    virtual ~BeePoint();

    virtual void draw();

    GLfloat* getCoords();

private:
    GLfloat coords[3];
};

// *** BeeGroup

/* This class is basically just a specialisation of the vector template for
 * BeeThing pointers. It's also a descendant of BeeThing to make it usable for
 * rendering. Use the methods/iterators of vector<> to create compound objects
 * and those of BeeThing for rendering.
 */
class BeeGroup : public BeeThing, public vector<BeeThing*> {
public:
    BeeGroup();
    virtual ~BeeGroup();

    virtual void draw();

    virtual bool isCachable();
    virtual void cache();
};

#endif // _bee_primitives_hh_
