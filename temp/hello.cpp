#include "stdincludes.h"
#include <random>

#define FALSE 0
#define TRUE 1
#define ADDRSZ 6
#define MAXMSGSZ 512
#define TFAIL 8
#define TCLEANUP 16
#define TGOSSIPENTRY 3

/**
 * Message Types
 */
enum MsgTypes {
    JOINREQ,
    JOINREP,
    PING,
    PONG,
    IPING,
    IPONG,
    PGYPING,
    PGYPONG,
    IPGYPING,
    IPGYPONG
};

typedef enum Statuses {
  ALIVE,
  SUSPECT,
  DEAD
} Statuses;

class nodeEntry {
private:
  char addr[ADDRSZ];
  long hb, myhb;
  time_t tstamp;
  Statuses status;
  nodeEntry *prev, *next;
public:
  nodeEntry() { setmyhb((long) 0); };
  nodeEntry(char *addr, long hb, long myhb, Statuses status);
  nodeEntry(char **b, long myhb=0) { decode(b); setmyhb(myhb); };
  ~nodeEntry() {};
  nodeEntry *add(nodeEntry *i);
  nodeEntry *find(char *addr);
  nodeEntry *unlink();
  nodeEntry *getNext() { return this->next; };
  nodeEntry *getPrev() { return this->prev; };
  char *getaddr() { return this->addr; };
  long gethb() { return this->hb; };
  long getmyhb() { return this->myhb; };
  Statuses getstatus() { return this->status; };
  void setaddr(char *addr) {
    memcpy(this->addr, addr, sizeof(this->addr));
  }
  bool sethb(long hb) {
    bool r = (this->hb < hb);
    if(r) this->hb = hb;
    return r;
  }
  bool setmyhb(long myhb) {
    this->myhb = myhb;
    time(&this->tstamp);
    return TRUE;
  }
  long myhbinc() {
    this->myhb++;
    time(&this->tstamp);
    return this->myhb;
  }
  bool setstatus(Statuses status) {
    bool r = (this->status != status);
    if(r) this->status = status;
    return r;
  }
  char *encode(char **b);
  char *decode(char **b);
  void show();
  bool isFirst() { return !(this->prev); }
  bool isLast() { return !(this->next); }
  nodeEntry& operator =(const nodeEntry &anotherNodeEntry);
  bool operator ==(const nodeEntry &anotherNodeEntry);
  bool operator <(const nodeEntry &anotherNodeEntry);
  bool operator >(const nodeEntry &anotherNodeEntry);
};

class Operation {
private:
  nodeEntry *peersList, *me, *first, *last;
  vector<nodeEntry *> pingList, gossipList;
  size_t recsz;
  char msgBff[MAXMSGSZ];
public:
  Operation() {};
  Operation(char *addr, long hb=0, long myhb=0, Statuses status=ALIVE);
  ~Operation() {};
  void destroyPeersList(nodeEntry *n);
  void destroyPeersList(nodeEntry *n);
  void initPingList();
  char *getHeader(MsgTypes t, char *iAddr=NULL);
  int updtGossipLst(int n);
  char *addPayload(char *b);
  void updatePeersList(nodeEntry *n);
  void Operations::showPeersList():
  void Operations::showGossipList();
/*
  joinreq();
  joinrep();
  ping();
  pong();
  iping();
  ipong();
  pgyping();
  pgypong();
  ipgyping();
  ipgypong(); */
};

Operation::Operation(char *addr, long hb=0, long myhb=0, Statuses status=ALIVE) {
  me = new nodeEntry(addr, hb, myhb, status);
  recsz = sizeof(addr)+sizeof(hb)+sizeof(status);
  peersList = first = last = NULL;
}

void Operation::destroyPeersList(nodeEntry *n) {
  nodeEntry *x;
  while((x = n->getPrev())) n = x;
  while((x = n->getNext())) {
    delete n;
    n = x;
  }
  delete n;
}

Operation::~Operation() {
  destroyPeersList(peersList);
  delete me;
  pingList.clear();
  gossipList.clear();
};

void Operation::initPingList() {
  nodeEntry *n = this->first;
  random_device rd;
  pingList.clear();
  while(n) {
    pingList.push_back(n);
    n = n->next;
  }
  mt19937 generator(rd());
  shuffle(pingList.begin(), pingList.end(), generator);
}

char *Operation::getHeader(MsgTypes t, char *iAddr=NULL) {
  char *b = msgBff;
  memcpy(b, &t, sizeof(t)); b+=sizeof(t);
  me->encode(&b);
  if(iAddr) memcpy(b, iAddr, sizeof(iAddr)); b+=sizeof(iAddr);
  return b;
}

int Operation::updtGossipLst(int n) {
  nodeEntry *p, *b;
  vector<nodeEntry *>::iterator it;
  int i;

  for(it=gossipList.begin() ; it != gossipList.end(); )
    if(difftime(timer, *it->tstamp) > TGOSSIPENTRY) it = gossipList.erase(it);
    else it++;

  p = b = peersList;
  i = gossipList.size();
  while(i<n) {
    if(p->isLast()) p = first;  // reached the end
    else p = p->getNext();
    if(p == b) break;           // whole round
    if(find(gossipList.begin(), gossipList.end(), p)==gossipList.end()) {
      gossipList.push_back(p);
      i++;
    }
  }
  peersList = p;
  if(i>n) i = n;

  return i;
}

char *Operation::addPayload(char *b) {
  int n = updtGossipLst(((b - msgBff) / recsz));
  vector<nodeEntry *>::iterator it;

  memcpy(b, n, sizeof(n)); b += sizeof(n);
  for(it=gossipList.begin() ; it != gossipList.end(); it++)
    it->encode(&b);
  return b;
}

void Operation::updatePeersList(nodeEntry *n) {
  nodeEntry *x = NULL;
  bool gssp = FALSE;
  if(peersList) x = peersList->find(n->getaddr());
  if(gssp = (!x)) {
    x = n;
    x->setmyhb(me->getmyhb());
    peersList->add(x);
    if(x->isFirst()) first = x;
    if(x->isLast()) last = x;
  } else {
    if(gssp = (x->sethb(n->gethb()) || x->setstatus(n->getstatus())))
      x->setmyhb(me->getmyhb()));
  }
  if(gssp) gossipList.insert(gossipList.begin(), x);
}

void Operations::showPeersList() {
  nodeEntry *p = first;
  int i = 0;
  cout << "\nPeers List ------------------------------------\n"
  cout << "me: ";
  me->show();
  while(p) {
    cout << i++ << ": ";
    p->show();
    p = p->getNext();
  }
}

void Operations::showGossipList() {
  vector<nodeEntry *>::iterator it;
  int i=0;
  cout << "\nGossip List ------------------------------------\n"
  for(it=gossipList.begin() ; it != gossipList.end(); it++) {
    cout << i++ << ": ";
    it->show();
  }
}

/* ------------------- nodeEntry ----------------------- */

nodeEntry::nodeEntry(char *addr, long hb, long myhb, Statuses status) {
  memcpy(this->addr, addr, sizeof(this->addr));
  this->hb = hb;
  setmyhb(myhb);
  this->status = status;
  this->prev = this->next = NULL;
}

nodeEntry *nodeEntry::add(nodeEntry *i) {
  if(*i == *this) { cout << "dup!\n"; return this; } // consider update
  if(!this->prev && *i < *this) {  // first
    i->next = this;
    i->prev = NULL;
    this->prev = i;
    return i;
  }
  if(!this->next && *i > *this) {  // last
    i->prev = this;
    i->next = NULL;
    this->next = i;
    return i;
  }
  if(*i < *this) {
    if(*i > *this->prev) {
      i->next = this;
      i->prev = this->prev;
      this->prev->next = i;
      this->prev = i;
      return i;
    } else {
      return this->prev->add(i);
    }
  } else if(*i > *this) {
    if(*i < *this->next) {
      i->prev = this;
      i->next = this->next;
      this->next->prev = i;
      this->next = i;
      return i;
    } else {
      return this->next->add(i);
    }
  }
  cout << "skipped" << endl;
  return this;
}

nodeEntry *nodeEntry::find(char *addr) {
  int i = memcmp(this->addr, addr, sizeof(this->addr));
  if(!i) return this;
  if(i > 0) {
    if(this->prev) return this->prev->find(addr);
    else return NULL;
  }
  if(i < 0) {
    if(this->next) return this->next->find(addr);
    else return NULL;
  }
  return NULL;
}

nodeEntry *nodeEntry::unlink() {
  nodeEntry *r = NULL;
  if(this->prev) { this->prev->next = this->next; r = this->next; }
  if(this->next) { this->next->prev = this->prev; r = this->prev; }
  return r;
}

char *nodeEntry::encode(char **b) {
  memcpy(*b, this->addr, sizeof(this->addr));
  *b += sizeof(this->addr);
  memcpy(*b, &this->hb, sizeof(this->hb));
  *b += sizeof(this->hb);
  memcpy(*b, &this->status, sizeof(this->status));
  *b += sizeof(this->status);
  return *b;
}

char *nodeEntry::decode(char **b) {
  memcpy(this->addr, *b, sizeof(this->addr));
  *b += sizeof(this->addr);
  memcpy(&this->hb, *b, sizeof(this->hb));
  *b += sizeof(this->hb);
  memcpy(&this->status, *b, sizeof(this->status));
  *b += sizeof(this->status);
  return *b;
}

void nodeEntry::show() {
  short port;
  time_t timer;
  memcpy(&port, &addr[4], sizeof(port));
  printf("%u.%u.%u.%u:%u, hb=%ld, myhb=%ld, st=%d, elapt=%.f\n",
                             (unsigned char) addr[0],
                             (unsigned char) addr[1],
                             (unsigned char) addr[2],
                             (unsigned char) addr[3],
                             (unsigned short) port,
                             difftime(timer, tstamp),
                             hb, myhb, status);
}

nodeEntry& nodeEntry::operator =(const nodeEntry &anotherNodeEntry) {
  memcpy(&this->addr, &anotherNodeEntry.addr, sizeof(this->addr));
  this->hb = anotherNodeEntry.hb;
  this->setmyhb(anotherNodeEntry.myhb);
  this->status = anotherNodeEntry.status;
  this->tstamp = anotherNodeEntry.tstamp;
  this->prev = anotherNodeEntry.prev;
  this->next = anotherNodeEntry.next;
  return *this;
}

bool nodeEntry::operator ==(const nodeEntry& anotherNodeEntry) {
	return !memcmp(this->addr, anotherNodeEntry.addr, sizeof(this->addr));
}

bool nodeEntry::operator <(const nodeEntry& anotherNodeEntry) {
	return (memcmp(this->addr, anotherNodeEntry.addr, sizeof(this->addr)) < 0);
}

bool nodeEntry::operator >(const nodeEntry& anotherNodeEntry) {
	return (memcmp(this->addr, anotherNodeEntry.addr, sizeof(this->addr)) > 0);
}

/* -------------------------------------------------------------- */
/* Testing */
/* -------------------------------------------------------------- */

char *getRndAddr(char *r) {
  int i;

  for(i=0; i<6; i++) r[i] = rand() % 255;
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
  char a[6], b[6];
  short port;
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
  memcpy(&port, &b[4], sizeof(port));
  printf("%u.%u.%u.%u:%u\n", (unsigned char) b[0],
                             (unsigned char) b[1],
                             (unsigned char) b[2],
                             (unsigned char) b[3],
                             (unsigned short) port);
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
  char a[6];
  Operation *o;
  nodeEntry *n = new nodeEntry();

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
  delete o;
  delete n;
}


int main() {

  test04();

  return 0;
}
