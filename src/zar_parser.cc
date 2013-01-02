/*
 * ZarBee - zar_parser.cc
 *
 * Author: seb
 * Created on: mer fév 16 18:23:08 CET 2005 *
 */

#include "zar_parser.hh"


// =============== CONSTRUCTORS/DESTRUCTORS ===============

ZarParser::ZarParser() {

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION

}


ZarParser::~ZarParser() {
}



// =============== TOP-LEVEL METHODS ===============

World*
ZarParser::parseScene(char* filename, ZarParamTable* params) {
  init(filename, params);
  run();

  // Everything is fine, return the created world
  if(validate()) {
    World* world;

    // No scene duration, give the music
    if(duration == 0) {
      world = new World(groupTable[xmlCharStrdup("world")],
                        cameraList,
                        lightList,
                        paramTable,
                        new Music(musicFilename, musicLoop),
                        cameraSchedule);
    }
    // Pass the duration of the scene too
    else {
      world = new World(groupTable[xmlCharStrdup("world")],
                        cameraList,
                        lightList,
                        paramTable,
                        cameraSchedule,
                        duration);
    }

    destroy();
    return world;
  }
  else {
    destroy();
    return NULL;
  }
}


void
ZarParser::init(char* filename, ZarParamTable* params) {
  paramTable = params;

  // Parse the file and get the DOM
  doc = xmlReadFile(filename, NULL, 0);
  if(doc == NULL) {
    cerr << "[Parser] Error: could not parse the file '" << filename
         << "'!  Parsing failed, stop execution...";
    exit(1);
  }

  // Get the root element node
  rootElement = xmlDocGetRootElement(doc);

  // Create lists to be passed to the World (which will destroy them)
  cameraList = new vector<Camera*>();
  lightList = new list<Light*>();
  cameraSchedule = new schedule_t();

  groupTable.clear();
  pointTable.clear();
  textureTable.clear();

  musicFilename = NULL;
  musicLoop = false;

  duration = 0;
}


void
ZarParser::destroy() {
  // Free the document
  xmlFreeDoc(doc);

  // Free the global variables that may have been allocated by the parser.
  xmlCleanupParser();
}


void
ZarParser::run() {
  // Read the duration attribute, if there is one
  parseDuration(rootElement);

  // For all children of the root node
  for(xmlNode* cur = rootElement->children; cur; cur = cur->next) {
    // Only check for elements
    if(cur->type != XML_ELEMENT_NODE) {
      continue;
    }

    // If <include/> parse tree of the linked doc
    if(nodeNameEquals(cur, "include")) {
      parseInclude(cur);
    }

    // If <param/> parse it and save it in the table
    else if(nodeNameEquals(cur, "param")) {
      parseParam(cur);
    }

    // If <group/> parse it
    else if(nodeNameEquals(cur, "group")) {
      parseGroup(cur);
    }

    // If <camera-list/> parse and save cameras
    else if(nodeNameEquals(cur, "camera-list")) {
      parseCameraList(cur);
    }

    // If <light-list/> parse and save cameras
    else if(nodeNameEquals(cur, "light-list")) {
      parseLightList(cur);
    }

    // If <camera-schedule/> parse and save the camera schedule
    else if(nodeNameEquals(cur, "camera-schedule")) {
      parseCameraSchedule(cur);
    }

    // If <music/> parse and save music parameters
    else if(nodeNameEquals(cur, "music")) {
      parseMusic(cur);
    }

    // Invalid children of the root node! Display an error, but continue
    else {
      cerr << "[Parser] Warning: invalid node at top-level : " << cur->name
           << " [line " << cur->line << "]" << endl;
    }

  }
}


bool
ZarParser::validate() {
  // Run the init, that checks tree consistency
  paramTable->initialize();

  // If no "world" group, print error and quit
  if(groupTable[xmlCharStrdup("world")] == NULL) {
    cerr << "[Parser] Error: no 'world' group found!"
         << " Parsing failed, stop execution..." << endl;
    exit(1);
  }

  // If no camera, print error and quit
  if(cameraList->empty()) {
    cerr << "[Parser] Error: no camera found!"
         << " Parsing failed, stop execution..." << endl;
    exit(1);
  }

  // If no light, print darkness warning
  if(lightList->empty()) {
    cerr << "[Parser] Warning: no light found, it's gonna be dark..." << endl;
  }

  return true;
}



// =============== VOID PARSERS ===============

void
ZarParser::parseInclude(xmlNode* node) {
  // FIXME: Code the include parsing
}


/**
 * Parse a param node, create the corresponding ZarNode and add it to
 * the parameter table.
 *
 * @node The parameter node to parse.
 */
void
ZarParser::parseParam(xmlNode* node) {
  // Read 'name' attribute (needed, otherwise print an error)
  char* nameAttr = (char*)getNodeAttribute(node, "name");
  if(nameAttr == NULL) {
    cerr << "[Parser] Warning: unnamed parameter, ignored!"
         << " [line " << node->line << "]" << endl;
    return;
  }

  // If already present, print an error
  if(paramTable->getParam(nameAttr) != NULL) {
    cerr << "[Parser] Warning: ignored parameter '" << nameAttr
         << "' redeclaration!"
         << " [line " << node->line << "]" << endl;
    return;
  }

  // Retrieve the param value and add the node to the param table
  ZarNode* paramNode = getXmlNodeArgument(node, "value");
  paramTable->addParam(nameAttr, paramNode);
}


/**
 * Parse a camera-list element, creating a Camera object for all the
 * valid cameras and adding them to the list.
 *
 * @param node The pointer to the camera-list node.
 */
void
ZarParser::parseCameraList(xmlNode* node) {
  Camera* newCam;
  for (xmlNode* cur = node->children; cur; cur = cur->next) {
    // Only check for elements
    if(cur->type != XML_ELEMENT_NODE) continue;

    // Add to the list if valid
    newCam = parseCamera(cur);
    if(newCam != NULL) {
      cameraList->push_back(newCam);
    }
  }
}


/**
 * Parse a light-list element, creating a Light object for all the
 * valid lights and adding them to the list.
 *
 * @param node The pointer to the light-list node.
 */
void
ZarParser::parseLightList(xmlNode* node) {
  Light* newLight;
  for (xmlNode* cur = node->children; cur; cur = cur->next) {
    // Only check for elements
    if(cur->type != XML_ELEMENT_NODE) continue;

    // Add to the list if valid
    newLight = parseLight(cur);
    if(newLight != NULL) {
      lightList->push_back(newLight);
    }
  }
}


/**
 * Parse a camera-schedule element, filling the camera schedule object
 * with time-camera pairs.
 *
 * @param node The pointer to the camera-schedule node.
 */
void
ZarParser::parseCameraSchedule(xmlNode* node) {
  for (xmlNode* cur = node->children; cur; cur = cur->next) {
    // Only check for elements
    if(cur->type != XML_ELEMENT_NODE) continue;

    if(nodeNameEquals(cur, "checkpoint")) {
      // Add to the list if valid
      try {
        pair<int,int> checkpoint = parseScheduleCheckpoint(cur);
        cameraSchedule->push_back(checkpoint);
      }
      catch(NumberFormatException &e) {
        cerr << "[Parser] Warning: invalid checkpoint in the "
             << " camera schedule list! "
             << " [line " << node->line << "]" << endl;
      }
    }
    else {
      cerr << "[Parser] Warning: invalid element '" << cur->name
           << "' in the camera schedule list ignored! "
           << " [line " << node->line << "]" << endl;
    }
  }
}


/**
 * Parse a music element, loading the music filename and loop flag
 * into the parser member variables.
 *
 * @param node The pointer to the music node.
 */
void
ZarParser::parseMusic(xmlNode* node) {
  // Read the filename of the music file
  musicFilename = (char*)getNodeAttribute(node, "filename");
  if(musicFilename == NULL) {
    cerr << "[Parser] Warning: missing the 'filename' attribute"
         << " of the '" << node->name << "' node!"
         << " [line " << node->line << "]" << endl;
    return;
  }

  // Whether the music should loop or not (default: no)
  musicLoop = false;
  xmlChar* loopAtt = getNodeAttribute(node, "loop");
  if((loopAtt != NULL) && !strcmp((char*) loopAtt, "true")) {
    musicLoop = true;
  }
}


/**
 * Parse the duration attribute of the scene, if there is one.
 *
 * @param node The pointer to the top scene node.
 */
void
ZarParser::parseDuration(xmlNode* node) {
  char* durationAttr = (char*)getNodeAttribute(node, "duration");
  if(durationAttr != NULL) {
    try {
      duration = (int)charToLong(durationAttr);
    }
    catch(NumberFormatException &e) {
      cerr << "[Parser] Warning: invalid duration '"
           << durationAttr << "' for the scene!"
           << " [line " << node->line << "]" << endl;
    }
  }
}



// =============== OBJECT PARSERS ===============

/**
 * Parse the argument of the given name in the given node, checking
 * the attributes and child elements (using getXmlNodeArgument).  This
 * function must be used instead of getXmlNodeArgument to retrieve
 * arguments of nodes, so that the created ZarNode* are added to the
 * anonymous ZarNodes in the parameter table.
 *
 * @param node  The node in which to look for the argument.
 * @param argument  The name of the argument to look for.
 * @return A pointer to a ZarNode representing the argument.
 */
ZarNode*
ZarParser::parseArgument(xmlNode* node, char* argument) {
  ZarNode* n = getXmlNodeArgument(node, argument, true);
  return n;
}

/**
 * Parse the optional argument of the given name in the given node,
 * checking the attributes and child elements (using
 * getXmlNodeArgument).  This function must be used instead of
 * getXmlNodeArgument to retrieve arguments of nodes, so that the
 * created ZarNode* are added to the anonymous ZarNodes in the
 * parameter table.
 *
 * @param node  The node in which to look for the argument.
 * @param argument  The name of the argument to look for.
 * @return A pointer to a ZarNode representing the argument.
 */
ZarNode*
ZarParser::parseOptionalArgument(xmlNode* node, char* argument) {
  ZarNode* n = getXmlNodeArgument(node, argument, true, true);
  return n;
}


/**
 * Parse a group node and create the corresponding BeeThing tree, the
 * top of which is stored in the group table.
 * The parameters contained in the group are simply parsed.
 * The groups and primitives contained in the group are stored as
 * children of the group.
 * The modifiers contained in the group are applied sequentially on
 * the top group pointer.
 *
 * @param node The group node to parse.
 */
BeeThing*
ZarParser::parseGroup(xmlNode* node) {
  // Read 'id' attribute
  xmlChar* idAttr = getNodeAttribute(node, "id");
  list<xmlNode*> modNodes;

  // Create empty BeeGroup and a top-level pointer
  BeeGroup* group = new BeeGroup();
  BeeThing* top = group;

  // For all children, fill the group with its content
  int tmpType;
  xmlNode* cur = NULL;
  for (cur = node->children; cur; cur = cur->next) {
    // Only check for elements
    if(cur->type != XML_ELEMENT_NODE) continue;

    // If <param>, simply parse it
    if(nodeNameEquals(cur, "param")) {
      parseParam(cur);
    }

    // If <modifier>, store it in the modifier list
    else if((tmpType = checkModifierNode(cur)) != MODIFIER_NONE) {
      modNodes.push_back(cur);
    }

    // If <texture>, store it in the modifier list
    else if((tmpType = checkTextureNode(cur)) != TEXTURE_NONE) {
      modNodes.push_back(cur);
    }

    // If <group>, recursively parse it and add to group children
    else if(nodeNameEquals(cur, "group")) {
      group->push_back(parseGroup(cur));
    }

    // If <primitive>, parse it and add to group children
    else if((tmpType = checkPrimitiveNode(cur)) != PRIMITIVE_NONE) {
      BeeThing* tmpPrim = parsePrimitive(cur, tmpType);
      if(tmpPrim != NULL) {
        group->push_back(tmpPrim);
      }
    }

    // If <insert-group>, find referenced group and add to group children
    else if(nodeNameEquals(cur, "insert-group")) {
      BeeThing* tmpRef = parseReference(cur);
      if(tmpRef != NULL) {
        group->push_back(tmpRef);
      }
    }

    // If unknown element, signal it
    else {
      cerr << "[Parser] Warning: unknown element in group: '" << cur->name << "'!"
           << " [line " << node->line << "]" << endl;
    }
  }

  // If the group only contains one BeeThing, drop the group and only keep the thing
  if(group->size() == 1) {
    top = group->back();
    group->pop_back();
    delete group;
  }

  // For all modifiers, wrap them around the group
  while(!modNodes.empty()) {
    cur = modNodes.front();
    modNodes.pop_front();

    // If <modifier>, apply it to and replace the top BeeThing
    if((tmpType = checkModifierNode(cur)) != MODIFIER_NONE) {
      top = parseModifier(cur, top, tmpType);
    }

    // If <texture>, apply it to and replace the top BeeThing
    else if((tmpType = checkTextureNode(cur)) != TEXTURE_NONE) {
      // FIXME: Check we only load *ONE* texture
      top = parseTexture(cur, top, tmpType);
    }

    // No 'else', it cannot happen
  }


  // Register the group in the group table (if it has an id)
  if(idAttr != NULL) {
    groupTable[idAttr] = top;
  }

  return top;
}


/**
 * Create a BeePrimitive object from the given node.
 * If the node is not valid, NULL is returned.
 *
 * @param node The node containing the primitive element.
 * @param primId The type of primitive contained in the node (see class constants)
 * @return The BeePrimitive object created from the primitive node, or
 *   NULL if the node was not a valid primitive.
 */
BeeThing*
ZarParser::parsePrimitive(xmlNode* node, const int primId) {
  // C++ doesn't allow us to declare these in the switch
  xmlChar* idAttr;
  BeePoint* p;
  xmlNode* cur;

  // Needed for vertex parsing
  int i;
  vector<ZarNode**> vertexList;
  vector<ZarNode**>::iterator vertexIt;
  ZarNode** tmpVertex;
  ZarNode** vertices;

  switch(primId) {
  // Create point, add it to the point table and return the object
  case PRIMITIVE_POINT:
    // Read 'id' attribute
    idAttr = getNodeAttribute(node, "id");
    if(idAttr == NULL) {
      cerr << "[Parser] Warning: point element ignored (missing id attribute)!"
           << " [line " << node->line << "]" << endl;
      return NULL;
    }

    // Check for duplicate in the table
    if(pointTable[idAttr] != NULL) {
      cerr << "[Parser] Warning: point element ignored (duplicate '"
           << idAttr << "' id)!"
           << " [line " << node->line << "]" << endl;
      return NULL;
    }

    // Add to point table and return it
    p = new BeePoint();
    pointTable[idAttr] = p;
    return p;
    break;

  // Find sphere arguments and return the object
  case PRIMITIVE_SPHERE:
    return new BeeSphere(parseArgument(node, "radius"),
                         parseArgument(node, "slices"),
                         parseArgument(node, "stacks"));
    break;

  // Find cube arguments and return the object
  case PRIMITIVE_CUBE:
    return new BeeCube(parseArgument(node, "size"));
    break;

  // Find cylinder arguments and return the object
  case PRIMITIVE_CYLINDER:
    return new BeeCylinder(parseArgument(node, "bottom"),
                           parseArgument(node, "top"),
                           parseArgument(node, "height"),
                           parseArgument(node, "slices"),
                           parseArgument(node, "stacks"));
    break;

  // Find polygon vertices and return the object
  case PRIMITIVE_POLYGON:
    // Get the list of vertices
    for(cur = node->children; cur; cur = cur->next) {
      if(cur->type != XML_ELEMENT_NODE) continue;

      tmpVertex = parseVertex(cur);
      if(tmpVertex != NULL) {
        vertexList.push_back(tmpVertex);
      }
    }

    // FIXME: Check there are enough vertices
    if(vertexList.size() < 3) {
      cerr << "[Parser] Warning: a polygon must have at least 3 vertices! element ignored!"
           << " [line " << node->line << "]" << endl;
      return NULL;
    }

    // Convert the STL list into an array
    vertices = new ZarNode*[vertexList.size() * 3];
    for(i = 0, vertexIt = vertexList.begin(); vertexIt != vertexList.end(); ++vertexIt) {
      vertices[i++] = *(*vertexIt);
      vertices[i++] = *(*vertexIt + 1);
      vertices[i++] = *(*vertexIt + 2);
    }
    
    return new BeePoly(vertices, vertexList.size());
    break;

  // Find circle arguments and return the object
  case PRIMITIVE_CIRCLE:
    return new BeeCircle(parseArgument(node, "radius"),
                         parseArgument(node, "vertex-count"));
    break;

  // Find terrain arguments and return the object
  case PRIMITIVE_TERRAIN:
    return new BeeTerrain(parseArgument(node, "width"),
                          parseArgument(node, "height"),
                          parseArgument(node, "subdivisions"),
                          parseArgument(node, "seed"));
    break;


  // Cannot happen except in case of nuclear war
  case PRIMITIVE_NONE:
  default:
    cerr << "[Parser] Warning: primitive parsing error!"
         << " [line " << node->line << "]" << endl;
    return NULL;
    break;
  }
}


/**
 * Return the BeeThing pointed referenced from the given node.
 * If the reference is not valid, NULL is returned.
 *
 * @param node The node containing the reference.
 * @return The BeeThing object referenced from the give node, or NULL
 *   if the reference was not valid.
 */
BeeThing*
ZarParser::parseReference(xmlNode* node) {
  BeeThing* refThing;
  xmlChar* ref = getNodeAttribute(node, "reference");
  if(ref == NULL) {
    // Missing reference argument
    cerr << "[Parser] Warning: '" << node->name << "' require a 'reference' attribute,"
         << " element ignored!"
         << " [line " << node->line << "]" << endl;
    return NULL;
  }

  refThing = groupTable[ref];
  if(refThing == NULL) {
    // Reference to non-existing group
    cerr << "[Parser] Warning: '" << node->name << "' references an unkown group '"
         << ref << "', element ignored!"
         << " [line " << node->line << "]" << endl;
    return NULL;
  }

  // Everything is fine, return the pointed BeeThing
  return refThing;
}


/**
 * Create a BeeModifier object from the given node wrapping the base
 * BeeThing.  If the node is not valid, NULL is returned.
 *
 * @param node The node containing the modifier element.
 * @param base The BeeThing object to which the modifier must be applied.
 * @param primId The type of modifier contained in the node (see class constants)
 * @return The BeeModifier object created from the modifier node, or
 *   NULL if the node was not a valid modifier.
 */
BeeThing*
ZarParser::parseModifier(xmlNode* node, BeeThing* base, const int modId) {
  // C++ doesn't allow us to declare these in the switch
  ZarNode* angle;
  ZarNode* vect[3];
  ZarNode* instances;
  ZarNode* shearing[6];
  ZarNode *red, *green, *blue, *alpha;

  // Get arguments and create modifiers wrapped around the base
  switch(modId) {
  case MODIFIER_ROTATION:
    angle = parseArgument(node, "angle");
    vect[0] = parseArgument(node, "axis-x");
    vect[1] = parseArgument(node, "axis-y");
    vect[2] = parseArgument(node, "axis-z");
    return new BeeRotation(base, angle, vect);
    break;

  case MODIFIER_TRANSLATION:
    vect[0] = parseArgument(node, "x");
    vect[1] = parseArgument(node, "y");
    vect[2] = parseArgument(node, "z");
    return new BeeTranslation(base, vect);
    break;

  case MODIFIER_SCALE:
    vect[0] = parseArgument(node, "x");
    vect[1] = parseArgument(node, "y");
    vect[2] = parseArgument(node, "z");
    return new BeeScale(base, vect);

  case MODIFIER_SPINARRAY:
    angle = parseArgument(node, "angle");
    vect[0] = parseArgument(node, "axis-x");
    vect[1] = parseArgument(node, "axis-y");
    vect[2] = parseArgument(node, "axis-z");
    instances = parseArgument(node, "instances");
    return new BeeSpinArray(base, angle, vect, instances);
    break;

  case MODIFIER_SHEARING:
    // Get all parameters (all optional)
    shearing[0] = parseOptionalArgument(node, "y-xcoeff");
    shearing[1] = parseOptionalArgument(node, "z-xcoeff");
    shearing[2] = parseOptionalArgument(node, "x-ycoeff");
    shearing[3] = parseOptionalArgument(node, "z-ycoeff");
    shearing[4] = parseOptionalArgument(node, "x-zcoeff");
    shearing[5] = parseOptionalArgument(node, "y-zcoeff");

    // Missing parameters are replaced by zeroes
    for(int i = 0; i < 6; ++i) {
      if(shearing[i] == NULL) {
        shearing[i] = new ZarConst(0);
      }
    }

    return new BeeShearing(base, shearing);
    break;

  case MODIFIER_EXTRUDE:
    angle = parseArgument(node, "angle");
    vect[0] = parseArgument(node, "axis-x");
    vect[1] = parseArgument(node, "axis-y");
    vect[2] = parseArgument(node, "axis-z");
    instances = parseArgument(node, "stacks");
    return new BeeExtrude(base, vect, angle, instances);
    break;

  case MODIFIER_COLOR:
    red   = parseArgument(node, "red");
    green = parseArgument(node, "green");
    blue  = parseArgument(node, "blue");
    alpha = parseArgument(node, "alpha");
    return new BeeTexColor(base, red, green, blue, alpha);

  case MODIFIER_NONE:
  default:
    // Unknown element, shouldn't happen
    cerr << "[Parser] Warning: unknown modifier element '"
         << node->name << "' ignored!"
         << " [line " << node->line << "]" << endl;
    break;
  }

  return NULL;
}


/**
 * Create a texture BeeModifier object from the given node wrapping
 * the base BeeThing.  If the node is not valid, NULL is returned.
 *
 * If the texture had already been loaded, only use a reference to the
 * existing Texture object.
 *
 * @param node The node containing the texture element.
 * @param base The BeeThing object to which the texture must be applied.
 * @param texId The type of texture contained in the node (see class constants)
 * @return The texture BeeModifier object created from the texture
 *   node, or NULL if the node was not a valid texture.
 */
BeeThing*
ZarParser::parseTexture(xmlNode* node, BeeThing* base, const int texId) {
  Texture* tex;

  // Get filename
  xmlChar* filename = getNodeAttribute(node, "filename");
  if(filename == NULL) {
    // Missing filename attribute, don't apply the tex modifier
    cerr << "[Parser] Warning: the '" << node->name
         << "' element need a 'filename' attribute, element ignored!"
         << " [line " << node->line << "]" << endl;
    return base;
  }

  // If texture does not exist yet, create it and add to the list
  tex = textureTable[filename];
  if(tex == NULL) {
    tex = new Texture((char*)filename);
    textureTable[filename] = tex;
  }

  // Create texture wrapped around the base
  switch(texId) {
  case TEXTURE_LINEAR:      return new BeeTexObjectLinear(base, tex);
  case TEXTURE_SPHERICAL:   return new BeeTexSphereMap(base, tex);
  case TEXTURE_NONE:
  default:
    // Can *not* happen (one-legged mutant spider raid excepted)
    break;
  }

  // Won't happen anyway
  return NULL;
}


ZarNode**
ZarParser::parseVertex(xmlNode* node) {
  if(!nodeNameEquals(node, "vertex")) {
    cerr << "[Parser] Warning: invalid element '" << node->name
         << "' found instead of vertex list ignored!"
         << " [line " << node->line << "]" << endl;
    return NULL;
  }

  ZarNode** vertex = new ZarNode*[3];
  vertex[0] = parseArgument(node, "x");
  vertex[1] = parseArgument(node, "y");
  vertex[2] = parseArgument(node, "z");
  return vertex;
}


Camera*
ZarParser::parseCamera(xmlNode* node) {
  // C++ forbids variable declaration in a switch
  xmlChar* keyAttr;
  xmlChar* positionAttr;
  xmlChar* lookAtAttr;

  BeePoint* position;
  BeePoint* lookAt;

  int key;
  GLfloat opening;

  // If this is a camera element, load the arguments
  int camType = checkCameraNode(node);
  bool valid = true;
  switch(camType) {
  case CAMERA_FIXED:
  case CAMERA_FREE:
  case CAMERA_PAN:
  case CAMERA_FREE_PAN:

    // Get arguments, print error if missing
    keyAttr      = getNodeAttribute(node, "key");
    positionAttr = getNodeAttribute(node, "position");
    lookAtAttr   = getNodeAttribute(node, "lookat");

    // Do we have a 'key' attribute?
    if(keyAttr == NULL) {
      warningMissingArgument(node, "key");
      valid = false;
    }
    else {
      // Check the validity of the number argument
      try {
        key = charToLong((char*)keyAttr);
      }
      catch(NumberFormatException &e) {
        cerr << "[Parser] Warning: invalid key '"
             << keyAttr << "' for a camera!"
             << " [line " << node->line << "]" << endl;
        valid = false;
        break;
      }
    }

    // Get the 'opening' attribute
    try {
      opening = parseDoubleAttribute(node, "opening");
    }
    catch(NumberFormatException &e) {
      warningNoValidDouble(node, "opening");
      valid = false;
    }

    // Do we have a 'position' attribute?
    if(positionAttr == NULL) {
      warningMissingArgument(node, "position");
      valid = false;
    }
    else {
      // Get point references, print error if unknown
      position = pointTable[positionAttr];
      if(position == NULL) {
        warningUndefinedPointRef(node, positionAttr, "position");
        valid = false;
      }
    }

    // Do we have a 'lookat' attribute?
    if(lookAtAttr == NULL) {
      warningMissingArgument(node, "lookat");
      valid = false;
    }
    else {
      // Get point references, print error if unknown
      lookAt = pointTable[lookAtAttr];
      if(position == NULL) {
        warningUndefinedPointRef(node, lookAtAttr, "lookat");
        valid = false;
      }
    }
    break;

  // Unkown camera type, print error and leave
  case CAMERA_NONE:
  default:
    cerr << "[Parser] Warning: unknown camera element '"
         << node->name << "' ignored!"
         << " [line " << node->line << "]" << endl;
    return NULL;
    break;
  }

  // Leave now if we found invalid data
  if(!valid) {
    cerr << "                  camera element ignored!" << endl;
    return NULL;
  }

  // Create and return the object
  switch(camType) {
  case CAMERA_FIXED:    return new FixedCamera(position, lookAt, opening);
  case CAMERA_FREE:     return new FreeLookCamera(position, lookAt, opening);
  case CAMERA_PAN:      return new PanCamera(position, lookAt, opening);
  case CAMERA_FREE_PAN: return new FreeLookPanCamera(position, lookAt, opening);
  case CAMERA_NONE:
  default:
    // Can *not* happen (zombie wasps invasion excepted)
    break;
  }

  // Won't happen anyway
  return NULL;
}


Light*
ZarParser::parseLight(xmlNode* node) {
  // C++ forbids variable declaration in a switch
  xmlChar* positionAttr;
  xmlChar* lookAtAttr;
  xmlChar* directionAttr;

  BeePoint* position;
  BeePoint* lookAt;
  BeePoint* direction;

  ZarNode *cutoff, *exponent;
  ZarNode* intensity;
  ZarNode* colors[3];

  // If this is a light element, load the arguments
  int lightType = checkLightNode(node);
  bool valid = true;
  bool hasIntensity, hasColors;

  // Special for the sun, get 'direction'
  if(lightType == LIGHT_SUN) {
    directionAttr = getNodeAttribute(node, "direction");

    // Do we have a 'direction' attribute?
    if(directionAttr == NULL) {
      warningMissingArgument(node, "direction");
      valid = false;
    }
    else {
      // Get point references, print error if unknown
      direction = pointTable[directionAttr];
      if(direction == NULL) {
        warningUndefinedPointRef(node, directionAttr, "direction");
        valid = false;
      }
    }
  }

  switch(lightType) {
  case LIGHT_SPOT:

    // Get arguments, print error if missing
    lookAtAttr   = getNodeAttribute(node, "lookat");

    // Do we have a 'lookat' attribute?
    if(lookAtAttr == NULL) {
      warningMissingArgument(node, "lookat");
      valid = false;
    }
    else {
      // Get point references, print error if unknown
      lookAt = pointTable[lookAtAttr];
      if(lookAt == NULL) {
        warningUndefinedPointRef(node, lookAtAttr, "lookat");
        valid = false;
      }
    }

    // Get the 'cutoff' attribute
    cutoff = parseArgument(node, "cutoff");
    if(cutoff == NULL) {
      warningNoValidDouble(node, "cutoff");
      valid = false;
    }

    // Get the 'exponent' attribute
    exponent = parseArgument(node, "exponent");
    if(exponent == NULL) {
      warningNoValidDouble(node, "exponent");
      valid = false;
    }


  case LIGHT_LAMP:
    // Get arguments, print error if missing
    positionAttr = getNodeAttribute(node, "position");

    // Do we have a 'position' attribute?
    if(positionAttr == NULL) {
      warningMissingArgument(node, "position");
      valid = false;
    }
    else {
      // Get point references, print error if unknown
      position = pointTable[positionAttr];
      if(position == NULL) {
        warningUndefinedPointRef(node, positionAttr, "position");
        valid = false;
      }
    }

  case LIGHT_AMBIENT:
  case LIGHT_SUN:
    // Get the 'intensity' attribute
    intensity = parseOptionalArgument(node, "intensity");
    hasIntensity = (intensity != NULL);

    // Get the per color 'intensity' attribute
    colors[0] = parseOptionalArgument(node, "red-intensity");
    colors[1] = parseOptionalArgument(node, "green-intensity");
    colors[2] = parseOptionalArgument(node, "blue-intensity");
    hasColors = (colors[0] != NULL) && (colors[1] != NULL) && (colors[2] != NULL);

    // Need at least one type of intensity
    if(!hasIntensity && !hasColors) {
      cerr << "[Parser] Warning: light element '" << node->name
           << "' missing intensity value (general or per-color)!"
           << " [line " << node->line << "]" << endl;
      valid = false;
    }
    // Only accept one type of intensity
    else if(hasIntensity && hasColors) {
      cerr << "[Parser] Warning: light element '" << node->name
           << "' can only have either general OR per-color intensity!"
           << " [line " << node->line << "]" << endl;
      valid = false;
    }
    break;

  // Unkown light type, print error and leave
  case LIGHT_NONE:
  default:
    cerr << "[Parser] Warning: unknown light element '"
         << node->name << "' ignored!"
         << " [line " << node->line << "]" << endl;
    return NULL;
    break;
  }

  // Leave now if we found invalid data
  if(!valid) {
    cerr << "                  light element ignored!" << endl;
    return NULL;
  }

  // Create and return the object
  switch(lightType) {
  case LIGHT_SPOT:
    if(hasIntensity) return new Spot(position, lookAt, cutoff, exponent, intensity);
    else             return new Spot(position, lookAt, cutoff, exponent, colors);

  case LIGHT_LAMP:   
    if(hasIntensity) return new Lamp(position, intensity);
    else             return new Lamp(position, colors);

  case LIGHT_AMBIENT:
    if(hasIntensity) return new AmbientLight(intensity);
    else             return new AmbientLight(colors);

  case LIGHT_SUN:
    if(hasIntensity) return new Sun(direction, intensity);
    else             return new Sun(direction, colors);

  case LIGHT_NONE:
  default:
    // Can *not* happen (radioactive aquatic spider bite excepted)
    break;
  }

  // Won't happen anyway
  return NULL;
}


pair<int,int>
ZarParser::parseScheduleCheckpoint(xmlNode* node) throw(NumberFormatException) {
  int time, camera;
  bool valid = true;

  // Parse the time attribute
  try {
    time = (int)parseLongAttribute(node, "time");
  }
  catch(NumberFormatException &e) {
    cerr << "[Parser] Warning: checkpoint misses the time attribute!"
         << " [line " << node->line << "]" << endl;
    valid = false;
  }

  // Parse the camera attribute
  try {
    camera = (int)parseLongAttribute(node, "camera");
  }
  catch(NumberFormatException &e) {
    cerr << "[Parser] Warning: checkpoint misses the camera attribute!"
         << " [line " << node->line << "]" << endl;
    valid = false;
  }

  if(valid) {
    return pair<int,int>(time, camera);
  }

  throw NumberFormatException("invalid checkpoint");
}


/**
 * Parse an integer attribute for a node.
 *
 * @param node The node containing the attribute.
 * @param attName The name of the attribute to parse.
 * @return The long value of the attribute.
 * @throw NumberFormatException Thrown if no valid value is found.
 */
long
ZarParser::parseLongAttribute(xmlNode* node, char* attName) throw(NumberFormatException) {
  xmlChar* attr = getNodeAttribute(node, attName);
  if(attr != NULL) {
    int value = charToLong((char*)attr);
    return value;
  }

  // No attribute found
  throw NumberFormatException("no value found for the attribute");
}


/**
 * Parse a floating point attribute for a node.
 *
 * @param node The node containing the attribute.
 * @param attName The name of the attribute to parse.
 * @return The double value of the attribute.
 * @throw NumberFormatException Thrown if no valid value is found.
 */
double
ZarParser::parseDoubleAttribute(xmlNode* node, char* attName) throw(NumberFormatException) {
  xmlChar* attr = getNodeAttribute(node, attName);
  if(attr != NULL) {
    double value = charToDouble((char*)attr);
    return value;
  }

  // No attribute found
  throw NumberFormatException("no value found for the attribute");
}



// =============== NODE/TREE METHODS ===============

/**
 * Return the text of the attName attribute of the given node, or
 * NULL if no such attribute is found in the node.
 *
 * @param node The node containing the attribute.
 * @param attName The name of the attribute.
 * @return The string value of the attribute, or NULL if none found.
 */
xmlChar*
ZarParser::getNodeAttribute(xmlNode* node, char* attName) {
  // Look for the attribute in the children of the node
  xmlAttr *cur = NULL;
  for (cur = node->properties; cur; cur = cur->next) {
    if((cur->type == XML_ATTRIBUTE_NODE) && attrNameEquals(cur, attName)) {
      // Get text from the attribute
      xmlNode* value = cur->children;
      if(xmlNodeIsText(value)) {
        return value->content;
      }
    }
  }

  return NULL;
}


/**
 * Find an argument in the given node.  Look if the argument is
 * present as an attribute, in which case it is either a constant
 * value or a reference to an existing parameter, or if the argument
 * is a child node, in which case it is a function tree.  Work on the
 * whole thing to retrieve (or create) a pointer to a ZarNode
 * corresponding to the argument.
 *
 * @param node The node containing the argument.
 * @param argument The name of the argument to look for.
 * @param registerAnon If true, any ZarConst created in this method
 *   will be added to the anonymous params in the table.
 * @param optional Whether the argument is optional; if not, a fatal
     error is triggered if the argument is not found.
 * @return A pointer to the ZarNode representing the argument.
 */
ZarNode*
ZarParser::getXmlNodeArgument(xmlNode* node, char* argument, bool registerAnon, bool optional) {
  // Check if present as an attribute
  char* attr = (char*)getNodeAttribute(node, argument);

  // Check if present as a child element
  xmlNode *elem = NULL;
  for (elem = node->children; elem; elem = elem->next) {
    // Only check for elements
    if(elem->type != XML_ELEMENT_NODE) continue;

    // Found an element with the same node name?
    if(nodeNameEquals(elem, argument)) break;
  }
  
  // Present as both attribute and element, print an error
  if((attr != NULL) && (elem != NULL)) {
    cerr << "[Parser] Warning: node '" << node->name << "' has '"
         << argument << "' argument both as attribute and element!"
         << " using attribute only."
         << " [line " << node->line << "]" << endl;
  }

  // Parse the inline const value or the identifier pointer
  if(attr != NULL) {
    // If it is a valid value, create a ZarConst for it and add it to the table
    try {
      zartype value = charToDouble(attr);
      ZarNode* anon = new ZarConst(value);
      if(registerAnon) {
        paramTable->addAnonymousParam(anon);
      }
      return anon;
    }
    // It must be an identifier, find the ZarParam pointer
    catch(NumberFormatException &e) {
      // If undefined parameter, print and error and QUIT
      ZarNode* arg = paramTable->getParam(attr);
      if(arg == NULL) {
        cerr << "[Parser] Error: argument '" << argument << "' of the '"
             << node->name << "' name points to undefined parameter '"
             << attr << "'! Parsing failed, stop execution..."
             << " [line " << node->line << "]" << endl;
        exit(1);
      }

      // New reference to that node, increment instances
      arg->addInstance();

      return arg;
    }
  }

  // Return the ZarNode tree of the inline function
  if(elem != NULL) {
    int numChildren = 0;
    for(xmlNode *it = elem->children; it; it = it->next) {
      // Only check for elements
      if(it->type != XML_ELEMENT_NODE) continue;
      elem = it;
      ++numChildren;
    }

    // No child, no function tree, print an error
    if(numChildren == 0) {
      cerr << "[Parser] Error: node '" << node->name
           << "' has an empty argument '" << argument
           << "'! Parsing failed, stop execution..."
           << " [line " << node->line << "]" << endl;
      exit(1);
    }

    // We only need one child, print an error
    if(numChildren > 1) {
      cerr << "[Parser] Warning: too many '" << argument
           << "' argument for the '" << node->name
           << "' node! Only reading the last one."
           << " [line " << node->line << "]" << endl;
    }

    // Register the tree if we are asked to
    ZarNode* anon = buildFunctionTree(elem);
    if(registerAnon) {
      paramTable->addAnonymousParam(anon);
    }
    return anon;
  }


  // None is present, print an error and QUIT
  if(!optional) {
    errorMissingArgument(node, argument);
    exit(1);
  }
     
  return NULL;
}


/**
 * Recursively build the function tree corresponding to the function
 * nodes (the top of which is the given node). Arguments to functions
 * are retrieved with the getXmlNodeArgument, which recursively calls
 * buildFunctionTree back.
 *
 * @node The top node of the function tree to parse.
 * @return A pointer to the top ZarNode of the ZarNode tree
 *   corresponding to the functions.
 */
ZarNode*
ZarParser::buildFunctionTree(xmlNode* node) {
  // C++ forbids variable declaration in a switch
  ZarNode* operand;
  ZarNode* left;
  ZarNode* right;

  int funcId = checkFunctionNode(node);

  switch(funcId) {
  // Unary functions, get operand
  case FUNCTION_SIN:
  case FUNCTION_COS:
  case FUNCTION_ASIN:
  case FUNCTION_ACOS:
  case FUNCTION_SQUARE_WAVE:
    operand = getXmlNodeArgument(node, "operand");

    // Compulsory argument, print error if absent
    if(operand == NULL) {
      errorMissingArgument(node, "operand");
      exit(1);
    }
    break;

  // Binary functions, get left and right operand
  case FUNCTION_ADD:
  case FUNCTION_MULT:
  case FUNCTION_SUB:
  case FUNCTION_DIV:
  case FUNCTION_POW:
  case FUNCTION_MIN:
  case FUNCTION_MAX:
    left  = getXmlNodeArgument(node, "left");
    right = getXmlNodeArgument(node, "right");

    // Compulsory arguments, print error if absent
    if(left == NULL) {
      errorMissingArgument(node, "left");
    }
    if(right == NULL) {
      errorMissingArgument(node, "right");
    }
    if((left == NULL) || (right == NULL)) {
      exit(1);
    }
    break;

  case FUNCTION_NONE:
  default:
    cerr << "[Parser] Error: expected function node, found '"
         << node->name << "'! Parsing failed, stop execution..."
         << " [line " << node->line << "]" << endl;
    exit(1);
    break;
  }

  // Create and return the correct ZarNode
  switch(funcId) {
  case FUNCTION_SIN:   return new ZarSin(operand);
  case FUNCTION_COS:   return new ZarCos(operand);
  case FUNCTION_ASIN:  return new ZarAsin(operand);
  case FUNCTION_ACOS:  return new ZarAcos(operand);
  case FUNCTION_ADD:   return new ZarAdd(left, right);
  case FUNCTION_MULT:  return new ZarMult(left, right);
  case FUNCTION_SUB:   return new ZarSub(left, right);
  case FUNCTION_DIV:   return new ZarDiv(left, right);
  case FUNCTION_POW:   return new ZarPow(left, right);
  case FUNCTION_MIN:   return new ZarMin(left, right);
  case FUNCTION_MAX:   return new ZarMax(left, right);
  case FUNCTION_SQUARE_WAVE: return new ZarSquareWave(operand);
  case FUNCTION_NONE:
  default:
    // Can *not* happen (transsexual alien attack excepted)
    break;
  }

  // That's unpossible!
  return NULL;
}



// =============== NODE CHECKERS ===============

/**
 * Determine whether the given node is a primitive, and return the id
 * corresponding to that primitive.  When new primitive elements are
 * added to the scene DTD, they should be added here as well!
 */
int
ZarParser::checkPrimitiveNode(xmlNode* node) {
  if(nodeNameEquals(node, "point"))    return PRIMITIVE_POINT;
  if(nodeNameEquals(node, "sphere"))   return PRIMITIVE_SPHERE;
  if(nodeNameEquals(node, "cube"))     return PRIMITIVE_CUBE;
  if(nodeNameEquals(node, "cylinder")) return PRIMITIVE_CYLINDER;
  if(nodeNameEquals(node, "polygon"))  return PRIMITIVE_POLYGON;
  if(nodeNameEquals(node, "circle"))   return PRIMITIVE_CIRCLE;
  if(nodeNameEquals(node, "terrain"))  return PRIMITIVE_TERRAIN;

  return PRIMITIVE_NONE;
}


/**
 * Determine whether the given node is a modifier, and return the id
 * corresponding to that modifier.  .  When new modifier
 * elements are added to the scene DTD, they should be added here as
 * well!
 */
int
ZarParser::checkModifierNode(xmlNode* node) {
  if(nodeNameEquals(node, "rotate"))    return MODIFIER_ROTATION;
  if(nodeNameEquals(node, "translate")) return MODIFIER_TRANSLATION;
  if(nodeNameEquals(node, "scale"))     return MODIFIER_SCALE;
  if(nodeNameEquals(node, "spin-array")) return MODIFIER_SPINARRAY;
  if(nodeNameEquals(node, "shear"))     return MODIFIER_SHEARING;
  if(nodeNameEquals(node, "extrude"))   return MODIFIER_EXTRUDE;
  if(nodeNameEquals(node, "color"))     return MODIFIER_COLOR;

  return MODIFIER_NONE;
}


/**
 * Determine whether the given node is a function, and return the id
 * corresponding to that function.  When new function elements are
 * added to the scene DTD, they should be added here as well!
 */
int
ZarParser::checkFunctionNode(xmlNode* node) {
  if(nodeNameEquals(node, "sin"))    return FUNCTION_SIN;
  if(nodeNameEquals(node, "cos"))    return FUNCTION_COS;
  if(nodeNameEquals(node, "asin"))   return FUNCTION_ASIN;
  if(nodeNameEquals(node, "acos"))   return FUNCTION_ACOS;

  if(nodeNameEquals(node, "add"))    return FUNCTION_ADD;
  if(nodeNameEquals(node, "mult"))   return FUNCTION_MULT;
  if(nodeNameEquals(node, "sub"))    return FUNCTION_SUB;
  if(nodeNameEquals(node, "div"))    return FUNCTION_DIV;

  if(nodeNameEquals(node, "min"))    return FUNCTION_MIN;
  if(nodeNameEquals(node, "max"))    return FUNCTION_MAX;
  
  if(nodeNameEquals(node, "square_wave"))    return FUNCTION_SQUARE_WAVE;

  return FUNCTION_NONE;
}

/**
 * Determine whether the given node is a camera, and return the id
 * corresponding to that camera.  When new camera elements are
 * added to the scene DTD, they should be added here as well!
 */
int
ZarParser::checkCameraNode(xmlNode* node) {
  if(nodeNameEquals(node, "fixed-camera"))    return CAMERA_FIXED;
  if(nodeNameEquals(node, "free-camera"))     return CAMERA_FREE;
  if(nodeNameEquals(node, "pan-camera"))      return CAMERA_PAN;
  if(nodeNameEquals(node, "free-pan-camera")) return CAMERA_FREE_PAN;

  return CAMERA_NONE;
}

/**
 * Determine whether the given node is a light, and return the id
 * corresponding to that light.  When new light elements are
 * added to the scene DTD, they should be added here as well!
 */
int
ZarParser::checkLightNode(xmlNode* node) {
  if(nodeNameEquals(node, "lamp"))    return LIGHT_LAMP;
  if(nodeNameEquals(node, "spot"))    return LIGHT_SPOT;
  if(nodeNameEquals(node, "ambient")) return LIGHT_AMBIENT;
  if(nodeNameEquals(node, "sun"))     return LIGHT_SUN;

  return LIGHT_NONE;
}

/**
 * Determine whether the given node is a texture, and return the id
 * corresponding to that texture.  When new texture elements are
 * added to the scene DTD, they should be added here as well!
 */
int
ZarParser::checkTextureNode(xmlNode* node) {
  if(nodeNameEquals(node, "linear-texture"))    return TEXTURE_LINEAR;
  if(nodeNameEquals(node, "spherical-texture")) return TEXTURE_SPHERICAL;

  return TEXTURE_NONE;
}



// =============== UTILITIES ===============

/**
 * Determine whether the name of the given node is equal to the given
 * name.
 */
bool
ZarParser::nodeNameEquals(xmlNode* node, char* name) {
  return (!strcmp((char*) node->name, name));
}

/**
 * Determine whether the name of the given attribute is equal to the
 * given name.
 */
bool
ZarParser::attrNameEquals(xmlAttr* node, char* name) {
  return (!strcmp((char*) node->name, name));
}


/**
 * Convert a char* string into the floating point number contained
 * in that string.
 *
 * @param number The string containing the number.
 * @throw NumberFormatException Thrown if the string does not contain
 *   a proper number.
 * @return The number parsed from the string.
 */
double
ZarParser::charToDouble(char* number) throw(NumberFormatException) {
  char* end = number;
  double value = strtod(number, &end);

  // Not all characters were parsed, throw an error
  if(end != (number + strlen(number))) {
    throw NumberFormatException("incorrect number in string");
  }

  return value;
}


/**
 * Convert a char* string into the integer number contained in that
 * string.
 *
 * @param number The string containing the number.
 * @throw NumberFormatException Thrown if the string does not contain
 *   a proper number.
 * @return The number parsed from the string.
 */
long
ZarParser::charToLong(char* number) throw(NumberFormatException) {
  char* end = number;
  int radix = 10;
  long value = strtol(number, &end, radix);

  // Not all characters were parsed, throw an error
  if(end != (number + strlen(number))) {
    throw NumberFormatException("incorrect number in string");
  }

  return value;
}



// =============== ERROR DISPLAY ===============

/**
 * Display a warning message saying that the given node has an
 * argument pointing to an undefined point.
 */
void
ZarParser::warningUndefinedPointRef(xmlNode* node, xmlChar* ref, char* argument) {
  cerr << "[Parser] Warning: reference to undefined point '"
       << ref << "' in argument '" << argument
       << "' of node '" << node->name << "'!"
       << " [line " << node->line << "]" << endl;
}

/**
 * Display a warning message saying that the given node requires an
 * argument.
 */
void
ZarParser::warningMissingArgument(xmlNode* node, char* argument) {
  cerr << "[Parser] Warning: missing argument '"
       << argument << "' for the node '" << node->name << "'!"
       << " [line " << node->line << "]" << endl;
}

/**
 * Display an error message saying that the given node requires an
 * argument.
 */
void
ZarParser::errorMissingArgument(xmlNode* node, char* argument) {
  cerr << "[Parser] Error: missing argument '"
       << argument << "' for the node '" << node->name << "'!"
       << " Parsing failed, stop execution..."
       << " [line " << node->line << "]" << endl;
}

/**
 * Display a warning saying that the given node does not have a valid
 * double value for its attribute.
 */
void
ZarParser::warningNoValidDouble(xmlNode* node, char* argument) {
  cerr << "[Parser] Warning: no valid value for the '"
       << argument << "' of the '" << node->name << "' node!"
       << " [line " << node->line << "]" << endl;
}
