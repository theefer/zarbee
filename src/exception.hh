/*
 * ZarBee - exception.hh
 *
 * Author: seb
 * Created on: ven fév 25 18:01:46 CET 2005 *
 */

#ifndef _exception_hh_
#define _exception_hh_


class Exception {
protected:
  char* message;

public:
  Exception(char*);
  ~Exception();

  char* getMessage();
};

#endif // _exception_hh_
