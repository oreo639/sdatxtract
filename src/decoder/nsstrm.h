/**
 * nsstrm.h: nds strm functions
 * written by loveemu, feel free to redistribute
 */


#ifndef NSSTRM_H
#define NSSTRM_H


#include "cioutil.h"


typedef struct TagNSStrm
{
  int waveType;
  bool hasLoop;
  int channels;
  int rate;
  int time;
  int loopStart;
  int numSamp;
  int numBlocks;
  size_t lenBlock;
  int sampPerBlock;
  size_t lenLastBlock;
  int sampPerLastBlock;
  int bps;
  int decodedSampSize;
  byte* data;
  size_t dataSize;
} NSStrm;

NSStrm* nsStrmCreate(const byte* strm, size_t size);
void nsStrmDelete(NSStrm* strm);
NSStrm* nsStrmReadFile(const char* path);
size_t nsStrmGetWaveSize(NSStrm* strm);
bool nsStrmWriteToWave(NSStrm* strm, byte* buf, size_t bufSize);
bool nsStrmWriteToWaveFile(NSStrm* strm, const char* path);


#endif /* !NSSTRM_H */
