#include "event.h"
#include "err.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

#define RM(x) \
  do { \
    CURLMcode ret = x; \
    if (ret) { \
      printf("%s:%d: curl error: %s\n", __FILE__, __LINE__, \
             curl_multi_strerror(ret)); \
      exit(1); \
    } \
  } while (0)

int wait_something(net *n, irc *irc)
{
  int ret = 0;
  long t;
  struct timeval tv;
  struct timeval *_tv;
  fd_set r, w, e;
  int maxfd;
  int still_running, nq;
  CURLMsg *msg;

  if (n->busy[0] && n->busy[1]) irc = 0;

  DBG("wait...");

  FD_ZERO(&r);
  FD_ZERO(&w);
  FD_ZERO(&e);
  RM(curl_multi_fdset(n->multi, &r, &w, &e, &maxfd));
  if (irc) FD_SET(irc->sock, &r);

  RM(curl_multi_timeout(n->multi, &t));
  if (t != -1) {
    tv.tv_sec = t / 1000;
    tv.tv_usec = (t % 1000) * 1000;
    _tv = &tv;
  } else
    _tv = 0;

  if (irc && maxfd < irc->sock) maxfd = irc->sock;

  if (select(maxfd+1, &r, &w, &e, _tv) == -1) ERR("select");

  if (irc && FD_ISSET(irc->sock, &r)) ret |= IRC_EVENT;

  RM(curl_multi_perform(n->multi, &still_running));

  while ((msg = curl_multi_info_read(n->multi, &nq))) {
    if (msg->msg != CURLMSG_DONE) continue;
    if (msg->easy_handle == n->curl[0]) {
      curl_multi_remove_handle(n->multi, n->curl[0]);
      n->busy[0] = 0;
      ret |= NET_EVENT_1;
    }
    if (msg->easy_handle == n->curl[1]) {
      curl_multi_remove_handle(n->multi, n->curl[1]);
      n->busy[1] = 0;
      ret |= NET_EVENT_2;
    }
  }

  DBG("...done (%d)", ret);

  return ret;
}
