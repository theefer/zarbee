#include "zar_nodes.hh"
#include <iostream>

int main() {
  using namespace std;

  ZarConst* two  = new ZarConst(2.0);
  ZarVar* four = new ZarVar();
  ZarConst* x    = new ZarConst(M_PI / 6.0);
  ZarSin*   sin  = new ZarSin(x);
  ZarAdd*   sum  = new ZarAdd(two, four);
  ZarMult*  mult = new ZarMult(sum, sin);

  four->setValue(4);

  ZarNode* top  = mult;
  top->initialize();
  top->updateCache(true);

  cout << (*top);
  cout << "top=" << top->getValue() << endl;

  cout << endl;


  four->setValue(8);
  top->updateCache();

  cout << (*top);
  cout << "top=" << top->getValue() << endl;

  delete top;
}
