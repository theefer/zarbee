#include "zar_nodes.hh"



// =============== CONSTRUCTORS ===============

ZarNode::ZarNode() : instances(1) { }
ZarNode::ZarNode(char* node_type) : instances(1), nodeType(node_type) { }

ZarConst::ZarConst(zartype x) : ZarNode("const") { value = x; }
ZarConst::ZarConst(zartype x, char* nt) : ZarNode(nt) { value = x; }
ZarVar::ZarVar() : ZarConst(0, "var") { }
ZarVar::ZarVar(zartype x) : ZarConst(x, "var") { }

ZarUnary::ZarUnary(ZarNode* op, char* nt) : ZarNode(nt), operand(op) { }
ZarBinary::ZarBinary(ZarNode* l, ZarNode* r, char* nt) : ZarNode(nt), left(l), right(r) { }

ZarSin::ZarSin(ZarNode* op) : ZarUnary(op, "sin") { }
ZarCos::ZarCos(ZarNode* op) : ZarUnary(op, "cos") { }
ZarAsin::ZarAsin(ZarNode* op) : ZarUnary(op, "asin") { }
ZarAcos::ZarAcos(ZarNode* op) : ZarUnary(op, "acos") { }

ZarAdd::ZarAdd(ZarNode* l, ZarNode* r) : ZarBinary(l, r, "add") { }
ZarMult::ZarMult(ZarNode* l, ZarNode* r) : ZarBinary(l, r, "mult") { }
ZarSub::ZarSub(ZarNode* l, ZarNode* r) : ZarBinary(l, r, "sub") { }
ZarDiv::ZarDiv(ZarNode* l, ZarNode* r) : ZarBinary(l, r, "div") { }
ZarPow::ZarPow(ZarNode* l, ZarNode* r) : ZarBinary(l, r, "pow") { }

ZarMin::ZarMin(ZarNode* l, ZarNode* r) : ZarBinary(l, r, "min") { }
ZarMax::ZarMax(ZarNode* l, ZarNode* r) : ZarBinary(l, r, "max") { }

ZarSquareWave::ZarSquareWave(ZarNode* op) : ZarUnary(op, "sqare-wave") { }

// =============== DESTRUCTORS ===============

ZarNode::~ZarNode() { }
ZarConst::~ZarConst() { }

ZarUnary::~ZarUnary() {
  operand->removeInstance();
}

ZarBinary::~ZarBinary() {
  left->removeInstance();
  right->removeInstance();
}


// =============== INSTANCE MANAGERS ===============

/**
 * Register a new reference to the object.
 *
 * @return The object itself.
 */
ZarNode* ZarNode::addInstance() {
    ++instances;
    return this;
}

/**
 * Remove a reference to the object.  If the object is no longer
 * referenced, delete it.
 *
 * @return The object itself, or NULL if it has been deleted.
 */
ZarNode* ZarNode::removeInstance() {
    if(--instances <= 0) {
        delete this;
        return NULL;
    }
    else {
        return this;
    }
}



// =============== VALIDATORS ===============

bool
ZarConst::initialize() throw(IncompleteTreeException) {
  cachable = true;
  return cachable;
}

bool
ZarVar::initialize() throw(IncompleteTreeException) {
  cachable = false;
  return cachable;
}


bool
ZarUnary::initialize() throw(IncompleteTreeException) {
  if(operand == NULL) {
    throw IncompleteTreeException("missing operand");
  }

  // Cachable if operand is cachable too
  cachable = operand->initialize();
  return cachable;
}

bool
ZarBinary::initialize() throw(IncompleteTreeException) {
  if(left == NULL) {
    throw IncompleteTreeException("missing left operand");
  }

  if(right == NULL) {
    throw IncompleteTreeException("missing right operand");
  }
  
  // Only cachable if both operands are cachable too
  cachable = left->initialize();
  cachable = right->initialize() && cachable;
  return cachable;
}



// =============== CACHE UPDATES ===============

void
ZarNode::updateCache() {
  updateCache(false);
}


void
ZarSin::updateCache(bool force) {
  if(!cachable || force) {
    operand->updateCache(force);
    value = sin(operand->getValue());
  }
}

void
ZarAsin::updateCache(bool force) {
  if(!cachable || force) {
    operand->updateCache(force);
    value = asin(operand->getValue());
  }
}

void
ZarCos::updateCache(bool force) {
  if(!cachable || force) {
    operand->updateCache(force);
    value = cos(operand->getValue());
  }
}

void
ZarAcos::updateCache(bool force) {
  if(!cachable || force) {
    operand->updateCache(force);
    value = acos(operand->getValue());
  }
}


void
ZarAdd::updateCache(bool force) {
  if(!cachable || force) {
    left->updateCache(force);
    right->updateCache(force);
    value = left->getValue() + right->getValue();
  }
}

void
ZarMult::updateCache(bool force) {
  if(!cachable || force) {
    left->updateCache(force);
    right->updateCache(force);
    value = left->getValue() * right->getValue();
  }
}

void
ZarSub::updateCache(bool force) {
  if(!cachable || force) {
    left->updateCache(force);
    right->updateCache(force);
    value = left->getValue() - right->getValue();
  }
}

void
ZarDiv::updateCache(bool force) {
  if(!cachable || force) {
    left->updateCache(force);
    right->updateCache(force);

    // Avoid division by zero
    zartype rightVal = right->getValue();
    value = left->getValue() / (rightVal != 0.0 ? rightVal : INFINITY);
  }
}

void
ZarPow::updateCache(bool force) {
  if(!cachable || force) {
    left->updateCache(force);
    right->updateCache(force);
    value = pow(left->getValue(), right->getValue());
  }
}


void
ZarMin::updateCache(bool force) {
  if(!cachable || force) {
    left->updateCache(force);
    right->updateCache(force);
    value = (left->getValue() < right->getValue()) ? left->getValue() : right->getValue();
  }
}

void
ZarMax::updateCache(bool force) {
  if(!cachable || force) {
    left->updateCache(force);
    right->updateCache(force);
    value = (left->getValue() > right->getValue()) ? left->getValue() : right->getValue();
  }
}

void
ZarSquareWave::updateCache(bool force) {
  if(!cachable || force) {
    operand->updateCache(force);
    value = (((long int) round(operand->getValue())) % 2 == 0) ? 0 : 1;
  }
}


// =============== PRINTERS ===============

// Allow the nodes to be outputted
std::ostream& operator<<(std::ostream& os, const ZarNode& node) {
  node.print(os);
  return os;
}


void
ZarNode::append(std::ostream& os, int indent) const {
  for(int i = 0; i < indent; ++i) {
    os << " ";
  }

  // Node type
  if(nodeType != NULL)
    os << "+- " << nodeType;
  else
    os << "+- " << "UNDEF node";
  
  // Value
  if(cachable)
    os << " {" << value << "}";
  else
    os << " [" << value << "]";

  os << std::endl;
}

/**
 * Default is no indentation.
 */
void
ZarNode::print(std::ostream& os) const {
  print(os, 0);
}

void
ZarNode::print(std::ostream& os, int indent) const {
  append(os, indent);
}

void
ZarUnary::print(std::ostream& os, int indent) const {
  append(os, indent);
  operand->print(os, indent + OUTPUT_INDENT_SIZE);
}

void
ZarBinary::print(std::ostream& os, int indent) const {
  append(os, indent);
  left->print(os, indent + OUTPUT_INDENT_SIZE);
  right->print(os, indent + OUTPUT_INDENT_SIZE);
}
