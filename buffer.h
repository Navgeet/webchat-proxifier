#ifndef _BUFFER_H_
#define _BUFFER_H_

typedef struct {
  char *data;
  int size;
  int maxsize;
} buffer;

void init_buffer(buffer *, int);
void buffer_add_char(buffer *b, int);
void buffer_add_string(buffer *b, const char *);
void buffer_add_cchar(buffer *b, int);
void buffer_add_cstring(buffer *b, const char *);

#endif /* _BUFFER_H_ */
