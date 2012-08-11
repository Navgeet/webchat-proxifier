#include "net.h"
#include "buffer.h"
#include "err.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define R(x) \
  do { \
    CURLcode ret = x; \
    if (ret) { \
      printf("%s:%d: curl error: %s\n", __FILE__, __LINE__, \
             curl_easy_strerror(ret)); \
      exit(1); \
    } \
  } while (0)

#define RM(x) \
  do { \
    CURLMcode ret = x; \
    if (ret) { \
      printf("%s:%d: curl error: %s\n", __FILE__, __LINE__, \
             curl_multi_strerror(ret)); \
      exit(1); \
    } \
  } while (0)

void init_net(net *n)
{
  R(curl_global_init(CURL_GLOBAL_ALL));

  n->curl[0] = curl_easy_init(); if (!n->curl[0]) ERR("curl init err");
  R(curl_easy_setopt(n->curl[0], CURLOPT_HEADER, 0));
  n->curl[1] = curl_easy_init(); if (!n->curl[1]) ERR("curl init err");
  R(curl_easy_setopt(n->curl[1], CURLOPT_HEADER, 0));
  n->busy[0] = 0;
  n->busy[1] = 0;
  n->multi = curl_multi_init(); if (!n->multi) ERR("curl init err");
}

static size_t din(const void *ptr, size_t size, size_t nmemb, void *_b)
{
  buffer *b = _b;
  size_t l = size * nmemb;
  if (l + b->size > b->maxsize) ERR("buffer size too small");
  memcpy(b->data + b->size, ptr, l);
  b->size += l;
  return nmemb;
}

void net_get(net *n, buffer *b, const char *url)
{
  DBG("getting '%s'...", url);
  b->size = 0;
  R(curl_easy_setopt(n->curl[0], CURLOPT_URL, url));
  R(curl_easy_setopt(n->curl[0], CURLOPT_WRITEFUNCTION, din));
  R(curl_easy_setopt(n->curl[0], CURLOPT_FILE, b));
  R(curl_easy_perform(n->curl[0]));
  if (b->size == b->maxsize) ERR("buffer size too small");
  b->data[b->size] = 0; b->size++;
  DBG("...done");
}

void net_post(net *n, int c, buffer *b, const char *url, const char *pdata)
{
  int still_running;
  DBG("posting '%s' with '%s' on connection %d...", url, pdata, c);
  R(curl_easy_setopt(n->curl[c], CURLOPT_URL, url));
  R(curl_easy_setopt(n->curl[c], CURLOPT_WRITEFUNCTION, din));
  R(curl_easy_setopt(n->curl[c], CURLOPT_FILE, b));
  R(curl_easy_setopt(n->curl[c], CURLOPT_POSTFIELDS, pdata));
  b->size = 0;
  RM(curl_multi_add_handle(n->multi, n->curl[c]));
  RM(curl_multi_perform(n->multi, &still_running));
  n->busy[c] = 1;
  DBG("...posting started");
}

void net_connect(net *n, buffer *b, int c)
{
  b->size = 0;
  buffer_add_string(b, "s=");
  buffer_add_string(b, n->session);
  buffer_add_char(b, 0);
  net_post(n, c, b,
           "https://webchat.freenode.net/dynamic/test/e/s",
           b->data);
}
