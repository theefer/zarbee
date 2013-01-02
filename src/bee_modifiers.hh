/*
 * ZarBee - bee_modifiers.hh
 *
 * Author: sast
 * Created on: Sun Feb 13 20:54:14 CET 2005 *
 */

#ifndef _bee_modifiers_hh_
#define _bee_modifiers_hh_

#include "bee_base.hh"
#include "texture.hh"

// **********************************
// *** The linear Transformations ***
// **********************************

// *** BeeRotation

class BeeRotation : public BeeLinearModifier {
public:
    BeeRotation(BeeThing* base, ZarNode* angle, ZarNode* axis[]);
    virtual ~BeeRotation();
  
    virtual bool isCachable();

protected:
    virtual void applyMatrix();

    ZarNode* angle;
    ZarNode* axis[3];
};

// *** BeeTranslation

class BeeTranslation : public BeeLinearModifier {
public:
    BeeTranslation(BeeThing* base, ZarNode* v[]);
    virtual ~BeeTranslation();
    
    virtual bool isCachable();

protected:
    virtual void applyMatrix();

    ZarNode* v[3];
};

// *** BeeScale

class BeeScale : public BeeLinearModifier {
public:
    BeeScale(BeeThing* base, ZarNode* aspect[]);
    virtual ~BeeScale();
  
    virtual bool isCachable();

protected:
    virtual void applyMatrix();

    ZarNode* aspect[3];
};

// *** BeeMatMul

class BeeMatMul : public BeeLinearModifier {
public:
    BeeMatMul(BeeThing* base, ZarNode* m[]);
    virtual ~BeeMatMul();

    virtual bool isCachable();

protected:
    virtual void applyMatrix();

    ZarNode* m[16];
};

// *** BeeShearing

class BeeShearing : public BeeLinearModifier {
public:
    BeeShearing(BeeThing* base, ZarNode* m[]);
    virtual ~BeeShearing();

    virtual bool isCachable();

protected:
    virtual void applyMatrix();

    ZarNode* coeffs[6];

    GLfloat m[16];
};

// **************************************
// *** The non linear Transformations ***
// **************************************

// *** BeeExtrude

class BeeExtrude : public BeeModifier {
public:
    BeeExtrude(BeeThing* base, ZarNode* direction[], ZarNode* angle,
        ZarNode* stacks);
    virtual ~BeeExtrude();
    
    virtual void draw();

    virtual int getVertexCount();
    virtual Vertex* getVertices();

    virtual int getFaceCount();
    virtual GLuint* getFaces();

    virtual bool isCachable();
    virtual void cache();

protected:
    ZarNode* direction[3];
    ZarNode* angle;
    ZarNode* stacks;

    void calcNorm(Vertex* vc, GLfloat* vp, GLfloat* vn, GLfloat* d,
        GLfloat* e);

    void setNormBottom(Vertex* vc, GLfloat* vp, GLfloat* vn, GLfloat* vt);
    void setNormInter(Vertex* vc, GLfloat* vp, GLfloat* vn, GLfloat* vb,
        GLfloat* vt);
    void setNormTop(Vertex* vc, GLfloat* vp, GLfloat* vn, GLfloat* vb);
};

// *** BeeSpinArray

class BeeSpinArray : public BeeModifier {
public:
    BeeSpinArray(BeeThing* base, ZarNode* angle, ZarNode* axis[],
        ZarNode* instances);
    virtual ~BeeSpinArray();

    virtual void draw();

    virtual bool isCachable();

protected:
    ZarNode* axis[3];
    ZarNode* angle;
    ZarNode* instances;
};


// ********************
// *** The Textures ***
// ********************

// *** BeeTexColor

class BeeTexColor : public BeeModifier {
public:
    BeeTexColor (BeeThing* base, ZarNode* r, ZarNode* g, ZarNode* b,
        ZarNode* a);
    virtual ~BeeTexColor();
    virtual void draw();

    virtual bool isCachable();

private:
    GLuint texNum;
    ZarNode* cv[4];
};

// *** BeeTexObjectLinear

class BeeTexObjectLinear : public BeeModifier {
public:
    BeeTexObjectLinear (BeeThing* base, Texture* tex);
    virtual ~BeeTexObjectLinear() { };
    virtual void draw();

private:
    Texture* tex;
};

// *** BeeTexSphereMap

class BeeTexSphereMap : public BeeModifier {
public:
    BeeTexSphereMap (BeeThing* base, Texture* tex);
    virtual ~BeeTexSphereMap() { };
    virtual void draw();

private:
    Texture* tex;
};

#endif // _bee_modifiers_hh_
