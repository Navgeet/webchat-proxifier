#ifndef _ERR_H_
#define _ERR_H_

#include <stdio.h>
#include <stdlib.h>

#define ERR(...) \
  do { \
    printf("%s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); \
    printf("\n"); \
    exit(1); \
  } while (0)

#define OOM() ERR("out of memory")

#define DBG(...) \
  do { \
    printf("DBG: %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); \
    printf("\n"); \
    fflush(stdout); \
  } while (0)

#endif /* _ERR_H_ */
