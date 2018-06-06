#ifndef __LIB_STRING_H
#define __LIB_STRING_H

#include "stddef.h"

/* Standard. */
void *memcpy (void *, const void *, size_t);
size_t strlen (const char *);

/* Extensions. */
size_t strlcat (char *, const char *, size_t);
size_t strnlen (const char *, size_t);

#endif /* lib/string.h */
