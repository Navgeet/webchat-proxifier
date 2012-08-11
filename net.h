#ifndef _NET_H_
#define _NET_H_

#include "buffer.h"

typedef struct {
  void *multi;
  void *curl[2];
  int busy[2];
  char *session;
} net;

void init_net(net *);
void net_get(net *, buffer *, const char *);
void net_post(net *, int c, buffer *, const char *, const char *);
void net_connect(net *, buffer *, int);

#endif /* _NET_H_ */
