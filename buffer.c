#include "buffer.h"
#include "err.h"

#include <stdlib.h>

void init_buffer(buffer *b, int l)
{
  b->data = malloc(l); if (!b->data) OOM();
  b->size = 0;
  b->maxsize = l;
}

void buffer_add_char(buffer *b, int c)
{
  if (b->size == b->maxsize) ERR("buffer full");
  b->data[b->size] = c;
  b->size++;
}

void buffer_add_string(buffer *b, const char *c)
{
  while (*c) { buffer_add_char(b, *c); c++; }
}

static int isunreserved(unsigned char in)
{
  switch (in) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '-': case '.': case '_': case '~':
      return 1;
    default:
      break;
  }
  return 0;
}

void buffer_add_cstring(buffer *b, const char *c)
{
  char t[32];
  while (*c) {
    if (isunreserved(*c))
      buffer_add_char(b, *c);
    else {
      sprintf(t, "%%%02X", (unsigned char)*c);
      buffer_add_string(b, t);
    }
    c++;
  }
}

void buffer_add_cchar(buffer *b, int c)
{
  char t[32];
  if (isunreserved(c))
    buffer_add_char(b, c);
  else {
    sprintf(t, "%%%02X", (unsigned char)c);
    buffer_add_string(b, t);
  }
}
