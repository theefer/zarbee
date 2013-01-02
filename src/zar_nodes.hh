/*
 * ZarBee - zar_nodes.hh
 *
 * Author: seb
 * Created on: Sun Feb 13 20:54:24 CET 2005 *
 */

#ifndef _zar_nodes_hh_
#define _zar_nodes_hh_

#include "exception.hh"
#include "math.h"
#include <iostream>

typedef double zartype;



/**
 * Exception thrown when the tree validation discovers that the tree
 * is not valid (incomplete).
 */
class IncompleteTreeException : public Exception {
public:
  IncompleteTreeException(char* e) : Exception(e) { }
};



class ZarNode {
private:
  int instances;

protected:
  const static int OUTPUT_INDENT_SIZE = 3;

  // A short string describing the type of the node for debut output
  char* nodeType;

  // Whether the node should be recomputed when updateCache is called
  bool cachable;

  // Store the actual value of the node
  zartype value;

  void append(std::ostream& os, int indent) const;

public:

  ZarNode();
  ZarNode(char* node_type);
  virtual ~ZarNode();

  ZarNode* addInstance();
  ZarNode* removeInstance();

  inline zartype getValue() { return value; }
  inline bool isCachable() { return cachable; }

  virtual bool initialize() = 0;
  virtual void updateCache(bool force) = 0;
  void updateCache();

  // Used for debug display of the nodes
  void print(std::ostream& os) const;
  virtual void print(std::ostream& os, int indent) const;

  // Allow the nodes to be outputted
  friend std::ostream& operator<<(std::ostream&, const ZarNode&);
};


class ZarConst : public ZarNode {
protected:
  ZarConst(zartype x, char* nt);

public:
  ZarConst(zartype x);
  ~ZarConst();

  virtual bool initialize() throw(IncompleteTreeException);
  virtual void updateCache(bool force) { };
};


class ZarVar : public ZarConst {
public:
  ZarVar();
  ZarVar(zartype x);

  inline void setValue(zartype newValue) { value = newValue; };

  virtual bool initialize() throw(IncompleteTreeException);
  virtual void updateCache(bool force) { };
};


class ZarUnary : public ZarNode {
protected:
  ZarNode* operand;

  ZarUnary(ZarNode* op, char* nt);

public:
  ~ZarUnary();

  virtual bool initialize() throw(IncompleteTreeException);

  virtual void print(std::ostream& os, int indent) const;
};


class ZarBinary : public ZarNode {
protected:
  ZarNode* left;
  ZarNode* right;

  ZarBinary(ZarNode* l, ZarNode* r, char* nt);

public:
  ~ZarBinary();

  virtual bool initialize() throw(IncompleteTreeException);

  virtual void print(std::ostream& os, int indent) const;
};


/* Unary trigonometric operations */
class ZarSin  : public ZarUnary {
public:
  ZarSin(ZarNode* op);

  virtual void updateCache(bool force);
};

class ZarAsin : public ZarUnary {
public:
  ZarAsin(ZarNode* op);

  virtual void updateCache(bool force);
};

class ZarCos  : public ZarUnary {
public:
  ZarCos(ZarNode* op);

  virtual void updateCache(bool force);
};

class ZarAcos : public ZarUnary {
public:
  ZarAcos(ZarNode* op);

  virtual void updateCache(bool force);
};


/* Binary arithmetic operations */
class ZarAdd : public ZarBinary {
public:
  ZarAdd(ZarNode* l, ZarNode* r);

  virtual void updateCache(bool force);
};

class ZarMult : public ZarBinary {
public:
  ZarMult(ZarNode* l, ZarNode* r);

  virtual void updateCache(bool force);
};

class ZarSub : public ZarBinary {
public:
  ZarSub(ZarNode* l, ZarNode* r);

  virtual void updateCache(bool force);
};

class ZarDiv : public ZarBinary {
public:
  ZarDiv(ZarNode* l, ZarNode* r);

  virtual void updateCache(bool force);
};

class ZarPow : public ZarBinary {
public:
  ZarPow(ZarNode* l, ZarNode* r);

  virtual void updateCache(bool force);
};


class ZarMin : public ZarBinary {
public:
  ZarMin(ZarNode* l, ZarNode* r);

  virtual void updateCache(bool force);
};

class ZarMax : public ZarBinary {
public:
  ZarMax(ZarNode* l, ZarNode* r);

  virtual void updateCache(bool force);
};

/* Square wave function */
class ZarSquareWave  : public ZarUnary {
public:
  ZarSquareWave(ZarNode* op);

  virtual void updateCache(bool force);
};



#endif  // _zar_nodes_hh_
