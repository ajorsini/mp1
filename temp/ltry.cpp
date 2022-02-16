#include "stdincludes.h"
#include <list>
#include <random>


int main() {
  vector<int> v;
  int i, n;

  for(i=0; i<16; i++) v.push_back(i);
  for(n : v) cout << n << ", ";
  cout << newl;

  for(n : v)
    if(n % 2) v.erase(n);

  for(n : v) cout << n << ", ";
  cout << newl;

  return(0);
}
