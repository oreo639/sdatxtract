/*
 * ciotuil.c: simple i/o routines for C
 */


#include <stdio.h>
#include <string.h>
#include "cioutil.h"


/* remove path extention (SUPPORTS ASCII ONLY!) */
char* removeExt(char* path)
{
  size_t i;

  i = strlen(path);
  if(i > 1)
  {
    i--;
    for(; i > 0; i--)
    {
      char c = path[i];

      if(c == '.')
      {
        path[i] = '\0';
        break;
      }
      else if(c == '/' || c == '\\')
      {
        break;
      }
    }
  }
  return path;
}

/* unsigned byte to signed */
 int utos1(unsigned int value)
{
  return (value & 0x80) ? -(signed)(value ^ 0xff)-1 : value;
}

/* unsigned 2bytes to signed */
 int utos2(unsigned int value)
{
  return (value & 0x8000) ? -(signed)(value ^ 0xffff)-1 : value;
}

/* unsigned 3bytes to signed */
 int utos3(unsigned int value)
{
  return (value & 0x800000) ? -(signed)(value ^ 0xffffff)-1 : value;
}

/* unsigned 4bytes to signed */
 int utos4(unsigned int value)
{
  return (value & 0x80000000) ? -(signed)(value ^ 0xffffffff)-1 : value;
}

/* get a byte */
 int mget1(const byte* data)
{
  return data[0];
}

/* get 2bytes as little-endian */
 int mget2l(const byte* data)
{
  return data[0] | (data[1] * 0x0100);
}

/* get 3bytes as little-endian */
 int mget3l(const byte* data)
{
  return data[0] | (data[1] * 0x0100) | (data[2] * 0x010000);
}

/* get 4bytes as little-endian */
 int mget4l(const byte* data)
{
  return data[0] | (data[1] * 0x0100) | (data[2] * 0x010000) | (data[3] * 0x01000000);
}

/* get 2bytes as big-endian */
 int mget2b(const byte* data)
{
  return data[1] | (data[0] * 0x0100);
}

/* get 3bytes as big-endian */
 int mget3b(const byte* data)
{
  return data[2] | (data[1] * 0x0100) | (data[0] * 0x010000);
}

/* get 4bytes as big-endian */
 int mget4b(const byte* data)
{
  return data[3] | (data[2] * 0x0100) | (data[1] * 0x010000) | (data[0] * 0x01000000);
}

/* put a byte */
 int mput1(int value, byte* data)
{
  int lastPut = value;
  data[0] = lastPut & 0xff;
  return lastPut & 0xff;
}

/* put 2bytes as little-endian */
 int mput2l(int value, byte* data)
{
  int lastPut = value;
  data[0] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  return lastPut & 0xff;
}

/* put 3bytes as little-endian */
 int mput3l(int value, byte* data)
{
  int lastPut = value;
  data[0] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[2] = lastPut & 0xff;
  return lastPut & 0xff;
}

/* put 4bytes as little-endian */
 int mput4l(int value, byte* data)
{
  int lastPut = value;
  data[0] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[2] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[3] = lastPut & 0xff;
  return lastPut & 0xff;
}

/* put 2bytes as big-endian */
 int mput2b(int value, byte* data)
{
  int lastPut = value;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[0] = lastPut & 0xff;
  return lastPut & 0xff;
}

/* put 3bytes as big-endian */
 int mput3b(int value, byte* data)
{
  int lastPut = value;
  data[2] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[0] = lastPut & 0xff;
  return lastPut & 0xff;
}

/* put 4bytes as big-endian */
 int mput4b(int value, byte* data)
{
  int lastPut = value;
  data[3] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[2] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[1] = lastPut & 0xff;
  lastPut /= 0x0100;
  data[0] = lastPut & 0xff;
  return lastPut & 0xff;
}

/* get a byte from file */
 int fget1(FILE* stream)
{
  return fgetc(stream);
}

/* get 2bytes as little-endian from file */
 int fget2l(FILE* stream)
{
  int b1;
  int b2;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF))
  {
    return b1 | (b2 * 0x0100);
  }
  return EOF;
}

/* get 3bytes as little-endian from file */
 int fget3l(FILE* stream)
{
  int b1;
  int b2;
  int b3;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  b3 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF) && (b3 != EOF))
  {
    return b1 | (b2 * 0x0100) | (b3 * 0x010000);
  }
  return EOF;
}

/* get 4bytes as little-endian from file */
 int fget4l(FILE* stream)
{
  int b1;
  int b2;
  int b3;
  int b4;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  b3 = fgetc(stream);
  b4 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF) && (b3 != EOF) && (b4 != EOF))
  {
    return b1 | (b2 * 0x0100) | (b3 * 0x010000) | (b4 * 0x01000000);
  }
  return EOF;
}

/* get 2bytes as big-endian from file */
 int fget2b(FILE* stream)
{
  int b1;
  int b2;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF))
  {
    return b2 | (b1 * 0x0100);
  }
  return EOF;
}

/* get 3bytes as big-endian from file */
 int fget3b(FILE* stream)
{
  int b1;
  int b2;
  int b3;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  b3 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF) && (b3 != EOF))
  {
    return b3 | (b2 * 0x0100) | (b1 * 0x010000);
  }
  return EOF;
}

/* get 4bytes as big-endian from file */
 int fget4b(FILE* stream)
{
  int b1;
  int b2;
  int b3;
  int b4;

  b1 = fgetc(stream);
  b2 = fgetc(stream);
  b3 = fgetc(stream);
  b4 = fgetc(stream);
  if((b1 != EOF) && (b2 != EOF) && (b3 != EOF) && (b4 != EOF))
  {
    return b4 | (b3 * 0x0100) | (b2 * 0x010000) | (b1 * 0x01000000);
  }
  return EOF;
}

/* put a byte to file */
 int fput1(int value, FILE* stream)
{
  return fputc(value & 0xff, stream);
}

/* put 2bytes as little-endian to file */
 int fput2l(int value, FILE* stream)
{
  int result;

  result = fputc(value & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 8) & 0xff, stream);
  }
  return result;
}

/* put 3bytes as little-endian to file */
 int fput3l(int value, FILE* stream)
{
  int result;

  result = fputc(value & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 8) & 0xff, stream);
    if(result != EOF)
    {
      result = fputc((value >> 16) & 0xff, stream);
    }
  }
  return result;
}

/* put 4bytes as little-endian to file */
 int fput4l(int value, FILE* stream)
{
  int result;

  result = fputc(value & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 8) & 0xff, stream);
    if(result != EOF)
    {
      result = fputc((value >> 16) & 0xff, stream);
      if(result != EOF)
      {
        result = fputc((value >> 24) & 0xff, stream);
      }
    }
  }
  return result;
}

/* put 2bytes as big-endian to file */
 int fput2b(int value, FILE* stream)
{
  int result;

  result = fputc((value >> 8) & 0xff, stream);
  if(result != EOF)
  {
    result = fputc(value & 0xff, stream);
  }
  return result;
}

/* put 3bytes as big-endian to file */
 int fput3b(int value, FILE* stream)
{
  int result;

  result = fputc((value >> 16) & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 8) & 0xff, stream);
    if(result != EOF)
    {
      result = fputc(value & 0xff, stream);
    }
  }
  return result;
}

/* put 4bytes as big-endian to file */
 int fput4b(int value, FILE* stream)
{
  int result;

  result = fputc((value >> 24) & 0xff, stream);
  if(result != EOF)
  {
    result = fputc((value >> 16) & 0xff, stream);
    if(result != EOF)
    {
      result = fputc((value >> 8) & 0xff, stream);
      if(result != EOF)
      {
        result = fputc(value & 0xff, stream);
      }
    }
  }
  return result;
}
