/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include "MP1Node.h"

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node(Member *member, Params *params, EmulNet *emul, Log *log, Address *address) {
	for( int i = 0; i < 6; i++ ) {
		NULLADDR[i] = 0;
	}
	this->memberNode = member;
	this->emulNet = emul;
	this->log = log;
	this->par = params;
	this->memberNode->addr = *address;
}

/**
 * Destructor of the MP1Node class
 */
MP1Node::~MP1Node() {}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int MP1Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue
 */
int MP1Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void MP1Node::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MP1Node::initThisNode(Address *joinaddr) {
	/*
	 * This function is partially implemented and may require changes
	 */
	int id = *(int*)(&memberNode->addr.addr);
	int port = *(short*)(&memberNode->addr.addr[4]);

	memberNode->bFailed = false;
	memberNode->inited = true;
	memberNode->inGroup = false;
    // node is up!
	memberNode->nnb = 0;
	memberNode->heartbeat = 0;
	memberNode->pingCounter = TFAIL;
	memberNode->timeOutCounter = -1;
    initMemberListTable(memberNode);

    return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MP1Node::introduceSelfToGroup(Address *joinaddr) {
	MessageHdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if ( 0 == memcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr), sizeof(memberNode->addr.addr))) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Starting up group...");
#endif
        memberNode->inGroup = true;
    }
    else {
        size_t msgsize = sizeof(MessageHdr) + sizeof(joinaddr->addr) + sizeof(long) + 1;
        msg = (MessageHdr *) malloc(msgsize * sizeof(char));

        // create JOINREQ message: format of data is {struct Address myaddr}
        msg->msgType = JOINREQ;
        memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg+1) + 1 + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

#ifdef DEBUGLOG
        sprintf(s, "Trying to join...");
        log->LOG(&memberNode->addr, s);
#endif

        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);

        free(msg);
    }

    return 1;

}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int MP1Node::finishUpThisNode(){
   /*
    * Your code goes here
    */
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MP1Node::nodeLoop() {
    if (memberNode->bFailed) {
    	return;
    }

    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if( !memberNode->inGroup ) {
    	return;
    }

    // ...then jump in and share your responsibilites!
    nodeLoopOps();

    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MP1Node::checkMessages() {
    void *ptr;
    int size;

    // Pop waiting messages from memberNode's mp1q
    while ( !memberNode->mp1q.empty() ) {
    	ptr = memberNode->mp1q.front().elt;
    	size = memberNode->mp1q.front().size;
    	memberNode->mp1q.pop();
    	recvCallBack((void *)memberNode, (char *)ptr, size);
    }
    return;
}

/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MP1Node::recvCallBack(void *env, char *data, int size ) {
	/*
	 * Your code goes here
	 */
}

/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				Propagate your membership list
 */
void MP1Node::nodeLoopOps() {

	/*
	 * Your code goes here
	 */

    return;
}

/**
 * FUNCTION NAME: isNullAddress
 *
 * DESCRIPTION: Function checks if the address is NULL
 */
int MP1Node::isNullAddress(Address *addr) {
	return (memcmp(addr->addr, NULLADDR, 6) == 0 ? 1 : 0);
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MP1Node::getJoinAddress() {
    Address joinaddr;

    memset(&joinaddr, 0, sizeof(Address));
    *(int *)(&joinaddr.addr) = 1;
    *(short *)(&joinaddr.addr[4]) = 0;

    return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MP1Node::initMemberListTable(Member *memberNode) {
	memberNode->memberList.clear();
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
void MP1Node::printAddress(Address *addr)
{
    printf("%d.%d.%d.%d:%d \n",  addr->addr[0],addr->addr[1],addr->addr[2],
                                                       addr->addr[3], *(short*)&addr->addr[4]) ;
}



/* ------------------------ myClasses Code ----------------------------------------------------
	 --------------------------------------------------------------------------------------------
	 -------------------------------------------------------------------------------------------- */

/* ------------------------- node.cpp ------------------------------ */

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
    if(this->prev) {
      if(memcmp(this->prev->addr, addr, sizeof(this->addr)) < 0) return NULL;
      else return this->prev->find(addr);
    } else return NULL;
  }
  if(i < 0) {
    if(this->next) {
      if(memcmp(this->next->addr, addr, sizeof(this->addr)) > 0) return NULL;
      else return this->next->find(addr);
    } else return NULL;
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
  time_t now = time(NULL);
  printf("%u.%u.%u.%u:%u, hb=%ld, myhb=%ld, st=%d, elapt=%.00e\n",
                             (unsigned char) addr[0],
                             (unsigned char) addr[1],
                             (unsigned char) addr[2],
                             (unsigned char) addr[3],
                             (unsigned short) addr[4],
                             hb, myhb, status,
                             difftime(now, tstamp));
}

nodeEntry& nodeEntry::operator =(const nodeEntry &anotherNodeEntry) {
  memcpy(this->addr, anotherNodeEntry.addr, sizeof(this->addr));
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

/* ----------------------- end node.cpp ---------------------------- */

/* ------------------------- op.cpp ------------------------------ */

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

/* ----------------------- end op.cpp ---------------------------- */
