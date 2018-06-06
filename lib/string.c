#include "string.h"
// #include "debug.h"
//------------------------------------------------------------------------------
//  MEMCPY
//------------------------------------------------------------------------------
/* Copies SIZE bytes from SRC to DST, which must not overlap.
   Returns DST. */
void *memcpy(void *dst_, const void *src_, size_t size) {
    unsigned char *dst = dst_;
    const unsigned char *src = src_;
    if (src == NULL || dst == NULL || size == 0) return NULL;
    while (size-- > 0) *dst++ = *src++;
    return dst_;
}
//------------------------------------------------------------------------------
// STRLEN
//------------------------------------------------------------------------------
// Returns the length of STRING.
size_t strlen(const char *string) {
    const char *p;
    for (p = string; *p != '\0'; p++) continue;
    return p - string;
}
//------------------------------------------------------------------------------
// STRNLEN
//------------------------------------------------------------------------------
// If STRING is less than MAXLEN characters in length, returns its actual
// length.  Otherwise, returns MAXLEN.
size_t strnlen(const char *string, size_t maxlen) {
    size_t length;
    for (length = 0; string[length] != '\0' && length < maxlen; length++)
        continue;
    return length;
}
//------------------------------------------------------------------------------
// STRLCAT
//------------------------------------------------------------------------------
/* Concatenates string SRC to DST.  The concatenated string is
   limited to SIZE - 1 characters.  A null terminator is always
   written to DST, unless SIZE is 0.  Returns the length that the
   concatenated string would have assuming that there was
   sufficient space, not including a null terminator.

   strlcat() is not in the standard C library, but it is an
   increasingly popular extension.  See
   http://www.courtesan.com/todd/papers/strlcpy.html for
   information on strlcpy().
*/
size_t strlcat (char *dst, const char *src, size_t size) {
    size_t src_len, dst_len;
    if (dst == NULL || src == NULL) return -1;
    src_len = strlen (src);
    dst_len = strlen (dst);
    if (size > 0 && dst_len < size) {
        size_t copy_cnt = size - dst_len - 1;
        if (src_len < copy_cnt) copy_cnt = src_len;
        memcpy(dst + dst_len, src, copy_cnt);
        dst[dst_len + copy_cnt] = '\0';
    }
    return src_len + dst_len;
}
//------------------------------------------------------------------------------