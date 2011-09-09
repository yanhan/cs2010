#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "util.h"

void die(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	exit(1);
}

void *xcalloc(size_t nmemb, size_t sz)
{
	void *ptr = calloc(nmemb, sz);
	if (!ptr)
		die("xcalloc failed\n");
	return ptr;
}

void *xrealloc(void *ptr, size_t sz)
{
	ptr = realloc(ptr, sz);
	if (!ptr)
		die("xrealloc failed\n");
	return ptr;
}

char *xstrdup(const char *s)
{
	char *p = strdup(s);
	if (!p)
		die("xstrdup failed\n");
	return p;
}
