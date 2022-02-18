#include "opnnode.h"

char *getRndAddr(char *r) {
  int i;

  for(i=0; i<ADDRSZ; i++) r[i] = rand() % 255;
  return r;
}

void showList(nodeEntry *n) {
  nodeEntry *x;
  int i = 0;
  while((x = n->getPrev())) n = x;
  while((x = n->getNext())) {
    cout << i++ << ", ";
    n->show();
    n = x;
  }
  cout << i++ << ", ";
  n->show();
}

nodeEntry *test01() {
  char a[ADDRSZ], b[ADDRSZ];
  int i;
  nodeEntry *x, *n = new nodeEntry(getRndAddr(a), (long) 1, (long) 1, ALIVE);

  srand(time(NULL));

  for (i=0; i<50; i++) {
    x = new nodeEntry(getRndAddr(a), (long) (i + rand() % 50), (long) 1, (Statuses) (rand() % 3));
    if(i == 24) memcpy(b, a, sizeof(a));
    n->add(x);
  }

  showList(n);

  cout << endl;
  printf("%u.%u.%u.%u:%u\n", (unsigned char) b[0],
                             (unsigned char) b[1],
                             (unsigned char) b[2],
                             (unsigned char) b[3],
                             (unsigned short) b[4]);
  x = n->find(b);
  x->show();
  n->show();

  cout << endl;
  x->show();
  n = x->unlink(); delete x; x = n;
  x->show();
  cout << endl;
  showList(x);

  return x;
}

void test02(nodeEntry *x) {
  char b[64], *p;
  nodeEntry *n = new nodeEntry();
  p=b;
  cout << "-- test 2 --\n";
  x->encode(&p);
  cout << p-b << endl;
  p=b;
  n->decode(&p);
  x->show();
  n->show();
  delete n;
}

void test03(nodeEntry *x) {
  char b[64], *p;
  nodeEntry *n;
  p=b;
  cout << "-- test 3 --\n";
  x->encode(&p);
  cout << p-b << endl;
  p=b;
  n = new nodeEntry(&p, 45);
  x->show();
  n->show();
  delete n;
}

void test04() {
  char a[ADDRSZ], iAddr[ADDRSZ];
  Operation *o;
  nodeEntry *n = new nodeEntry();
  int i;

  srand(time(NULL));
  o = new Operation(getRndAddr(a), 1, 1, ALIVE);

  for (i=0; i<50; i++) {
    n->setaddr(getRndAddr(a));
    n->sethb((long) (i + rand() % 50));
    n->setmyhb((long) 1);
    n->setstatus((Statuses) (rand() % 3));
    o->updatePeersList(n);
  }
  o->showPeersList();
  o->showGossipList();

  cout << "Encode / Decode ---------------------------\n";
  strncpy(iAddr, "dummy-", 6);
  o->encode(IPGYPING, iAddr);
  o->decode();

  cout << "\n\nPeers after ---------------------------\n";

  o->showPeersList();
cout << "sali 1\n";
  delete o;
cout << "sali 2\n";
  delete n;
cout << "sali 3\n";
}


int main() {

  test04();

  return 0;
}
