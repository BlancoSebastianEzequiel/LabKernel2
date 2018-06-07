/*
 * Part of the Pintos project (http://pintos-os.org/).
 *
 * Copyright (C) 2004, 2005, 2006 Board of Trustees, Leland Stanford
 * Jr. University.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Changes 2017-07-31 (dato@fi.uba.ar):
 *   - rename include guard not to start with an underscore, to appease clang
 * Changes 2017-09-20 (dato@fi.uba.ar):
 *   - remove non-functional macros for str[n]cpy, str[n]cat, strtok.
 */

#ifndef STRING_H
#define STRING_H

#include "stddef.h"

/* Standard. */
void *memcpy(void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
int strcmp(const char *, const char *);
void *memchr(const void *, int, size_t);
char *strchr(const char *, int);
size_t strcspn(const char *, const char *);
char *strpbrk(const char *, const char *);
char *strrchr(const char *, int);
size_t strspn(const char *, const char *);
char *strstr(const char *, const char *);
void *memset(void *, int, size_t);
size_t strlen(const char *);

/* Extensions. */
size_t strlcpy(char *, const char *, size_t);
size_t strlcat(char *, const char *, size_t);
char *strtok_r(char *, const char *, char **);
size_t strnlen(const char *, size_t);

#endif /* lib/string.h */