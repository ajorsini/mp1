#include "opnnode.h"

Operation::Operation(char *addr, long hb, long myhb, Statuses status) {
  me = new nodeEntry(addr, hb, myhb, status);
  recsz = sizeof(addr)+sizeof(hb)+sizeof(status);
  peersList = first = last = NULL;
}

void Operation::destroyPeersList(nodeEntry *n) {
  nodeEntry *x;
  while((x = n->getNext())) {
    delete n;
    n = x;
  }
}

Operation::~Operation() {
  destroyPeersList(first);
  delete me;
  pingList.clear();
  gossipList.clear();
}

void Operation::initPingList() {
  nodeEntry *n = this->first;
  random_device rd;
  pingList.clear();
  while(n) {
    pingList.push_back(n);
    n = n->getNext();
  }
  mt19937 generator(rd());
  shuffle(pingList.begin(), pingList.end(), generator);
}

char *Operation::getHeader(MsgTypes t, char *iAddr) {
  char *b = msgBff;
  memcpy(b, &t, sizeof(t)); b+=sizeof(t);
  me->encode(&b);
  if(iAddr) {
    memcpy(b, iAddr, ADDRSZ);
    b+=ADDRSZ;
  }
  return b;
}

int Operation::updtGossipLst(int n) {
  nodeEntry *p, *b;
  vector<nodeEntry *>::iterator it;
  time_t now;
  int i;

  time(&now);
  for(it=gossipList.begin() ; it != gossipList.end(); )
    if(difftime(now, (*it)->gettstamp()) > TGOSSIPENTRY) it = gossipList.erase(it);
    else it++;

  p = b = peersList;
  if((i = gossipList.size()) > n) {
    gossipList.erase(gossipList.begin()+n, gossipList.end());
    peersList = gossipList.back();
    i = n;
  } else {
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
  }
cout << "GossipList Size: " << gossipList.size() << endl;
cout << "n: " << n << endl;
cout << "i: " << i << endl;

  return i;
}

char *Operation::addPayload(char **b) {
  int n = updtGossipLst(((MAXMSGSZ - sizeof(n) - (*b - msgBff)) / recsz));
int i;
  vector<nodeEntry *>::iterator it;
cout << "bavail=" << (MAXMSGSZ - (*b - msgBff)) << ", ---payload --------\n";
cout << "recsz=" << recsz << ", ---payload --------\n";
cout << "n=" << n << ", ---payload --------\n";
  memcpy(*b, &n, sizeof(n)); *b += sizeof(n);
i = 0;
  for(it=gossipList.begin() ; it != gossipList.end(); it++) {
    i++;
    (*it)->encode(b);
    (*it)->show();
  }
cout << "i: " << i << endl;
  return *b;
}

void Operation::updatePeersList(nodeEntry *n) {
  nodeEntry *x = NULL;
  bool gssp = FALSE;
  if(peersList) x = peersList->find(n->getaddr());
  if((gssp = !x)) {
    x = new nodeEntry();
    *x = *n;
    x->setmyhb(me->getmyhb());
    if(peersList) peersList->add(x);
    else peersList = x;
    if(x->isFirst()) first = x;
    if(x->isLast()) last = x;
  } else {
    if((gssp = x->sethb(n->gethb())) || (x->setstatus(n->getstatus())))
      x->setmyhb(me->getmyhb());
  }
  if(gssp) gossipList.insert(gossipList.begin(), x);
}

void Operation::showPeersList() {
  nodeEntry *p = first;
  int i = 0;
  cout << "\nPeers List ------------------------------------\n";
  cout << "me: ";
  me->show();
  while(p) {
    cout << i++ << ": ";
    p->show();
    p = p->getNext();
  }
}

void Operation::showGossipList() {
  vector<nodeEntry *>::iterator it;
  int i=0;
  cout << "\nGossip List ------------------------------------\n";
  for(it=gossipList.begin() ; it != gossipList.end(); it++) {
    cout << i++ << ": ";
    (*it)->show();
  }
}

int Operation::encode(MsgTypes t, char *h) {
  char *b;
  b = getHeader(t, h);
  b = addPayload(&b);
  return (int ) (b - msgBff);
}

void Operation::decode() {
  char *b = msgBff, iAddr[ADDRSZ];
  bool ind=FALSE, pyld=FALSE;
  int nr=0, i;
  MsgTypes t;
  nodeEntry n;
  memcpy(&t, b, sizeof(t)); b+=sizeof(t);
  switch(t) {
    case JOINREQ:
      cout << "JOINREQ, ";
      pyld = TRUE;
      break;
    case JOINREP:
      cout << "JOINREP, ";
      break;
    case PING:
      cout << "PING, ";
      break;
    case PONG:
      cout << "PONG, ";
      break;
    case IPING:
      cout << "IPING, ";
      ind = TRUE;
      break;
    case IPONG:
      cout << "IPONG, ";
      ind = TRUE;
      break;
    case PGYPING:
      cout << "PGYPING, ";
      pyld = TRUE;
      break;
    case PGYPONG:
      cout << "PGYPONG, ";
      pyld = TRUE;
      break;
    case IPGYPING:
      cout << "IPGYPING, ";
      ind = TRUE;
      pyld = TRUE;
      break;
    case IPGYPONG:
      cout << "IPGYPONG, ";
      ind = TRUE;
      pyld = TRUE;
      break;
    default:
      cout << "Wrong MSG Code";
      break;
  }
  n.decode(&b);
  n.show();
  if(ind) {
    memcpy(iAddr, b, ADDRSZ); b+=ADDRSZ;
    printf("Indirect -> %u.%u.%u.%u:%u\n",
                               (unsigned char) iAddr[0],
                               (unsigned char) iAddr[1],
                               (unsigned char) iAddr[2],
                               (unsigned char) iAddr[3],
                               (unsigned short) iAddr[4]);
  }
  if(pyld) {
    memcpy(&nr, b, sizeof(nr)); b+=sizeof(nr);
    for(i=0; i<nr; i++) {
      n.decode(&b);
      n.show();
    }
  }
}
