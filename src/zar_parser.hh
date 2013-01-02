/*
 * ZarBee - zar_parser.hh
 *
 * Author: seb
 * Created on: mer fév 16 18:23:08 CET 2005 *
 */

#ifndef _zar_parser_hh_
#define _zar_parser_hh_

#include "exception.hh"
#include "bee_base.hh"
#include "bee_primitives.hh"
#include "bee_modifiers.hh"
#include "zar_param_table.hh"

#include "world.hh"

#include <vector>
#include <list>
#include <map>

#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;
using namespace ZarBee;

/**
 * Exception thrown when a number conversion fails because it is not
 * properly formatted.
 */
class NumberFormatException : public Exception {
public:
  NumberFormatException(char* e) : Exception(e) { }
};


/**
 * Wrapper for the comparison function between two xmlChar*, needed
 * for the member maps.
 */
struct xmlCharCmp {
  bool operator() (xmlChar* s1, xmlChar* s2) const {
    return xmlStrcmp(s1, s2) < 0;
  }
};


class ZarParser {
protected:

  // Constants representing the primitives
  static const int PRIMITIVE_NONE     = 0;
  static const int PRIMITIVE_POINT    = 1;
  static const int PRIMITIVE_SPHERE   = 2;
  static const int PRIMITIVE_CUBE     = 3;
  static const int PRIMITIVE_CYLINDER = 4;
  static const int PRIMITIVE_POLYGON  = 5;
  static const int PRIMITIVE_CIRCLE   = 6;
  static const int PRIMITIVE_TERRAIN  = 7;

  // Constants representing the modifiers
  static const int MODIFIER_NONE         = 0;
  static const int MODIFIER_ROTATION     = 1;
  static const int MODIFIER_TRANSLATION  = 2;
  static const int MODIFIER_SCALE        = 3;
  static const int MODIFIER_SPINARRAY    = 4;
  static const int MODIFIER_SHEARING     = 5;
  static const int MODIFIER_EXTRUDE      = 6;
  static const int MODIFIER_COLOR        = 7;

  // Constants representing the functions
  static const int FUNCTION_NONE     = 0;
  static const int FUNCTION_SIN      = 1;
  static const int FUNCTION_COS      = 3;
  static const int FUNCTION_ASIN     = 4;
  static const int FUNCTION_ACOS     = 5;
  static const int FUNCTION_ADD      = 6;
  static const int FUNCTION_MULT     = 7;
  static const int FUNCTION_SUB      = 8;
  static const int FUNCTION_DIV      = 9;
  static const int FUNCTION_POW      = 10;
  static const int FUNCTION_MIN      = 11;
  static const int FUNCTION_MAX      = 12;
  static const int FUNCTION_SQUARE_WAVE = 13;

  // Constants representing the cameras
  static const int CAMERA_NONE     = 0;
  static const int CAMERA_FIXED    = 1;
  static const int CAMERA_FREE     = 2;
  static const int CAMERA_PAN      = 3;
  static const int CAMERA_FREE_PAN = 4;

  // Constants representing the lights
  static const int LIGHT_NONE     = 0;
  static const int LIGHT_LAMP     = 1;
  static const int LIGHT_SPOT     = 2;
  static const int LIGHT_AMBIENT  = 3;
  static const int LIGHT_SUN      = 4;

  // Constants representing the texture modes
  static const int TEXTURE_NONE      = 0;
  static const int TEXTURE_LINEAR    = 1;
  static const int TEXTURE_SPHERICAL = 2;


  // Pointer to param table
  ZarParamTable* paramTable;

  // Map of id-group
  map<xmlChar*, BeeThing*, xmlCharCmp> groupTable;

  // Map of id-points
  map<xmlChar*, BeePoint*, xmlCharCmp> pointTable;

  // Map of filename-textures
  map<xmlChar*, Texture*, xmlCharCmp> textureTable;

  // List of cameras
  vector<Camera*>* cameraList;

  // List of lights
  list<Light*>* lightList;

  // Camera schedule list
  schedule_t* cameraSchedule;

  // Music object
  char* musicFilename;
  bool  musicLoop;

  // Duration of the scene (0 if no duration given)
  int duration;


  xmlDoc* doc;
  xmlNode* rootElement;

  void init(char* filename, ZarParamTable* params);
  void run();
  void destroy();
  bool validate();

  void parseInclude(xmlNode* node);
  void parseParam(xmlNode* node);
  void parseCameraList(xmlNode* node);
  void parseLightList(xmlNode* node);
  void parseCameraSchedule(xmlNode* node);
  void parseMusic(xmlNode* node);
  void parseDuration(xmlNode* node);

  ZarNode* parseArgument(xmlNode* node, char* argument);
  ZarNode* parseOptionalArgument(xmlNode* node, char* argument);
  long parseLongAttribute(xmlNode* node, char* attName) throw(NumberFormatException);
  double parseDoubleAttribute(xmlNode* node, char* attName) throw(NumberFormatException);
  BeeThing* parseGroup(xmlNode* node);
  BeeThing* parsePrimitive(xmlNode* node, const int primId);
  BeeThing* parseReference(xmlNode* node);
  BeeThing* parseModifier(xmlNode* node, BeeThing* base, const int modId);
  BeeThing* parseTexture(xmlNode* node, BeeThing* base, const int texId);
  ZarNode** parseVertex(xmlNode* node);

  Camera* parseCamera(xmlNode* node);
  Light* parseLight(xmlNode* node);
  pair<int,int> parseScheduleCheckpoint(xmlNode* node) throw(NumberFormatException);

  ZarNode* getXmlNodeArgument(xmlNode* node, char* argument, bool registerAnon = false, bool optional = false);
  xmlChar* getNodeAttribute(xmlNode* node, char* attName);
  ZarNode* buildFunctionTree(xmlNode* node);

  int checkPrimitiveNode(xmlNode* node);
  int checkModifierNode(xmlNode* node);
  int checkFunctionNode(xmlNode* node);
  int checkCameraNode(xmlNode* node);
  int checkLightNode(xmlNode* node);
  int checkTextureNode(xmlNode* node);

  bool nodeNameEquals(xmlNode* node, char* name);
  bool attrNameEquals(xmlAttr* node, char* name);

  double charToDouble(char* number) throw(NumberFormatException);
  long charToLong(char* number) throw(NumberFormatException);

  void warningUndefinedPointRef(xmlNode* node, xmlChar* ref, char* argument);
  void warningMissingArgument(xmlNode* node, char* argument);
  void errorMissingArgument(xmlNode* node, char* argument);
  void warningNoValidDouble(xmlNode* node, char* argument);

public:

  ZarParser();
  ~ZarParser();

  World* parseScene(char* filename, ZarParamTable* params);
};

#endif // _zar_parser_hh_
