/*
 * ZarBee - bee_modifiers.cc
 *
 * Author: sast
 * Created on: Sun Feb 13 20:54:14 CET 2005 *
 */

#include "bee_modifiers.hh"

#include <math.h>

// **********************************
// *** The linear Transformations ***
// **********************************

// *** BeeRotation

BeeRotation::BeeRotation(BeeThing* base, ZarNode* angle, ZarNode* axis[])
    : BeeLinearModifier(base), angle(angle) {
    // TODO: Arrrrgggffff! isn't there a cleaner way?
    this->axis[0] = axis[0];
    this->axis[1] = axis[1];
    this->axis[2] = axis[2];  
}

BeeRotation::~BeeRotation() {
    angle->removeInstance();
    axis[0]->removeInstance();
    axis[1]->removeInstance();
    axis[2]->removeInstance();
}

bool BeeRotation::isCachable() {
    return BeeLinearModifier::isCachable()
        && axis[0]->isCachable()
        && axis[1]->isCachable()
        && axis[2]->isCachable()
        && angle->isCachable();
}

void BeeRotation::applyMatrix() {
    glRotatef(
        (GLfloat)angle->getValue(),
        (GLfloat)axis[0]->getValue(),
        (GLfloat)axis[1]->getValue(),
        (GLfloat)axis[2]->getValue());
}

// *** BeeTranslation

BeeTranslation::BeeTranslation(BeeThing* base, ZarNode* v[])
    : BeeLinearModifier(base) {
    // TODO: Arrrrgggffff! isn't there a cleaner way?
    this->v[0] = v[0];
    this->v[1] = v[1];
    this->v[2] = v[2];  
}

BeeTranslation::~BeeTranslation() {
    v[0]->removeInstance();
    v[1]->removeInstance();
    v[2]->removeInstance();
}

bool BeeTranslation::isCachable() {
    return BeeLinearModifier::isCachable()
        && v[0]->isCachable()
        && v[1]->isCachable()
        && v[2]->isCachable();
}

void BeeTranslation::applyMatrix() {
    glTranslatef(
        (GLfloat)v[0]->getValue(),
        (GLfloat)v[1]->getValue(),
        (GLfloat)v[2]->getValue());
}

// *** BeeScale

BeeScale::BeeScale(BeeThing* base, ZarNode* aspect[])
    : BeeLinearModifier(base) {
    // TODO: Arrrrgggffff! isn't there a cleaner way?
    this->aspect[0] = aspect[0];
    this->aspect[1] = aspect[1];
    this->aspect[2] = aspect[2];  
}

BeeScale::~BeeScale() {
    aspect[0]->removeInstance();
    aspect[1]->removeInstance();
    aspect[2]->removeInstance();  
}

bool BeeScale::isCachable() {
    return BeeLinearModifier::isCachable()
        && aspect[0]->isCachable()
        && aspect[1]->isCachable()
        && aspect[2]->isCachable();
}

void BeeScale::applyMatrix() {
    glScalef(
        (GLfloat)aspect[0]->getValue(),
        (GLfloat)aspect[1]->getValue(),
        (GLfloat)aspect[2]->getValue());
}

// *** BeeMatMul

BeeMatMul::BeeMatMul(BeeThing* base, ZarNode* m[])
    : BeeLinearModifier(base) {
  
    //FIXME: is there no damn memcpy function in C++???
    ZarNode** end = &m[16];
    ZarNode** srcp = m;
    ZarNode** dstp = this->m;

    while (srcp < end) {
        *dstp = *srcp;
        ++srcp;
        ++dstp;
    }
}

BeeMatMul::~BeeMatMul() {
    // remove the 16 ZarNode*
    for (int i = 0; i < 16; ++i) {
        m[i]->removeInstance();
    }
}

bool BeeMatMul::isCachable() {
    for (int i = 0; i < 16; ++i) {
        if (!m[i]->isCachable()) {
            return false;
        }
    }
    return BeeLinearModifier::isCachable();
}

void BeeMatMul::applyMatrix() {
    // create a matrix buffer
    GLfloat tmpM[16];

    // loop through the ZarNode matrix, evaluate it and put the result into
    // the buffer.
    ZarNode** end = &m[16];
    ZarNode** srcp = m;
    GLfloat* dstp = tmpM;
    for (; srcp < end; ++srcp, ++dstp) {
        *dstp = (GLfloat)(*srcp)->getValue();
    }
    
    // perform the multiplication
    glMultMatrixf(tmpM);
}

// *** BeeShearing

BeeShearing::BeeShearing(BeeThing* base, ZarNode* shearing[])
    : BeeLinearModifier(base) {

    // register shearing parameters
    ZarNode** end = &shearing[6];
    ZarNode** srcp = shearing;
    ZarNode** dstp = coeffs;

    while (srcp < end) {
        *dstp = *srcp;
        ++srcp;
        ++dstp;
    }

    // build mult matrix: diagonal of ones ...
    m[0] = 1;  m[5] = 1;  m[10] = 1;  m[15] = 1;

    // ... and zeroes around
    m[3] = 0;  m[7] = 0;  m[11] = 0;  m[12] = 0;  m[13] = 0;  m[14] = 0;
}

BeeShearing::~BeeShearing() {
    // remove the 6 ZarNode*
    for(int i = 0; i < 6; ++i) {
        coeffs[i]->removeInstance();
    }
}

bool BeeShearing::isCachable() {
    for (int i = 0; i < 6; ++i) {
        if (!coeffs[i]->isCachable()) {
            return false;
        }
    }
    return BeeLinearModifier::isCachable();
}

void BeeShearing::applyMatrix() {
    // update shearing parameters in the matrix
    m[4] = coeffs[0]->getValue();
    m[8] = coeffs[1]->getValue();

    m[1] = coeffs[2]->getValue();
    m[9] = coeffs[3]->getValue();

    m[2] = coeffs[4]->getValue();
    m[6] = coeffs[5]->getValue();

    // perform the multiplication
    glMultMatrixf(m);
}

// **************************************
// *** The non linear Transformations ***
// **************************************

// *** BeeExtrude

BeeExtrude::BeeExtrude(BeeThing* base, ZarNode* direction[],
    ZarNode* angle, ZarNode* stacks)
    : BeeModifier(base), angle(angle), stacks(stacks) {
    
    for (int i = 0; i < 3; ++i) {
        this->direction[i] = direction[i];
    }
}
    
BeeExtrude::~BeeExtrude() {    
    for (int i = 0; i < 3; ++i) {
        direction[i]->removeInstance();
    }

    angle->removeInstance();
    stacks->removeInstance();
}

void BeeExtrude::draw() {
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

int BeeExtrude::getVertexCount() {
    return base->getBorderVertexCount() * ((int)stacks->getValue() + 1);
}

void BeeExtrude::calcNorm(Vertex* vc, GLfloat* vp, GLfloat* vn, GLfloat* d,
    GLfloat* e) {

    // a = vc - vp, b = vn - vc, 
    GLfloat a[3], b[3];
    for (int i = 0; i < 3; ++i) {
        a[i] = vc->pos[i] - vp[i];
        b[i] = vn[i] - vc->pos[i];
    }
    
    // calculate n = a x d + b x e
    vc->norm[0] = a[1] * d[2] - a[2] * d[1] + b[1] * e[2] - b[2] * e[1];
    vc->norm[1] = a[2] * d[0] - a[0] * d[2] + b[2] * e[0] - b[0] * e[2];
    vc->norm[2] = a[0] * d[1] - a[1] * d[0] + b[0] * e[1] - b[1] * e[0];    
}

void BeeExtrude::setNormBottom(Vertex* vc, GLfloat* vp, GLfloat* vn,
    GLfloat* vt) {

    // set the normals of the bottom stack
    GLfloat d[3];
    for (int i = 0; i < 3; ++i) {
        d[i] = vt[i] - vc->pos[i];
    }

    calcNorm(vc, vp, vn, d, d);
}

void BeeExtrude::setNormInter(Vertex* vc, GLfloat* vp, GLfloat* vn,
    GLfloat* vb, GLfloat* vt) {

    // set the normals of an intermediate stack
    GLfloat e[3], d[3];
    for (int i = 0; i < 3; ++i) {
        d[i] = vt[i] - vc->pos[i];
        e[i] = vc->pos[i] - vb[i];
    }

    calcNorm(vc, vp, vn, d, e);
}

void BeeExtrude::setNormTop(Vertex* vc, GLfloat* vp, GLfloat* vn,
    GLfloat* vb) {

    // set the normals of the top stack
    GLfloat e[3];
    for (int i = 0; i < 3; ++i) {
        e[i] = vc->pos[i] - vb[i];
    }

    calcNorm(vc, vp, vn, e, e);
}

Vertex* BeeExtrude::getVertices() {
    // init some variables
    int st = (int)stacks->getValue();
    int bvc = base->getBorderVertexCount();

    // check if the base can actually be extruded
    if (bvc == 0 || st < 1) return NULL;
    
    Vertex* v = base->getVertices();
    GLuint* bv = base->getBorderVertices();

    // get the direction vector and the angle
    GLfloat d[3];
    for (int i = 0; i < 3; ++i) {
        d[i] = direction[i]->getValue();
    }
    GLfloat a = angle->getValue() * M_PI / 180; // conversion deg -> rad

    // init the vertex buffer
    Vertex* vertices = new Vertex[bvc * (st + 1)];

    // init the base position of the resulting object
    GLfloat basepos[3];
    for (int i = 0; i < 3; ++i) {
        basepos[i] = -d[i] * (GLfloat)st / 2.0;
    }

    // init some 2D vectors for the rotation
    GLdouble rbase[2] = { 1.0, 0.0 };
    GLdouble rcos = cos(a), rsin = sin(a);
    GLdouble rbmem;

    // loop through the stacks and generate the vertices
    for (int sp = 0, se = st * bvc; sp <= se; sp += bvc) {

        // loop through vertices and calculate them
        Vertex *bvp, *vp = &vertices[sp];
        for (int bvi = 0; bvi < bvc; ++bvi) {
            bvp = &v[bv[bvi]];

            vp->pos[0] = basepos[0]
                + rbase[0] * (GLdouble)bvp->pos[0]
                - rbase[1] * (GLdouble)bvp->pos[1];
            vp->pos[1] = basepos[1]
                + rbase[1] * (GLdouble)bvp->pos[0]
                + rbase[0] * (GLdouble)bvp->pos[1];
            vp->pos[2] = basepos[2] + bvp->pos[2];

            ++vp;
        }

        // update the base position for the next stack
        for (int i = 0; i < 3; ++i) {
            basepos[i] += d[i];
        }

        // update the rotation base for the next stack
        rbmem = rcos * rbase[0] - rsin * rbase[1];
        rbase[1] = rsin * rbase[0] + rcos * rbase[1];
        rbase[0] = rbmem;
    }

    // free the vertex and border vertex buffers
    delete v;
    delete bv;

    // define some pointers for the normal calculation    
    Vertex *vprev, *vcurr, *vnext, *vtop, *vbot, *vend;

    // prepare for the bottom stack
    vcurr = vertices;
    vprev = &vertices[bvc - 1];
    vnext = &vertices[1];
    vtop = &vertices[bvc];
    vend = vprev;

    // generate the normals for the bottom stack
    for (; vcurr < vend; vprev = vcurr, vcurr = vnext++, ++vtop) {
        setNormBottom(vcurr, vprev->pos, vnext->pos, vtop->pos);
    }
    setNormBottom(vcurr, vprev->pos, vertices->pos, vtop->pos);
    
    // init the bottom vertex pointer
    vbot = vertices - 1; 

    // generate the normals for the intermediate stacks
    for (int sp = 1; sp < st; ++sp) {

        // prepare the pointers
        vcurr = vnext++;
        vprev = vcurr + bvc - 1;
        ++vbot;
        ++vtop;
        vend = vprev;

        // generate the normals
        for (; vcurr < vend; vprev = vcurr, vcurr = vnext++, ++vtop, ++vbot) {
            setNormInter(vcurr, vprev->pos, vnext->pos, vbot->pos, vtop->pos);
        }
        setNormInter(vcurr, vprev->pos, vnext[-bvc].pos, vbot->pos, vtop->pos);
    }

    // prepare for the top stack
    vcurr = vnext++;
    vprev = vcurr + bvc - 1;
    ++vbot;
    vend = vprev;

    // generate the normals for the top stack
    for (; vcurr < vend; vprev = vcurr, vcurr = vnext++, ++vbot) {
        setNormTop(vcurr, vprev->pos, vnext->pos, vbot->pos);
    }
    setNormTop(vcurr, vprev->pos, vnext[-bvc].pos, vbot->pos);

    // finally return the vertices
    return vertices;
}

int BeeExtrude::getFaceCount() {  
    return base->getBorderVertexCount() * ((int)stacks->getValue()) * 2;
}

GLuint* BeeExtrude::getFaces() {
    // get the number of stacks and border vertices
    int stc = (int)stacks->getValue();
    int bvc = base->getBorderVertexCount();

    // check if the base can actually be extruded
    if (bvc == 0 || stc < 1) return NULL;

    // allocate the face buffer
    GLuint* faces = new GLuint[bvc * stc * 6];
    
    GLuint* fp = faces;
    GLuint vb = 0, vt = bvc;

    // loop through the stacks and generate the faces
    for (int st = 0; st < stc; ++st) {

        // generate quads with the vertices between this stack and the next one
        for (int bv = 1; bv < bvc; ++bv) {
            // 1st triangle of the quad
            *(fp++) = vb;
            *(fp++) = ++vb;
            *(fp++) = vt;

            // 2nd triangle of the quad
            *(fp++) = vt;
            *(fp++) = vb;
            *(fp++) = ++vt;
        }

        // close the ring:
        // 1st triangle of the quad
        *(fp++) = vb;
        *(fp++) = ++vb - bvc;
        *(fp++) = vt;

        // 2nd triangle of the quad
        *(fp++) = vt;
        *(fp++) = vb - bvc;
        *(fp++) = vb; ++vt;
    }

    return faces;
}

bool BeeExtrude::isCachable() {
    return BeeModifier::isCachable()
        && direction[0]->isCachable()
        && direction[1]->isCachable()
        && direction[2]->isCachable()
        && angle->isCachable()
        && stacks->isCachable();
}

void BeeExtrude::cache() {
    /* the implementation of this function is necessary in order to avoid
     * conflicts between glNewList() and glGet() (called when a linear modifier
     * is applied to the base of the extruder) during the compilation of the
     * display list.
     */

    if (isCachable() && listName == 0) {
        // get an OpenGL display list name
        listName = glGenLists(1);
        if (listName != 0) {
            // get the number of faces to draw
            int fc = getFaceCount();

            // check if there is actually something to draw
            if (fc <= 0) return;

            // get the vertex and face buffer
            Vertex* vertexBuffer = getVertices();
            GLuint* faceBuffer = getFaces();
    
            // compile the list
            glNewList(listName, GL_COMPILE);
            glInterleavedArrays(GL_N3F_V3F, 0, vertexBuffer);
            glDrawElements(GL_TRIANGLES, getFaceCount() * 3, GL_UNSIGNED_INT,
                faceBuffer);
            glEndList();

            // free the vertex buffer
            delete vertexBuffer;
            delete faceBuffer;
        }
    }
}

// *** BeeSpinArray

BeeSpinArray::BeeSpinArray(BeeThing* base, ZarNode* angle, ZarNode* axis[],
    ZarNode* instances)
    : BeeModifier(base), angle(angle), instances(instances) {
    
    // TODO: Arrrrgggffff! isn't there a cleaner way?
    this->axis[0] = axis[0];
    this->axis[1] = axis[1];
    this->axis[2] = axis[2];  
}

BeeSpinArray::~BeeSpinArray() {
  angle->removeInstance();
  axis[0]->removeInstance();
  axis[1]->removeInstance();
  axis[2]->removeInstance();
}

bool BeeSpinArray::isCachable() {
    return BeeModifier::isCachable()
        && angle->isCachable()
        && axis[0]->isCachable()
        && axis[1]->isCachable()
        && axis[2]->isCachable()
        && instances->isCachable();
}

void BeeSpinArray::draw() {
    // check the number of instances to draw
    int count = (int)instances->getValue();

    // draw the original base...
    if (count > 0) base->flush();

    // ...and the rest
    if (count > 1) {
        // cache the parameters
        GLfloat axis[3];
        for (int i = 0; i < 3; ++i) {
            axis[i] = (GLfloat)this->axis[i]->getValue();
        }
        GLfloat angle = (GLfloat)this->angle->getValue();

        // save the current model view matrix        
        glPushMatrix();

        // is the legth of the axis zero?
        bool axisNonZero = axis[0] != 0.0 || axis[1] != 0.0 || axis[2] != 0.0;

        // perform the rotation anyway (arround the z-axis)
        if (!axisNonZero) {
            axis[2] = 1.0;
        }

        // perform the spin and draw the base the specified number of times
        for (int i = 1; i < count; ++i) {
            
            // perform the translation
            if (axisNonZero)
                glTranslatef(axis[0], axis[1], axis[2]);

            // perform the rotation
            glRotatef(angle, axis[0], axis[1], axis[2]);

            // draw the base
            base->flush();        
        }

        // restore the model view matrix
        glPopMatrix();
    }
}

// ********************
// *** The Textures ***
// ********************

// *** BeeTexColor

BeeTexColor::BeeTexColor (BeeThing* base, ZarNode* r, ZarNode* g, ZarNode* b,
    ZarNode* a)
    : BeeModifier(base) {
    //copy the pointers
    cv[0] = r;
    cv[1] = g;
    cv[2] = b;
    cv[3] = a;

    // get an OpenGL texture name
    glGenTextures(1, &texNum);
}

BeeTexColor::~BeeTexColor() {
    for (int i = 0; i < 4; ++i) {
        cv[i]->removeInstance();
    }

    // free the texture name
    glDeleteTextures(1, &texNum);
}

bool BeeTexColor::isCachable() {
    for (int i = 0; i < 4; ++i) {
        if (!cv[i]->isCachable()) {
            return false;
        }
    }
    return BeeModifier::isCachable();
}

void BeeTexColor::draw() {
    // create the texture buffer (1x1x4).
    GLfloat tex[4];
    for (int i = 0; i < 4; ++i) {
        tex[i] = cv[i]->getValue();
    }

    // save the current texture attributes
    glPushAttrib(GL_TEXTURE_BIT);

    // bind our texture name
    glBindTexture(GL_TEXTURE_2D, texNum);

    // set the texture properties
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // check for rgba
    GLenum format;
    GLint colComp;
    if (tex[3] == 1.0) {
        format = GL_RGB;
        colComp = 3;
    } else {
        format = GL_RGBA;
        colComp = 4;
    }

    // register our pixel as a texture object
    glTexImage2D(GL_TEXTURE_2D, 0, colComp, 1, 1, 0, format, GL_FLOAT, tex);

    // enable automatic texture generation (object linear mapping)
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    // handle  transparency
    if (format == GL_RGBA) {
        // if necessary draw the back side of base first
        glCullFace(GL_FRONT);
        base->flush();
 
        // switch to backface culling again
        glCullFace(GL_BACK);
    }

    // finally draw the front side of base    
    base->flush();

    // restore the previous texture attributes
    glPopAttrib();
}

// *** BeeTexObjectLinear

BeeTexObjectLinear::BeeTexObjectLinear (BeeThing* base, Texture* tex)
    : BeeModifier(base), tex(tex) { }

void BeeTexObjectLinear::draw() {

    // save the current texture attributes
    glPushAttrib(GL_TEXTURE_BIT);

    // activate the automatic texture generation...
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    // ...bind the texture...
    tex->select();

    // ...set texture generation mode to object linear
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

    // handle  transparency
    if (tex->isRGBA()) {
        // if necessary draw the back side of base first
        glCullFace(GL_FRONT);
        base->flush();
 
        // switch to backface culling again
        glCullFace(GL_BACK);
    }

    // finally draw the front side of base    
    base->flush();

    // restore the previous texture attributes
    glPopAttrib();
}

// *** BeeTexSphereMap

BeeTexSphereMap::BeeTexSphereMap (BeeThing* base, Texture* tex)
    : BeeModifier(base), tex(tex) { }

void BeeTexSphereMap::draw() {

    // save the current texture attributes
    glPushAttrib(GL_TEXTURE_BIT);

    // activate the automatic texture generation...
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    // ...bind the texture...
    tex->select();

    // ...set texture generation mode to Sphere map
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    // handle  transparency
    if (tex->isRGBA()) {
        // if necessary draw the back side of base first
        glCullFace(GL_FRONT);
        base->flush();
 
        // switch to backface culling again
        glCullFace(GL_BACK);
    }

    // finally draw the front side of base    
    base->flush();

    // restore the previous texture attributes
    glPopAttrib();
}
