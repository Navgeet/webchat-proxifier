#include "net.h"
#include "irc.h"
#include "buffer.h"
#include "err.h"
#include "gui.h"
#include "cJSON.h"
#include "event.h"

#include <string.h>

char *get_challenge_response(const char *img, int l)
{
  char rep[2048];
  int i, o;
  login_gui lg;
  go_gui(&lg, img, l);
  i = o = 0;
  while (o != 2047 && lg.in[i]) {
    rep[o] = lg.in[i];
#if 0
    if (rep[o] == ' ') {
      if (o > 2047 - 3) break;
      rep[o] = '%'; rep[o+1] = '2'; rep[o+2] = '0';
      o+=2;
    }
#endif
    i++; o++;
  }
  rep[o] = 0;
  DBG("response='%s'", rep);
  free(lg.in);
  return strdup(rep);
}

void login(net *n, buffer *b)
{
  char *s, *t;
  int l;
  char *key;
  char *challenge;
  cJSON *c;
  net_get(n, b, "https://webchat.freenode.net");
  s = strstr(b->data, "recaptchaKey"); if (!s) ERR("recaptchaKey not found");
  while (*s && *s != '"') s++; if (*s) s++;
  while (*s && *s != '"') s++; if (*s) s++;
  t = s; while (*t && *t != '"') t++;
  l = t - s;
  if (!*s || !*t || !l) ERR("bad captcha key");
  key = malloc(l+1); if (!key) OOM(); memcpy(key, s, l); key[l] = 0;
  DBG("recaptchaKey='%s'", key);

  b->size = 0;
  buffer_add_string(b, "http://www.google.com/recaptcha/api/challenge?k=");
  buffer_add_string(b, key);
  buffer_add_string(b, "&ajax=1");
  buffer_add_char(b, 0);
  net_get(n, b, b->data);

  free(key);

  s = strstr(b->data, "challenge"); if (!s) ERR("challenge not found");
  while (*s && *s != '\'') s++; if (*s) s++;
  t = s; while (*t && *t != '\'') t++;
  l = t - s;
  if (!*s || !*t || !l) ERR("bad challenge");
  challenge = malloc(l+1); if (!challenge) OOM();
  memcpy(challenge, s, l); challenge[l] = 0;
  DBG("challenge='%s'", challenge);

  b->size = 0;
  buffer_add_string(b, "http://www.google.com/recaptcha/api/image?c=");
  buffer_add_string(b, challenge);
  buffer_add_char(b, 0);
  net_get(n, b, b->data);
  if (b->size <= 1) ERR("no image returned");
  key = get_challenge_response(b->data, b->size-1);

  b->size = 0;
  buffer_add_string(b, "recaptchaC=");
  buffer_add_cstring(b, challenge);
  buffer_add_string(b, "&recaptchaR=");
  buffer_add_cstring(b, key);
  buffer_add_char(b, 0);

  free(key);
  free(challenge);

  net_post(n, 0, b, "https://webchat.freenode.net/dynamic/test/e/r", 
           b->data);
  while (1) { int r = wait_something(n, 0); if (r) break; }
  buffer_add_char(b, 0);
  c = cJSON_Parse(b->data);
  if (!c || c->type != cJSON_Array || cJSON_GetArraySize(c) != 2 ||
      cJSON_GetArrayItem(c, 0)->type != cJSON_True ||
      cJSON_GetArrayItem(c, 1)->type != cJSON_String ||
      strlen(cJSON_GetArrayItem(c, 1)->valuestring) == 0 ||
      strlen(cJSON_GetArrayItem(c, 1)->valuestring) > 256)
    ERR("JSON error '%s'", b->data);
  n->session = strdup(cJSON_GetArrayItem(c, 1)->valuestring);
  cJSON_Delete(c);
  if (!n->session) OOM();
  DBG("net session '%s'", n->session);

  b->size = 0;
  buffer_add_string(b, "session=");
  buffer_add_string(b, n->session);
  buffer_add_string(b, "&nick=fr33n5d3");
  buffer_add_char(b, 0);
  net_post(n, 0, b, "https://webchat.freenode.net/dynamic/test/e/n",
           b->data);
  while (1) { int r = wait_something(n, 0); if (r) break; }
  buffer_add_char(b, 0);
}

int main(int argc, char **v)
{
  irc irc;
  net n;
  buffer b;
  buffer b2;
  buffer bi;

  irc_wait_connect(&irc);

  init_buffer(&b, 1024 * 1024);
  init_buffer(&b2, 1024 * 1024);
  init_buffer(&bi, 1024 * 1024);
  init_net(&n);
  login(&n, &b);
  net_connect(&n, &b, 0);

  while (1) {
    int w = wait_something(&n, &irc);
    if (w & NET_EVENT_1) {
      buffer_add_char(&b, 0);
      DBG("NET_EVENT_1 gives data: '%s'\n", b.data);
      irc_send(&irc, &bi, &b);
    }
    if (w & NET_EVENT_2) {
      buffer_add_char(&b2, 0);
      DBG("NET_EVENT_2 gives data: '%s'\n", b2.data);
      irc_send(&irc, &bi, &b2);
    }
    if (w & IRC_EVENT) {
      if (!n.busy[0]) {
        b.size = 0;
        buffer_add_string(&b, "s=");
        buffer_add_string(&b, n.session);
        buffer_add_string(&b, "&c=");
        irc_recv(&irc, &b);
        DBG("IRC_EVENT gives data: '%s'\n", b.data);
        if (b.size)
            net_post(&n, 0, &b,
                     "https://webchat.freenode.net/dynamic/test/e/p",
                     b.data);
      } else {
        b2.size = 0;
        buffer_add_string(&b2, "s=");
        buffer_add_string(&b2, n.session);
        buffer_add_string(&b2, "&c=");
        irc_recv(&irc, &b2);
        DBG("IRC_EVENT gives data: '%s'\n", b2.data);
        if (b2.size)
          net_post(&n, 1, &b2,
                   "https://webchat.freenode.net/dynamic/test/e/p",
                   b2.data);
      }
    }
    if (!n.busy[0] && !n.busy[1])
      net_connect(&n, &b, 0);
  }

  return 0;
}
