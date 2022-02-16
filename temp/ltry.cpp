#include "stdincludes.h"
#include <list>
#include <random>


int main() {
  vector<int> v;
  vector<int>::iterator it;
  int i;

  for(i=0; i<16; i++) v.push_back(i+1);

  for(it=v.begin() ; it != v.end(); it++)
    cout << *it << ", ";
  cout << endl;

  for(it=v.begin() ; it != v.end(); )
    if(*it % 2)
      it = v.erase(it);
    else
      it++;

  for(it=v.begin() ; it != v.end(); it++)
    cout << *it << ", ";
  cout << endl;

  return(0);
}
