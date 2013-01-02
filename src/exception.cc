/*
 * ZarBee - exception.cc
 *
 * Author: seb
 * Created on: ven fév 25 18:01:46 CET 2005 *
 */

#include "exception.hh"


Exception::Exception(char* msg) : message(msg) { }
Exception::~Exception() { }

char*
Exception::getMessage() {
  return message;
}
