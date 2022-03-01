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
  time_t gettstamp() { return this->tstamp; };
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
  vector<nodeEntry *> pingList, gossipList, newsList;
  size_t recsz;
  char msgBff[MAXMSGSZ];
public:
  Operation() {};
  Operation(char *addr, long hb, long myhb, Statuses status);
  ~Operation();
  void destroyPeersList(nodeEntry *n);
  void initPingList();
  char *getHeader(MsgTypes t, char *iAddr=NULL);
  int updtGossipLst(int n);
  char *addPayload(char **b);
  void updatePeersList(nodeEntry *n);
  void showPeersList();
  void showGossipList();
  int encode(MsgTypes t, char *h);
  void decode();
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
