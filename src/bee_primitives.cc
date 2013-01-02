/*
 * ZarBee - bee_primitives.cc
 *
 * Author: sast
 * Created on: Sun Feb 13 20:54:24 CET 2005 *
 */

#include "bee_primitives.hh"

#include <math.h>

// *************************
// *** The 2D Primitives ***
// *************************

// *** BeePoly

BeePoly::BeePoly(ZarNode* vertices[], int vertexCount)
    : BeePrimitive(), vertexCount(vertexCount), faceCount(0) {
    // create the vertex array
    this->vertices = new ZarNode*[vertexCount * 3];
    
    // fill the vertex array
    ZarNode** srcp = vertices;
    ZarNode** dstp = this->vertices;
    ZarNode** dstend = &(this->vertices[vertexCount * 3]);
    for (; dstp < dstend; ++dstp, ++srcp) {
        *dstp = *srcp;
    }
    
    // subdivsion of the polygon into triangles (if possible)
    if (vertexCount > 2) {
        // create the face array
        faceCount = vertexCount - 2;
        faces = new GLuint[faceCount * 3];

        // *** to seb: get your vomit bag before you go on reading,
        // ***         this code is 1337 :)
        // fill the face array
        GLuint* p = faces;
        for (int s = 1, t = 2; t < vertexCount; s = t++, p += 3) {
            // set the three vertices of the triangle
            p[0] = 0;  // first vertex
            p[1] = s;  // second vertex
            p[2] = t;  // third vertex
        }
    }
}

BeePoly::~BeePoly() {
    // clean up
    delete vertices;
    delete faces;
}

void BeePoly::draw() {
    // get the vertex buffer
    Vertex* vertexBuffer = getVertices();

    // disable face culling (we want to see back and front face!)
    bool faceCullingOn = glIsEnabled(GL_CULL_FACE);
    if (faceCullingOn) glDisable(GL_CULL_FACE);

    // draw all faces
    glInterleavedArrays(GL_N3F_V3F, 0, vertexBuffer);
    glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, faces);

    // free the vertex buffer
    delete vertexBuffer;

    // reenable face culling if it was enabled previously
    if (faceCullingOn) glEnable(GL_CULL_FACE);
}

int BeePoly::getVertexCount() {
    return vertexCount;
}

Vertex* BeePoly::getVertices() {
    Vertex* vertexBuffer = new Vertex[vertexCount];

    // calculate the normal
    GLfloat norm[3];
    getNormal(norm);

    //fill in the positions
    ZarNode** vp = vertices;
    for (int i = 0; i < vertexCount; ++i) {
        for (int j = 0; j < 3; ++j, ++vp) {
            vertexBuffer[i].pos[j] = (GLfloat)(*vp)->getValue();
            vertexBuffer[i].norm[j] = norm[j];
        }
    }
   
    return vertexBuffer;
}

int BeePoly::getFaceCount() {
    return faceCount;
}

GLuint* BeePoly::getFaces() {
    GLuint* faceBuffer = new GLuint[faceCount * 3];
    
    GLuint* s = faceBuffer;
    GLuint* send = &faceBuffer[faceCount * 3];
    GLuint* d = faceBuffer;
    // copy faces to faceBuffer...
    for ( ; s < send; ++d, ++s) {
        *s = *d;
    }

    // ...and return it    
    return faceBuffer;
}

void BeePoly::getNormal(GLfloat result[]) {
    // get two independant vectors a, b
    // (assuming the whole polygon is in a plane)

    // a = v2 - v1
    GLfloat a[3];
    for (int i = 0; i < 3; ++i) {
        a[i] = vertices[6 + i]->getValue() - vertices[3 + i]->getValue();
    }

    // b = v0 - v1
    GLfloat b[3];
    for (int i = 0; i < 3; ++i) {
        b[i] = vertices[i]->getValue() - vertices[3 + i]->getValue();
    }

    // nx = ay * bz - by * az 
    result[0] = a[1] * b[2] - b[1] * a[2];
    // ny = az * bx - bz * ax 
    result[1] = a[2] * b[0] - b[2] * a[0];
    // nz = ax * by - bx * ay 
    result[2] = a[0] * b[1] - b[0] * a[1];

    // FIXME: should we normalize it?
}

int BeePoly::getBorderVertexCount() {
    return vertexCount;
}

GLuint* BeePoly::getBorderVertices() {
    GLuint* borderVertices = new GLuint[vertexCount];
    
    for (int i = 0; i < vertexCount; ++i) {
        borderVertices[i] = i;
    }

    return borderVertices;
}

bool BeePoly::isCachable() {
    for (int i = 0; i < vertexCount; ++i) {
        if(!vertices[i]->isCachable()) {
            return false;
        }
    }
    return true;
}

// *** BeeCircle

BeeCircle::BeeCircle(ZarNode* radius, ZarNode* vertexCount)
    : BeePrimitive(), radius(radius), vertexCount(vertexCount) {

    // get a glu quadric name
    quadName = gluNewQuadric();
}

BeeCircle::~BeeCircle() {
    // free the ZarNodes
    radius->removeInstance();
    vertexCount->removeInstance();

    // free the quad name
    gluDeleteQuadric(quadName);
}

void BeeCircle::draw() {
    // disable face culling (we want to see back and front face!)
    bool faceCullingOn = glIsEnabled(GL_CULL_FACE);
    if (faceCullingOn) glDisable(GL_CULL_FACE);

    // set the quadrics properties
    gluQuadricNormals(quadName, GLU_FLAT);
    gluQuadricDrawStyle(quadName, GLU_FILL);

    // draw the quadric
    gluDisk(quadName, 0.0, radius->getValue(), (int)vertexCount->getValue(), 1);

    // reenable face culling if it was enabled previously
    if (faceCullingOn) glEnable(GL_CULL_FACE);

}

int BeeCircle::getVertexCount() {
    return (int)vertexCount->getValue() + 1;
}

Vertex* BeeCircle::getVertices() {
    // create the vertex buffer
    int vc = getVertexCount();
    Vertex* vertices = new Vertex[vc];

    // init the center
    vertices[0].pos[0] = 0.0;
    vertices[0].pos[1] = 0.0;
    vertices[0].pos[2] = 0.0;
    vertices[0].norm[0] = 0.0;
    vertices[0].norm[1] = 0.0;
    vertices[0].norm[2] = 1.0;

    // init the angle and delta-angle
    GLfloat angle = 0.0;
    GLfloat dangle = 2 * M_PI / (GLfloat)(vc - 1);
    
    // get the radius
    GLfloat rad = radius->getValue();

    // loop through the vertex buffer and fill it
    for (Vertex* p = &vertices[1], *pend = &vertices[vc]; p < pend; ++p) {
        p->pos[0] = cos(angle) * rad;
        p->pos[1] = sin(angle) * rad;
        p->pos[2] = 0.0;
        angle += dangle;

        p->norm[0] = 0.0; p->norm[1] =  0.0; p->norm[2] = 1.0;
    }
    
    return vertices;
}

int BeeCircle::getFaceCount() {
    return (int)vertexCount->getValue();
}

GLuint* BeeCircle::getFaces() {
    // create the face buffer
    int fc = getFaceCount() * 3;
    GLuint* faces = new GLuint[fc];

    // loop through the face buffer and fill it
    GLuint v = 1;
    GLuint* p = faces, *pend = &faces[fc - 3];
    for ( ; p < pend; ++p)
    {
        *p = 0; ++p;
        *p = v; ++p;
        *p = ++v;
    }
    
    // close the circle
    p[0] = 0; p[1] = v; p[2] = 1;

    return faces;
}

int BeeCircle::getBorderVertexCount() {
    return (int)vertexCount->getValue();
}

GLuint* BeeCircle::getBorderVertices() {
    int vc = (int)vertexCount->getValue();
    GLuint* borderVertices = new GLuint[vc];
    
    for (int i = 0, j = 0; i < vc; j = i) {
        borderVertices[j] = ++i;
    }

    return borderVertices;
}

bool BeeCircle::isCachable() {
    return radius->isCachable()
        && vertexCount->isCachable();
}

// *************************
// *** The 3D Primitives ***
// *************************

// *** BeeCube

BeeCube::BeeCube(ZarNode* size)
    : BeePrimitive(), size(size) { }

BeeCube::~BeeCube() {
    size->removeInstance();
}

void BeeCube::draw() {
    //FIXME: replace the glut function with a glu function...
    glutSolidCube((GLfloat)size->getValue());
}

bool BeeCube::isCachable() {
    return size->isCachable();
}

// *** BeeSphere

BeeSphere::BeeSphere(ZarNode* radius, ZarNode* slices, ZarNode* stacks)
    : BeePrimitive(), radius(radius), slices(slices), stacks(stacks) {

    // get a glu quadric name
    quadName = gluNewQuadric();
}

BeeSphere::~BeeSphere() {
    // free the ZarNodes
    radius->removeInstance();
    slices->removeInstance();
    stacks->removeInstance();

    // free the quad name
    gluDeleteQuadric(quadName);
}

void BeeSphere::draw() {
    // set the quadrics properties
    gluQuadricNormals(quadName, GLU_SMOOTH);
    gluQuadricDrawStyle(quadName, GLU_FILL);

    // draw the quadric
    gluSphere(
        quadName,
        radius->getValue(),
        (GLint)slices->getValue(),
        (GLint)stacks->getValue());
}

bool BeeSphere::isCachable() {
    return radius->isCachable()
        && slices->isCachable()
        && stacks->isCachable();
}

// *** BeeCylinder

BeeCylinder::BeeCylinder(ZarNode* bottom, ZarNode* top, ZarNode* height,
    ZarNode* slices, ZarNode* stacks)
    : BeePrimitive(), bottom(bottom), top(top), height(height), slices(slices),
        stacks(stacks) {

    // get a glu quadric name
    quadName = gluNewQuadric();
}

BeeCylinder::~BeeCylinder() {
    // free the ZarNodes
    bottom->removeInstance();
    top->removeInstance();
    height->removeInstance();
    slices->removeInstance();
    stacks->removeInstance();

    // free the quad name
    gluDeleteQuadric(quadName);
}

void BeeCylinder::draw() {
    // set the quadrics properties
    gluQuadricNormals(quadName, GLU_SMOOTH);
    gluQuadricDrawStyle(quadName, GLU_FILL);

    // draw the quadric
    gluCylinder(
        quadName,
        bottom->getValue(),
        top->getValue(),
        height->getValue(),
        (GLint)slices->getValue(),
        (GLint)stacks->getValue());
}

bool BeeCylinder::isCachable() {
    return bottom->isCachable()
        && top->isCachable()
        && height->isCachable()
        && slices->isCachable()
        && stacks->isCachable();
}

// *** BeeTerrain

BeeTerrain::BeeTerrain(ZarNode* width, ZarNode* height, ZarNode* levels,
    ZarNode* seed)
    : width(width), height(height), levels(levels), seed(seed) { }

BeeTerrain::~BeeTerrain() {
    width->removeInstance();
    height->removeInstance();
    levels->removeInstance();
    seed->removeInstance();
}

void BeeTerrain::draw() {
    // get the number of faces to draw
    int fc = getFaceCount();

    // check if there is actually something to draw
    if (fc <= 0) return;

    // get the vertex and face buffer
    Vertex* vertexBuffer = getVertices();
    GLuint* faceBuffer = getFaces();
    
    // draw all faces
    glInterleavedArrays(GL_N3F_V3F, 0, vertexBuffer);
    glDrawElements(GL_TRIANGLES, getFaceCount() * 3, GL_UNSIGNED_INT,
        faceBuffer);

    // free the vertex buffer
    delete vertexBuffer;
    delete faceBuffer;
}
            
int BeeTerrain::getVertexCount() {
    // check whether 'levels' is positive
    int levs = (int)levels->getValue();
    if (levs < 0) return 0;

    // calculate vertexCount
    int steps = (1 << levs) + 1;
    return steps * steps;
}

Vertex* BeeTerrain::getVertices() {
    // check whether we have to do something
    int levs = (int)levels->getValue();
    if (levs < 0) return NULL;

    // init some constants
    int steps = (1 << levs) + 1;
    int vc = steps * steps;

    GLfloat w = width->getValue() / 2;
    GLfloat heightFac = height->getValue() / (GLfloat)0x1FFFE;

    // saturate the 'seed' parameter
    GLfloat seedFloat = seed->getValue();
    if (seedFloat < -1.0) seedFloat = -1.0;
    if (seedFloat > 1.0) seedFloat = 1.0;
    unsigned int sd = (unsigned int)(seedFloat * (GLfloat)0xFFFE);
    
    // create the vertex buffer
    Vertex* vertices = new Vertex[vc];

    // init the 4 corners of the terrain
    Vertex* v0 = vertices;              // (-w, -w)
    v0->pos[0]  = -w;   v0->pos[1]  = sd * heightFac;   v0->pos[2]  = -w;
    v0->norm[0] = 0.0;  v0->norm[1] = 1.0;              v0->norm[2] = 0.0;
    sd = getNextRand(sd);

    Vertex* v1 = &vertices[steps - 1];   // (w, -w)
    v1->pos[0]  = w;    v1->pos[1]  = sd * heightFac;   v1->pos[2]  = -w;
    v1->norm[0] = 0.0;  v1->norm[1] = 1.0;              v1->norm[2] = 0.0;
    sd = getNextRand(sd);

    Vertex* v2 = &vertices[vc - steps]; // (-w, w)
    v2->pos[0]  = -w;   v2->pos[1]  = sd * heightFac;   v2->pos[2]  = w;
    v2->norm[0] = 0.0;  v2->norm[1] = 1.0;              v2->norm[2] = 0.0; 
    sd = getNextRand(sd);

    Vertex* v3 = &vertices[vc - 1];     // (w, w)
    v3->pos[0]  = w;    v3->pos[1]  = sd * heightFac;   v3->pos[2]  = w;
    v3->norm[0] = 0.0;  v3->norm[1] = 1.0;              v3->norm[2] = 0.0; 
    sd = getNextRand(sd);

    if (levs > 0) {
        // recursively subdivide the corner vertices
        sd = subdivideVertices2D(v0, v1, heightFac, levs, sd);
        sd = subdivideVertices2D(v0, v2, heightFac, levs, sd);
        
        // recursively subdivide the rest of the vertices
        sd = subdivideVertices3D(v0, v1, v2, v3, heightFac, 1 << (levs - 1),
            sd);
    }

    // calculate the normals of the borders far/near
    Vertex *vp1 = v0, *vc1 = vp1 + 1, *vn1 = vc1 + 1;
    Vertex *vp2 = v2, *vc2 = vp2 + 1, *vn2 = vc2 + 1;
    for ( ; vc1 < v1; vp1 = vc1, vc1 = vn1, ++vn1,
        vc2 = vn2, vp2 = vc2, ++vn2) {

        //far
        vc1->norm[0] = vp1->pos[1] - vn1->pos[1];
        vc1->norm[1] = 1.0; vc1->norm[2] = 0.0;

        //near
        vc2->norm[0] = vp2->pos[1] - vn2->pos[1];
        vc2->norm[1] = 1.0; vc2->norm[2] = 0.0;
    }

    // calculate the normals of the borders far/near
    vp1 = v0, vc1 = vp1 + steps, vn1 = vc1 + steps;
    vp2 = v1, vc2 = vp2 + steps, vn2 = vc2 + steps;
    for ( ; vc1 < v2; vp1 = vc1, vc1 = vn1, vn1 += steps,
        vc2 = vn2, vp2 = vc2, vn2 += steps) {

        // left
        vc1->norm[0] = 0.0; vc1->norm[1] = 1.0;
        vc1->norm[2] = vp1->pos[1] - vn1->pos[1];

        // right
        vc2->norm[0] = 0.0; vc2->norm[1] = 1.0;
        vc2->norm[2] = vp2->pos[1] - vn2->pos[1];
    }
    
    // calculate the rest of the normals
    vp1 = v0 + steps, vc1 = vp1 + 1, vn1 = vc1 + 1;
    vp2 = v0 + 1, vn2 = vc1 + steps;
    for ( ; vc1 < v2 ; vp1 += 2, vc1 += 2, vn1 += 2, vp2 += 2, vn2 += 2) {
        vc2 = vn2 - 2;
        for ( ; vc1 < vc2; vp1 = vc1, vc1 = vn1, ++vn1, ++vp2, ++vn2) {
            
            vc1->norm[0] = vp1->pos[1] - vn1->pos[1];
            vc1->norm[1] = 1.0;
            vc1->norm[2] = vp2->pos[1] - vn2->pos[1];
        }
    }

    // finally return the vertex buffer
    return vertices;
}

int BeeTerrain::getFaceCount() {
    // check whether 'levels' is positive
    int levs = (int)levels->getValue();
    if (levs < 0) return 0;

    // calculate vertexCount
    int steps = 1 << levs;
    return steps * steps * 2;
}

GLuint* BeeTerrain::getFaces() {

    // check whether 'levels' is positive
    int levs = (int)levels->getValue();
    if (levs < 0) return NULL;
    
    // set some constansts
    int steps = 1 << levs;
    int fc = steps * steps * 6;

    // create the face buffer
    GLuint* faces = new GLuint[fc];

    // fill the face buffer with quads (2 triangles per quad)
    GLuint vf = 0, vn = steps + 1; 
    for (GLuint *fp = faces, *fe = faces + fc; fp < fe; ++vf, ++vn) {
        for (GLuint ve = vn - 1; vf < ve; ) {
            // 1st triangle of the quad
            *fp = vf;   ++fp;
            *fp = vn;   ++fp;
            *fp = ++vf; ++fp;

            // 2nd triangle of the quad
            *fp = vf;   ++fp;
            *fp = vn;   ++fp;
            *fp = ++vn; ++fp;
        }
    }

    // return the face buffer
    return faces;
}

bool BeeTerrain::isCachable() {
    return width->isCachable()
        && height->isCachable()
        && levels->isCachable()
        && seed->isCachable();
}

unsigned int BeeTerrain::getNextRand(unsigned int seed) {
    return (seed * 0x7F + 1) % 0x1FFFF;
}

void BeeTerrain::center(Vertex* dest, Vertex* a, Vertex* b) {
    for (int i = 0; i < 3; ++i) {
        dest->pos[i] = (a->pos[i] + b->pos[i]) / 2.0;
    }
}

unsigned int BeeTerrain::subdivideVertices2D(Vertex* start, Vertex* end,
    GLfloat heightFac, unsigned int level, unsigned int seed) {

    // get the position of the midpoint
    Vertex* midpoint = (start + (end - start) / 2);

    // interpolate the midpoint
    center(midpoint, start, end);

    // add some randomness
    midpoint->pos[1] += heightFac * (GLfloat)((int)seed - 0xFFFE);
    seed = getNextRand(seed);

    // subdivide further if necessary
    if (level > 1) {
        --level;
        heightFac /= 2;

        seed = subdivideVertices2D(start, midpoint, heightFac, level, seed);
        seed = subdivideVertices2D(midpoint, end, heightFac, level, seed);
    }

    return seed;
}

unsigned int BeeTerrain::subdivideVertices3D(Vertex* e0, Vertex* e1, Vertex* e2,
    Vertex* e3, GLfloat heightFac, unsigned int step, unsigned int seed) {

    // set the pointer to the midpoints of (e0, e1) and (e0, e2)
    Vertex* m0 = e0 + step;
    Vertex* m1 = (e0 + (e2 - e0) / 2);

    // set the pointers to the target vertices
    Vertex* t0 = m1 + step;
    Vertex* t1 = t0 + step;
    Vertex* t2 = e2 + step;

    // interpolate the midpoints of (e1, e3) and (e2, e3)
    center(t1, e1, e3);
    center(t2, e2, e3);

    // add some randomness
    t1->pos[1] += heightFac * (GLfloat)((int)seed - 0xFFFE);
    seed = getNextRand(seed);
    t2->pos[1] += heightFac * (GLfloat)((int)seed - 0xFFFE);
    seed = getNextRand(seed);

    // interpolate the midpoint of the current quad
    center(t0, m0, t2);

    // add some randomness
    t0->pos[1] += heightFac * (GLfloat)((int)seed - 0xFFFE);
    seed = getNextRand(seed);
    
    // subdivide further if necessary
    if (step > 1) {
        step /= 2;
        heightFac /= 2;

        seed = subdivideVertices3D(e0, m0, m1, t0, heightFac, step, seed);
        seed = subdivideVertices3D(m0, e1, t0, t1, heightFac, step, seed);
        seed = subdivideVertices3D(m1, t0, e2, t2, heightFac, step, seed);
        seed = subdivideVertices3D(t0, t1, t2, e3, heightFac, step, seed);
    }

    return seed;
}

// **************************
// *** Special Primitives ***
// **************************

// *** BeePoint
BeePoint::BeePoint()
    : BeeThing() {}

BeePoint::~BeePoint() {}

void BeePoint::draw() {
    GLfloat mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    for (int i = 0; i < 3; ++i) {
        coords[i] = mat[12 + i];
    }
}

GLfloat* BeePoint::getCoords() {
    return coords;
}

// *** BeeGroup

BeeGroup::BeeGroup()
    : BeeThing(), vector<BeeThing*>() { }

BeeGroup::~BeeGroup() {
    // delete all sub elements first
    vector<BeeThing*>::iterator iter;
    vector<BeeThing*>::const_iterator end = this->end();

    for (iter = this->begin(); iter < end; ++iter) {
        (*iter)->removeInstance();
    }

    this->clear();
}

void BeeGroup::draw() {
    vector<BeeThing*>::iterator iter;
    vector<BeeThing*>::const_iterator end = this->end();

    for (iter = this->begin(); iter < end; ++iter) {
        (*iter)->flush();
    }
}

bool BeeGroup::isCachable() {
    vector<BeeThing*>::iterator iter;
    vector<BeeThing*>::const_iterator end = this->end();

    for (iter = this->begin(); iter < end; ++iter) {
        if(!(*iter)->isCachable()) {
            return false;
        }
    }
    return true;
}

#include <iostream>

void BeeGroup::cache() {
    vector<BeeThing*>::iterator iter;
    vector<BeeThing*>::const_iterator end = this->end();

    for (iter = this->begin(); iter < end; ++iter) {
        (*iter)->cache();
    }
    BeeThing::cache();
}
