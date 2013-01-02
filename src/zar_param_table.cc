/*
 * ZarBee - zar_param_table.cc
 *
 * Author: seb
 * Created on: mer fév 16 15:55:31 CET 2005 *
 */

#include "zar_param_table.hh"


ZarParamTable::ZarParamTable() { }
ZarParamTable::~ZarParamTable() {
  ParamTableMap::iterator pos;
  for(pos = table.begin(); pos != table.end(); ++pos) {
    pos->second->removeInstance();
  }
}

void
ZarParamTable::addParam(char* name, ZarNode* param) {
  table[name] = param;
}

void
ZarParamTable::addAnonymousParam(ZarNode* param) {
  anonList.push_back(param);
}

/**
 * Return the parameter with the given name.
 *
 * @param name The name of the parameter
 */ 
ZarNode*
ZarParamTable::getParam(char* name) {
  return table[name];
}

/**
 * Initialize the node trees stored in the table and the anonymous
 * list, i.e. running the initialize() function of all the top nodes.
 */
void
ZarParamTable::initialize() {
  ParamTableMap::iterator pos;
  for(pos = table.begin(); pos != table.end(); ++pos) {
    pos->second->initialize();
    pos->second->updateCache(true);
  }

  AnonParamList::iterator pos2;
  for(pos2 = anonList.begin(); pos2 != anonList.end(); ++pos2) {
    (*pos2)->initialize();
    (*pos2)->updateCache(true);
  }
}

/**
 * Update the cache in all the nodes stored in the table and the
 * anonymous list.
 */
void
ZarParamTable::updateCache() {
  ParamTableMap::iterator pos;
  for(pos = table.begin(); pos != table.end(); ++pos) {
    pos->second->updateCache();
  }

  AnonParamList::iterator pos2;
  for(pos2 = anonList.begin(); pos2 != anonList.end(); ++pos2) {
    (*pos2)->updateCache();
  }
}



void
ZarParamTable::print(std::ostream& os) {
  ParamTableMap::iterator pos;
  for(pos = table.begin(); pos != table.end(); ++pos) {
    os << "[" << pos->first << "]" << std::endl;
    os << (*pos->second) << std::endl;
  }

  int i = 0;
  AnonParamList::iterator pos2;
  for(pos2 = anonList.begin(); pos2 != anonList.end(); ++pos2, ++i) {
    os << "(anonymous " << i << ")" << std::endl;
    os << (**pos2) << std::endl;
  }
}


std::ostream&
operator<<(std::ostream& os, ZarParamTable& table) {
  table.print(os);
  return os;
}
