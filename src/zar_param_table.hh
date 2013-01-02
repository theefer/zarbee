/*
 * ZarBee - zar_param_table.hh
 *
 * Author: seb
 * Created on: mer fév 16 15:55:31 CET 2005 *
 */

#ifndef _zar_param_table_hh_
#define _zar_param_table_hh_

#include "bee_base.hh"
#include "zar_nodes.hh"

#include <iostream>
#include <map>
#include <list>
#include <cstring>


/**
 * Wrapper for the comparison function between two char*, needed
 * for the member maps.
 */
struct charCmp {
  bool operator() (char* s1, char* s2) const {
    return strcmp(s1, s2) < 0;
  }
};

typedef std::map<char*, ZarNode*, charCmp> ParamTableMap;
typedef std::list<ZarNode*> AnonParamList;


class ZarParamTable {
protected:
  ParamTableMap table;
  AnonParamList anonList;  
 
public:
  ZarParamTable();
  ~ZarParamTable();

  void addParam(char* name, ZarNode* param);
  void addAnonymousParam(ZarNode* param);

  ZarNode* getParam(char* name);

  void initialize();
  void updateCache();


  // Allow the table to be outputted
  void print(std::ostream& os);
  friend std::ostream& operator<<(std::ostream&, ZarParamTable&);
};


#endif // _zar_param_table_hh_
