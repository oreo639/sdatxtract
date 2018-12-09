/*
 * ciotuil.h: simple i/o routines for C
 */


#ifndef CIOUTIL_H
#define CIOUTIL_H


#include <stdio.h>


#if !defined(bool) && !defined(__cplusplus)
  typedef int bool;
  #define true    1
  #define false   0
#endif /* !bool */

#ifndef byte
  typedef unsigned char byte;
#endif /* !byte */
#ifndef sbyte
  typedef signed char sbyte;
#endif /* !sbyte */

#ifndef countof
  #define countof(a)    (sizeof(a) / sizeof(a[0]))
#endif

char* removeExt(char* path);

/* unsigned byte to signed */
 int utos1(unsigned int value);

/* unsigned 2bytes to signed */
 int utos2(unsigned int value);

/* unsigned 3bytes to signed */
 int utos3(unsigned int value);

/* unsigned 4bytes to signed */
 int utos4(unsigned int value);

/* get a byte */
 int mget1(const byte* data);

/* get 2bytes as little-endian */
 int mget2l(const byte* data);

/* get 3bytes as little-endian */
 int mget3l(const byte* data);

/* get 4bytes as little-endian */
 int mget4l(const byte* data);

/* get 2bytes as big-endian */
 int mget2b(const byte* data);

/* get 3bytes as big-endian */
 int mget3b(const byte* data);

/* get 4bytes as big-endian */
 int mget4b(const byte* data);

/* put a byte */
 int mput1(int value, byte* data);

/* put 2bytes as little-endian */
 int mput2l(int value, byte* data);

/* put 3bytes as little-endian */
 int mput3l(int value, byte* data);

/* put 4bytes as little-endian */
 int mput4l(int value, byte* data);

/* put 2bytes as big-endian */
 int mput2b(int value, byte* data);

/* put 3bytes as big-endian */
 int mput3b(int value, byte* data);

/* put 4bytes as big-endian */
 int mput4b(int value, byte* data);

/* get a byte from file */
 int fget1(FILE* stream);

/* get 2bytes as little-endian from file */
 int fget2l(FILE* stream);

/* get 3bytes as little-endian from file */
 int fget3l(FILE* stream);

/* get 4bytes as little-endian from file */
 int fget4l(FILE* stream);

/* get 2bytes as big-endian from file */
 int fget2b(FILE* stream);

/* get 3bytes as big-endian from file */
 int fget3b(FILE* stream);

/* get 4bytes as big-endian from file */
 int fget4b(FILE* stream);

/* put a byte to file */
 int fput1(int value, FILE* stream);

/* put 2bytes as little-endian to file */
 int fput2l(int value, FILE* stream);

/* put 3bytes as little-endian to file */
 int fput3l(int value, FILE* stream);

/* put 4bytes as little-endian to file */
 int fput4l(int value, FILE* stream);

/* put 2bytes as big-endian to file */
 int fput2b(int value, FILE* stream);

/* put 3bytes as big-endian to file */
 int fput3b(int value, FILE* stream);

/* put 4bytes as big-endian to file */
 int fput4b(int value, FILE* stream);


#endif /* !CIOUTIL_H */
