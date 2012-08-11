#ifndef _IRC_H_
#define _IRC_H_

#include "buffer.h"

typedef struct {
  int sock;
  int first_nick_done;
  int first_user_done;
} irc;

void irc_wait_connect(irc *);
void irc_send(irc *, buffer *, buffer *);
void irc_recv(irc *, buffer *);

#endif /* _IRC_H_ */
