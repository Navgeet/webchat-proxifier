#ifndef _EVENT_H_
#define _EVENT_H_

#include "net.h"
#include "irc.h"

#define NET_EVENT_1  0x01
#define NET_EVENT_2  0x02
#define IRC_EVENT    0x04

int wait_something(net *, irc *);

#endif /* _EVENT_H_ */
