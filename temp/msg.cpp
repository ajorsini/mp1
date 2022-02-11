#include "stdincludes.h"
#include <random>

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

/**
 * STRUCT NAME: MessageHdr
 *
 * DESCRIPTION: Header and content of a message
 */
typedef struct myMessage {
	enum MsgTypes msgType;
} myMessage;


/* --------------------- my code ------------------------ */
#define FALSE 0
#define TRUE 1
#define MSGSZ 10

typedef enum Statuses {
  ALIVE,
  SUSPECT,
  DEAD
} Statuses;

typedef struct nodeEntry {
  int node;
  long hb, myhb;
  Statuses status;
  nodeEntry *prev, *next;
} nodeEntry;

class peersTable {
private:
  nodeEntry *first, *curr, *p, decoded[MSGSZ];
  vector<nodeEntry *> t;
  int msgsz, nDecoded;
  char *msg;

public:
  peersTable(); // Constructor
  ~peersTable(); // Destructor
  void addNode(int node, long hb, long myhb, Statuses status);
  void delNode(int node);
  nodeEntry *findNode(int id, bool exact);
  void show();
  void fillt();
  void shufflet();
  char *msgEncode();
  nodeEntry *msgDecode(char *msg);
  void showDecoded();
};

peersTable::peersTable() {
  int i;
  this->first = NULL;
  this->curr = NULL;
  msgsz = sizeof(int) + MSGSZ * (sizeof(p->node) + sizeof(p->hb) + sizeof(p->myhb) + sizeof(p->status)) + 1;
  msg = (char *) malloc(msgsz);
  for(i=0; i<MSGSZ-1; i++)
    decoded[i].next = &decoded[i+1];
  for(i=1; i<MSGSZ; i++)
      decoded[i].prev = &decoded[i-1];
  decoded[0].prev = NULL;
  decoded[MSGSZ-1].next = NULL;
  nDecoded = 0;
}

peersTable::~peersTable() {
  curr = first;
  while(curr) {
      first = curr->next;
      free(curr);
      curr = first;
  }
  t.clear();
  free(msg);
}

nodeEntry *peersTable::findNode(int node, bool exact) {
  curr = first;
  while(curr) {
    if(curr->node < node && curr->next)
      curr = curr->next;
    else
      break;
  }
  if(exact && curr->node != node)
    return NULL;
  else
    return curr;
}

void peersTable::addNode(int node, long hb, long myhb, Statuses status) {
  nodeEntry *n = (nodeEntry *) malloc(sizeof(nodeEntry));
  n->node = node;
  n->hb = hb;
  n->myhb = myhb;
  n->status = status;
  if(!first) {
    first = n;
    n->prev = n->next = NULL;
  } else {
    findNode(node, FALSE);
    if(curr->node > node) {
      if(curr == first) {
        n->prev = NULL;
        n->next = first;
        n->next->prev = n;
        first = curr = n;
      } else {
        curr = curr->prev;
        n->prev = curr;
        n->next = curr->next;
        curr->next = n;
        n->next->prev = n;
        curr = n;
      }
    } else {
      n->prev = curr;
      n->next = curr->next;
      if(n->next)
        n->next->prev = n;
      curr->next = n;
      curr = n;
    }
  }
}

void peersTable::delNode(int node) {
  nodeEntry *n = findNode(node, TRUE), *j, *k;
  if(n) {
    if(n == first) {
      curr = first = n->next;
      first->prev = NULL;
    } else {
      j = n->prev;
      k = n->next;
      if(j) j->next = k;
      if(k) k->prev = j;
      curr = j;
    }
    if(n == p) p = curr;
    free(n);
  }
}

void peersTable::show() {
  for (nodeEntry *n : t)
    printf("n=%d, hb=%ld, myhb=%ld, st=%d\n", n->node, n->hb, n->myhb, n->status);
}

void peersTable::fillt() {
  nodeEntry *n = first;
  while(n) {
    t.push_back(n);
    n = n->next;
  }
}

void peersTable::shufflet() {
  random_device rd;
  mt19937 generator(rd());
  t.clear();
  fillt();
  shuffle(t.begin(), t.end(), generator);
  p = first;
}

char *peersTable::msgEncode() {
  int l = t.size();
  char *m = msg;
  if(l > MSGSZ) l = MSGSZ;
  memcpy(m, &l, sizeof(l));
  m += sizeof(l);
  for(int i=0; i<l; i++) {
    memcpy(m, &p->node, sizeof(p->node));
    m += sizeof(p->node);
    memcpy(m, &p->hb, sizeof(p->hb));
    m += sizeof(p->hb);
    memcpy(m, &p->myhb, sizeof(p->myhb));
    m += sizeof(p->myhb);
    memcpy(m, &p->status, sizeof(p->status));
    m += sizeof(p->status);
    if(p->next) p = p->next;
    else p = first;
  }
  return(msg);
}

nodeEntry *peersTable::msgDecode(char *msg) {
  int i;
  memcpy(&nDecoded, msg, sizeof(nDecoded));
  msg += sizeof(nDecoded);
  for(i=0; i<nDecoded; i++) {
    memcpy(&decoded[i].node, msg, sizeof(decoded[i].node));
    msg += sizeof(decoded[i].node);
    memcpy(&decoded[i].hb, msg, sizeof(decoded[i].hb));
    msg += sizeof(decoded[i].hb);
    memcpy(&decoded[i].myhb, msg, sizeof(decoded[i].myhb));
    msg += sizeof(decoded[i].myhb);
    memcpy(&decoded[i].status, msg, sizeof(decoded[i].status));
    msg += sizeof(decoded[i].status);
  }
  return(decoded);
}

void peersTable::showDecoded() {
  for(int i=0; i<nDecoded; i++) {
    printf("n=%d, hb=%ld, myhb=%ld, st=%d\n", decoded[i].node, decoded[i].hb, decoded[i].myhb, decoded[i].status);
    if(!decoded[i].next)
      break;
  }
}

/*
msgEncode() {
  myMessage *msg

  size_t msgsize = sizeof(myMessage) + sizeof(joinaddr->addr) + sizeof(long) + 1;
  msg = (MessageHdr *) malloc(msgsize * sizeof(char));

  msg->msgType = JOINREQ;
  memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
  memcpy((char *)(msg+1) + 1 + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

  emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);

  free(msg)

}

msgDecode() {

}
*/

/*
header(enum msgTypes ) {
  switch()
}
*/

int main() {
  peersTable *x = new peersTable();
  char *c;
  MsgTypes kk = IPGYPING;

  x->addNode(8, 10, 1, ALIVE);
  x->addNode(5, 11, 2, SUSPECT);
  x->addNode(2, 12, 3, DEAD);
  x->addNode(7, 13, 4, ALIVE);
  x->addNode(21, 14, 5, SUSPECT);
  x->addNode(12, 15, 6, DEAD);
  x->addNode(33, 16, 7, ALIVE);
  x->addNode(1, 17, 8, SUSPECT);
  x->addNode(19, 18, 9, DEAD);
  x->addNode(11, 19, 10, ALIVE);
  x->addNode(3, 20, 11, SUSPECT);
  x->fillt();
  cout << "-----------------\n";
  x->show();
  cout << "-----------------\n";
  x->shufflet();
  x->show();
  cout << "-----------------\n";
  x->shufflet();
  x->show();
  cout << "--------Encode/Decode---------\n";
  c = x->msgEncode();
  x->msgDecode(c);
  x->showDecoded();
  cout << "--------Encode/Decode---------\n";
  c = x->msgEncode();
  x->msgDecode(c);
  x->showDecoded();
  cout << "--------Encode/Decode---------\n";
  x->delNode(7);
  x->delNode(1);
  x->delNode(33);
  x->shufflet();
  x->show();
  cout << "--------Encode/Decode---------\n";
  c = x->msgEncode();
  cout << c << "\n";
  cout << "--------Encode/Decode---------\n";
  x->msgDecode(c);
  x->showDecoded();
  cout << "--------Encode/Decode---------\n";
  delete x;


  printf("kk=%d\n", kk);
  switch(kk) {
    case JOINREQ: cout << "JOINREQ\n"; break;
    case JOINREP: cout << "JOINREP\n"; break;
    case PING: cout << "PING\n"; break;
    case PONG: cout << "PONG\n"; break;
    case IPING: cout << "IPING\n"; break;
    case IPONG: cout << "IPONG\n"; break;
    case PGYPING: cout << "PGYPING\n"; break;
    case PGYPONG: cout << "PGYPONG\n"; break;
    case IPGYPING: cout << "IPGYPING\n"; break;
    case IPGYPONG: cout << "IPGYPONG\n"; break;
    default: cout << "error\n"; break;
  }


  return(0);
}
