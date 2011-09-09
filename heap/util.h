#ifndef HUFFMAN_UTIL_H
#define HUFFMAN_UTIL_H

void die(const char *msg);
void *xcalloc(size_t nmemb, size_t sz);
void *xrealloc(void *ptr, size_t sz);
char *xstrdup(const char *s);

#endif
