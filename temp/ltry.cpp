#include "stdincludes.h"
#include <list>
#include <random>

list<int>::iterator listFind(list<int> *l, int n) {
  list<int>::iterator t = l->begin(), b = l->end(), i;
  unsigned int k;
  k = l->size() / 2;
  while (k > 0) {
    if (*i > n) t = t+k;
    else if(*i < n) b = b-k;
    else break;
    k = k/2;
  }
  return i;
}


int main() {
  list<int> myIntLst;
  list<int>::iterator it;
  default_random_engine generator;
  uniform_int_distribution<int> distribution(1, 100);

  myIntLst.resize(30);
  for(it=myIntLst.begin(); it != myIntLst.end(); it++)
    *it = distribution(generator);
  for(it=myIntLst.begin(); it != myIntLst.end(); ++it)
    cout << ' ' << *it;
  cout << '\n';
  myIntLst.sort();
  for(it=myIntLst.begin(); it != myIntLst.end(); ++it)
    cout << ' ' << *it;
  cout << '\n';
  it = listFind(&myIntLst, 30);
  cout << ' ' << *it;
  cout << '\n';


  return(0);
}
