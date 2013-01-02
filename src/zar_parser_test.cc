#include "zar_parser.hh"
#include "zar_param_table.hh"
#include <iostream>

int main(int argc, char** argv) {
  using namespace std;
  if (argc < 2) {
    cout << "please specify a filename. tack!" << endl;
    return 1;
  }

  ZarParser* zp = new ZarParser();
  ZarVar* time = new ZarVar();

  ZarParamTable* zt = new ZarParamTable();
  zt->addParam("time", time);

  zp->parseScene(argv[1], zt);

  cout << (*zt) << endl;

  delete zp;
  return 0;
}
