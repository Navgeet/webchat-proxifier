#include <curl/curl.h>
#include <stdlib.h>

#define R(x) \
  do { \
    int ret = x; \
    if (ret) { \
      printf("%s:%d: curl error: %s\n", __FILE__, __LINE__, \
             curl_easy_strerror(ret)); \
      exit(1); \
    } \
  } while (0)

int main(void)
{
  CURL *c;

  R(curl_global_init(CURL_GLOBAL_ALL));

  c = curl_easy_init();
  R(curl_easy_setopt(c, CURLOPT_URL, "http://localhost:1025"));
  R(curl_easy_setopt(c, CURLOPT_HEADER, 0));
  R(curl_easy_perform(c));
printf("second perform\n");
  R(curl_easy_perform(c));
  curl_easy_cleanup(c);

  curl_global_cleanup();
  return 0;
}
