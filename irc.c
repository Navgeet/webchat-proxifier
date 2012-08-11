#include "irc.h"
#include "err.h"
#include "cJSON.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void irc_wait_connect(irc *irc)
{
  int sock;
  int s2;
  struct sockaddr_in addr;
  socklen_t size;
  int reuse;

  DBG("wait irc...");

  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == -1) ERR("socket");

  reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
    ERR("setsockopt (%s)", strerror(errno));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(6667);

  if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    ERR("bind");

  listen(sock, 5);

  size = sizeof(struct sockaddr_in);
  if ((s2 = accept(sock, (struct sockaddr *)&addr, &size)) == -1)
    ERR("accept");

  close(sock);

  irc->sock = s2;
  irc->first_nick_done = 0;
  irc->first_user_done = 0;

  DBG("...done");
}

static int is_message(cJSON *c)
{
  if (c->type != cJSON_Array) return 0;
  if (cJSON_GetArraySize(c) != 4) return 0;
  if (c->child->type != cJSON_String ||
      strcmp(c->child->valuestring, "c")) return 0;
  if (c->child->next->type != cJSON_String) return 0;
  if (c->child->next->next->type != cJSON_String) return 0;
  c = c->child->next->next->next;
  if (c->type != cJSON_Array) return 0;
  c = c->child;
  if (!c) return 0;
  while (c) {
    if (c->type != cJSON_String) return 0;
    c = c->next;
  }
  return 1;
}

static void decode(buffer *from, buffer *to)
{
  cJSON *c, *d;
  to->size = 0;

  c = cJSON_Parse(from->data); if (!c) goto err;
  if (c->type != cJSON_Array) goto err;
  if (!c->child) {
    buffer_add_string(to, ":ok NOTICE ok :got []\r\n");
    buffer_add_char(to, 0);
    goto end;
  }
  d = c->child;
  if (d->type == cJSON_True) {
    d = d->next;
    if (d && d->type == cJSON_True) {
      DBG("message decoded as [true, true], skipping");
      goto end;
    }
    if (!d || d->type != cJSON_String) goto err;
    buffer_add_string(to, ":ok NOTICE ok :");
    buffer_add_string(to, d->valuestring);
    buffer_add_string(to, "\r\n");
    buffer_add_char(to, 0);
    goto end;
  } else if (d->type == cJSON_False) {
    d = d->next; if (!d || d->type != cJSON_String) goto err;
    buffer_add_string(to, ":error NOTICE error :");
    buffer_add_string(to, d->valuestring);
    buffer_add_string(to, "\r\n");
    buffer_add_char(to, 0);
    goto end;
  }
  if (d->type != cJSON_Array) goto err;
  while (d) {
    cJSON *e = d->child;
    cJSON *f;
    if (!is_message(d)) {
      if (d->type == cJSON_Array &&
          d->child && d->child->type == cJSON_String &&
          !strcmp(d->child->valuestring, "disconnect")) ERR("disconnected");
      d = d->next; continue;
    }
    if (e->next->next->valuestring[0]) {
      buffer_add_string(to, ":");
      buffer_add_string(to, e->next->next->valuestring);
      buffer_add_char(to, ' ');
    }
    buffer_add_string(to, e->next->valuestring);
    f = e->next->next->next->child;
    while (f) {
      buffer_add_char(to, ' ');
      if (!f->next) buffer_add_char(to, ':');
      buffer_add_string(to, f->valuestring);
      f = f->next;
    }
    buffer_add_string(to, "\r\n");
    d = d->next;
  }
  buffer_add_char(to, 0);

end:
  cJSON_Delete(c);
  return;

err:
  to->size = 0;
  buffer_add_string(to, ":error NOTICE error :Bad message from freenode\r\n");
  buffer_add_char(to, 0);
  cJSON_Delete(c);
  DBG("bad message '%s'", from->data);
}

void irc_send(irc *irc, buffer *t, buffer *m)
{
  int l;
  char *p;
  decode(m, t);
  if (t->size == 0) return;

  DBG("sending to irc: '%s' (%d)\n", t->data, t->size-1);

  p = t->data;
  l = t->size-1;
  while (l) {
    int r = write(irc->sock, p, l);
    DBG("write returns %d", r);
    if (r == -1) ERR("irc socket error (%s)", strerror(errno));
    l -= r;
    p += r;
  }
}

void irc_recv(irc *irc, buffer *t)
{
  int n;
  char c;
  char *b = t->data + t->size;

  /* bad, use a buffer, too much syscalls... */
  while (t->size < 2|| 
         t->data[t->size-2] != '\r' || t->data[t->size-1] != '\n') {
    n = read(irc->sock, &c, 1);
    if (n == -1)
      ERR("irc socket receive, read returns -1 (%s)", strerror(errno));
    if (n == 0) ERR("irc socket has closed");
    if (c != '\r' && c != '\n')
      buffer_add_cchar(t, c);
    else
      buffer_add_char(t, c);
  }
  t->size -= 2;
  buffer_add_char(t, 0);

  if (!strncmp(b, "NICK", 4) && !irc->first_nick_done) {
    DBG("skip first NICK command");
    irc->first_nick_done = 1;
    t->size = 0;
    t->data[0] = 0;
  }

  if (!strncmp(b, "USER", 4) && !irc->first_user_done) {
    DBG("skip first USER command");
    irc->first_user_done = 1;
    t->size = 0;
    t->data[0] = 0;
  }
}
